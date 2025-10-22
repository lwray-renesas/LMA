#include "LMA_Port.h"

#include "iodefine.h"
#include "math.h"

#include "r_cg_dsadc.h"
#include "r_cg_macrodriver.h"
#include "r_cg_rtc.h"
#include "r_cg_tau.h"

#include "Trap_integrator.h"

void LMA_AccPhaseRun(LMA_Phase *const p_phase)
{
  /* Signed MAC Mode*/
  MULC = 0xC0U;

  /* Irms*/
  MULR0 = *((uint8_t *)&(p_phase->accs.temp.i_acc));
  MULR1 = *(((uint8_t *)&(p_phase->accs.temp.i_acc)) + 1);
  MULR2 = *(((uint8_t *)&(p_phase->accs.temp.i_acc)) + 2);
  MULR3 = *(((uint8_t *)&(p_phase->accs.temp.i_acc)) + 3);

  MAC32SL = *((uint16_t *)&(p_phase->inputs.i_sample));
  MAC32SH = *(((uint16_t *)&(p_phase->inputs.i_sample)) + 1);
  MULBL = *((uint16_t *)&(p_phase->inputs.i_sample));
  MULBH = *(((uint16_t *)&(p_phase->inputs.i_sample)) + 1);
  NOP();
  NOP();
  *((uint8_t *)&(p_phase->accs.temp.i_acc)) = MULR0;
  NOP();
  *(((uint8_t *)&(p_phase->accs.temp.i_acc)) + 1) = MULR1;
  *(((uint8_t *)&(p_phase->accs.temp.i_acc)) + 2) = MULR2;
  *(((uint8_t *)&(p_phase->accs.temp.i_acc)) + 3) = MULR3;

  /* P*/
  MULR0 = *((uint8_t *)&(p_phase->accs.temp.p_acc));
  MULR1 = *(((uint8_t *)&(p_phase->accs.temp.p_acc)) + 1);
  MULR2 = *(((uint8_t *)&(p_phase->accs.temp.p_acc)) + 2);
  MULR3 = *(((uint8_t *)&(p_phase->accs.temp.p_acc)) + 3);

  MULBL = *((uint16_t *)&(p_phase->inputs.v_sample));
  MULBH = *(((uint16_t *)&(p_phase->inputs.v_sample)) + 1);
  NOP();
  NOP();
  *((uint8_t *)&(p_phase->accs.temp.p_acc)) = MULR0;
  NOP();
  *(((uint8_t *)&(p_phase->accs.temp.p_acc)) + 1) = MULR1;
  *(((uint8_t *)&(p_phase->accs.temp.p_acc)) + 2) = MULR2;
  *(((uint8_t *)&(p_phase->accs.temp.p_acc)) + 3) = MULR3;

  /* Q*/
  MULR0 = *((uint8_t *)&(p_phase->accs.temp.q_acc));
  MULR1 = *(((uint8_t *)&(p_phase->accs.temp.q_acc)) + 1);
  MULR2 = *(((uint8_t *)&(p_phase->accs.temp.q_acc)) + 2);
  MULR3 = *(((uint8_t *)&(p_phase->accs.temp.q_acc)) + 3);

  MULBL = *((uint16_t *)&(p_phase->inputs.v90_sample));
  MULBH = *(((uint16_t *)&(p_phase->inputs.v90_sample)) + 1);
  NOP();
  NOP();
  *((uint8_t *)&(p_phase->accs.temp.q_acc)) = MULR0;
  NOP();
  *(((uint8_t *)&(p_phase->accs.temp.q_acc)) + 1) = MULR1;
  *(((uint8_t *)&(p_phase->accs.temp.q_acc)) + 2) = MULR2;
  *(((uint8_t *)&(p_phase->accs.temp.q_acc)) + 3) = MULR3;

  /* Vrms*/
  MULR0 = *((uint8_t *)&(p_phase->accs.temp.v_acc));
  MULR1 = *(((uint8_t *)&(p_phase->accs.temp.v_acc)) + 1);
  MULR2 = *(((uint8_t *)&(p_phase->accs.temp.v_acc)) + 2);
  MULR3 = *(((uint8_t *)&(p_phase->accs.temp.v_acc)) + 3);

  MAC32SL = *((uint16_t *)&(p_phase->inputs.v_sample));
  MAC32SH = *(((uint16_t *)&(p_phase->inputs.v_sample)) + 1);
  MULBL = *((uint16_t *)&(p_phase->inputs.v_sample));
  MULBH = *(((uint16_t *)&(p_phase->inputs.v_sample)) + 1);
  NOP();
  NOP();
  *((uint8_t *)&(p_phase->accs.temp.v_acc)) = MULR0;
  NOP();
  *(((uint8_t *)&(p_phase->accs.temp.v_acc)) + 1) = MULR1;
  *(((uint8_t *)&(p_phase->accs.temp.v_acc)) + 2) = MULR2;
  *(((uint8_t *)&(p_phase->accs.temp.v_acc)) + 3) = MULR3;

  if (NULL != p_phase->p_neutral)
  {
    /* Irms - NEUTRAL*/
    MULR0 = *((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp));
    MULR1 = *(((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp)) + 1);
    MULR2 = *(((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp)) + 2);
    MULR3 = *(((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp)) + 3);

    MAC32SL = *((uint16_t *)&(p_phase->p_neutral->inputs.i_sample));
    MAC32SH = *(((uint16_t *)&(p_phase->p_neutral->inputs.i_sample)) + 1);
    MULBL = *((uint16_t *)&(p_phase->p_neutral->inputs.i_sample));
    MULBH = *(((uint16_t *)&(p_phase->p_neutral->inputs.i_sample)) + 1);
    NOP();
    NOP();
    *((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp)) = MULR0;
    NOP();
    *(((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp)) + 1) = MULR1;
    *(((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp)) + 2) = MULR2;
    *(((uint8_t *)&(p_phase->p_neutral->accs.i_acc_temp)) + 3) = MULR3;
  }

  p_phase->accs.temp.sample_count = p_phase->accs.temp.sample_count + (uint32_t)1;
}

void LMA_AccPhaseReset(LMA_Phase *const p_phase)
{
  p_phase->accs.temp.v_acc = (acc_t)0LL;
  p_phase->accs.temp.i_acc = (acc_t)0LL;
  p_phase->accs.temp.p_acc = (acc_t)0LL;
  p_phase->accs.temp.q_acc = (acc_t)0LL;
  if (NULL != p_phase->p_neutral)
  {
    p_phase->p_neutral->accs.i_acc_temp = (acc_t)0LL;
  }
  p_phase->accs.temp.sample_count = (uint32_t)0L;

  /* Resetting the integrator to ensure no drift*/
  Trap_reset(&rogowski_integrator);
}

void LMA_AccPhaseLoad(LMA_Phase *const p_phase)
{
  p_phase->accs.snapshot.v_acc = p_phase->accs.temp.v_acc;
  p_phase->accs.snapshot.i_acc = p_phase->accs.temp.i_acc;
  p_phase->accs.snapshot.p_acc = p_phase->accs.temp.p_acc;
  p_phase->accs.snapshot.q_acc = p_phase->accs.temp.q_acc;
  p_phase->accs.snapshot.sample_count = p_phase->accs.temp.sample_count;
  if (NULL != p_phase->p_neutral)
  {
    p_phase->p_neutral->accs.i_acc_snapshot = p_phase->p_neutral->accs.i_acc_temp;
  }
}

void LMA_ADC_Init(void)
{
  /* Create called at system init*/
}

void LMA_ADC_Start(void)
{
  R_DSADC_Set_OperationOn();
  R_DSADC_Start();
}

void LMA_ADC_Stop(void)
{
  R_DSADC_Set_OperationOff();
  R_DSADC_Stop();
}

void LMA_TMR_Init(void)
{
  /* Create called at system init*/
}

void LMA_TMR_Start(void)
{
  R_TAU0_Channel0_Start();
}

void LMA_TMR_Stop(void)
{
  R_TAU0_Channel0_Stop();
}

void LMA_RTC_Init(void)
{
  /* Create called at system init*/
}

void LMA_RTC_Start(void)
{
  R_RTC_Start();
}

void LMA_RTC_Stop(void)
{
  /* TODO: Populate*/
}

void LMA_IMP_ActiveOn(void)
{
  P1_bit.no2 = 1U;
}

void LMA_IMP_ActiveOff(void)
{
  P1_bit.no2 = 0U;
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
