#include "LMA_Port.h"

void LMA_AccPhaseRun(LMA_Phase *const p_phase)
{
  R_MACL->MULC = 0xC0; /* MAC, Signed Integer*/

  if (0 == p_phase->phase_number)
  {
    R_MACL->MAC32S = *((uint32_t *)&(p_phase->inputs.i_sample));
    R_MACL->MULB0 = *((uint32_t *)&(p_phase->inputs.i_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB1 = *((uint32_t *)&(p_phase->inputs.v_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB2 = *((uint32_t *)&(p_phase->inputs.v90_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MAC32S = *((uint32_t *)&(p_phase->inputs.v_sample));
    R_MACL->MULB3 = *((uint32_t *)&(p_phase->inputs.v_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    ++p_phase->accs.temp.sample_count;
  }
  else if (1 == p_phase->phase_number)
  {
    R_MACL->MAC32S = *((uint32_t *)&(p_phase->inputs.i_sample));
    R_MACL->MULB4 = *((uint32_t *)&(p_phase->inputs.i_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB5 = *((uint32_t *)&(p_phase->inputs.v_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB6 = *((uint32_t *)&(p_phase->inputs.v90_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MAC32S = *((uint32_t *)&(p_phase->inputs.v_sample));
    R_MACL->MULB7 = *((uint32_t *)&(p_phase->inputs.v_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    ++p_phase->accs.temp.sample_count;
  }
  else if (2 == p_phase->phase_number)
  {
    R_MACL->MAC32S = *((uint32_t *)&(p_phase->inputs.i_sample));
    R_MACL->MULB8 = *((uint32_t *)&(p_phase->inputs.i_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB9 = *((uint32_t *)&(p_phase->inputs.v_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB10 = *((uint32_t *)&(p_phase->inputs.v90_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MAC32S = *((uint32_t *)&(p_phase->inputs.v_sample));
    R_MACL->MULB11 = *((uint32_t *)&(p_phase->inputs.v_sample));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    ++p_phase->accs.temp.sample_count;
  }
  else
  {
    /* Invalid Phase*/
  }
}

void LMA_AccPhaseReset(LMA_Phase *const p_phase)
{
  /* Not using temp accs in p_phase->accs.temp - instead using the MACL registers as buffers*/
  if (0 == p_phase->phase_number)
  {
    R_MACL->MULR0.MULRL = 0;
    R_MACL->MULR0.MULRH = 0;
    R_MACL->MULR1.MULRL = 0;
    R_MACL->MULR1.MULRH = 0;
    R_MACL->MULR2.MULRL = 0;
    R_MACL->MULR2.MULRH = 0;

    p_phase->accs.temp.sample_count = 0;
  }
  else if (1 == p_phase->phase_number)
  {
    R_MACL->MULR4.MULRL = 0;
    R_MACL->MULR4.MULRH = 0;
    R_MACL->MULR5.MULRL = 0;
    R_MACL->MULR5.MULRH = 0;
    R_MACL->MULR6.MULRL = 0;
    R_MACL->MULR6.MULRH = 0;

    p_phase->accs.temp.sample_count = 0;
  }
  else if (2 == p_phase->phase_number)
  {
    R_MACL->MULR8.MULRL = 0;
    R_MACL->MULR8.MULRH = 0;
    R_MACL->MULR9.MULRL = 0;
    R_MACL->MULR9.MULRH = 0;
    R_MACL->MULR10.MULRL = 0;
    R_MACL->MULR10.MULRH = 0;

    p_phase->accs.temp.sample_count = 0;
  }
  else
  {
    /* Invalid Phase*/
  }
}

void LMA_AccPhaseLoad(LMA_Phase *const p_phase)
{
  if (0 == p_phase->phase_number)
  {
    *((uint32_t *)(&p_phase->accs.snapshot.i_acc)) = R_MACL->MULR0.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.i_acc) + 1) = R_MACL->MULR0.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.p_acc)) = R_MACL->MULR1.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.p_acc) + 1) = R_MACL->MULR1.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.q_acc)) = R_MACL->MULR2.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.q_acc) + 1) = R_MACL->MULR2.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.v_acc)) = R_MACL->MULR3.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.v_acc) + 1) = R_MACL->MULR3.MULRH;

    p_phase->accs.snapshot.sample_count = p_phase->accs.temp.sample_count;
  }
  else if (1 == p_phase->phase_number)
  {
    *((uint32_t *)(&p_phase->accs.snapshot.i_acc)) = R_MACL->MULR4.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.i_acc) + 1) = R_MACL->MULR4.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.p_acc)) = R_MACL->MULR5.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.p_acc) + 1) = R_MACL->MULR5.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.q_acc)) = R_MACL->MULR6.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.q_acc) + 1) = R_MACL->MULR6.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.v_acc)) = R_MACL->MULR7.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.v_acc) + 1) = R_MACL->MULR7.MULRH;

    p_phase->accs.snapshot.sample_count = p_phase->accs.temp.sample_count;
  }
  else if (2 == p_phase->phase_number)
  {
    *((uint32_t *)(&p_phase->accs.snapshot.i_acc)) = R_MACL->MULR8.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.i_acc) + 1) = R_MACL->MULR8.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.p_acc)) = R_MACL->MULR9.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.p_acc) + 1) = R_MACL->MULR9.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.q_acc)) = R_MACL->MULR10.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.q_acc) + 1) = R_MACL->MULR10.MULRH;

    *((uint32_t *)(&p_phase->accs.snapshot.v_acc)) = R_MACL->MULR11.MULRL;
    *((uint32_t *)(&p_phase->accs.snapshot.v_acc) + 1) = R_MACL->MULR11.MULRH;

    p_phase->accs.snapshot.sample_count = p_phase->accs.temp.sample_count;
  }
  else
  {
    /* Invalid Phase*/
  }
}

void LMA_PhaseResetHook(LMA_Phase *const p_phase)
{
  (void)p_phase;
  /* TODO: Populate*/
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
