#include "LMA_Core.h"
#include "LMA_Utils/LMA_Types.h"
#include <stdbool.h>
#include <string.h>

/* Locally Used Types*/
/** @brief internal calibration structure for smapling frequency - singleton*/
typedef struct LMA_CalibFs
{
  bool start;           /**< flag to indicate starting fs calibration routine */
  bool running;         /**< flag to indicate we are running */
  bool finished;        /**< flag to indicate calibration is finished */
  uint32_t rtc_counter; /**< counter to count rtc cycles for accumulation */
  uint32_t adc_counter; /**< counter to count number of ADC cycles have acumulated. */
  LMA_Phase *p_phase;   /**< ointer to phase to work on */
} LMA_CalibFs;

/* Static/Local Variable Declarations*/
static LMA_Config *p_config = NULL;    /**< Internal copy of the meter configuration */
static LMA_Phase *p_phase_list = NULL; /**< Linked list of phases*/
static LMA_CalibFs calib_fs;           /**< Instance of the fs calibration data */
static LMA_SystemEnergy sys_energy = {.energy.unit.act = 0,
                                      .energy.unit.react = 0,
                                      .energy.unit.app = 0,

                                      .energy.accumulator.act_imp_ws = 0,
                                      .energy.accumulator.act_exp_ws = 0,
                                      .energy.accumulator.c_react_imp_ws = 0,
                                      .energy.accumulator.c_react_exp_ws = 0,
                                      .energy.accumulator.l_react_imp_ws = 0,
                                      .energy.accumulator.l_react_exp_ws = 0,
                                      .energy.accumulator.app_imp_ws = 0,
                                      .energy.accumulator.app_exp_ws = 0,

                                      .energy.counter.act_imp = 0,
                                      .energy.counter.act_exp = 0,
                                      .energy.counter.c_react_imp = 0,
                                      .energy.counter.c_react_exp = 0,
                                      .energy.counter.l_react_imp = 0,
                                      .energy.counter.l_react_exp = 0,
                                      .energy.counter.app_imp = 0,
                                      .energy.counter.app_exp = 0,

                                      .impulse.led_on_count = 0,
                                      .impulse.active_counter = 0,
                                      .impulse.reactive_counter = 0,
                                      .impulse.apparent_counter = 0,
                                      .impulse.active_on = false,
                                      .impulse.reactive_on = false,
                                      .impulse.apparent_on = false};

/* Static/Local functions*/

/** @brief Check for zero cross on given phase.
 * @param[inout] p_phase - pointer to the phase block to update for zero cross.
 * @param[in] v_sample - copy of the current voltage sample to work on.
 * @return true if new zero cross detected - false otherwise.
 */
static inline bool Phase_zero_cross(LMA_Phase *const p_phase, const int32_t v_sample)
{
  if ((!p_phase->voltage.v_zc.zero_cross_debounce) && (p_phase->voltage.v_zc.last_voltage_sample <= 0) && (v_sample > 0))
  {
    p_phase->voltage.v_zc.zero_cross_debounce = true;

    if (p_phase->voltage.v_zc.v_sync_zc)
    {
      /* Only increment the counter if we are synch'd (already detected our first zero cross)*/
      ++p_phase->voltage.v_zc.zero_cross_counter;
    }
    else
    {
      /* Otherwise synch us up and reset the counter*/
      p_phase->voltage.v_zc.zero_cross_counter = 0;
      p_phase->voltage.v_zc.v_sync_zc = true;
    }
  }
  else
  {
    p_phase->voltage.v_zc.zero_cross_debounce = false;
  }

  p_phase->voltage.v_zc.last_voltage_sample = v_sample;

  return p_phase->voltage.v_zc.zero_cross_debounce;
}
/* END OF FUNCTION*/

/** @brief Complete hard reset on a phase
 * @details Will reset the zero cross synch flag so we wait for the next full zero cross to be detected.
 * And resets all accumulators to zero.
 * @param[inout] p_phase - pointer to the phase block to reset
 */
static inline void Phase_hard_reset(LMA_Phase *const p_phase)
{
  p_phase->voltage.v_sample_counter = 0;
  p_phase->voltage.v_zc.zero_cross_counter = 0;
  p_phase->voltage.v_zc.last_voltage_sample = 0;
  p_phase->voltage.v_zc.zero_cross_debounce = false;
  p_phase->voltage.v_zc.v_sync_zc = false;

  p_phase->voltage.v_acc = 0;
  p_phase->voltage.fline_acc = 0;
  p_phase->current.i_acc = 0;
  p_phase->power.p_acc = 0;
  p_phase->power.q_acc = 0;

  p_phase->energy.act_unit = 0;
  p_phase->energy.react_unit = 0;
  p_phase->energy.app_unit = 0;

  p_phase->disable_acc = false;
  p_phase->calibrating = false;
}
/* END OF FUNCTION*/

/** @brief Processes computations for a phase
 * @param[inout] p_phase - pointer to the phase block to work on
 * @param[in] p_samples - pointer to the samples to work on.
 */
static inline void Phase_process(LMA_Phase *const p_phase, const LMA_Samples *p_samples)
{
  /* Zero cross - voltage*/
  Phase_zero_cross(p_phase, p_samples->voltage);

  /* Handle active & apparent component once synched with zero cross and accumulation is enabled */
  if (p_phase->voltage.v_zc.v_sync_zc && !p_phase->disable_acc)
  {
    ++p_phase->voltage.v_sample_counter;

    /* VRMS - accumulation*/
    p_phase->voltage.v_acc = Accumulate_sample(p_phase->voltage.v_acc, p_samples->voltage, p_samples->voltage);
    /* IRMS - accumulation*/
    p_phase->current.i_acc = Accumulate_sample(p_phase->current.i_acc, p_samples->current, p_samples->current);
    /* Active Power (P) - accumulation*/
    p_phase->power.p_acc = Accumulate_sample(p_phase->power.p_acc, p_samples->voltage, p_samples->current);
    /* Reactive Power (Q) - accumulation*/
    p_phase->power.q_acc = Accumulate_sample(p_phase->power.q_acc, p_samples->voltage90, p_samples->current);

    /* Line Frequency */
    ++p_phase->voltage.fline_acc;

    /* If appropriate number of line cycles have passed - process results*/
    if (p_phase->voltage.v_zc.zero_cross_counter >= p_config->update_interval)
    {
      /* VRMS*/
      p_phase->voltage.v_acc /= p_phase->voltage.v_sample_counter;
      p_phase->voltage.v_acc = Acc_sqrt(p_phase->voltage.v_acc);
      /* IRMS*/
      p_phase->current.i_acc /= p_phase->voltage.v_sample_counter;
      p_phase->current.i_acc = Acc_sqrt(p_phase->current.i_acc);
      /* Active Power (P)*/
      p_phase->power.p_acc /= p_phase->voltage.v_sample_counter;
      /* Reactive Power (Q)*/
      p_phase->power.q_acc /= p_phase->voltage.v_sample_counter;
      /* Line Frequency */
      p_phase->voltage.fline = Param_div(p_config->gcalib.fline_coeff, PARAM_FROM_INT(p_phase->voltage.fline_acc));

      if (!p_phase->calibrating)
      {
        /* VRMS*/
        p_phase->voltage.v_rms = Param_div(PARAM_FROM_ACC(p_phase->voltage.v_acc), p_phase->calib.vrms_coeff);
        /* IRMS*/
        p_phase->current.i_rms = Param_div(PARAM_FROM_ACC(p_phase->current.i_acc), p_phase->calib.irms_coeff);

        if (p_phase->current.i_rms < p_config->no_load_i)
        {
          p_phase->status |= LMA_NO_LOAD;
          /* Active Power (P)*/
          p_phase->power.p = 0;
          /* Reactive Power (Q)*/
          p_phase->power.q = 0;
          /* Apparent Power (S)*/
          p_phase->power.s = 0;
          /* Active Energy*/
          p_phase->energy.act_unit = 0;
          /* Reactive Energy*/
          p_phase->energy.react_unit = 0;
          /* Apparent Energy*/
          p_phase->energy.app_unit = 0;
        }
        else
        {
          p_phase->status &= ~LMA_NO_LOAD;
          /* Active Power (P)*/
          p_phase->power.p = Param_div(PARAM_FROM_ACC(p_phase->power.p_acc), p_phase->calib.p_coeff);
          /* Reactive Power (Q)*/
          p_phase->power.q = Param_div(PARAM_FROM_ACC(p_phase->power.q_acc), p_phase->calib.p_coeff);
          /* Apparent Power (S)*/
          p_phase->power.s = Param_mul(p_phase->voltage.v_rms, p_phase->current.i_rms);

          /* Active Power (P) & Energy*/
          if (p_phase->power.p < p_config->no_load_p)
          {
            p_phase->status |= LMA_NO_ACTIVE_LOAD;
            p_phase->power.p = 0;
            p_phase->energy.act_unit = 0;
          }
          else
          {
            p_phase->status &= ~LMA_NO_ACTIVE_LOAD;
            p_phase->energy.act_unit = Param_div(p_phase->power.p, p_config->gcalib.fs);
          }

          /* Reactive Power (Q) & Energy*/
          if (p_phase->power.q < p_config->no_load_p)
          {
            p_phase->status |= LMA_NO_REACTIVE_LOAD;
            p_phase->power.q = 0;
            p_phase->energy.react_unit = 0;
          }
          else
          {
            p_phase->status &= ~LMA_NO_REACTIVE_LOAD;
            p_phase->energy.react_unit = Param_div(p_phase->power.q, p_config->gcalib.fs);
          }

          /* Apparent Power (S) & Energy*/
          if(p_phase->power.p < p_config->no_load_p && p_phase->power.q < p_config->no_load_p)
          {
            p_phase->power.s = 0;
            p_phase->energy.app_unit = 0;
          }
          else 
          {
            p_phase->energy.app_unit = Param_div(p_phase->power.s, p_config->gcalib.fs);
          }
        }

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

        /* Reset*/
        p_phase->voltage.v_acc = Accumulate_sample(0, p_samples->voltage, p_samples->voltage);
        p_phase->current.i_acc = Accumulate_sample(0, p_samples->current, p_samples->current);
        p_phase->power.p_acc = Accumulate_sample(0, p_samples->voltage, p_samples->current);
        p_phase->power.q_acc = Accumulate_sample(0, p_samples->voltage90, p_samples->current);

        /* Reset Parameters to continue*/
        p_phase->voltage.v_sample_counter = 0;
        p_phase->voltage.v_zc.zero_cross_counter = 0;
        p_phase->voltage.fline_acc = 0;
      }
      else
      {
        /* Disable accumulation on active channel*/
        p_phase->disable_acc = true;
      }
    }
  }
}
/* END OF FUNCTION*/

/** @brief Performs necessary energy computations between system energy and phase list*/
static inline void Energy_process(void)
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

  /* Active LED Management*/
  if (sys_energy.impulse.apparent_on)
  {
    ++sys_energy.impulse.apparent_counter;
    if (sys_energy.impulse.apparent_counter > sys_energy.impulse.led_on_count)
    {
      sys_energy.impulse.apparent_on = false;
      LMA_IMP_ApparentOff();
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
      sys_energy.energy.accumulator.c_react_exp_ws += sys_energy.energy.unit.react;
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
    sys_energy.energy.accumulator.act_exp_ws += sys_energy.energy.unit.act;
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
      LMA_IMP_ApparentOn();
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
      sys_energy.energy.accumulator.l_react_exp_ws += sys_energy.energy.unit.react;
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
/* END OF FUNCTION*/

/* Externally Available Functions*/

void LMA_Init(LMA_Config *const p_config_arg)
{
  p_config = p_config_arg;

  LMA_ADC_Init();
  LMA_RTC_Init();
}

void LMA_PhaseRegister(LMA_Phase *const p_phase)
{
  Phase_hard_reset(p_phase);

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
}

void LMA_PhaseLoadCalibration(LMA_Phase *const p_phase, const LMA_PhaseCalibration *const p_calib)
{
  memcpy(&(p_phase->calib), p_calib, sizeof(LMA_PhaseCalibration));
}

void LMA_Start(void)
{
  LMA_ADC_Start();
  LMA_RTC_Start();
}

void LMA_Stop(void)
{
  LMA_ADC_Stop();
  LMA_RTC_Stop();
}

void LMA_PhaseCalibrate(LMA_PhaseCalibArgs *const calib_args)
{
  uint32_t backup_update_interval = p_config->update_interval;

  LMA_ADC_Stop();
  p_config->update_interval = calib_args->line_cycles;
  Phase_hard_reset(calib_args->p_phase);
  calib_args->p_phase->calibrating = true;

  LMA_ADC_Start();

  while (!calib_args->p_phase->disable_acc)
  {
    /* Wait until the accumulation has stopped*/
  }

  LMA_ADC_Stop();

  /* Update Coefficients*/
  calib_args->p_phase->calib.vrms_coeff = Param_div(PARAM_FROM_ACC(calib_args->p_phase->voltage.v_acc), calib_args->vrms_tgt);
  calib_args->p_phase->calib.irms_coeff = Param_div(PARAM_FROM_ACC(calib_args->p_phase->current.i_acc), calib_args->irms_tgt);
  calib_args->p_phase->calib.p_coeff = Param_div(PARAM_FROM_ACC(calib_args->p_phase->power.p_acc), calib_args->p_tgt);

  /* Restore operation*/
  p_config->update_interval = backup_update_interval;
  Phase_hard_reset(calib_args->p_phase);

  LMA_ADC_Start();

  /* pseudo code

void Phase_angle_compute(void)
{
        static pa_state state = WAIT_FOR_CURRENT_ZERO_CROSS;

        static const param_t degrees_per_sample = (360.00 * fline_tgt) / fs = (360.00 * 50.00) / 3906.25 = 4.608 [deg/sample

        if(WAIT_FOR_CURRENT_ZERO_CROSS == state)
        {
                izc = sample [i] > 0 && sample [i-1] <= 0
                if(izc)
                {
                        i_fraction = sample [i] / (sample [i] - sample [i-1])
                        state = WAIT_FOR_VOLTAGE_ZERO_CROSS
                }
        }

        if(WAIT_FOR_VOLTAGE_ZERO_CROSS == state)
        {
                vzc = sample [v] > 0 && sample [v-1] <= 0
                if(vzc)
                {
                        v_fraction = sample [v] / (sample [v] - sample [v-1])
                        state = COMPUTE
                }
                else
                {
                        ++v_integer;
                }
        }

        if(COMPUTE == state)
        {
                sample_diff = (v_integer + v_fraction) - i_fraction; // might be (1-i_fraction) - can't be arsed just trial and
error the logic. ps_acc += sample_diff * degrees_per_sample;
                ++line_cycle;
                if(line_cycle == enough)
                {
                        state = FINALISE
                }
                else
                {
                        state = WAIT_FOR_CURRENT_ZERO_CROSS;
                }
        }

        if(FINALISE == state)
        {
                ps = ps_acc / line_cycle;

                if(ps approx 180.00)
                        V and I switched (or neutral sampling).
                else if(ps approx 90 || ps approx 270)
                        Reactive load set - must be UPF.
                else
                        maybe some more validity checks

                state = DONE;
        }
}

  */
}

void LMA_GlobalCalibrate(LMA_GlobalCalibArgs *const calib_args)
{
  LMA_CRITICAL_SECTION_PREPARE();

  LMA_ADC_Stop();

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

  /* Update Coefficients*/
  p_config->gcalib.fs = Param_div(PARAM_FROM_INT(calib_fs.adc_counter),
                                  Param_mul(calib_args->rtc_period, PARAM_FROM_INT(calib_args->rtc_cycles)));
  p_config->gcalib.fline_coeff = Param_mul(p_config->gcalib.fs, PARAM_FROM_INT(p_config->update_interval));

  LMA_ADC_Start();
}

void LMA_EnergySet(LMA_SystemEnergy *const p_energy)
{
  memcpy(&sys_energy, p_energy, sizeof(LMA_SystemEnergy));
}

void LMA_EnergyGet(LMA_SystemEnergy *const p_energy)
{
  memcpy(p_energy, &sys_energy, sizeof(LMA_SystemEnergy));
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

param_t LMA_VRMS_Get(const LMA_Phase *const p_phase)
{
  return p_phase->voltage.v_rms;
}

param_t LMA_IRMS_Get(const LMA_Phase *const p_phase)
{
  return p_phase->current.i_rms;
}

param_t LMA_FLine_Get(const LMA_Phase *const p_phase)
{
  return p_phase->voltage.fline;
}

param_t LMA_ActivePower_Get(const LMA_Phase *const p_phase)
{
  return p_phase->power.p;
}

param_t LMA_ReactivePower_Get(const LMA_Phase *const p_phase)
{
  return p_phase->power.q;
}

param_t LMA_ApparentPower_Get(const LMA_Phase *const p_phase)
{
  return p_phase->power.s;
}

void LMA_MeasurementsGet(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements)
{
  p_measurements->vrms = p_phase->voltage.v_rms;
  p_measurements->irms = p_phase->current.i_rms;
  p_measurements->fline = p_phase->voltage.fline;
  p_measurements->p = p_phase->power.p;
  p_measurements->q = p_phase->power.q;
  p_measurements->s = p_phase->power.s;
}

void LMA_CB_ADC_SinglePhase(const LMA_Samples *p_samples)
{
  /* If we are running fs calibration - increment counter*/
  if (calib_fs.running)
  {
    ++calib_fs.adc_counter;
  }
  else
  {
    Phase_process(p_phase_list, p_samples);

    sys_energy.energy.unit.act = p_phase_list->energy.act_unit;
    sys_energy.energy.unit.react = p_phase_list->energy.react_unit;
    sys_energy.energy.unit.app = p_phase_list->energy.app_unit;

    Energy_process();
  }
}

void LMA_CB_ADC_PolyPhase(const LMA_Samples *p_sample_list_first, const LMA_Samples *p_sample_list_last)
{
  LMA_Phase *p_phase = p_phase_list;
  const LMA_Samples *p_samples = p_sample_list_first;

  /* If we are running fs calibration - increment counter*/
  if (calib_fs.running)
  {
    ++calib_fs.adc_counter;
  }
  else
  {
    sys_energy.energy.unit.act = 0;
    sys_energy.energy.unit.react = 0;
    sys_energy.energy.unit.app = 0;

    while (NULL != p_phase && p_samples <= p_sample_list_last)
    {
      Phase_process(p_phase, p_samples);

      sys_energy.energy.unit.act += p_phase->energy.act_unit;
      sys_energy.energy.unit.react += p_phase->energy.react_unit;
      sys_energy.energy.unit.app += p_phase->energy.app_unit;

      p_phase = p_phase->p_next;
      ++p_samples;
    }

    Energy_process();
  }
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
