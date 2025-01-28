#include "LMA_Port.h"
#include "LMA_Core.h"
#include "simulation.h"
#include <stdio.h>
#include <threads.h>
#include <time.h>
#include <math.h>

extern const simulation_params *p_g_sim_params;
extern LMA_Phase phase;
static bool tmr_running = false;
static bool adc_running = false;
static bool rtc_running = false;
bool driver_thread_running = false;

/** @brief thread to emulate drivers running asynch to main thread
 * @param[inout] pointer to args for thread - passing simulation_params for controlling the sim
 */
int Driver_thread(void *p_arg);

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
    uint32_t tmp = *(uint32_t*)(&a) & 0x7FFFFFFF;
    return *(float*)&tmp;
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

    if(buffer_index_19 > 21)
    {
        buffer_index_19 -= 21;
    }

    if(buffer_index_20 > 21)
    {
        buffer_index_20 -= 21;
    }

    /* Append new voltage*/
    voltage_buffer[buffer_index] = new_voltage;

    /* Interpolate 19.53 samples - just take the mid point*/
    int32_t interpolated_value =
            ((voltage_buffer[buffer_index_19] * 60) >> 7) + ((voltage_buffer[buffer_index_20] * 68) >> 7);

    buffer_index = buffer_index_20;

    /* Convert back to its 32b value*/
    return interpolated_value;
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

void LMA_IMP_ApparentOn(void)
{
  printf("APPARENT LED ON");
}

void LMA_IMP_ApparentOff(void)
{
  printf("APPARENT LED OFF");
}

static int Driver_thread(void *p_arg)
{
  size_t sample = 0;
  size_t rtc_counter = 0;
  size_t tmr_counter = 0;
  size_t sleep_counter = 0;
  const simulation_params *const p_sim_params = (simulation_params *)p_arg;
  const uint32_t one_sec = (uint32_t)(p_sim_params->fs);
  const uint32_t ten_msec = (uint32_t)(p_sim_params->fs) / 40;
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

    /* TMR Handling*/
    if (tmr_running)
    {
      ++tmr_counter;

      /* Every 10msec in ADC samples - simulate TMR callback*/
      if (tmr_counter >= ten_msec)
      {
        tmr_counter = 0;
        LMA_CB_TMR();
      }
    }

    /* ADC Handling*/
    if (adc_running)
    {
      /* We are removing the bottom 3 bits of noise*/
      phase.ws.samples.voltage = p_sim_params->voltage_samples[sample];
      phase.ws.samples.current = p_sim_params->current_samples[sample];
      phase.ws.samples.voltage90 = LMA_PhaseShift90(phase.ws.samples.voltage);

      LMA_CB_ADC();

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
