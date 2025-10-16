#include "LMA_Port.h"
#include "LMA_Core.h"

void LMA_AccReset(LMA_TempData *const p_ws, const uint32_t phase_id)
{
  (void)phase_id;
  R_MACL->MULC = 0xC0; /* MAC, Signed Integer*/
  R_MACL->MAC32S = *((uint32_t *)&(p_ws->samples.current));
  R_MACL->MULR0.MULRL = 0;
  R_MACL->MULR0.MULRH = 0;
  R_MACL->MULR1.MULRL = 0;
  R_MACL->MULR1.MULRH = 0;
  R_MACL->MULR2.MULRL = 0;
  R_MACL->MULR2.MULRH = 0;

  R_MACL->MULB0 = *((uint32_t *)&(p_ws->samples.current));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  R_MACL->MULB1 = *((uint32_t *)&(p_ws->samples.voltage));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  R_MACL->MULB2 = *((uint32_t *)&(p_ws->samples.voltage90));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();

  R_MACL->MAC32S = *((uint32_t *)&(p_ws->samples.voltage));
  R_MACL->MULR3.MULRL = 0;
  R_MACL->MULR3.MULRH = 0;
  R_MACL->MULB3 = *((uint32_t *)&(p_ws->samples.voltage));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();

  *((uint32_t *)(&p_ws->accs.iacc)) = R_MACL->MULR0.MULRL;
  *((uint32_t *)(&p_ws->accs.iacc) + 1) = R_MACL->MULR0.MULRH;
  *((uint32_t *)(&p_ws->accs.pacc)) = R_MACL->MULR1.MULRL;
  *((uint32_t *)(&p_ws->accs.pacc) + 1) = R_MACL->MULR1.MULRH;
  *((uint32_t *)(&p_ws->accs.qacc)) = R_MACL->MULR2.MULRL;
  *((uint32_t *)(&p_ws->accs.qacc) + 1) = R_MACL->MULR2.MULRH;
  *((uint32_t *)(&p_ws->accs.vacc)) = R_MACL->MULR3.MULRL;
  *((uint32_t *)(&p_ws->accs.vacc) + 1) = R_MACL->MULR3.MULRH;

  p_ws->accs.sample_count = 1;
}

void LMA_AccRun(LMA_TempData *const p_ws, const uint32_t phase_id)
{
  (void)phase_id;
  R_MACL->MULC = 3; /* MAC, Signed Integer*/
  R_MACL->MAC32S = *((uint32_t *)&(p_ws->samples.current));
  R_MACL->MULB0 = *((uint32_t *)&(p_ws->samples.current));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  R_MACL->MULB1 = *((uint32_t *)&(p_ws->samples.voltage));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  R_MACL->MULB2 = *((uint32_t *)&(p_ws->samples.voltage90));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  R_MACL->MAC32S = *((uint32_t *)&(p_ws->samples.voltage));
  R_MACL->MULB3 = *((uint32_t *)&(p_ws->samples.voltage));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();

  ++p_ws->accs.sample_count;
}

void LMA_AccLoad(LMA_TempData *const p_ws, LMA_Accumulators *const p_accs, const uint32_t phase_id)
{
  (void)p_ws;
  (void)phase_id;
  *((uint32_t *)(&p_accs->iacc)) = R_MACL->MULR0.MULRL;
  *((uint32_t *)(&p_accs->iacc) + 1) = R_MACL->MULR0.MULRH;
  *((uint32_t *)(&p_accs->pacc)) = R_MACL->MULR1.MULRL;
  *((uint32_t *)(&p_accs->pacc) + 1) = R_MACL->MULR1.MULRH;
  *((uint32_t *)(&p_accs->qacc)) = R_MACL->MULR2.MULRL;
  *((uint32_t *)(&p_accs->qacc) + 1) = R_MACL->MULR2.MULRH;
  *((uint32_t *)(&p_accs->vacc)) = R_MACL->MULR3.MULRL;
  *((uint32_t *)(&p_accs->vacc) + 1) = R_MACL->MULR3.MULRH;

  p_accs->sample_count = p_ws->accs.sample_count;
}

spl_t LMA_PhaseShift90(spl_t new_voltage)
{
  static spl_t voltage_buffer[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t buffer_index = 0;

  uint8_t buffer_index_19 = buffer_index + 2;
  uint8_t buffer_index_20 = buffer_index + 1;

  if (buffer_index_19 > 21)
  {
    buffer_index_19 -= 21;
  }

  if (buffer_index_20 > 21)
  {
    buffer_index_20 -= 21;
  }

  /* Append new voltage*/
  voltage_buffer[buffer_index] = new_voltage;

  /* Interpolate 19.53 samples - just take the mid point*/
  int32_t interpolated_value = ((voltage_buffer[buffer_index_19] * 60) >> 7) + ((voltage_buffer[buffer_index_20] * 68) >> 7);

  buffer_index = buffer_index_20;

  /* Convert back to its 32b value*/
  return interpolated_value;
}

void LMA_ADC_Init(void)
{
  R_SDADC_B_Open(&g_adc0_ctrl, &g_adc0_cfg);
}

void LMA_ADC_Start(void)
{
  R_SDADC_B_ScanStart(&g_adc0_ctrl);
}

void LMA_ADC_Stop(void)
{
  R_SDADC_B_ScanStop(&g_adc0_ctrl);
}

void LMA_TMR_Init(void)
{
  R_AGT_Open(&g_timer0_ctrl, &g_timer0_cfg);
}

void LMA_TMR_Start(void)
{
  R_AGT_Start(&g_timer0_ctrl);
}

void LMA_TMR_Stop(void)
{
  R_AGT_Stop(&g_timer0_ctrl);
}

void LMA_RTC_Init(void)
{
  R_RTC_Open(&g_rtc0_ctrl, &g_rtc0_cfg);
}

void LMA_RTC_Start(void)
{
  rtc_time_t set_time = {
      .tm_sec = 0,
      .tm_min = 0,
      .tm_hour = 0,
      .tm_mday = 15,
      .tm_wday = 3,
      .tm_mon = 1,
      .tm_year = 125,
  };
  R_RTC_PeriodicIrqRateSet(&g_rtc0_ctrl, RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_2_SECOND);
  R_RTC_CalendarTimeSet(&g_rtc0_ctrl, &set_time);
}

void LMA_RTC_Stop(void)
{
  /* TODO: Populate*/
}

void LMA_IMP_ActiveOn(void)
{
  R_PORT1->PODR_b.PODR2 = 1; /* P102 = HIGH*/
}

void LMA_IMP_ActiveOff(void)
{
  R_PORT1->PODR_b.PODR2 = 0; /* P102 = HIGH*/
}

void LMA_IMP_ReactiveOn(void)
{
  /* TODO: Populate*/
}

void LMA_IMP_ReactiveOff(void)
{
  /* TODO: Populate*/
}

void LMA_IMP_ApparentOn(void)
{
  /* TODO: Populate*/
}

void LMA_IMP_ApparentOff(void)
{
  /* TODO: Populate*/
}
