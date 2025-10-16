/** \addtogroup Porting
 *  @{
 *
 * @file LMA_Port.c
 * @brief Porting file definitions for the LMA codebase.
 *
 * @details This file provides definitions of the LMA porting requirements - everythin in this file must be considered when
 * porting between platforms.
 */

/** @}*/

#include "LMA_Port.h"

bool tmr_running = false;
bool adc_running = false;
bool rtc_running = false;

void LMA_AccPhaseRun(LMA_Phase *const p_phase)
{
  p_phase->accs.temp.v_acc += ((acc_t)p_phase->inputs.v_sample * (acc_t)p_phase->inputs.v_sample);
  p_phase->accs.temp.i_acc += ((acc_t)p_phase->inputs.i_sample * (acc_t)p_phase->inputs.i_sample);
  p_phase->accs.temp.p_acc += ((acc_t)p_phase->inputs.v_sample * (acc_t)p_phase->inputs.i_sample);
  p_phase->accs.temp.q_acc += ((acc_t)p_phase->inputs.v90_sample * (acc_t)p_phase->inputs.i_sample);

  if (NULL != p_phase->p_neutral)
  {
    p_phase->p_neutral->accs.i_acc_temp +=
        ((acc_t)p_phase->p_neutral->inputs.i_sample * (acc_t)p_phase->p_neutral->inputs.i_sample);
  }

  ++p_phase->accs.temp.sample_count;
}

void LMA_AccPhaseReset(LMA_Phase *const p_phase)
{
  p_phase->accs.temp.v_acc = ((acc_t)p_phase->inputs.v_sample * (acc_t)p_phase->inputs.v_sample);
  p_phase->accs.temp.i_acc = ((acc_t)p_phase->inputs.i_sample * (acc_t)p_phase->inputs.i_sample);
  p_phase->accs.temp.p_acc = ((acc_t)p_phase->inputs.v_sample * (acc_t)p_phase->inputs.i_sample);
  p_phase->accs.temp.q_acc = ((acc_t)p_phase->inputs.v90_sample * (acc_t)p_phase->inputs.i_sample);

  if (NULL != p_phase->p_neutral)
  {
    p_phase->p_neutral->accs.i_acc_temp =
        ((acc_t)p_phase->p_neutral->inputs.i_sample * (acc_t)p_phase->p_neutral->inputs.i_sample);
  }

  p_phase->accs.temp.sample_count = (uint32_t)0;
}

void LMA_ADC_Init(void)
{
}

void LMA_ADC_Start(void)
{
  adc_running = true;
}

void LMA_ADC_Stop(void)
{
  adc_running = false;
}

void LMA_TMR_Init(void)
{
}

void LMA_TMR_Start(void)
{
  tmr_running = true;
}

void LMA_TMR_Stop(void)
{
  tmr_running = false;
}

void LMA_RTC_Init(void)
{
}

void LMA_RTC_Start(void)
{
  rtc_running = true;
}

void LMA_RTC_Stop(void)
{
  rtc_running = false;
}

void LMA_IMP_ActiveOn(void)
{
  /* TODO: Populate*/
}

void LMA_IMP_ActiveOff(void)
{
  /* TODO: Populate*/
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
