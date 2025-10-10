/**
 * @file LMA_Core.c
 * @brief Core functionality definitions for LMA.
 *
 * @details This file provides definitions of the LMA core functionality exposed in LMA_Core header.
 */

#include "LMA_Core.h"
#include <string.h>

/* Locally Used Types*/

/**
 * @brief Internal fs calibration structure
 * @details Data structure containing calibration data for sampling frequency (singleton).
 */
typedef struct LMA_CalibFs
{
  bool start;           /**< flag to indicate starting fs calibration routine */
  bool running;         /**< flag to indicate we are running */
  bool finished;        /**< flag to indicate calibration is finished */
  uint32_t rtc_counter; /**< counter to count rtc cycles for accumulation */
  uint32_t adc_counter; /**< counter to count number of ADC cycles have accumulated. */
  LMA_Phase *p_phase;   /**< ointer to phase to work on */
} LMA_CalibFs;

/* Static/Local Variable Declarations*/
static LMA_Config *p_config = NULL;    /**< Internal copy of the meter configuration */
static LMA_Phase *p_phase_list = NULL; /**< Linked list of phases*/
static LMA_CalibFs calib_fs;           /**< Instance of the fs calibration data */
static LMA_SystemEnergy sys_energy =   /**< Instance of the fs calibration data */
    {
        .energy =
            {
                .unit = {.act = 0, .react = 0, .app = 0},
                .accumulator =
                    {
                        .act_imp_ws = 0,
                        .act_exp_ws = 0,
                        .app_imp_ws = 0,
                        .app_exp_ws = 0,
                        .c_react_imp_ws = 0,
                        .c_react_exp_ws = 0,
                        .l_react_imp_ws = 0,
                        .l_react_exp_ws = 0,
                    },
                .counter =
                    {
                        .act_imp = 0,
                        .act_exp = 0,
                        .app_imp = 0,
                        .app_exp = 0,
                        .c_react_imp = 0,
                        .c_react_exp = 0,
                        .l_react_imp = 0,
                        .l_react_exp = 0,
                    },
            },
        .impulse =
            {
                .led_on_count = 0,
                .active_counter = 0,
                .apparent_counter = 0,
                .reactive_counter = 0,
                .active_on = false,
                .apparent_on = false,
                .reactive_on = false,
            },
};

/* Static/Local functions*/

/** @brief Check for zero cross on given phase.
 * @param[inout] p_phase - pointer to the phase block to update for zero cross.
 * @return true if new zero cross detected - false otherwise.
 */
static inline bool Phase_zero_cross(LMA_ZeroCross *const p_zc, const spl_t new_spl)
{
  if ((!p_zc->zero_cross_debounce) && (p_zc->last_sample <= 0) && (new_spl > 0))
  {
    p_zc->zero_cross_debounce = true;

    if (p_zc->sync_zc)
    {
      /* Only increment the counter if we are synch'd (already detected our first zero cross)*/
      ++p_zc->zero_cross_counter;
    }
    else
    {
      /* Otherwise synch us up and reset the counter*/
      p_zc->zero_cross_counter = 0;
      p_zc->sync_zc = true;
    }
  }
  else
  {
    p_zc->zero_cross_debounce = false;
  }

  p_zc->last_sample = new_spl;

  return p_zc->zero_cross_debounce;
}
/* END OF FUNCTION*/

/** @brief Complete hard reset on a phase
 * @details Will reset the zero cross synch flag so we wait for the next full zero cross to be detected.
 * And resets all accumulators to zero.
 * @param[inout] p_phase - pointer to the phase block to reset
 */
static inline void Phase_hard_reset(LMA_Phase *const p_phase)
{
  p_phase->voltage.zc.zero_cross_counter = 0;
  p_phase->voltage.zc.last_sample = 0;
  p_phase->voltage.zc.zero_cross_debounce = false;
  p_phase->voltage.zc.sync_zc = false;

  p_phase->accs.vacc = 0;
  p_phase->accs.iacc = 0;
  p_phase->accs.pacc = 0;
  p_phase->accs.qacc = 0;
  p_phase->accs.sample_count = 0;

  p_phase->ws.samples.current = 0;
  p_phase->ws.samples.voltage = 0;
  p_phase->ws.samples.voltage90 = 0;

  p_phase->energy_units.act = 0;
  p_phase->energy_units.react = 0;
  p_phase->energy_units.app = 0;

  p_phase->pa_error.i_fraction = 0L;
  p_phase->pa_error.v_fraction = 0L;
  p_phase->pa_error.status = LMA_CALIB_OK;
  p_phase->pa_error.sample_counter = 0;
  p_phase->pa_error.i_last_sample = 0;
  p_phase->pa_error.v_last_sample = 0;

  LMA_AccReset(&(p_phase->ws), p_phase->phase_number);

  p_phase->sigs.accumulators_ready = false;
  p_phase->sigs.measurements_ready = false;

  p_phase->voltage.fline = 0.0f;
  p_phase->voltage.v_rms = 0.0f;

  p_phase->current.i_rms = 0.0f;

  p_phase->power.p = 0.0f;
  p_phase->power.q = 0.0f;
  p_phase->power.s = 0.0f;
}
/* END OF FUNCTION*/

/** @brief Processes phase angle error calibration
 * @param[inout] p_phase - pointer to the phase block to work on
 */
static inline void Phase_calibrate_angle_error(LMA_Phase *const p_phase)
{
  bool izc = (p_phase->ws.samples.current > 0 && p_phase->pa_error.i_last_sample <= 0);
  bool vzc = (p_phase->ws.samples.voltage > 0 && p_phase->pa_error.v_last_sample <= 0);

  if (izc && vzc)
  {
    /* Edge case*/
    if (0 == p_phase->pa_error.i_last_sample)
    {
      p_phase->pa_error.i_fraction += 0.0f;
    }
    else
    {
      p_phase->pa_error.i_fraction +=
          (float)(p_phase->ws.samples.current) / (float)(p_phase->ws.samples.current - p_phase->pa_error.i_last_sample);
    }

    /* Edge case*/
    if (0 == p_phase->pa_error.v_last_sample)
    {
      p_phase->pa_error.v_fraction += 0.0f;
    }
    else
    {
      p_phase->pa_error.v_fraction +=
          (float)(p_phase->ws.samples.voltage) / (float)(p_phase->ws.samples.voltage - p_phase->pa_error.v_last_sample);
    }

    ++p_phase->pa_error.sample_counter;

    if (p_phase->pa_error.sample_counter > p_config->update_interval)
    {
      p_phase->pa_error.status = LMA_CALIB_OK;
      p_phase->sigs.calibrate_angle_error = false;
    }
  }
  else if (izc)
  {
    /* Error - phase angle too large*/
    p_phase->pa_error.status = LMA_CALIB_PHASE_ANGLE_ERROR;
  }
  else if (vzc)
  {
    /* Error - phase angle too large*/
    p_phase->pa_error.status = LMA_CALIB_PHASE_ANGLE_ERROR;
  }
  else
  {
    /* Do Nothing - no zero cross detected*/
  }

  p_phase->pa_error.v_last_sample = p_phase->ws.samples.voltage;
  p_phase->pa_error.i_last_sample = p_phase->ws.samples.current;
}
/* END OF FUNCTION*/

/* Externally Available Functions*/

void LMA_Init(LMA_Config *const p_config_arg)
{
  p_config = p_config_arg;
  LMA_IMP_ActiveOff();
  LMA_IMP_ApparentOff();
  LMA_IMP_ReactiveOff();
  LMA_ADC_Init();
  LMA_TMR_Init();
  LMA_RTC_Init();
}

void LMA_Deinit(void)
{
  /* invalidate phase list*/
  LMA_Phase *tmp = p_phase_list;
  LMA_Phase *del = tmp;

  /* Walk the list*/
  while (NULL != tmp->p_next)
  {
    /* Update tmp to next entry*/
    tmp = tmp->p_next;

    /* Delete the link in the prvious phase before updating*/
    del->p_next = NULL;
    del = tmp;
  }

  p_phase_list = NULL;
}

void LMA_PhaseRegister(LMA_Phase *const p_phase)
{
  static uint32_t phase_count = 0;

  if (NULL == p_phase_list)
  {
    p_phase_list = p_phase;
  }
  else
  {
    LMA_Phase *tmp = p_phase_list;
    while (NULL != tmp->p_next)
    {
      tmp = tmp->p_next;
    }
    tmp->p_next = p_phase;
  }

  p_phase->p_next = NULL;

  ++phase_count;
  p_phase->phase_number = phase_count;

  Phase_hard_reset(p_phase);
}

void LMA_PhaseLoadCalibration(LMA_Phase *const p_phase, const LMA_PhaseCalibration *const p_calib)
{
  memcpy(&(p_phase->calib), p_calib, sizeof(LMA_PhaseCalibration));
}

void LMA_Start(void)
{
  LMA_ADC_Start();
  LMA_TMR_Start();
  LMA_RTC_Start();
}

void LMA_Stop(void)
{
  LMA_ADC_Stop();
  LMA_TMR_Stop();
  LMA_RTC_Stop();
}

void LMA_PhaseCalibrate(LMA_PhaseCalibArgs *const calib_args)
{
  uint32_t backup_update_interval = p_config->update_interval;
  float sample_count_fp;

  LMA_ADC_Stop();
  LMA_TMR_Stop();

  p_config->update_interval = calib_args->line_cycles;
  Phase_hard_reset(calib_args->p_phase);

  LMA_ADC_Start();

  while (!calib_args->p_phase->sigs.accumulators_ready)
  {
    /* Wait until the accumulation has stopped*/
  }

  LMA_ADC_Stop();

  sample_count_fp = (float)calib_args->p_phase->accs.sample_count;

  /* Update Coefficients*/
  calib_args->p_phase->calib.vrms_coeff =
      LMA_FPSqrt_Fast((float)(calib_args->p_phase->accs.vacc) / sample_count_fp) / calib_args->vrms_tgt;
  calib_args->p_phase->calib.irms_coeff =
      LMA_FPSqrt_Fast((float)(calib_args->p_phase->accs.iacc) / sample_count_fp) / calib_args->irms_tgt;
  calib_args->p_phase->calib.p_coeff = calib_args->p_phase->calib.vrms_coeff * calib_args->p_phase->calib.irms_coeff;

  /* Restore operation*/
  p_config->update_interval = backup_update_interval;
  Phase_hard_reset(calib_args->p_phase);

  /* Start on the phase angle error*/
  calib_args->p_phase->sigs.calibrate_angle_error = true;
  calib_args->p_phase->pa_error.i_last_sample = calib_args->p_phase->ws.samples.current;
  calib_args->p_phase->pa_error.v_last_sample = calib_args->p_phase->ws.samples.voltage;

  LMA_ADC_Start();

  while (calib_args->p_phase->sigs.calibrate_angle_error)
  {
    /* Wait until the accumulation has stopped*/
  }

  LMA_ADC_Stop();

  sample_count_fp = (float)calib_args->p_phase->pa_error.sample_counter;

  calib_args->p_phase->pa_error.i_fraction /= sample_count_fp;
  calib_args->p_phase->pa_error.v_fraction /= sample_count_fp;

  /* Compute from Q32.32 to float*/
  calib_args->p_phase->calib.vi_phase_correction =
      calib_args->p_phase->pa_error.i_fraction - calib_args->p_phase->pa_error.v_fraction;

  /* Convert to degrees*/
  calib_args->p_phase->calib.vi_phase_correction *= p_config->gcalib.deg_per_sample;

  Phase_hard_reset(calib_args->p_phase);

  LMA_TMR_Start();
  LMA_ADC_Start();
}

void LMA_GlobalCalibrate(LMA_GlobalCalibArgs *const calib_args)
{
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_ADC_Stop();
  LMA_TMR_Stop();

  LMA_CRITICAL_SECTION_ENTER();

  calib_fs.finished = false;
  calib_fs.running = false;
  calib_fs.rtc_counter = calib_args->rtc_cycles;
  calib_fs.adc_counter = 0;
  calib_fs.start = true;

  LMA_CRITICAL_SECTION_EXIT();

  /* The RTC will now start the ADC to synch the sampling.
   * Once the active accumulation has stopped - we can update the sampling frequency coefficient.
   */
  while (!calib_fs.finished)
  {
    /* Wait until the accumulation has stopped*/
  }

  calib_fs.finished = false;

  /* Care more about accuracy here than speed*/
  p_config->gcalib.fs = (float)calib_fs.adc_counter / ((float)calib_args->rtc_period * (float)calib_args->rtc_cycles);
  p_config->gcalib.fline_coeff = p_config->gcalib.fs * (float)p_config->update_interval;
  p_config->gcalib.deg_per_sample = (360.00f * p_config->fline_target) / p_config->gcalib.fs;

  LMA_TMR_Start();
  LMA_ADC_Start();
}

void LMA_EnergySet(LMA_SystemEnergy *const p_energy)
{
  LMA_CRITICAL_SECTION_PREPARE();
  LMA_CRITICAL_SECTION_ENTER();
  memcpy(&sys_energy, p_energy, sizeof(LMA_SystemEnergy));
  LMA_CRITICAL_SECTION_EXIT();
}

void LMA_EnergyGet(LMA_SystemEnergy *const p_energy)
{
  LMA_CRITICAL_SECTION_PREPARE();
  LMA_CRITICAL_SECTION_ENTER();
  memcpy(p_energy, &sys_energy, sizeof(LMA_SystemEnergy));
  LMA_CRITICAL_SECTION_EXIT();
}

LMA_Status LMA_StatusGet(const LMA_Phase *const p_phase)
{
  LMA_Status temp = LMA_OK;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  temp = p_phase->status;
  LMA_CRITICAL_SECTION_EXIT();

  return temp;
}

float LMA_VrmsGet(LMA_Phase *const p_phase)
{
  float vrms_tmp;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  vrms_tmp = p_phase->voltage.v_rms;
  LMA_CRITICAL_SECTION_EXIT();

  return vrms_tmp;
}

float LMA_IrmsGet(LMA_Phase *const p_phase)
{
  float irms_tmp;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  irms_tmp = p_phase->current.i_rms;
  LMA_CRITICAL_SECTION_EXIT();

  return irms_tmp;
}

float LMA_FLineGet(LMA_Phase *const p_phase)
{
  float fline_tmp;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  fline_tmp = p_phase->voltage.fline;
  LMA_CRITICAL_SECTION_EXIT();

  return fline_tmp;
}

float LMA_ActivePowerGet(const LMA_Phase *const p_phase)
{
  float p_tmp;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  p_tmp = p_phase->power.p;
  LMA_CRITICAL_SECTION_EXIT();

  return p_tmp;
}

float LMA_ReactivePowerGet(const LMA_Phase *const p_phase)
{
  float q_tmp;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  q_tmp = p_phase->power.q;
  LMA_CRITICAL_SECTION_EXIT();

  return q_tmp;
}

float LMA_ApparentPowerGet(const LMA_Phase *const p_phase)
{
  float s_tmp;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  s_tmp = p_phase->power.s;
  LMA_CRITICAL_SECTION_EXIT();

  return s_tmp;
}

void LMA_MeasurementsGet(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements)
{
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();

  p_measurements->vrms = p_phase->voltage.v_rms;
  p_measurements->irms = p_phase->current.i_rms;
  p_measurements->fline = p_phase->voltage.fline;
  p_measurements->p = p_phase->power.p;
  p_measurements->q = p_phase->power.q;
  p_measurements->s = p_phase->power.s;

  LMA_CRITICAL_SECTION_EXIT();
}

void LMA_ConsumptionDataGet(const LMA_SystemEnergy *const p_se, LMA_ConsumptionData *const p_ec)
{
  p_ec->act_imp_energy_wh =
      (((float)p_se->energy.counter.act_imp * p_config->meter_constant) + p_se->energy.accumulator.act_imp_ws) / 3600.00f;
  p_ec->act_exp_energy_wh =
      (((float)p_se->energy.counter.act_exp * p_config->meter_constant) + p_se->energy.accumulator.act_exp_ws) / 3600.00f;
  p_ec->app_imp_energy_wh =
      (((float)p_se->energy.counter.app_imp * p_config->meter_constant) + p_se->energy.accumulator.app_imp_ws) / 3600.00f;
  p_ec->app_exp_energy_wh =
      (((float)p_se->energy.counter.app_exp * p_config->meter_constant) + p_se->energy.accumulator.app_exp_ws) / 3600.00f;
  p_ec->c_imp_energy_wh =
      (((float)p_se->energy.counter.c_react_imp * p_config->meter_constant) + p_se->energy.accumulator.c_react_imp_ws) /
      3600.00f;
  p_ec->c_exp_energy_wh =
      (((float)p_se->energy.counter.c_react_exp * p_config->meter_constant) + p_se->energy.accumulator.c_react_exp_ws) /
      3600.00f;
  p_ec->l_imp_energy_wh =
      (((float)p_se->energy.counter.l_react_imp * p_config->meter_constant) + p_se->energy.accumulator.l_react_imp_ws) /
      3600.00f;
  p_ec->l_exp_energy_wh =
      (((float)p_se->energy.counter.l_react_exp * p_config->meter_constant) + p_se->energy.accumulator.l_react_exp_ws) /
      3600.00f;
}

bool LMA_MeasurementsReady(LMA_Phase *const p_phase)
{
  bool tmp;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();
  tmp = p_phase->sigs.measurements_ready;
  p_phase->sigs.measurements_ready = false;
  LMA_CRITICAL_SECTION_EXIT();

  return tmp;
}

void LMA_CB_ADC(void)
{
  LMA_Phase *p_phase = p_phase_list;
  bool process_energy = true;

  /* If we are running fs calibration - increment the counter*/
  if (calib_fs.running)
  {
    ++calib_fs.adc_counter;
  }
  else
  {
    while (NULL != p_phase)
    {
      if (p_phase->sigs.calibrate_angle_error)
      {
        process_energy = false;
        Phase_calibrate_angle_error(p_phase);
      }
      else
      {
        /* Zero cross - voltage*/
        Phase_zero_cross(&(p_phase->voltage.zc), p_phase->ws.samples.voltage);

        /* Handle active & apparent component once synched with zero cross and accumulation is enabled */
        if (p_phase->voltage.zc.sync_zc)
        {
          LMA_AccRun(&(p_phase->ws), p_phase->phase_number);

          /* If appropriate number of line cycles have passed - process results*/
          if (p_phase->voltage.zc.zero_cross_counter >= p_config->update_interval)
          {
            /* Copy the accumulators across*/
            LMA_AccLoad(&(p_phase->ws), &(p_phase->accs), p_phase->phase_number);

            /* Signal Accumulators are ready*/
            p_phase->sigs.accumulators_ready = true;

            /* Reset*/
            LMA_AccReset(&(p_phase->ws), p_phase->phase_number);

            /* Reset Zerocross counter to continue*/
            p_phase->voltage.zc.zero_cross_counter = 0;
          }
        }
      }

      p_phase = p_phase->p_next;
    }

    if (process_energy)
    {
      /* Active LED Management*/
      if (sys_energy.impulse.active_on)
      {
        ++sys_energy.impulse.active_counter;
        if (sys_energy.impulse.active_counter > sys_energy.impulse.led_on_count)
        {
          sys_energy.impulse.active_on = false;
          LMA_IMP_ActiveOff();
        }
      }

      /* Apparent LED Management*/
      if (sys_energy.impulse.apparent_on)
      {
        ++sys_energy.impulse.apparent_counter;
        if (sys_energy.impulse.apparent_counter > sys_energy.impulse.led_on_count)
        {
          sys_energy.impulse.apparent_on = false;
          LMA_IMP_ApparentOff();
        }
      }

      /* Reactive LED Management*/
      if (sys_energy.impulse.reactive_on)
      {
        ++sys_energy.impulse.reactive_counter;
        if (sys_energy.impulse.reactive_counter > sys_energy.impulse.led_on_count)
        {
          sys_energy.impulse.reactive_on = false;
          LMA_IMP_ReactiveOff();
        }
      }

      /*Energy accumulation*/
      if (sys_energy.energy.unit.act >= 0)
      {
        sys_energy.energy.accumulator.act_imp_ws += sys_energy.energy.unit.act;
        if (sys_energy.energy.accumulator.act_imp_ws >= p_config->meter_constant)
        {
          sys_energy.energy.accumulator.act_imp_ws -= p_config->meter_constant;
          ++sys_energy.energy.counter.act_imp;

          /* Trigger Pulse*/
          sys_energy.impulse.active_counter = 0;
          sys_energy.impulse.active_on = true;
          LMA_IMP_ActiveOn();
        }

        sys_energy.energy.accumulator.app_imp_ws += sys_energy.energy.unit.app;
        if (sys_energy.energy.accumulator.app_imp_ws >= p_config->meter_constant)
        {
          sys_energy.energy.accumulator.app_imp_ws -= p_config->meter_constant;
          ++sys_energy.energy.counter.app_imp;

          /* Trigger Pulse*/
          sys_energy.impulse.apparent_counter = 0;
          sys_energy.impulse.apparent_on = true;
          LMA_IMP_ApparentOn();
        }

        if (sys_energy.energy.unit.react >= 0)
        {
          /* QI - Active From Grid (Import) & Inductive From Grid (Import) - Apparent From Grid (Import)*/
          sys_energy.energy.accumulator.l_react_imp_ws += sys_energy.energy.unit.react;
          if (sys_energy.energy.accumulator.l_react_imp_ws >= p_config->meter_constant)
          {
            sys_energy.energy.accumulator.l_react_imp_ws -= p_config->meter_constant;
            ++sys_energy.energy.counter.l_react_imp;

            /* Trigger Pulse*/
            sys_energy.impulse.reactive_counter = 0;
            sys_energy.impulse.reactive_on = true;
            LMA_IMP_ReactiveOn();
          }
        }
        else
        {
          /* QIV - Active From Grid (Import) & Capacitive To Grid (Export) - Apparent From Grid (Import)*/
          sys_energy.energy.accumulator.c_react_exp_ws -= sys_energy.energy.unit.react;
          if (sys_energy.energy.accumulator.c_react_exp_ws >= p_config->meter_constant)
          {
            sys_energy.energy.accumulator.c_react_exp_ws -= p_config->meter_constant;
            ++sys_energy.energy.counter.c_react_exp;

            /* Trigger Pulse*/
            sys_energy.impulse.reactive_counter = 0;
            sys_energy.impulse.reactive_on = true;
            LMA_IMP_ReactiveOn();
          }
        }
      }
      else
      {
        sys_energy.energy.accumulator.act_exp_ws -= sys_energy.energy.unit.act;
        if (sys_energy.energy.accumulator.act_exp_ws >= p_config->meter_constant)
        {
          sys_energy.energy.accumulator.act_exp_ws -= p_config->meter_constant;
          ++sys_energy.energy.counter.act_exp;

          /* Trigger Pulse*/
          sys_energy.impulse.active_counter = 0;
          sys_energy.impulse.active_on = true;
          LMA_IMP_ActiveOn();
        }

        sys_energy.energy.accumulator.app_exp_ws += sys_energy.energy.unit.app;
        if (sys_energy.energy.accumulator.app_exp_ws >= p_config->meter_constant)
        {
          sys_energy.energy.accumulator.app_exp_ws -= p_config->meter_constant;
          ++sys_energy.energy.counter.app_exp;

          /* Trigger Pulse*/
          sys_energy.impulse.apparent_counter = 0;
          sys_energy.impulse.apparent_on = true;
          LMA_IMP_ActiveOn();
        }

        if (sys_energy.energy.unit.react >= 0)
        {
          /* QII - Active To Grid (Export) & Capacitive From Grid (Import) - Apparent To Grid (Export)*/
          sys_energy.energy.accumulator.c_react_imp_ws += sys_energy.energy.unit.react;
          if (sys_energy.energy.accumulator.c_react_imp_ws >= p_config->meter_constant)
          {
            sys_energy.energy.accumulator.c_react_imp_ws -= p_config->meter_constant;
            ++sys_energy.energy.counter.c_react_imp;

            /* Trigger Pulse*/
            sys_energy.impulse.reactive_counter = 0;
            sys_energy.impulse.reactive_on = true;
            LMA_IMP_ReactiveOn();
          }
        }
        else
        {
          /* QIII - Active To Grid (Export) & Inductive To Grid (Export) - Apparent To Grid (Export)*/
          sys_energy.energy.accumulator.l_react_exp_ws -= sys_energy.energy.unit.react;
          if (sys_energy.energy.accumulator.l_react_exp_ws >= p_config->meter_constant)
          {
            sys_energy.energy.accumulator.l_react_exp_ws -= p_config->meter_constant;
            ++sys_energy.energy.counter.l_react_exp;

            /* Trigger Pulse*/
            sys_energy.impulse.reactive_counter = 0;
            sys_energy.impulse.reactive_on = true;
            LMA_IMP_ReactiveOn();
          }
        }
      }
    }
  }
}

void LMA_CB_TMR(void)
{
  LMA_CRITICAL_SECTION_PREPARE();
  LMA_Phase *p_phase = p_phase_list;
  static float act_energy_unit_tmp = 0.0f;
  static float react_energy_unit_tmp = 0.0f;
  static float app_energy_unit_tmp = 0.0f;

  /* Reset the energy units*/
  act_energy_unit_tmp = 0.0f;
  react_energy_unit_tmp = 0.0f;
  app_energy_unit_tmp = 0.0f;

  while (NULL != p_phase)
  {

    if (p_phase->sigs.accumulators_ready)
    {
      const float sample_count_fp = LMA_AccToFloat((acc_t)p_phase->accs.sample_count);
      p_phase->sigs.accumulators_ready = false;

      /* Frequency*/
      p_phase->voltage.fline = LMA_FPDiv_Fast(p_config->gcalib.fline_coeff, sample_count_fp);

      /* Check for valid frequency input*/
      if (p_phase->voltage.fline < p_config->fline_tol_high && p_phase->voltage.fline > p_config->fline_tol_low)
      {
        const float power_divisor = LMA_FPMul_Fast(sample_count_fp, p_phase->calib.p_coeff);
        const float vacc_fp = LMA_AccToFloat(p_phase->accs.vacc);
        const float iacc_fp = LMA_AccToFloat(p_phase->accs.iacc);

        /* Vrms*/
        p_phase->voltage.v_rms =
            LMA_FPDiv_Fast(LMA_FPSqrt_Fast(LMA_FPDiv_Fast(vacc_fp, sample_count_fp)), p_phase->calib.vrms_coeff);
        /* Irms*/
        p_phase->current.i_rms =
            LMA_FPDiv_Fast(LMA_FPSqrt_Fast(LMA_FPDiv_Fast(iacc_fp, sample_count_fp)), p_phase->calib.irms_coeff);
        /* Active Power (P)*/
        p_phase->power.p = LMA_FPDiv_Fast(LMA_AccToFloat(p_phase->accs.pacc), power_divisor);
        /* Reactive Power (Q)*/
        p_phase->power.q = LMA_FPDiv_Fast(LMA_AccToFloat(p_phase->accs.qacc), power_divisor);
        /* Apparent Power (S)*/
        p_phase->power.s = LMA_FPDiv_Fast(LMA_FPSqrt_Fast(LMA_FPMul_Fast(iacc_fp, vacc_fp)), power_divisor);

        /* V SAG AND SWELL*/
        if (p_phase->voltage.v_rms < p_config->v_sag)
        {
          p_phase->status |= LMA_VOLTAGE_SAG;
          p_phase->status &= ~LMA_VOLTAGE_SWELL;
        }
        else if (p_phase->voltage.v_rms > p_config->v_swell)
        {
          p_phase->status = LMA_VOLTAGE_SWELL;
          p_phase->status &= ~LMA_VOLTAGE_SAG;
        }
        else
        {
          p_phase->status &= ~LMA_VOLTAGE_SAG;
          p_phase->status &= ~LMA_VOLTAGE_SWELL;
        }

        /* Active Power (P) & Energy*/
        if (LMA_FPAbs_Fast(p_phase->power.p) < p_config->no_load_p)
        {
          p_phase->status |= LMA_NO_ACTIVE_LOAD;
          p_phase->power.p = 0;
          p_phase->energy_units.act = 0;
        }
        else
        {
          p_phase->status &= ~LMA_NO_ACTIVE_LOAD;
          p_phase->energy_units.act = LMA_FPDiv_Fast(p_phase->power.p, p_config->gcalib.fs);
        }

        /* Reactive Power (Q) & Energy*/
        if (LMA_FPAbs_Fast(p_phase->power.q) < p_config->no_load_p)
        {
          p_phase->status |= LMA_NO_REACTIVE_LOAD;
          p_phase->power.q = 0;
          p_phase->energy_units.react = 0;
        }
        else
        {
          p_phase->status &= ~LMA_NO_REACTIVE_LOAD;
          p_phase->energy_units.react = LMA_FPDiv_Fast(p_phase->power.q, p_config->gcalib.fs);
        }

        /* Apparent Power (S) & Energy*/
        if (LMA_FPAbs_Fast(p_phase->power.s) < p_config->no_load_p)
        {
          p_phase->status |= LMA_NO_APPARENT_LOAD;
          p_phase->power.s = 0;
          p_phase->energy_units.app = 0;
        }
        else
        {
          p_phase->status &= ~LMA_NO_APPARENT_LOAD;
          p_phase->energy_units.app = LMA_FPDiv_Fast(p_phase->power.s, p_config->gcalib.fs);
        }
      }
      else
      {
        /* Handle Invalid Frequency*/
        /* Vrms*/
        p_phase->voltage.v_rms = 0.0f;
        /* Irms*/
        p_phase->current.i_rms = 0.0f;
        /* Frequency*/
        p_phase->voltage.fline = 0.0f;
        /* Active Power (P)*/
        p_phase->power.p = 0.0f;
        /* Reactive Power (Q)*/
        p_phase->power.q = 0.0f;
        /* Apparent Power (S)*/
        p_phase->power.s = 0.0f;

        /* Active Energy*/
        p_phase->energy_units.act = 0;
        /* Reactive Energy*/
        p_phase->energy_units.react = 0;
        /* Apparent Energy*/
        p_phase->energy_units.app = 0;
      }

      p_phase->sigs.measurements_ready = true;
    }

    act_energy_unit_tmp += p_phase->energy_units.act;
    react_energy_unit_tmp += p_phase->energy_units.react;
    app_energy_unit_tmp += p_phase->energy_units.app;

    p_phase = p_phase->p_next;
  }

  /* Overwrite the energy units in the system energy manager*/
  LMA_CRITICAL_SECTION_ENTER();
  sys_energy.energy.unit.act = act_energy_unit_tmp;
  sys_energy.energy.unit.react = react_energy_unit_tmp;
  sys_energy.energy.unit.app = app_energy_unit_tmp;
  LMA_CRITICAL_SECTION_EXIT();
}

void LMA_CB_RTC(void)
{
  /* If we are calibrating, synch the ADC sampling window to the RTC*/
  if (calib_fs.start)
  {
    calib_fs.start = false;
    calib_fs.running = true;
    calib_fs.finished = false;

    LMA_ADC_Start();
  }
  else if (calib_fs.running)
  {
    --calib_fs.rtc_counter;
    if (0 == calib_fs.rtc_counter)
    {
      LMA_ADC_Stop();

      calib_fs.start = false;
      calib_fs.running = false;
      calib_fs.finished = true; /* Signal to calibration routine we are done*/
    }
  }
  else
  {
    /* Do Nothing */
  }
}
