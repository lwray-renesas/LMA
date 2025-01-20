#include "simulation.h"
#include "LMA_Core.h"
#include "LMA_Types.h"
#include <stdio.h>
#include <threads.h>

/** @brief thread to emulate drivers running asynch to main thread
 * @param[inout] pointer to args for thread - passing simulation_params for controlling the sim
 */
extern int Driver_thread(void *p_arg);

extern bool driver_thread_running;

/** Drvier thread object */
static thrd_t driver_thread;

/* global pointer for the drvier thread to access sim params*/
const simulation_params *p_g_sim_params;

/* Configuration required for configuring the library 4500 ws/imp = 800 imp/kwh*/
#if (FIXED_POINT_SUPPORT == 0U)
static LMA_Config config = {.gcalib = {.fs = 3906.25f, .fline_coeff = 3906.25f * 25.00f},
                            .update_interval = 25,
                            .target_system_frequency = 50.00f,
                            .meter_constant = 4500.00f,
                            .no_load_i = 0.01f,
                            .no_load_p = 2.00f,
                            .v_sag = 50.00f,
                            .v_swell = 280.00f};
#else
static LMA_Config config = {.gcalib = {.fs = PARAM_FROM_FLOAT(3906.25f), .fline_coeff = PARAM_FROM_FLOAT(3906.25f * 25.00f)},
                            .update_interval = 25,
                            .target_system_frequency = PARAM_FROM_FLOAT(50.00f),
                            .meter_constant = PARAM_FROM_FLOAT(4500.00f),
                            .no_load_i = PARAM_FROM_FLOAT(0.01f),
                            .no_load_p = PARAM_FROM_FLOAT(2.00f),
                            .v_sag = PARAM_FROM_FLOAT(50.00f),
                            .v_swell = PARAM_FROM_FLOAT(280.00f)};
#endif

/* Global Calibration Data */
#if (FIXED_POINT_SUPPORT == 0U)
static LMA_GlobalCalibration gcalib_data = {};
#else
static LMA_GlobalCalibration gcalib_data = {};
#endif

/* Now define our phase pointing to our data structure*/
LMA_Phase phase;

LMA_PhaseCalibration default_calib = {
#if (FIXED_POINT_SUPPORT == 0U)
    .vrms_coeff = 2647.12598f, .irms_coeff = 6710.60010f, .p_coeff = 17763860.0f
#else
    .vrms_coeff = 2647, .irms_coeff = 6711, .p_coeff = 17763845
#endif
};

/* Define our system energy struct*/
LMA_SystemEnergy system_energy = {.impulse = {.led_on_count = 39, /* 10ms at 3906 Hz sampling frequency*/
                                              .active_counter = 0,
                                              .reactive_counter = 0,
                                              .apparent_counter = 0,
                                              .active_on = false,
                                              .reactive_on = false,
                                              .apparent_on = false}};

void Simulation(const simulation_params *const p_sim_params)
{
  p_g_sim_params = (simulation_params *)p_sim_params;

#if (FIXED_POINT_SUPPORT == 0U)
  gcalib_data.fs = p_sim_params->fs;
#else
  gcalib_data.fs = PARAM_FROM_FLOAT(p_sim_params->fs);
#endif

  /* Initialise the framework*/
  LMA_Init(&config);

  /* Set system energy structure*/
  LMA_EnergySet(&system_energy);

  /* Initialise the phase(s)*/
  LMA_PhaseRegister(&phase);

  /* Load calibration data*/
  LMA_PhaseLoadCalibration(&phase, &default_calib);

  /* Start the ADC threads*/
  LMA_Start();

  /* Start Driver thread*/
  (void)thrd_create(&driver_thread, Driver_thread, (void *)p_g_sim_params);

  /* Wait for thread to start*/
  while (!driver_thread_running)
  {
    const struct timespec ts = {
        .tv_sec = 0, .tv_nsec = 50000000 /**< 50ms */
    };
    thrd_sleep(&ts, NULL);
  }

  LMA_PhaseCalibArgs ca = {
    .p_phase = &phase,
#if (FIXED_POINT_SUPPORT == 0U)
    .vrms_tgt = p_g_sim_params->vrms,
    .irms_tgt = p_g_sim_params->irms,
    .p_tgt = p_g_sim_params->vrms * p_g_sim_params->irms,
#else
    .vrms_tgt = PARAM_FROM_FLOAT(p_g_sim_params->vrms),
    .irms_tgt = PARAM_FROM_FLOAT(p_g_sim_params->irms),
    .p_tgt = PARAM_FROM_FLOAT(p_g_sim_params->vrms * p_g_sim_params->irms),
#endif
    .line_cycles = 100
  };

  LMA_GlobalCalibArgs gca = {
#if (FIXED_POINT_SUPPORT == 0U)
    .rtc_period = 1.00,
#else
    .rtc_period = PARAM_FROM_FLOAT(1.00),
#endif
    .rtc_cycles = 5,
  };

  /* Calibrate sampling frequency and phase on startup*/
  if (p_g_sim_params->calibrate)
  {
    (void)printf("\tCalibrating...");
    LMA_PhaseCalibrate(&ca);
    LMA_GlobalCalibrate(&gca);
    (void)printf("Finished!\n\r");

#if (FIXED_POINT_SUPPORT == 0U)
    (void)printf("\t\tVrms Coeffient:     %.4f\n\r"
                 "\t\tIrms Coeffient:     %.4f\n\r"
                 "\t\tPower Coeffient:    %.4f\n\r"
                 "\t\tPhase Correction:   %.4f\n\r"
                 "\t\tSampling Frequency: %.4f\n\r\n\r",
                 ca.p_phase->calib.vrms_coeff, ca.p_phase->calib.irms_coeff, ca.p_phase->calib.p_coeff,
                 ca.p_phase->calib.vi_phase_correction, config.gcalib.fs);
#else
    (void)printf("\t\tVrms Coeffient:     %.4f\n\r"
                 "\t\tIrms Coeffient:     %.4f\n\r"
                 "\t\tPower Coeffient:    %.4f\n\r"
                 "\t\tPhase Correction:   %.4f\n\r"
                 "\t\tSampling Frequency: %.4f\n\r\n\r",
                 PARAM_TO_FLOAT(ca.p_phase->calib.vrms_coeff), PARAM_TO_FLOAT(ca.p_phase->calib.irms_coeff),
                 PARAM_TO_FLOAT(ca.p_phase->calib.p_coeff), PARAM_TO_FLOAT(ca.p_phase->calib.vi_phase_correction),
                 PARAM_TO_FLOAT(config.gcalib.fs));
#endif
  }

  /************************
   * MAIN LOOP - START
   ************************/
  (void)printf("\tLive Measurement Output...\n\r");
  int str_len = 0;
  while (driver_thread_running)
  {
    /* Sleep first to give time for the metering lib to update*/
    const struct timespec ts = {
        .tv_sec = 0, .tv_nsec = 500000000 /**< 500ms */
    };
    thrd_sleep(&ts, NULL);

    if (0 != str_len)
    {
      (void)printf("\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K"
                   "\e[1F"
                   "\e[2K");
    }

    LMA_Measurements measurements;
    LMA_MeasurementsGet(&phase, &measurements);
    LMA_EnergyGet(&system_energy);

#if (FIXED_POINT_SUPPORT == 0U)
    float p_err = 100.00f * ((measurements.s / (p_g_sim_params->vrms * p_g_sim_params->irms)) - 1);
    float act_imp_energy_wh =
        ((system_energy.energy.counter.act_imp * config.meter_constant) + system_energy.energy.accumulator.act_imp_ws) / 3600;
    float act_exp_energy_wh =
        ((system_energy.energy.counter.act_exp * config.meter_constant) + system_energy.energy.accumulator.act_exp_ws) / 3600;
    float c_imp_energy_wh =
        ((system_energy.energy.counter.c_react_imp * config.meter_constant) + system_energy.energy.accumulator.c_react_imp_ws) /
        3600;
    float c_exp_energy_wh =
        ((system_energy.energy.counter.c_react_exp * config.meter_constant) + system_energy.energy.accumulator.c_react_exp_ws) /
        3600;
    float l_imp_energy_wh =
        ((system_energy.energy.counter.l_react_imp * config.meter_constant) + system_energy.energy.accumulator.l_react_imp_ws) /
        3600;
    float l_exp_energy_wh =
        ((system_energy.energy.counter.l_react_exp * config.meter_constant) + system_energy.energy.accumulator.l_react_exp_ws) /
        3600;
    float app_imp_energy_wh =
        ((system_energy.energy.counter.app_imp * config.meter_constant) + system_energy.energy.accumulator.app_imp_ws) / 3600;
    float app_exp_energy_wh =
        ((system_energy.energy.counter.app_exp * config.meter_constant) + system_energy.energy.accumulator.app_exp_ws) / 3600;

    str_len = printf("\t\tVrms:    %.4f[V]\n\r"
                     "\t\tIrms:    %.4f[A]\n\r"
                     "\t\tFline:   %.4f[Hz]\n\r"
                     "\t\tP:       %.4f[W]\n\r"
                     "\t\tQ:       %.4f[VAR]\n\r"
                     "\t\tS:       %.4f[VA]\n\r"
                     "\t\tS Error: %.4f[%%]\n\r"
                     "\t\tAct Imp: %.4f[Wh]\n\r"
                     "\t\tAct Exp: %.4f[Wh]\n\r"
                     "\t\tC Imp:   %.4f[Wh]\n\r"
                     "\t\tC Exp:   %.4f[Wh]\n\r"
                     "\t\tL Imp:   %.4f[Wh]\n\r"
                     "\t\tL Exp:   %.4f[Wh]\n\r"
                     "\t\tApp Imp: %.4f[Wh]\n\r"
                     "\t\tApp Exp: %.4f[Wh]\n\r",
                     measurements.vrms, measurements.irms, measurements.fline, measurements.p, measurements.q, measurements.s,
                     p_err, act_imp_energy_wh, act_exp_energy_wh, c_imp_energy_wh, c_exp_energy_wh, l_imp_energy_wh,
                     l_exp_energy_wh, app_imp_energy_wh, app_exp_energy_wh);
#else
    float p_err = 100.00f * ((PARAM_TO_FLOAT(measurements.s) / (p_g_sim_params->vrms * p_g_sim_params->irms)) - 1);
    param_t act_imp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.act_imp), config.meter_constant) +
                      system_energy.energy.accumulator.act_imp_ws,
                  PARAM_FROM_INT(3600));
    param_t act_exp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.act_exp), config.meter_constant) +
                      system_energy.energy.accumulator.act_exp_ws,
                  PARAM_FROM_INT(3600));
    param_t c_imp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.c_react_imp), config.meter_constant) +
                      system_energy.energy.accumulator.c_react_imp_ws,
                  PARAM_FROM_INT(3600));
    param_t c_exp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.c_react_exp), config.meter_constant) +
                      system_energy.energy.accumulator.c_react_exp_ws,
                  PARAM_FROM_INT(3600));
    param_t l_imp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.l_react_imp), config.meter_constant) +
                      system_energy.energy.accumulator.l_react_imp_ws,
                  PARAM_FROM_INT(3600));
    param_t l_exp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.l_react_exp), config.meter_constant) +
                      system_energy.energy.accumulator.l_react_exp_ws,
                  PARAM_FROM_INT(3600));
    param_t app_imp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.app_imp), config.meter_constant) +
                      system_energy.energy.accumulator.app_imp_ws,
                  PARAM_FROM_INT(3600));
    param_t app_exp_energy_wh =
        Param_div(Param_mul(PARAM_FROM_INT(system_energy.energy.counter.app_exp), config.meter_constant) +
                      system_energy.energy.accumulator.app_exp_ws,
                  PARAM_FROM_INT(3600));

    str_len = printf("\t\tVrms:    %.4f[V]\n\r"
                     "\t\tIrms:    %.4f[A]\n\r"
                     "\t\tFline:   %.4f[Hz]\n\r"
                     "\t\tP:       %.4f[W]\n\r"
                     "\t\tQ:       %.4f[VAR]\n\r"
                     "\t\tS:       %.4f[VA]\n\r"
                     "\t\tS Error: %.4f[%%]\n\r"
                     "\t\tAct Imp: %.4f[Wh]\n\r"
                     "\t\tAct Exp: %.4f[Wh]\n\r"
                     "\t\tC Imp:   %.4f[Wh]\n\r"
                     "\t\tC Exp:   %.4f[Wh]\n\r"
                     "\t\tL Imp:   %.4f[Wh]\n\r"
                     "\t\tL Exp:   %.4f[Wh]\n\r"
                     "\t\tApp Imp: %.4f[Wh]\n\r"
                     "\t\tApp Exp: %.4f[Wh]\n\r",
                     PARAM_TO_FLOAT(measurements.vrms), PARAM_TO_FLOAT(measurements.irms), PARAM_TO_FLOAT(measurements.fline),
                     PARAM_TO_FLOAT(measurements.p), PARAM_TO_FLOAT(measurements.q), PARAM_TO_FLOAT(measurements.s), p_err,
                     PARAM_TO_FLOAT(act_imp_energy_wh), PARAM_TO_FLOAT(act_exp_energy_wh), PARAM_TO_FLOAT(c_imp_energy_wh),
                     PARAM_TO_FLOAT(c_exp_energy_wh), PARAM_TO_FLOAT(l_imp_energy_wh), PARAM_TO_FLOAT(l_exp_energy_wh),
                     PARAM_TO_FLOAT(app_imp_energy_wh), PARAM_TO_FLOAT(app_exp_energy_wh));
#endif
  }

  /************************
   * MAIN LOOP - END
   ************************/

  /* Wait for Driver thread to finish processing all the samples*/
  thrd_join(driver_thread, NULL);

  /* Kill the drivers - not actually necessary in simulation but good practise*/
  LMA_Stop();
}
