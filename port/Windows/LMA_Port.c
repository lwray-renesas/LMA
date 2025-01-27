#include "LMA_Port.h"
#include "LMA_Core.h"
#include "simulation.h"
#include <stdio.h>
#include <threads.h>
#include <time.h>
#include <math.h>

extern const simulation_params *p_g_sim_params;
extern LMA_Phase phase;
static bool adc_running = false;
static bool rtc_running = false;
bool driver_thread_running = false;

static acc_t l_i_acc = 0;
static acc_t l_p_acc = 0;
static acc_t l_q_acc = 0;
static acc_t l_v_acc = 0;

/** @brief thread to emulate drivers running asynch to main thread
 * @param[inout] pointer to args for thread - passing simulation_params for controlling the sim
 */
int Driver_thread(void *p_arg);

/** @brief phase shifts voltage signal
 * @details
 * - 50Hz signal is 20ms.
 * - 50Hz signal being 360degree of period, to get 90degree we divide by 4.
 * - 20ms divided by 4 = 5ms.
 * - to delay 5ms with a 3906Hz clock we can do 0.005/(1/3906) = 19.53 samples - so we do 20
 * samples.
 *
 * @param[in] new_voltage - new voltage to store in the buffer
 * @return voltage sample 90degree (20 samples) ago.
 */
static int32_t Phase_shift_90deg(int32_t new_voltage);

float LMA_AccToFloat(acc_t acc)
{
  return (float)((double)acc);
}

float LMA_Fast_FPMul(float a, float b)
{
  return a * b;
}

float LMA_Fast_FPDiv(float a, float b)
{
  return a / b;
}

float LMA_Fast_FPSqrt(float a)
{
    return sqrtf(a);
}

float LMA_Fast_FPAbs(float a)
{
    uint32_t tmp = *(uint32_t*)(&a) & 0x7FFFFFFF;
    return *(float*)&tmp;
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
  l_i_acc += ((acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->current);
  l_p_acc += ((acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->voltage);
  l_q_acc += ((acc_t)p_ws->p_samples->current * (acc_t)p_ws->p_samples->voltage90);
  l_v_acc += ((acc_t)p_ws->p_samples->voltage * (acc_t)p_ws->p_samples->voltage);
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
}

void LMA_ADC_Start(void)
{
  adc_running = true;
}

void LMA_ADC_Stop(void)
{
  adc_running = false;
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
  printf("ACTIVE LED ON");
}

void LMA_IMP_ActiveOff(void)
{
  printf("ACTIVE LED OFF");
}

void LMA_IMP_ReactiveOn(void)
{
  printf("REACTIVE LED ON");
}

void LMA_IMP_ReactiveOff(void)
{
  printf("REACTIVE LED OFF");
}

static int32_t Phase_shift_90deg(int32_t new_voltage)
{
  static spl_t voltage_buffer[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t buffer_index = 0;

  uint8_t buffer_index_19 = (buffer_index + 2) % 21;
  uint8_t buffer_index_20 = (buffer_index + 1) % 21;

  /* Append new voltage*/
  voltage_buffer[buffer_index] = new_voltage;

/* 90 degree phase shift = a quarter of the samples in a full line cycle:
 * fs / (fline * 4) = 3906.25 / 50 = 19.53125
 * so interpolate 0.53125 between sample 19 and 20 in the delays.
 *
 * Over kill...
 * In reality - these values can be optimised at compile time for target sampling frequency and target line frequency.
 * Considering grid frequency varies maximally by +/- 1% and sampling frequency is even better.
 * However considering we are on  the PC we may as well work out best case and fit to needs on the embedded systems later.
 *
 * AKA. The difficult way
 */
#if 0
  static const param_t fs = 3906.25;
  static const param_t fline = 50.0;
  param_t sample_shift = fs / (fline * 4.00);
  uint8_t integer_part = (uint8_t)sample_shift + 2;
  param_t fractional_part = fractional_part - (float)integer_part;

   /* The fraction represents how much of the next sample (20) we want after full sample (19)*/
  param_t fractional_part19 = 1 - fractional_part;
  param_t fractional_part20 = fractional_part;

  int32_t interpolated_value =
      (int32_t) (param_t)((param_t)voltage_buffer[buffer_index_19] * fractional_part19) + ((param_t)voltage_buffer[buffer_index_20] * fractional_part20);
#endif

/* The easy way*/
#if 1
  /* Interpolate 19.53 samples - just take the mid point*/
  int32_t interpolated_value =
      ((voltage_buffer[buffer_index_19] * 480) >> 10) + ((voltage_buffer[buffer_index_20] * 544) >> 10);
#endif

  buffer_index = buffer_index_20;

  /* Convert back to its 32b value*/
  return interpolated_value;
}

static int Driver_thread(void *p_arg)
{
  size_t sample = 0;
  size_t rtc_counter = 0;
  size_t sleep_counter = 0;
  const simulation_params *const p_sim_params = (simulation_params *)p_arg;
  const uint32_t one_sec = (uint32_t)(p_sim_params->fs);
  driver_thread_running = true;

  /* Keep running while there are samples available in the simulation*/
  while (sample < p_sim_params->sample_count)
  {
    /* RTC Handling*/
    if (rtc_running)
    {
      ++rtc_counter;

      /* Every 1sec in ADC samples - simulate RTC callback*/
      if (rtc_counter >= one_sec)
      {
        rtc_counter = 0;
        LMA_CB_RTC();
      }
    }

    /* ADC Handling*/
    if (adc_running)
    {
      /* We are removing the bottom 3 bits of noise*/
      phase.samples.voltage = p_sim_params->voltage_samples[sample];
      phase.samples.current = p_sim_params->current_samples[sample];
      phase.samples.voltage90 = Phase_shift_90deg(phase.samples.voltage);

      LMA_CB_ADC_SinglePhase();

      ++sample;
    }

    ++sleep_counter;

    /* Sleep every line cycle (50hz)*/
    if (sleep_counter >= 78)
    {
      sleep_counter = 0;
      const struct timespec ts = {
          .tv_sec = 0, .tv_nsec = 20000000 /**< 20ms */
      };
      thrd_sleep(&ts, NULL);
    }
  }

  driver_thread_running = false;
  return 0;
}
