#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args_parser.h"
#include "simulation.h"

typedef enum APP_STATUS
{
  APP_SUCCESS = 0,
  APP_ERROR_ARGS,
  APP_ERROR_SIGNALS,
  APP_FINISED
} APP_STATUS;

typedef enum SIMULATION_STATE
{
  PARSE_ARGS = 0,
  CONSTRUCT_SIGNALS,
  SIMULATE
} SIMULATION_STATE;

/** Construct Application Argument Structures */
arg_t args[] = {
    {.flag = "ns=", .info = NULL, .flag_length = sizeof("ns=") - 1},
    {.flag = "fs=", .info = NULL, .flag_length = sizeof("fs=") - 1},
    {.flag = "fline=", .info = NULL, .flag_length = sizeof("fline=") - 1},
    {.flag = "vrms=", .info = NULL, .flag_length = sizeof("vrms=") - 1},
    {.flag = "irms=", .info = NULL, .flag_length = sizeof("irms=") - 1},
    {.flag = "ps=", .info = NULL, .flag_length = sizeof("ps=") - 1},
    {.flag = "calib=", .info = NULL, .flag_length = sizeof("calib=") - 1},
};
arg_data_t arg_data = {.p_args = args, .args_count = sizeof(args) / sizeof(arg_t)};

static size_t ns = 0;
static double fs = 0.0;
static double fline = 0.0;
static double vrms = 0.0;
static double irms = 0.0;
static double ps = 0.0;
static bool calib = false;

static int32_t *voltage_samples = NULL;
static int32_t *current_samples = NULL;

/** @brief Gracefully exits the application.
 * @param[in] APP_STATUS - the current applicai=tion state.
 * @return 0 = success, 1 = error state.
 */
static int Graceful_exit(APP_STATUS APP_STATUS)
{
  int ret_val = 0;

  switch (APP_STATUS)
  {
  case APP_FINISED:
  {
    printf("\n\r\n\rSimulation Finished!\n\r\n\r");
  }
  break;
  case APP_ERROR_ARGS:
  {
    printf("\n\r\n\rError in arguments, please verify!\n\r\n\r");
    ret_val = 1;
  }
  break;
  case APP_ERROR_SIGNALS:
  {
    printf("\n\r\n\rError in constructing signals, typically means not enough memory\n\r\n\r");
    ret_val = 1;
  }
  break;
  default:
    printf("\n\r\n\rUnknown App Status!\n\r\n\r");
    ret_val = 1;
    break;
  }

  return ret_val;
}

/**
 * Generates a sine wave scaled to a 24-bit signed ADC range.
 *
 * @param num_samples  Number of samples in the generated sine wave.
 * @param frequency    Frequency of the sine wave (in Hz).
 * @param phase_shift  Phase shift of the sine wave (in degrees).
 * @param rms_value    Desired RMS value of the sine wave.
 * @param gain         Gain applied to the sine wave amplitude.
 * @param div_ratio    Division ratio applied to the sine wave amplitude.
 * @param sample_rate  Sampling rate (in Hz).
 *
 * @return Pointer to an array to store the sine wave ADC readings.
 */
int32_t *Generate_sine_wave_adc(size_t num_samples, double frequency, double phase_shift, double rms_value, double gain,
                                double div_ratio, double sample_rate)
{

  int32_t *waveform = calloc(num_samples, sizeof(int32_t));
  if (waveform == NULL || sample_rate <= 0 || num_samples == 0 || div_ratio == 0)
  {
    (void)printf("\n\rInvalid waveform, potential memory issue, checks args and try reduce samples\n\r");
  }
  else
  {
    /* Calculate amplitude from RMS value and scale it by gain/division ratio */
    double amplitude = rms_value * sqrt(2) * gain * div_ratio;

    /* Angular frequency (ω = 2πf) */
    double angular_frequency = 2.0 * 3.141592653589793 * frequency;

    /* Time step between samples */
    double time_step = 1.0 / sample_rate;

    /* Generate sine wave ADC readings */
    for (size_t i = 0; i < num_samples; ++i)
    {
      /* Calculate time for the current sample */
      double time = i * time_step;

      /* Generate sine wave sample with scaling */
      double sample = amplitude * sin(angular_frequency * time + (phase_shift * 3.141592653589793 / 180.0));

      int32_t digital_code = (int32_t)round(sample * (1 << 23) / 0.5);

      waveform[i] = digital_code;
    }
  }

  return waveform;
}

int main(int argc, const char *argv[])
{
  APP_STATUS status = APP_SUCCESS;
  SIMULATION_STATE state = PARSE_ARGS;

  
  (void)printf("\n\rLWM Windows Simulation Application!\n\r\n\r");

  while (1)
  {
    switch (state)
    {
    case PARSE_ARGS:
    {
      const size_t num_args = sizeof(args) / sizeof(arg_t);
      status = (Args_Parse(argc, argv, &arg_data) == num_args) ? APP_SUCCESS : APP_ERROR_ARGS;
      if (APP_SUCCESS == status)
      {
        ns = strtoull(args[0].info, NULL, 10);
        fs = strtod(args[1].info, NULL);
        fline = strtod(args[2].info, NULL);
        vrms = strtod(args[3].info, NULL);
        irms = strtod(args[4].info, NULL);
        ps = strtod(args[5].info, NULL);
        calib = (0 != strtoull(args[6].info, NULL, 10));

        if (ns != 0 && fs != 0.0 && fline != 0.0 && vrms != 0.0 && irms != 0.0)
        {
          (void)printf("Number of samples:  %llu\n\r"
                 "Sampling Frequency: %.4f [Hz]\n\r"
                 "Line Frequency:     %.4f [Hz]\n\r"
                 "Vrms:               %.4f [V]\n\r"
                 "Irms:               %.4f [A]\n\r"
                 "Phase Shift:        %.4f [degrees]\n\r"
                 "Calibration:        %s\n\r",
                 ns, fs, fline, vrms, irms, ps, calib ? "Enabled" : "Disabled");

          status = APP_SUCCESS;
          state = CONSTRUCT_SIGNALS;
        }
        else
        {
          (void)printf("Failed to parse args:\n\r"
                 "ns: %s\n\r"
                 "fs: %s\n\r"
                 "fline: %s\n\r"
                 "vrms: %s\n\r"
                 "irms: %s\n\r"
                 "ps: %s\n\r"
                 "calib: %s\n\r",
                 args[0].info, args[1].info, args[2].info, args[3].info, args[4].info, args[5].info, args[6].info);
          status = APP_ERROR_ARGS;
        }
      }
      else
      {
        (void)printf("Invokation Args:\n\r"
               "\tns:    Number of samples to simulate - integer e.g., ns=39060\n\r"
               "\tfs:    Sampling frequency to simulate - double e.g., fs=3609.25\n\r"
               "\tfline: Line frequency to simulate - double e.g., fline=50.0\n\r"
               "\tvrms:  RMS Voltage to simulate - double e.g., vrms=230.0\n\r"
               "\tirms:  RMS Current to simulate - double e.g., irms=10.0\n\r"
               "\tps:    Phase shift of current signal relative to voltage signal to simulate - double e.g., ps=10.0\n\r"
               "\tcalib: 0 = disabled calibration on startup, !0 = enable calibration on startup\n\r");

        (void)printf("\n\rYou passed:\n\r"
               "\tns:    %s\n\r"
               "\tfs:    %s\n\r"
               "\tfline: %s\n\r"
               "\tvrms:  %s\n\r"
               "\tirms:  %s\n\r"
               "\tps:    %s\n\r"
               "\tcalib: %s\n\r",
               args[0].info, args[1].info, args[2].info, args[3].info, args[4].info, args[5].info, args[6].info);
      }
    }
    break;

    case CONSTRUCT_SIGNALS:
    {
      voltage_samples = Generate_sine_wave_adc(ns, fline, 0.0, vrms, 1, 0.0012623, fs);
      current_samples = Generate_sine_wave_adc(ns, fline, ps, irms, 8, 0.0004, fs);

      status = (NULL != current_samples && NULL != voltage_samples) ? APP_SUCCESS : APP_ERROR_SIGNALS;
      if (APP_SUCCESS == status)
      {
        state = SIMULATE;
      }
    }
    break;

    case SIMULATE:
    {
      (void)printf("\n\rSimulation Beginning!\n\r\n\r");

      simulation_params sp = {.current_samples = current_samples,
                              .voltage_samples = voltage_samples,
                              .sample_count = ns,
                              .phase_count = 1,
                              .vrms = vrms,
                              .irms = irms,
                              .fs = fs,
                              .calibrate = calib};

      Simulation(&sp);

      status = APP_FINISED;
    }
    break;

    default:
      /* Shouldn't get here*/
      break;
    }

    /* Break out of the infite while loop*/
    if (APP_SUCCESS != status)
    {
      break;
    }
  }

  return Graceful_exit(status);
}
