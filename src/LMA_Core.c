/**
 * @file LMA_Core.c
 * @brief Core functionality definitions for LMA.
 *
 * @details This file provides definitions of the LMA core functionality exposed in LMA_Core header.
 */

#include "LMA_Core.h"
#include <math.h>
#include <string.h>

/* Locally Used Types*/

/**
 * @brief Internal fs calibration structure
 * @details Data structure containing calibration data for sampling frequency (singleton).
 */
typedef struct LMA_CalibFs_str
{
  bool start;           /**< flag to indicate starting fs calibration routine */
  bool running;         /**< flag to indicate we are running */
  bool finished;        /**< flag to indicate calibration is finished */
  bool active;          /**< flag to indicate calibration of global params is active*/
  uint32_t rtc_counter; /**< counter to count rtc cycles for accumulation */
  uint32_t adc_counter; /**< counter to count number of ADC cycles have accumulated. */
  LMA_Phase *p_phase;   /**< pinter to phase to work on */
} LMA_CalibFs;

/**
 * @brief Internal type for handling phase linked list
 * @details Exists for convenience.
 */
typedef struct LMA_PhaseList_str
{
  LMA_Phase *p_first_phase; /**< pointer to the first phase in the list*/
  uint32_t phase_count;     /**< number of phases total*/
} LMA_PhaseList;

/* Static/Local Variable Declarations*/
static LMA_Config *p_config = NULL; /**< Internal copy of the meter configuration */
static LMA_CalibFs calib_fs = {false,       false,       false, false,
                               (uint32_t)0, (uint32_t)0, NULL}; /**< Instance of the fs calibration data */
static LMA_PhaseList phase_list = {NULL, (uint32_t)0};          /**< Internal phase list*/

static LMA_SystemEnergy sys_energy = /**< System Energy*/
    {
        .energy =
            {
                .unit = {.act = 0.0f, .react = 0.0f, .app = 0.0f},
                .accumulator =
                    {
                        .act_imp_ws = 0.0f,
                        .act_exp_ws = 0.0f,
                        .app_imp_ws = 0.0f,
                        .app_exp_ws = 0.0f,
                        .c_react_imp_ws = 0.0f,
                        .c_react_exp_ws = 0.0f,
                        .l_react_imp_ws = 0.0f,
                        .l_react_exp_ws = 0.0f,
                    },
                .counter =
                    {
                        .act_imp = (uint64_t)0,
                        .act_exp = (uint64_t)0,
                        .app_imp = (uint64_t)0,
                        .app_exp = (uint64_t)0,
                        .c_react_imp = (uint64_t)0,
                        .c_react_exp = (uint64_t)0,
                        .l_react_imp = (uint64_t)0,
                        .l_react_exp = (uint64_t)0,
                    },
            },
        .impulse =
            {
                .led_on_count = (uint32_t)0,
                .active_counter = (uint32_t)0,
                .apparent_counter = (uint32_t)0,
                .reactive_counter = (uint32_t)0,
                .active_on = false,
                .apparent_on = false,
                .reactive_on = false,
            },
};

/* Static/Local functions*/

/** @brief Check for zero cross
 * @param[inout] p_zc - pointer to the zero cross object to work on.
 * @note The zero cross runs a imple LPF with coefficient 0.5 to ensure stable crossing detection.
 * This is only used for sample synchronisation in computation windows and phase angle error detection during calibration.
 * The impacts of this should be evaluated in the end system.
 * @return true if new zero cross detected - false otherwise.
 */
static bool Zero_cross_detect(LMA_ZeroCross *const p_zc, const spl_t new_spl)
{
  spl_t filtered_new_sample = (spl_t)0;

  /* If we have running for the first time*/
  if (p_zc->already_run)
  {
    filtered_new_sample = (new_spl >> 1) + (p_zc->last_sample >> 1);
  }
  else
  {
    /* Otherwise prime the initial value*/
    filtered_new_sample = new_spl;
    p_zc->last_sample = new_spl;
    p_zc->already_run = true;
  }

  if ((!p_zc->debounce) && (p_zc->last_sample < (spl_t)0) && (filtered_new_sample >= (spl_t)0))
  {
    ++p_zc->count;
    p_zc->debounce = true;
    p_zc->first_event = true;
  }
  else
  {
    p_zc->debounce = false;
  }

  p_zc->last_sample = filtered_new_sample;

  return p_zc->debounce;
}
/* END OF FUNCTION*/

/** @brief Resets zero cross structure.
 * @param[inout] p_zc - pointer to the zero cross object to work on.
 */
static void Zero_cross_hard_reset(LMA_ZeroCross *const p_zc)
{
  p_zc->last_sample = (spl_t)0;
  p_zc->count = (uint32_t)0;
  p_zc->debounce = false;
  p_zc->first_event = false;
  p_zc->already_run = false;
}

/** @brief Complete hard reset on a phase
 * @details Will reset the zero cross synch flag so we wait for the next full zero cross to be detected.
 * And resets all accumulators to zero.
 * @param[inout] p_phase - pointer to the phase block to reset
 */
static void Phase_hard_reset(LMA_Phase *const p_phase)
{
  Zero_cross_hard_reset(&(p_phase->zero_cross_v));

  p_phase->inputs.v_sample = (spl_t)0;
  p_phase->inputs.v90_sample = (spl_t)0;
  p_phase->inputs.i_sample = (spl_t)0;

  p_phase->accs.snapshot.v_acc = (acc_t)0;
  p_phase->accs.snapshot.i_acc = (acc_t)0;
  p_phase->accs.snapshot.p_acc = (acc_t)0;
  p_phase->accs.snapshot.q_acc = (acc_t)0;
  p_phase->accs.snapshot.sample_count = (uint32_t)0;

  if (NULL != p_phase->p_neutral)
  {
    p_phase->p_neutral->inputs.i_sample = (spl_t)0;
    p_phase->p_neutral->accs.i_acc_snapshot = (acc_t)0;
  }

  p_phase->measurements.vrms = 0.0f;
  p_phase->measurements.irms = 0.0f;
  p_phase->measurements.p = 0.0f;
  p_phase->measurements.q = 0.0f;
  p_phase->measurements.s = 0.0f;
  p_phase->measurements.irms_neutral = 0.0f;
  p_phase->measurements.fline = 0.0f;

  p_phase->energy_units.act = 0.0f;
  p_phase->energy_units.react = 0.0f;
  p_phase->energy_units.app = 0.0f;

  LMA_AccPhaseReset(p_phase);

  LMA_PhaseResetHook(p_phase);

  p_phase->sigs.accumulators_ready = false;
  p_phase->sigs.measurements_ready = false;
  p_phase->sigs.calibrating = false;
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
  LMA_Phase *tmp = phase_list.p_first_phase;
  LMA_Phase *del = tmp;

  /* Walk the list*/
  while (NULL != tmp->p_next)
  {
    /* Update tmp to next entry*/
    tmp = tmp->p_next;

    /* Delete the link in the previous phase before updating*/
    del->p_next = NULL;
    del->phase_number = (uint32_t)0;
    del = tmp;
  }

  phase_list.p_first_phase = NULL;
  phase_list.phase_count = (uint32_t)0;
}

void LMA_PhaseRegister(LMA_Phase *const p_phase)
{
  if (NULL == phase_list.p_first_phase)
  {
    phase_list.p_first_phase = p_phase;
  }
  else
  {
    LMA_Phase *tmp = phase_list.p_first_phase;
    while (NULL != tmp->p_next)
    {
      tmp = tmp->p_next;
    }
    tmp->p_next = p_phase;
  }

  p_phase->p_next = NULL;
  p_phase->phase_number = phase_list.phase_count;
  p_phase->p_neutral = NULL;
  phase_list.phase_count += 1;

  Phase_hard_reset(p_phase);
}

void LMA_NeutralRegister(LMA_Phase *const p_phase, LMA_Neutral *const p_neutral)
{
  p_phase->p_neutral = p_neutral;

  p_neutral->accs.i_acc_snapshot = (acc_t)0;
  p_neutral->inputs.i_sample = (spl_t)0;
}

void LMA_ComputationHookRegister(LMA_Phase *const p_phase, float (*comp_hook)(float *i, float *v, float *f))
{
  p_phase->p_computation_hook = comp_hook;
}

void LMA_GlobalLoadCalibration(const LMA_GlobalCalibration *const p_calib)
{
  memcpy(&(p_config->gcalib), p_calib, sizeof(LMA_GlobalCalibration));
}

void LMA_PhaseLoadCalibration(LMA_Phase *const p_phase, const LMA_PhaseCalibration *const p_calib)
{
  memcpy(&(p_phase->calib), p_calib, sizeof(LMA_PhaseCalibration));
}

void LMA_NeutralLoadCalibration(LMA_Neutral *const p_neutral, const LMA_NeutralCalibration *const p_calib)
{
  memcpy(&(p_neutral->calib), p_calib, sizeof(LMA_NeutralCalibration));
}

void LMA_Start(void)
{
  LMA_Phase *tmp = phase_list.p_first_phase;
  LMA_CRITICAL_SECTION_PREPARE();

  /* Reset phases before starting LMA*/
  while (NULL != tmp->p_next)
  {
    Phase_hard_reset(tmp);
    tmp = tmp->p_next;
  }
  Phase_hard_reset(tmp);

  /* Start the ADC*/
  LMA_ADC_Start();

  /* Slow start - for each phase stabilise for the first update period (discard)*/
  tmp = phase_list.p_first_phase;
  while (NULL != tmp)
  {
    while (!tmp->sigs.accumulators_ready)
    {
      /* Wait until the accumulation has stopped*/
    }

    LMA_CRITICAL_SECTION_ENTER();
    tmp->sigs.accumulators_ready = false;
    LMA_CRITICAL_SECTION_EXIT();

    tmp = tmp->p_next;
  }

  /* Now start the RTC and TMR, knowing the ADC signal chain is stable*/
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
  double q, p = 0.0;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_ADC_Stop();
  LMA_TMR_Stop();

  Phase_hard_reset(calib_args->p_phase);

  calib_args->p_phase->sigs.calibrating = true;
  p_config->update_interval = calib_args->line_cycles_stability;

  LMA_ADC_Start();

  /* Stabilise Signal*/
  while (!calib_args->p_phase->sigs.accumulators_ready)
  {
    /* Wait until the accumulation has stopped*/
  }

  /* Accumulate Signal*/
  LMA_CRITICAL_SECTION_ENTER();
  calib_args->p_phase->sigs.accumulators_ready = false;
  p_config->update_interval = calib_args->line_cycles;
  LMA_CRITICAL_SECTION_EXIT();
  while (!calib_args->p_phase->sigs.accumulators_ready)
  {
    /* Wait until the accumulation has stopped*/
  }

  LMA_ADC_Stop();

  sample_count_fp = (float)calib_args->p_phase->accs.snapshot.sample_count;

  /* Update Coefficients*/
  calib_args->p_phase->calib.vrms_coeff =
      sqrtf((float)((double)calib_args->p_phase->accs.snapshot.v_acc) / sample_count_fp) / calib_args->vrms_tgt;
  calib_args->p_phase->calib.irms_coeff =
      sqrtf((float)((double)calib_args->p_phase->accs.snapshot.i_acc) / sample_count_fp) / calib_args->irms_tgt;
  calib_args->p_phase->calib.p_coeff = calib_args->p_phase->calib.vrms_coeff * calib_args->p_phase->calib.irms_coeff;

  if (NULL != calib_args->p_phase->p_neutral)
  {
    calib_args->p_phase->p_neutral->calib.irms_coeff =
        sqrtf((float)((double)calib_args->p_phase->p_neutral->accs.i_acc_snapshot) / sample_count_fp) / calib_args->irms_tgt;
  }

  /* Phase Correction*/
  q = (double)calib_args->p_phase->accs.snapshot.q_acc / calib_args->p_phase->calib.p_coeff;
  p = (double)calib_args->p_phase->accs.snapshot.p_acc / calib_args->p_phase->calib.p_coeff;
  calib_args->p_phase->calib.vi_phase_correction = atanf((float)q / p) * (180.0f / 3.14159265359f);

  /* Restore operation*/
  p_config->update_interval = backup_update_interval;
  Phase_hard_reset(calib_args->p_phase);

  LMA_TMR_Start();
  LMA_ADC_Start();
}

void LMA_GlobalCalibrate(LMA_GlobalCalibArgs *const calib_args)
{
  LMA_Phase *tmp = phase_list.p_first_phase;
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_ADC_Stop();
  LMA_TMR_Stop();

  LMA_CRITICAL_SECTION_ENTER();

  calib_fs.finished = false;
  calib_fs.running = false;
  calib_fs.rtc_counter = calib_args->rtc_cycles;
  calib_fs.adc_counter = (uint32_t)0;
  calib_fs.active = true;
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
  calib_fs.active = false;

  /* Compute system timing parameters*/
  p_config->gcalib.fs = (float)calib_fs.adc_counter / ((float)calib_args->rtc_period * (float)calib_args->rtc_cycles);
  p_config->gcalib.deg_per_sample = (360.00f * calib_args->fline_target) / p_config->gcalib.fs;

  LMA_ADC_Start();

  /* Slow start - for each phase stabilise for the first update period (discard)*/
  tmp = phase_list.p_first_phase;
  while (NULL != tmp)
  {
    while (!tmp->sigs.accumulators_ready)
    {
      /* Wait until the accumulation has stopped*/
    }

    LMA_CRITICAL_SECTION_ENTER();
    tmp->sigs.accumulators_ready = false;
    LMA_CRITICAL_SECTION_EXIT();

    tmp = tmp->p_next;
  }

  LMA_TMR_Start();
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

void LMA_MeasurementsGet(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements)
{
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_CRITICAL_SECTION_ENTER();

  p_measurements->vrms = p_phase->measurements.vrms;
  p_measurements->irms = p_phase->measurements.irms;
  p_measurements->fline = p_phase->measurements.fline;
  p_measurements->p = p_phase->measurements.p;
  p_measurements->q = p_phase->measurements.q;
  p_measurements->s = p_phase->measurements.s;

  if (NULL != p_phase->p_neutral)
  {
    p_measurements->irms_neutral = p_phase->measurements.irms_neutral;
  }
  else
  {
    p_measurements->irms_neutral = 0.0f;
  }

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

/** @details The ADC Callback handles:
 *  1. Sample processing and accumulation.
 *  2. Energy Impulse Management.
 *
 *  It does not update the measured parameters - this is done in the TMR callback.
 */
void LMA_CB_ADC(void)
{
  LMA_Phase *p_phase = phase_list.p_first_phase;
  bool process_energy = true;

  /* If we are running fs calibration - increment the counter*/
  if (!calib_fs.active)
  {
    while (NULL != p_phase)
    {
      if (p_phase->sigs.calibrating)
      {
        process_energy = false;
      }

      /* Zero cross - voltage*/
      (void)Zero_cross_detect(&(p_phase->zero_cross_v), p_phase->inputs.v_sample);

      /* Handle active & apparent component once synched with zero cross and accumulation is enabled */
      if (p_phase->zero_cross_v.first_event)
      {
        LMA_AccPhaseRun(p_phase);

        /* If appropriate number of line cycles have passed - process results*/
        if (p_phase->zero_cross_v.count >= p_config->update_interval)
        {
          /* Get snapshot of accumulators*/
          LMA_AccPhaseLoad(p_phase);

          /* Signal Accumulators are ready*/
          p_phase->sigs.accumulators_ready = true;

          /* Reset*/
          LMA_AccPhaseReset(p_phase);

          /* Reset Zerocross counter to continue*/
          p_phase->zero_cross_v.count = (uint32_t)0;
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
      if (sys_energy.energy.unit.act >= 0.0f)
      {
        sys_energy.energy.accumulator.act_imp_ws += sys_energy.energy.unit.act;
        if (sys_energy.energy.accumulator.act_imp_ws >= p_config->meter_constant)
        {
          sys_energy.energy.accumulator.act_imp_ws -= p_config->meter_constant;
          ++sys_energy.energy.counter.act_imp;

          /* Trigger Pulse*/
          sys_energy.impulse.active_counter = (uint32_t)0;
          sys_energy.impulse.active_on = true;
          LMA_IMP_ActiveOn();
        }

        sys_energy.energy.accumulator.app_imp_ws += sys_energy.energy.unit.app;
        if (sys_energy.energy.accumulator.app_imp_ws >= p_config->meter_constant)
        {
          sys_energy.energy.accumulator.app_imp_ws -= p_config->meter_constant;
          ++sys_energy.energy.counter.app_imp;

          /* Trigger Pulse*/
          sys_energy.impulse.apparent_counter = (uint32_t)0;
          sys_energy.impulse.apparent_on = true;
          LMA_IMP_ApparentOn();
        }

        if (sys_energy.energy.unit.react >= 0.0f)
        {
          /* QI - Active From Grid (Import) & Inductive From Grid (Import) - Apparent From Grid (Import)*/
          sys_energy.energy.accumulator.l_react_imp_ws += sys_energy.energy.unit.react;
          if (sys_energy.energy.accumulator.l_react_imp_ws >= p_config->meter_constant)
          {
            sys_energy.energy.accumulator.l_react_imp_ws -= p_config->meter_constant;
            ++sys_energy.energy.counter.l_react_imp;

            /* Trigger Pulse*/
            sys_energy.impulse.reactive_counter = (uint32_t)0;
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
            sys_energy.impulse.reactive_counter = (uint32_t)0;
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
          sys_energy.impulse.active_counter = (uint32_t)0;
          sys_energy.impulse.active_on = true;
          LMA_IMP_ActiveOn();
        }

        sys_energy.energy.accumulator.app_exp_ws += sys_energy.energy.unit.app;
        if (sys_energy.energy.accumulator.app_exp_ws >= p_config->meter_constant)
        {
          sys_energy.energy.accumulator.app_exp_ws -= p_config->meter_constant;
          ++sys_energy.energy.counter.app_exp;

          /* Trigger Pulse*/
          sys_energy.impulse.apparent_counter = (uint32_t)0;
          sys_energy.impulse.apparent_on = true;
          LMA_IMP_ActiveOn();
        }

        if (sys_energy.energy.unit.react >= 0.0f)
        {
          /* QII - Active To Grid (Export) & Capacitive From Grid (Import) - Apparent To Grid (Export)*/
          sys_energy.energy.accumulator.c_react_imp_ws += sys_energy.energy.unit.react;
          if (sys_energy.energy.accumulator.c_react_imp_ws >= p_config->meter_constant)
          {
            sys_energy.energy.accumulator.c_react_imp_ws -= p_config->meter_constant;
            ++sys_energy.energy.counter.c_react_imp;

            /* Trigger Pulse*/
            sys_energy.impulse.reactive_counter = (uint32_t)0;
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
            sys_energy.impulse.reactive_counter = (uint32_t)0;
            sys_energy.impulse.reactive_on = true;
            LMA_IMP_ReactiveOn();
          }
        }
      }
    }
  }
  else if (calib_fs.running)
  {
    ++calib_fs.adc_counter;
  }
  else
  {
    /* Do Nothing*/
  }
}

/** @details The TMR Callback computes and updates:
 *  1. Energy consumption in units (how much energy is consumed in Ws/VARs/VAs per ADC interval)
 *  2. Power
 *  3. Voltage
 *  4. Current
 *  5. Frequency
 *
 *  For each phase.
 */
void LMA_CB_TMR(void)
{
  LMA_CRITICAL_SECTION_PREPARE();
  LMA_Phase *p_phase = phase_list.p_first_phase;
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
      const float sample_count_fp = (float)p_phase->accs.snapshot.sample_count;
      p_phase->sigs.accumulators_ready = false;

      /* Frequency*/
      p_phase->measurements.fline = (p_config->gcalib.fs * (float)p_config->update_interval) / sample_count_fp;

      /* Check for valid frequency input*/
      if (p_phase->measurements.fline < p_config->fline_tol_high && p_phase->measurements.fline > p_config->fline_tol_low)
      {
        float power_divisor = sample_count_fp * p_phase->calib.p_coeff;
        const float vacc_fp = (float)((double)(p_phase->accs.snapshot.v_acc));
        const float iacc_fp = (float)((double)(p_phase->accs.snapshot.i_acc));
        const float pacc_fp = (float)((double)(p_phase->accs.snapshot.p_acc));
        const float qacc_fp = (float)((double)(p_phase->accs.snapshot.q_acc));

        /* Vrms*/
        p_phase->measurements.vrms = sqrtf(vacc_fp / sample_count_fp) / p_phase->calib.vrms_coeff;
        /* Irms*/
        p_phase->measurements.irms = sqrtf(iacc_fp / sample_count_fp) / p_phase->calib.irms_coeff;

        /* I, V & F Compensation - if applicable*/
        if (NULL != p_phase->p_computation_hook)
        {
          float comp = p_phase->p_computation_hook(&(p_phase->measurements.irms), &(p_phase->measurements.vrms),
                                                   &(p_phase->measurements.fline));
          /* Apply compensation to power divisor*/
          power_divisor /= comp;
        }

        /* Active Power (P)*/
        p_phase->measurements.p = pacc_fp / power_divisor;
        /* Reactive Power (Q)*/
        p_phase->measurements.q = qacc_fp / power_divisor;
        /* Apparent Power (S)*/
        p_phase->measurements.s = sqrtf(iacc_fp * vacc_fp) / power_divisor;

        /* Neutral Irms*/
        if (NULL != p_phase->p_neutral)
        {
          const float iacc_neutral_fp = (float)((double)(p_phase->p_neutral->accs.i_acc_snapshot));
          p_phase->measurements.irms_neutral = sqrtf(iacc_neutral_fp / sample_count_fp) / p_phase->p_neutral->calib.irms_coeff;
        }

        /* V SAG AND SWELL*/
        if (p_phase->measurements.vrms < p_config->v_sag)
        {
          p_phase->status |= LMA_VOLTAGE_SAG;
          p_phase->status &= ~LMA_VOLTAGE_SWELL;
        }
        else if (p_phase->measurements.vrms > p_config->v_swell)
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
        if (fabs(p_phase->measurements.p) < p_config->no_load_p)
        {
          p_phase->status |= LMA_NO_ACTIVE_LOAD;
          p_phase->measurements.p = 0.0f;
          p_phase->energy_units.act = 0.0f;
        }
        else
        {
          p_phase->status &= ~LMA_NO_ACTIVE_LOAD;
          p_phase->energy_units.act = p_phase->measurements.p / p_config->gcalib.fs;
        }

        /* Reactive Power (Q) & Energy*/
        if (fabs(p_phase->measurements.q) < p_config->no_load_p)
        {
          p_phase->status |= LMA_NO_REACTIVE_LOAD;
          p_phase->measurements.q = 0.0f;
          p_phase->energy_units.react = 0.0f;
        }
        else
        {
          p_phase->status &= ~LMA_NO_REACTIVE_LOAD;
          p_phase->energy_units.react = p_phase->measurements.q / p_config->gcalib.fs;
        }

        /* Apparent Power (S) & Energy*/
        if (fabs(p_phase->measurements.s) < p_config->no_load_p)
        {
          p_phase->status |= LMA_NO_APPARENT_LOAD;
          p_phase->measurements.s = 0.0f;
          p_phase->energy_units.app = 0.0f;
        }
        else
        {
          p_phase->status &= ~LMA_NO_APPARENT_LOAD;
          p_phase->energy_units.app = p_phase->measurements.s / p_config->gcalib.fs;
        }
      }
      else
      {
        /* Handle Invalid Frequency*/
        /* Vrms*/
        p_phase->measurements.vrms = 0.0f;
        /* Irms*/
        p_phase->measurements.irms = 0.0f;
        /* Frequency*/
        p_phase->measurements.fline = 0.0f;
        /* Active Power (P)*/
        p_phase->measurements.p = 0.0f;
        /* Reactive Power (Q)*/
        p_phase->measurements.q = 0.0f;
        /* Apparent Power (S)*/
        p_phase->measurements.s = 0.0f;

        /* Neutral Irms*/
        if (NULL != p_phase->p_neutral)
        {
          p_phase->measurements.irms_neutral = 0.0f;
        }

        /* Active Energy*/
        p_phase->energy_units.act = 0.0f;
        /* Reactive Energy*/
        p_phase->energy_units.react = 0.0f;
        /* Apparent Energy*/
        p_phase->energy_units.app = 0.0f;
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

/** @details The RTC isr calling this should ideally have nested interrupts enabled in which the ADC can interrupt us.
 * This callback allows us to calibrate sampling frequency.
 */
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
    if ((uint32_t)0 == calib_fs.rtc_counter)
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
