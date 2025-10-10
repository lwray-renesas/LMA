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
#include <math.h>

float LMA_AccToFloat(acc_t acc)
{
  return (float)((double)acc);
}

float LMA_FPMul_Fast(float a, float b)
{
  return a * b;
}

float LMA_FPDiv_Fast(float a, float b)
{
  return a / b;
}

float LMA_FPSqrt_Fast(float a)
{
  return sqrtf(a);
}

float LMA_FPAbs_Fast(float a)
{
  uint32_t tmp = *(uint32_t *)(&a) & 0x7FFFFFFF;
  return *(float *)&tmp;
}

void LMA_AccReset(LMA_Workspace *const p_ws, const uint32_t phase_id)
{
  p_ws->accs.iacc = ((acc_t)p_ws->samples.current * (acc_t)p_ws->samples.current);
  p_ws->accs.pacc = ((acc_t)p_ws->samples.current * (acc_t)p_ws->samples.voltage);
  p_ws->accs.qacc = ((acc_t)p_ws->samples.current * (acc_t)p_ws->samples.voltage90);
  p_ws->accs.vacc = ((acc_t)p_ws->samples.voltage * (acc_t)p_ws->samples.voltage);
  p_ws->accs.sample_count = 1;
}

void LMA_AccRun(LMA_Workspace *const p_ws, const uint32_t phase_id)
{
  p_ws->accs.iacc += ((acc_t)p_ws->samples.current * (acc_t)p_ws->samples.current);
  p_ws->accs.pacc += ((acc_t)p_ws->samples.current * (acc_t)p_ws->samples.voltage);
  p_ws->accs.qacc += ((acc_t)p_ws->samples.current * (acc_t)p_ws->samples.voltage90);
  p_ws->accs.vacc += ((acc_t)p_ws->samples.voltage * (acc_t)p_ws->samples.voltage);
  ++p_ws->accs.sample_count;
}

void LMA_AccLoad(LMA_Workspace *const p_ws, LMA_Accumulators *const p_accs, const uint32_t phase_id)
{
  p_accs->iacc = p_ws->accs.iacc;
  p_accs->pacc = p_ws->accs.pacc;
  p_accs->qacc = p_ws->accs.qacc;
  p_accs->vacc = p_ws->accs.vacc;
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
  /* TODO: Populate*/
}

void LMA_ADC_Start(void)
{
  /* TODO: Populate*/
}

void LMA_ADC_Stop(void)
{
  /* TODO: Populate*/
}

void LMA_TMR_Init(void)
{
  /* TODO: Populate*/
}

void LMA_TMR_Start(void)
{
  /* TODO: Populate*/
}

void LMA_TMR_Stop(void)
{
  /* TODO: Populate*/
}

void LMA_RTC_Init(void)
{
  /* TODO: Populate*/
}

void LMA_RTC_Start(void)
{
  /* TODO: Populate*/
}

void LMA_RTC_Stop(void)
{
  /* TODO: Populate*/
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
