#include "simulation.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <atomic>
#include <math.h>

extern "C"
{
#include "LMA_Core.h"
  bool tmr_running = false;
  bool adc_running = false;
  bool rtc_running = false;
}

std::shared_ptr<const int32_t> current_samples; /**< pointer to the current samples */
std::shared_ptr<const int32_t> voltage_samples; /**< pointer to the coltage samples */

static std::atomic<bool> driver_thread_running{false};
static std::thread driver_thread;

static LMA_Config config;
static LMA_Phase phase;
static LMA_PhaseCalibration default_calib;
static LMA_SystemEnergy system_energy;

// Sine wave generator
static std::shared_ptr<int32_t> GenerateSineWaveADC(size_t numSamples, double frequency, double phaseShift, double rmsValue,
                                               double gain, double divRatio, double sampleRate)
{
  if (sampleRate <= 0.0 || numSamples == 0 || divRatio == 0.0)
  {
    std::cerr << "\n\rInvalid waveform, potential memory issue, check args and try reducing samples\n\r";
    return nullptr;
  }

  std::shared_ptr<int32_t> waveform(new int32_t[numSamples], std::default_delete<int32_t[]>());
  const double amplitude = rmsValue * std::sqrt(2.0) * gain * divRatio;
  const double omega = 2.0 * 3.14159265358979323846 * frequency;
  const double dt = 1.0 / sampleRate;

  for (size_t i = 0; i < numSamples; ++i)
  {
    double time = i * dt;
    double sample = amplitude * std::sin(omega * time + (phaseShift * 3.14159265358979323846 / 180.0));
    waveform.get()[i] = static_cast<int32_t>(std::round(sample * (1 << 23) / 0.5));
  }

  return waveform;
}

static void Driver_thread(const SimulationParams *sim_params)
{
  size_t sample = 0;
  size_t rtc_counter = 0;
  size_t tmr_counter = 0;
  size_t sleep_counter = 0;

  const uint32_t one_sec = static_cast<uint32_t>(sim_params->fs);
  const uint32_t ten_ms = one_sec / 10;

  driver_thread_running = true;

  while (sample < sim_params->sample_count)
  {
    if (rtc_running && ++rtc_counter >= one_sec)
    {
      rtc_counter = 0;
      LMA_CB_RTC();
    }

    if (tmr_running && ++tmr_counter >= ten_ms)
    {
      tmr_counter = 0;
      LMA_CB_TMR();
    }

    if (adc_running)
    {
      phase.ws.samples.voltage = voltage_samples.get()[sample];
      phase.ws.samples.current = current_samples.get()[sample];
      phase.ws.samples.voltage90 = LMA_PhaseShift90(phase.ws.samples.voltage);

      LMA_CB_ADC();
      ++sample;
    }

    if (++sleep_counter >= 78)
    {
      sleep_counter = 0;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  }

  driver_thread_running = false;
}

void Simulation(const SimulationParams *sim_params)
{
  // Construct waveforms
  voltage_samples =
      GenerateSineWaveADC(sim_params->sample_count, sim_params->fline, 0.0, sim_params->vrms, 1, 0.0012623, sim_params->fs);
  
  if (sim_params->rogowski)
  {
      // TODO: Handle rogowski processing
  }
  else
  {
    current_samples =
        GenerateSineWaveADC(sim_params->sample_count, sim_params->fline, 0.0, sim_params->irms, 8, 0.0004, sim_params->fs);
  }
  // Config
  config.gcalib.fs = sim_params->fs;
  config.gcalib.fline_coeff = 97650.0f;
  config.gcalib.deg_per_sample = 4.608f;
  config.update_interval = 25;
  config.fline_target = sim_params->fline;
  config.fline_tol_low = sim_params->fline - (sim_params->fline/2);
  config.fline_tol_high = sim_params->fline + (sim_params->fline / 2);
  config.meter_constant = 4500.0f;
  config.no_load_i = 0.01f;
  config.no_load_p = 2.0f;
  config.v_sag = sim_params->vrms * 0.25;
  config.v_swell = sim_params->vrms * 1.25;

  // PhaseCalibration
  default_calib.vrms_coeff = 21177.2051f;
  default_calib.irms_coeff = 53685.3828f;
  default_calib.vi_phase_correction = 0.0f;
  default_calib.p_coeff = 1136906368.0f;

  // SystemEnergy
  system_energy.impulse.led_on_count = 0.01 / (1.0/sim_params->fs); // 10ms
  system_energy.impulse.active_counter = 0;
  system_energy.impulse.reactive_counter = 0;
  system_energy.impulse.active_on = false;
  system_energy.impulse.reactive_on = false;

  LMA_Init(&config);
  LMA_EnergySet(&system_energy);
  LMA_PhaseRegister(&phase);
  LMA_PhaseLoadCalibration(&phase, &default_calib);
  LMA_Start();

  driver_thread = std::thread(Driver_thread, sim_params);

  // Wait until the driver thread is running
  while (!driver_thread_running)
  {
  }

  LMA_PhaseCalibArgs ca;
  LMA_GlobalCalibArgs gca;

  ca.p_phase = &phase;
  ca.vrms_tgt = static_cast<float>(sim_params->vrms);
  ca.irms_tgt = static_cast<float>(sim_params->irms);
  ca.line_cycles = 25;

  gca.rtc_period = 1.0f;
  gca.rtc_cycles = 3;

  if (sim_params->calibrate)
  {
    std::cout << std::endl;
    std::cout << "\tCalibrating Phase...";
    LMA_PhaseCalibrate(&ca);
    std::cout << "Finished!" << std::endl;
    std::cout << "\tCalibrating Global...";
    LMA_GlobalCalibrate(&gca);
    std::cout << "Finished!" << std::endl;

    std::cout << std::fixed << std::setprecision(4) << "\t\tVrms Coefficient:     " << ca.p_phase->calib.vrms_coeff << "\n"
              << "\t\tIrms Coefficient:     " << ca.p_phase->calib.irms_coeff << "\n"
              << "\t\tPower Coefficient:    " << ca.p_phase->calib.p_coeff << "\n"
              << "\t\tPhase Correction:     " << ca.p_phase->calib.vi_phase_correction << "\n"
              << "\t\tSampling Frequency:   " << config.gcalib.fs << "\n"
              << "\t\tDegrees Per Sample:   " << config.gcalib.deg_per_sample << "\n"
              << "\t\tFline Coefficient:    " << config.gcalib.fline_coeff << "\n"
              << std::endl;
  }

  std::cout << "\tLive Measurement Output...\n" << std::endl;

  int str_len = 0;

  while (driver_thread_running)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    static LMA_Measurements measurements;
    static LMA_ConsumptionData energy;

    if (LMA_MeasurementsReady(&phase))
    {
      LMA_MeasurementsGet(&phase, &measurements);
      LMA_EnergyGet(&system_energy);
      LMA_ConsumptionDataGet(&system_energy, &energy);

      if (str_len != 0)
      {
        for (int i = 0; i < 14; ++i)
        {
          std::cout << "\033[2K\033[1F";
        }
      }

      std::cout << std::fixed << std::setprecision(4) << "\t\tVrms:    " << measurements.vrms << " [V]\n"
                << "\t\tIrms:    " << measurements.irms << " [A]\n"
                << "\t\tFline:   " << measurements.fline << " [Hz]\n"
                << "\t\tP:       " << measurements.p << " [W]\n"
                << "\t\tQ:       " << measurements.q << " [VAR]\n"
                << "\t\tS:       " << measurements.s << " [VA]\n"
                << "\t\tAct Imp: " << energy.act_imp_energy_wh << " [Wh]\n"
                << "\t\tAct Exp: " << energy.act_exp_energy_wh << " [Wh]\n"
                << "\t\tApp Imp: " << energy.app_imp_energy_wh << " [Wh]\n"
                << "\t\tApp Exp: " << energy.app_exp_energy_wh << " [Wh]\n"
                << "\t\tC Imp:   " << energy.c_imp_energy_wh << " [Wh]\n"
                << "\t\tC Exp:   " << energy.c_exp_energy_wh << " [Wh]\n"
                << "\t\tL Imp:   " << energy.l_imp_energy_wh << " [Wh]\n"
                << "\t\tL Exp:   " << energy.l_exp_energy_wh << " [Wh]\n"
                << std::flush;

      str_len = 1;
    }
  }

  if (driver_thread.joinable())
  {
    driver_thread.join();
  }

  LMA_Stop();
}
