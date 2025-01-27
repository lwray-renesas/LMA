#include "LMA_Core.h"
#include "LMA_Port.h"
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
static LMA_SystemEnergy sys_energy = {
                                      .energy.unit.act = 0,
                                      .energy.unit.react = 0,
                                      .energy.accumulator.act_imp_ws = 0,
                                      .energy.accumulator.act_exp_ws = 0,
                                      .energy.accumulator.app_imp_ws = 0,
                                      .energy.accumulator.app_exp_ws = 0,
                                      .energy.accumulator.c_react_imp_ws = 0,
                                      .energy.accumulator.c_react_exp_ws = 0,
                                      .energy.accumulator.l_react_imp_ws = 0,
                                      .energy.accumulator.l_react_exp_ws = 0,
                                      .energy.counter.act_imp = 0,
                                      .energy.counter.act_exp = 0,
                                      .energy.counter.app_imp = 0,
                                      .energy.counter.app_exp = 0,
                                      .energy.counter.c_react_imp = 0,
                                      .energy.counter.c_react_exp = 0,
                                      .energy.counter.l_react_imp = 0,
                                      .energy.counter.l_react_exp = 0,
                                      .impulse.led_on_count = 0,
                                      .impulse.active_counter = 0,
                                      .impulse.apparent_counter = 0,
                                      .impulse.reactive_counter = 0,
                                      .impulse.active_on = false,
                                      .impulse.apparent_on = false,
                                      .impulse.reactive_on = false,
};

/* Static/Local functions*/

/** @brief Check for zero cross on given phase.
 * @param[inout] p_phase - pointer to the phase block to update for zero cross.
 * @return true if new zero cross detected - false otherwise.
 */
static inline bool Phase_zero_cross(LMA_Phase *const p_phase)
{
    if ((!p_phase->voltage.v_zc.zero_cross_debounce) && (p_phase->voltage.v_zc.last_voltage_sample <= 0) &&
            (p_phase->samples.voltage > 0))
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

    p_phase->voltage.v_zc.last_voltage_sample = p_phase->samples.voltage;

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
    p_phase->power.s_acc.upper = 0;
    p_phase->power.s_acc.lower = 0;

    p_phase->energy.act_unit = 0;
    p_phase->energy.react_unit = 0;
    p_phase->energy.app_unit = 0;

    p_phase->samples.current = 0;
    p_phase->samples.voltage = 0;
    p_phase->samples.voltage90 = 0;

    LMA_AccReset(&(p_phase->ws));

    p_phase->disable_acc = false;
    p_phase->calibrating = false;
}
/* END OF FUNCTION*/

/** @brief Processes computations for a phase
 * @param[inout] p_phase - pointer to the phase block to work on
 */
static inline void Phase_process(LMA_Phase *const p_phase)
{
    /* Zero cross - voltage*/
    Phase_zero_cross(p_phase);

    /* Handle active & apparent component once synched with zero cross and accumulation is enabled */
    if (p_phase->voltage.v_zc.v_sync_zc && !p_phase->disable_acc)
    {
        ++p_phase->voltage.v_sample_counter;

        LMA_AccRun(&(p_phase->ws));

        /* Line Frequency */
        ++p_phase->voltage.fline_acc;

        /* If appropriate number of line cycles have passed - process results*/
        if (p_phase->voltage.v_zc.zero_cross_counter >= p_config->update_interval)
        {
            /* FP Sample Counter*/
            p_phase->voltage.v_sample_counter_fp = LMA_AccToFloat(p_phase->voltage.v_sample_counter);

            if (!p_phase->calibrating)
            {
                if(p_phase->voltage.fline_acc < p_config->gcalib.fline_acc_tol_high && p_phase->voltage.fline_acc > p_config->gcalib.fline_acc_tol_low)
                {
                    const float power_divisor = LMA_FPMul_Fast(p_phase->voltage.v_sample_counter_fp, p_phase->calib.p_coeff);
                    /* Update local accumulators*/
                    LMA_AccGet(&(p_phase->ws));
                    p_phase->voltage.fline_latch = p_phase->voltage.fline_acc;

                    /* Active Power (P)*/
                    p_phase->power.p = LMA_FPDiv_Fast(LMA_AccToFloat(p_phase->power.p_acc), power_divisor);

                    /* Reactive Power (Q)*/
                    p_phase->power.q = LMA_FPDiv_Fast(LMA_AccToFloat(p_phase->power.q_acc), power_divisor);

                    /* Apparent Power (S)*/
                    p_phase->power.s = LMA_FPDiv_Fast(LMA_FPSqrt_Fast(LMA_FPMul_Fast(LMA_AccToFloat(p_phase->current.i_acc), LMA_AccToFloat(p_phase->voltage.v_acc))),
                                                      power_divisor);

                    /* Active Power (P) & Energy*/
                    if (LMA_FPAbs_Fast(p_phase->power.p) < p_config->no_load_p)
                    {
                        p_phase->status |= LMA_NO_ACTIVE_LOAD;
                        p_phase->power.p = 0;
                        p_phase->energy.act_unit = 0;
                    }
                    else
                    {
                        p_phase->status &= ~LMA_NO_ACTIVE_LOAD;
                        p_phase->energy.act_unit = LMA_FPDiv_Fast(p_phase->power.p, p_config->gcalib.fs);
                    }

                    /* Reactive Power (Q) & Energy*/
                    if (LMA_FPAbs_Fast(p_phase->power.q) < p_config->no_load_p)
                    {
                        p_phase->status |= LMA_NO_REACTIVE_LOAD;
                        p_phase->power.q = 0;
                        p_phase->energy.react_unit = 0;
                    }
                    else
                    {
                        p_phase->status &= ~LMA_NO_REACTIVE_LOAD;
                        p_phase->energy.react_unit = LMA_FPDiv_Fast(p_phase->power.q, p_config->gcalib.fs);
                    }

                    /* Apparent Power (S) & Energy*/
                    if (LMA_FPAbs_Fast(p_phase->power.s) < p_config->no_load_p)
                    {
                        p_phase->status |= LMA_NO_APPARENT_LOAD;
                        p_phase->power.s = 0;
                        p_phase->energy.app_unit = 0;
                    }
                    else
                    {
                        p_phase->status &= ~LMA_NO_APPARENT_LOAD;
                        p_phase->energy.app_unit = LMA_FPDiv_Fast(p_phase->power.s, p_config->gcalib.fs);
                    }

                    /* Reset*/
                    LMA_AccReset(&(p_phase->ws));

                    /* Reset Parameters to continue*/
                    p_phase->voltage.v_sample_counter = 0;
                    p_phase->voltage.v_zc.zero_cross_counter = 0;
                    p_phase->voltage.fline_acc = 0;
                }
                else
                {
                    /* Reset*/
                    LMA_AccReset(&(p_phase->ws));

                    /* Reset Parameters to continue*/
                    p_phase->voltage.v_sample_counter = 0;
                    p_phase->voltage.v_zc.zero_cross_counter = 0;
                    p_phase->voltage.fline_acc = 0;

                    /* Set params to zero on invalid frequency*/
                    p_phase->power.p = 0.0f;
                    p_phase->power.q = 0.0f;
                    p_phase->power.s = 0.0f;
                    p_phase->voltage.fline_acc = 0;
                    p_phase->voltage.fline_latch = 0;
                    p_phase->voltage.v_acc = 0;
                    p_phase->current.i_acc = 0;
                }
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
/* END OF FUNCTION*/

/* Externally Available Functions*/

void LMA_Init(LMA_Config *const p_config_arg)
{
    p_config = p_config_arg;
    LMA_IMP_ActiveOff();
    LMA_IMP_ApparentOff();
    LMA_IMP_ReactiveOff();
    LMA_ADC_Init();
    LMA_RTC_Init();
}

void LMA_PhaseRegister(LMA_Phase *const p_phase)
{
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

    p_phase->ws.p_samples = &(p_phase->samples);
    p_phase->ws.p_vacc = &(p_phase->voltage.v_acc);
    p_phase->ws.p_iacc = &(p_phase->current.i_acc);
    p_phase->ws.p_pacc = &(p_phase->power.p_acc);
    p_phase->ws.p_qacc = &(p_phase->power.q_acc);

    Phase_hard_reset(p_phase);
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

    LMA_AccGet(&(calib_args->p_phase->ws));

    /* Update Coefficients*/
    calib_args->p_phase->calib.vrms_coeff =
            LMA_FPSqrt_Fast((float)calib_args->p_phase->voltage.v_acc / calib_args->p_phase->voltage.v_sample_counter_fp) /
            calib_args->vrms_tgt;
    calib_args->p_phase->calib.irms_coeff =
            LMA_FPSqrt_Fast((float)calib_args->p_phase->current.i_acc / calib_args->p_phase->voltage.v_sample_counter_fp) /
            calib_args->irms_tgt;
    calib_args->p_phase->calib.p_coeff = calib_args->p_phase->calib.vrms_coeff * calib_args->p_phase->calib.irms_coeff;

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
    float fline_typ_count = 0.0f;
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

    /* Care more about accuracy here than speed*/
    p_config->gcalib.fs = (float)calib_fs.adc_counter / ((float)calib_args->rtc_period * (float)calib_args->rtc_cycles);
    p_config->gcalib.deltat = 1.00f / p_config->gcalib.fs;
    p_config->gcalib.fline_coeff = p_config->gcalib.fs * (float)p_config->update_interval;

    /* TODO: Needs to be done at init, so we can check the line frequency for validity in this routine*/
    fline_typ_count = (p_config->gcalib.fs / p_config->target_system_frequency) * (float)p_config->update_interval;
    p_config->gcalib.fline_acc_tol_high = (uint32_t) (fline_typ_count + (fline_typ_count / 2.00f));
    p_config->gcalib.fline_acc_tol_low = (uint32_t) (fline_typ_count / 2.00f);

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

float LMA_VrmsGet(LMA_Phase *const p_phase)
{
    acc_t v_acc_tmp;
    float sc_tmp;
    LMA_CRITICAL_SECTION_PREPARE();

    LMA_CRITICAL_SECTION_ENTER();
    v_acc_tmp = p_phase->voltage.v_acc;
    sc_tmp = p_phase->voltage.v_sample_counter_fp;
    LMA_CRITICAL_SECTION_EXIT();

    p_phase->voltage.v_rms = (0 == v_acc_tmp) ? 0.0f :
            LMA_FPDiv_Fast(LMA_FPSqrt_Fast(LMA_FPDiv_Fast(LMA_AccToFloat(v_acc_tmp), sc_tmp)), p_phase->calib.vrms_coeff);

    return p_phase->voltage.v_rms;
}

float LMA_IrmsGet(LMA_Phase *const p_phase)
{
    acc_t i_acc_tmp;
    float sc_tmp;
    LMA_CRITICAL_SECTION_PREPARE();

    LMA_CRITICAL_SECTION_ENTER();
    i_acc_tmp = p_phase->current.i_acc;
    sc_tmp = p_phase->voltage.v_sample_counter_fp;
    LMA_CRITICAL_SECTION_EXIT();

    p_phase->current.i_rms = (0 == i_acc_tmp) ? 0.0f :
            LMA_FPDiv_Fast(LMA_FPSqrt_Fast(LMA_FPDiv_Fast(LMA_AccToFloat(i_acc_tmp), sc_tmp)), p_phase->calib.irms_coeff);

    return p_phase->current.i_rms;
}

float LMA_FLineGet(LMA_Phase *const p_phase)
{
    acc_t fline_acc_tmp;
    LMA_CRITICAL_SECTION_PREPARE();

    LMA_CRITICAL_SECTION_ENTER();
    fline_acc_tmp = (acc_t)p_phase->voltage.fline_latch;
    LMA_CRITICAL_SECTION_EXIT();

    p_phase->voltage.fline = (0 == fline_acc_tmp) ? 0.0f :
            LMA_FPDiv_Fast(p_config->gcalib.fline_coeff, LMA_AccToFloat(fline_acc_tmp));

    return p_phase->voltage.fline;
}

float LMA_ActivePowerGet(const LMA_Phase *const p_phase)
{
    return p_phase->power.p;
}

float LMA_ReactivePowerGet(const LMA_Phase *const p_phase)
{
    return p_phase->power.q;
}

float LMA_ApparentPowerGet(const LMA_Phase *const p_phase)
{
    return p_phase->power.s;
}

void LMA_MeasurementsGet(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements)
{
    p_measurements->vrms = LMA_VrmsGet(p_phase);
    p_measurements->irms = LMA_IrmsGet(p_phase);
    p_measurements->fline = LMA_FLineGet(p_phase);
    p_measurements->p = LMA_ActivePowerGet(p_phase);
    p_measurements->q = LMA_ReactivePowerGet(p_phase);
    p_measurements->s = LMA_ApparentPowerGet(p_phase);
}

void LMA_CB_ADC_SinglePhase(void)
{
    /* If we are running fs calibration - increment counter*/
    if (calib_fs.running)
    {
        ++calib_fs.adc_counter;
    }
    else
    {
        Phase_process(p_phase_list);

        sys_energy.energy.unit.act = p_phase_list->energy.act_unit;
        sys_energy.energy.unit.react = p_phase_list->energy.react_unit;

        Energy_process();
    }
}

void LMA_CB_ADC_PolyPhase(void)
{
    LMA_Phase *p_phase = p_phase_list;

    /* If we are running fs calibration - increment counter*/
    if (calib_fs.running)
    {
        ++calib_fs.adc_counter;
    }
    else
    {
        sys_energy.energy.unit.act = 0;
        sys_energy.energy.unit.react = 0;

        while (NULL != p_phase)
        {
            Phase_process(p_phase);

            sys_energy.energy.unit.act += p_phase->energy.act_unit;
            sys_energy.energy.unit.react += p_phase->energy.react_unit;

            p_phase = p_phase->p_next;
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
