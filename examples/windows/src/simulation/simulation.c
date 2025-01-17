#include "simulation.h"
#include "LMA_Core.h"
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

/* Configuration required for configuring the library*/
#if (FIXED_POINT_SUPPORT == 0U)
static LMA_Config config = {.update_interval = 25, .target_system_frequency = 50.00f};
#else
static LMA_Config config = {.update_interval = 25, .target_system_frequency = PARAM_FROM_FLOAT(50.00f)};
#endif

/* Global Calibration Data */
#if (FIXED_POINT_SUPPORT == 0U)
static LMA_GlobalCalibration gcalib_data = {.fs = 3906.25f, .fline_coeff = 3906.25f * 25.00f};
#else
static LMA_GlobalCalibration gcalib_data = {.fs = PARAM_FROM_FLOAT(3906.25f), .fline_coeff = PARAM_FROM_FLOAT(3906.25f * 25.00f)};
#endif

/* Now define our phase pointing to our data structure*/
LMA_Phase phase = {
#if (FIXED_POINT_SUPPORT == 0U)
    .calib = {.vrms_coeff = 2647.12598f, .irms_coeff = 6710.60010f, .p_coeff = 17763860.0f},
#else
    .calib = {.vrms_coeff = 2647, .irms_coeff = 6711, .p_coeff = 17763845},
#endif
    .p_gcalib = &gcalib_data,
    .calibrating = false,
    .disable_acc = false};

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

  /* Initialise the phase(s)*/
  LMA_PhaseInit(&phase);

  /* Start the ADC threads*/
  LMA_Start();

  /* Start Driver thread*/
  (void)thrd_create(&driver_thread, Driver_thread, (void *)p_g_sim_params);

  /* Wait for thread to start*/
  while (!driver_thread_running)
  {
  }

  Calibration_args ca = {
    .flags = SAMPLING_FREQUENCY | PHASE_PAIR,
    .p_gcalib = &gcalib_data,
    .p_phase = &phase,
#if (FIXED_POINT_SUPPORT == 0U)
    .vrms_tgt = p_g_sim_params->vrms,
    .irms_tgt = p_g_sim_params->irms,
    .p_tgt = p_g_sim_params->vrms * p_g_sim_params->irms,
    .rtc_period = 1.00,
#else
    .vrms_tgt = PARAM_FROM_FLOAT(p_g_sim_params->vrms),
    .irms_tgt = PARAM_FROM_FLOAT(p_g_sim_params->irms),
    .p_tgt = PARAM_FROM_FLOAT(p_g_sim_params->vrms * p_g_sim_params->irms),
    .rtc_period = PARAM_FROM_FLOAT(1.00),
#endif
    .rtc_cycles = 5,
    .line_cycles = 100
  };

  /* Calibrate sampling frequency and phase on startup*/
  if (p_g_sim_params->calibrate)
  {
    (void)printf("\tCalibrating...");
    LMA_Calibrate(&ca);
    (void)printf("Finished!\n\r");

#if (FIXED_POINT_SUPPORT == 0U)
    (void)printf("\t\tVrms Coeffient:     %.4f\n\r"
                 "\t\tIrms Coeffient:     %.4f\n\r"
                 "\t\tPower Coeffient:    %.4f\n\r"
                 "\t\tPhase Correction:   %.4f\n\r"
                 "\t\tSampling Frequency: %.4f\n\r\n\r",
                 ca.p_phase->calib.vrms_coeff, ca.p_phase->calib.irms_coeff, ca.p_phase->calib.p_coeff,
                 ca.p_phase->calib.vi_phase_correction, ca.p_phase->p_gcalib->fs);
#else
    (void)printf("\t\tVrms Coeffient:     %.4f\n\r"
                 "\t\tIrms Coeffient:     %.4f\n\r"
                 "\t\tPower Coeffient:    %.4f\n\r"
                 "\t\tPhase Correction:   %.4f\n\r"
                 "\t\tSampling Frequency: %.4f\n\r\n\r",
                 PARAM_TO_FLOAT(ca.p_phase->calib.vrms_coeff), PARAM_TO_FLOAT(ca.p_phase->calib.irms_coeff),
                 PARAM_TO_FLOAT(ca.p_phase->calib.p_coeff), PARAM_TO_FLOAT(ca.p_phase->calib.vi_phase_correction),
                 PARAM_TO_FLOAT(ca.p_phase->p_gcalib->fs));
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
                   "\e[2K");
    }

  LMA_Measurements measurements;
  LMA_Measurements_Get(&phase, &measurements);

#if (FIXED_POINT_SUPPORT == 0U)
    float p_err = 100.00f * (1 - (measurements.p) / (measurements.s));

    str_len = printf("\t\tVrms:    %.4f[V]\n\r"
                     "\t\tIrms:    %.4f[A]\n\r"
                     "\t\tFline:   %.4f[Hz]\n\r"
                     "\t\tP:       %.4f[W]\n\r"
                     "\t\tQ:       %.4f[VAR]\n\r"
                     "\t\tS:       %.4f[VA]\n\r"
                     "\t\tS Error: %.4f[%%]",
                     measurements.vrms, measurements.irms, measurements.fline, measurements.p,
                     measurements.q, measurements.s, p_err);
#else
    float p_err =
        100.00f * ((PARAM_TO_FLOAT(LMA_ActivePower_Get(&phase)) / (p_g_sim_params->vrms * p_g_sim_params->irms)) - 1);

    str_len = printf("\t\tVrms:    %.4f[V]\n\r"
                     "\t\tIrms:    %.4f[A]\n\r"
                     "\t\tFline:   %.4f[Hz]\n\r"
                     "\t\tP:       %.4f[W]\n\r"
                     "\t\tQ:       %.4f[VAR]\n\r"
                     "\t\tS:       %.4f[VA]\n\r"
                     "\t\tS Error: %.4f[%%]",
                     PARAM_TO_FLOAT(measurements.vrms), PARAM_TO_FLOAT(measurements.irms),
                     PARAM_TO_FLOAT(measurements.fline), PARAM_TO_FLOAT(measurements.p),
                     PARAM_TO_FLOAT(measurements.q), PARAM_TO_FLOAT(measurements.s), p_err);
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
