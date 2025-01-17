#include "LMA_Core.h"
#include <stdbool.h>
#include <string.h>

/** @brief internal calibration structure for smapling frequency - singleton*/
typedef struct LMA_CalibFs
{
  bool start;           /**< flag to indicate starting fs calibration routine */
  bool running;         /**< flag to indicate we are running */
  bool finished;        /**< flag to indicate calibration is finished */
  uint32_t rtc_counter; /**< counter to count rtc cycles for accumulation */
  LMA_Phase *p_phase;   /**< ointer to phase to work on */
} LMA_CalibFs;

/** @brief Check for zero cross on given phase.
 * @param[inout] p_phase - pointer to the phase block to update for zero cross.
 * @param[in] v_sample - copy of the current voltage sample to work on.
 * @return true if new zero cross detected - false otherwise.
 */
static bool Phase_zero_cross(LMA_Phase *const p_phase, const int32_t v_sample);

/** @brief Complete hard reset on a phase
 * @details Will reset the zero cross synch flag so we wait for the next full zero cross to be detected.
 * And resets all accumulators to zero.
 * @param[inout] p_phase - pointer to the phase block to reset
 */
static void Phase_hard_reset(LMA_Phase *const p_phase);

/** @brief Updates the energy accumulators for the phase.
 * @param[inout] p_phase - pointer to the phase block to update
 */
static void Phase_energy_accumulation(LMA_Phase *const p_phase);

static LMA_Config config;    /**< Internal copy of the meter configuration */
static LMA_CalibFs calib_fs; /**< Instance of the fs calibration data */

void LMA_Init(const LMA_Config *const p_config)
{
  memcpy(&config, p_config, sizeof(LMA_Config));

  LMA_ADC_Init();
  LMA_RTC_Init();
}

void LMA_PhaseInit(LMA_Phase *const p_phase)
{
  Phase_hard_reset(p_phase);
}

void LMA_Start(void)
{
  LMA_ADC_Start();
  LMA_RTC_Start();
}

void LMA_Stop(void)
{
  LMA_ADC_Stop();
  RTC_Stop();
}

void LMA_Calibrate(Calibration_args *const calib_args)
{
  uint32_t backup_update_interval = config.update_interval;
  LMA_CRITICAL_SECTION_PREPARE();

  if ((SAMPLING_FREQUENCY & calib_args->flags) != 0)
  {
    LMA_ADC_Stop();

    LMA_CRITICAL_SECTION_ENTER();

    calib_fs.finished = false;
    calib_fs.running = false;
    calib_fs.p_phase = calib_args->p_phase;
    Phase_hard_reset(calib_args->p_phase);
    calib_fs.rtc_counter = calib_args->rtc_cycles;
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
    calib_args->p_gcalib->fs = Param_div(PARAM_FROM_INT(calib_args->p_phase->voltage.fline_acc),
                                         Param_mul(calib_args->rtc_period, PARAM_FROM_INT(calib_args->rtc_cycles)));
    calib_args->p_gcalib->fline_coeff = Param_mul(calib_args->p_gcalib->fs, PARAM_FROM_INT(backup_update_interval));

    /* Restore operation*/
    config.update_interval = backup_update_interval;
    Phase_hard_reset(calib_args->p_phase);

    LMA_ADC_Start();
  }

  if ((PHASE_PAIR & calib_args->flags) != 0)
  {
    LMA_ADC_Stop();
    config.update_interval = calib_args->line_cycles;
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
    config.update_interval = backup_update_interval;
    Phase_hard_reset(calib_args->p_phase);

    LMA_ADC_Start();
  }

  if ((NEUTRAL_IRMS & calib_args->flags) != 0)
  {
    /* Currently UNSUPPORTED*/
  }
}

void LMA_EnergySet(LMA_Phase *const p_phase, LMA_Energy *const p_energy)
{
  memcpy(&(p_phase->energy), p_energy, sizeof(LMA_Energy));
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

void LMA_Measurements_Get(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements)
{
  p_measurements->vrms = p_phase->voltage.v_rms;
  p_measurements->irms = p_phase->current.i_rms;
  p_measurements->fline = p_phase->voltage.fline;
  p_measurements->p = p_phase->power.p;
  p_measurements->q = p_phase->power.q;
  p_measurements->s = p_phase->power.s;
}

void LMA_CB_ADC_Phase(LMA_Phase *const p_phase, const Samples *p_adc_samples)
{
  /* Zero cross - voltage*/
  Phase_zero_cross(p_phase, p_adc_samples->voltage);

  /* Handle active & apparent component once synched with zero cross and accumulation is enabled */
  if (p_phase->voltage.v_zc.v_sync_zc && !p_phase->disable_acc)
  {
    ++p_phase->voltage.v_sample_counter;

    /* VRMS - accumulation*/
    p_phase->voltage.v_acc = Accumulate_sample(p_phase->voltage.v_acc, p_adc_samples->voltage, p_adc_samples->voltage);
    /* IRMS - accumulation*/
    p_phase->current.i_acc = Accumulate_sample(p_phase->current.i_acc, p_adc_samples->current, p_adc_samples->current);
    /* Active Power (P) - accumulation*/
    p_phase->power.p_acc = Accumulate_sample(p_phase->power.p_acc, p_adc_samples->voltage, p_adc_samples->current);
    /* Reactive Power (Q) - accumulation*/
    p_phase->power.q_acc = Accumulate_sample(p_phase->power.q_acc, p_adc_samples->voltage90, p_adc_samples->current);

    /* Handle energy accumulation*/
    Phase_energy_accumulation(p_phase);

    /* Line Frequency */
    ++p_phase->voltage.fline_acc;

    /* If appropriate number of line cycles have passed - process results*/
    if (p_phase->voltage.v_zc.zero_cross_counter >= config.update_interval)
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

      if (!p_phase->calibrating_fs)
      {
        /* Line Frequency */
        p_phase->voltage.fline = Param_div(p_phase->p_gcalib->fline_coeff, PARAM_FROM_INT(p_phase->voltage.fline_acc));

        /* Reset*/
        p_phase->voltage.fline_acc = 0;
      }

      if (!p_phase->calibrating)
      {
        /* VRMS*/
        p_phase->voltage.v_rms = Param_div(PARAM_FROM_ACC(p_phase->voltage.v_acc), p_phase->calib.vrms_coeff);
        /* IRMS*/
        p_phase->current.i_rms = Param_div(PARAM_FROM_ACC(p_phase->current.i_acc), p_phase->calib.irms_coeff);
        /* Active Power (P)*/
        p_phase->power.p = Param_div(PARAM_FROM_ACC(p_phase->power.p_acc), p_phase->calib.p_coeff);
        /* Reactive Power (Q)*/
        p_phase->power.q = Param_div(PARAM_FROM_ACC(p_phase->power.q_acc), p_phase->calib.p_coeff);
        /* Apparent Power (S)*/
        p_phase->power.s = Param_mul(p_phase->voltage.v_rms, p_phase->current.i_rms);
        /* Active Energy*/
        p_phase->energy.unit.act = Param_div(p_phase->power.p, p_phase->p_gcalib->fs);
        p_phase->energy.unit.act = (p_phase->energy.unit.act >= 0) ? p_phase->energy.unit.act : -p_phase->energy.unit.act;
        /* Reactive Energy*/
        p_phase->energy.unit.react = Param_div(p_phase->power.q, p_phase->p_gcalib->fs);
        p_phase->energy.unit.react =
            (p_phase->energy.unit.react >= 0) ? p_phase->energy.unit.react : -p_phase->energy.unit.react;
        /* Apparent Energy*/
        p_phase->energy.unit.app = Param_div(p_phase->power.s, p_phase->p_gcalib->fs);
        p_phase->energy.unit.app = (p_phase->energy.unit.app >= 0) ? p_phase->energy.unit.app : -p_phase->energy.unit.app;

        /* Reset*/
        p_phase->voltage.v_acc = Accumulate_sample(0, p_adc_samples->voltage, p_adc_samples->voltage);
        p_phase->current.i_acc = Accumulate_sample(0, p_adc_samples->current, p_adc_samples->current);
        p_phase->power.p_acc = Accumulate_sample(0, p_adc_samples->voltage, p_adc_samples->current);
        p_phase->power.q_acc = Accumulate_sample(0, p_adc_samples->voltage90, p_adc_samples->current);

        /* Reset Parameters to continue*/
        p_phase->power.s_acc = 0;
        p_phase->voltage.v_sample_counter = 0;
        p_phase->voltage.v_zc.zero_cross_counter = 0;
      }
      else
      {
        /* Disable accumulation on active channel*/
        p_phase->disable_acc = true;
      }
    }
  }

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

void LMA_CB_RTC(void)
{
  /* If we are calibrating, synch the ADC sampling window to the RTC*/
  if (calib_fs.start)
  {
    calib_fs.start = false;
    calib_fs.running = true;
    calib_fs.finished = false;

    /* Signal to phase we are calibrating frequency so we don't want to loose the fline accumulator*/
    calib_fs.p_phase->calibrating_fs = true;

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

static bool Phase_zero_cross(LMA_Phase *const p_phase, const int32_t v_sample)
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

static void Phase_hard_reset(LMA_Phase *const p_phase)
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
  p_phase->power.s_acc = 0;

  p_phase->energy.accumulator.act_imp_ws = 0;
  p_phase->energy.accumulator.act_exp_ws = 0;
  p_phase->energy.accumulator.c_react_imp_ws = 0;
  p_phase->energy.accumulator.c_react_exp_ws = 0;
  p_phase->energy.accumulator.l_react_imp_ws = 0;
  p_phase->energy.accumulator.l_react_exp_ws = 0;

  p_phase->disable_acc = false;
  p_phase->calibrating = false;
  p_phase->calibrating_fs = false;
}

static void Phase_energy_accumulation(LMA_Phase *const p_phase)
{
  if (p_phase->power.p > 0 && p_phase->power.q <= 0)
  {
    /* QIV - Active From Grid (Import) & Capacitive To Grid (Export) - Apparent From Grid (Import)*/
    p_phase->energy.accumulator.act_imp_ws += p_phase->energy.unit.act;
    p_phase->energy.accumulator.c_react_exp_ws += p_phase->energy.unit.react;
    p_phase->energy.accumulator.app_imp_ws += p_phase->energy.unit.app;
  }
  else if (p_phase->power.p <= 0 && p_phase->power.q <= 0)
  {
    /* QIII - Active To Grid (Export) & Inductive To Grid (Export) - Apparent To Grid (Export)*/
    p_phase->energy.accumulator.act_exp_ws += p_phase->energy.unit.act;
    p_phase->energy.accumulator.l_react_exp_ws += p_phase->energy.unit.react;
    p_phase->energy.accumulator.app_exp_ws += p_phase->energy.unit.app;
  }
  else if (p_phase->power.p <= 0 && p_phase->power.q > 0)
  {
    /* QII - Active To Grid (Export) & Capacitive From Grid (Import) - Apparent To Grid (Export)*/
    p_phase->energy.accumulator.act_exp_ws += p_phase->energy.unit.act;
    p_phase->energy.accumulator.c_react_imp_ws += p_phase->energy.unit.react;
    p_phase->energy.accumulator.app_exp_ws += p_phase->energy.unit.app;
  }
  else if (p_phase->power.p > 0 && p_phase->power.q > 0)
  {
    /* QI - Active From Grid (Import) & Inductive To Grid (Import) - Apparent From Grid (Import)*/
    p_phase->energy.accumulator.act_imp_ws += p_phase->energy.unit.act;
    p_phase->energy.accumulator.l_react_imp_ws += p_phase->energy.unit.react;
    p_phase->energy.accumulator.app_imp_ws += p_phase->energy.unit.app;
  }
  else
  {
    /* Do Nothing - zero power*/
  }

  if (p_phase->energy.accumulator.act_imp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.act_imp_ws -= config.meter_constant;
    ++p_phase->energy.counter.act_imp;

    /* TODO: Trigger Pulse*/
  }

  if (p_phase->energy.accumulator.act_exp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.act_exp_ws -= config.meter_constant;
    ++p_phase->energy.counter.act_exp;

    /* TODO: Trigger Pulse*/
  }

  if (p_phase->energy.accumulator.l_react_imp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.l_react_imp_ws -= config.meter_constant;
    ++p_phase->energy.counter.l_react_imp;

    /* TODO: Trigger Pulse*/
  }

  if (p_phase->energy.accumulator.l_react_exp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.l_react_exp_ws -= config.meter_constant;
    ++p_phase->energy.counter.l_react_exp;

    /* TODO: Trigger Pulse*/
  }

  if (p_phase->energy.accumulator.c_react_imp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.c_react_imp_ws -= config.meter_constant;
    ++p_phase->energy.counter.c_react_imp;

    /* TODO: Trigger Pulse*/
  }

  if (p_phase->energy.accumulator.c_react_exp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.c_react_exp_ws -= config.meter_constant;
    ++p_phase->energy.counter.c_react_exp;

    /* TODO: Trigger Pulse*/
  }

  if (p_phase->energy.accumulator.app_imp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.app_imp_ws -= config.meter_constant;
    ++p_phase->energy.counter.app_imp;

    /* TODO: Trigger Pulse*/
  }

  if (p_phase->energy.accumulator.app_exp_ws >= config.meter_constant)
  {
    p_phase->energy.accumulator.app_exp_ws -= config.meter_constant;
    ++p_phase->energy.counter.app_exp;

    /* TODO: Trigger Pulse*/
  }
}
