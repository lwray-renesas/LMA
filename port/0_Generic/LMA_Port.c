#include "LMA_Port.h"
#include "LMA_Core.h"
#include "simulation.h"
#include <math.h>

static acc_t l_i_acc = 0;
static acc_t l_p_acc = 0;
static acc_t l_q_acc = 0;
static acc_t l_v_acc = 0;

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
    return *(uint32_t*)(&a) & 0x7FFFFFFF;
}

void LMA_AccReset(LMA_Workspace *const p_ws)
{
  l_i_acc = (acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->current;
  l_p_acc = (acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->voltage;
  l_q_acc = (acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->voltage90;
  l_v_acc = (acc_t)p_ws->p_samples->voltage * (acc_t)p_ws->p_samples->voltage;
}

void LMA_AccRun(LMA_Workspace *const p_ws)
{
  l_i_acc += (acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->current;
  l_p_acc += (acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->voltage;
  l_q_acc += (acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->voltage90;
  l_v_acc += (acc_t)p_ws->p_samples->voltage * (acc_t)p_ws->p_samples->voltage;
}

void LMA_AccGet(LMA_Workspace *const p_ws)
{
  *p_ws->p_iacc = l_i_acc;
  *p_ws->p_pacc = l_p_acc;
  *p_ws->p_qacc = l_q_acc;
  *p_ws->p_vacc = l_v_acc;
}

void LMA_AccMultiply(acc_ext_t *res, acc_t a, acc_t b)
{
  (void)a;
  (void)b;
  (void)res;
  /* TODO: Populate*/
}

acc_t LMA_AccSqrt(acc_t val)
{
  acc_t x = val, c = 0;
  if (val > 0)
  {
    acc_t d = (acc_t)1 << (acc_t)62;

    while (d > val)
    {
      d >>= 2;
    }

    while (d != 0)
    {
      if (x >= c + d)
      {
        x -= c + d;
        c = (c >> 1) + d;
      }
      else
      {
        c >>= 1;
      }
      d >>= 2;
    }
  }
  return c;
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
