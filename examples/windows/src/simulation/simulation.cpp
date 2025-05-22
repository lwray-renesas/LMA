#include "simulation.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

extern "C"
{
#include "LMA_Core.h"
  bool driver_thread_running = false;
  bool tmr_running = false;
  bool adc_running = false;
  bool rtc_running = false;
}

static const simulation_params *g_sim_params = nullptr;
static std::thread driver_thread;

static LMA_Config config = {.gcalib = {.fs = 3906.25f, .fline_coeff = 97650.0f, .deg_per_sample = 4.608f},
                            .update_interval = 25,
                            .fline_target = 50.0f,
                            .fline_tol_low = 25.0f,
                            .fline_tol_high = 75.0f,
                            .meter_constant = 4500.0f,
                            .no_load_i = 0.01f,
                            .no_load_p = 2.0f,
                            .v_sag = 50.0f,
                            .v_swell = 280.0f};

static LMA_Phase phase;

static LMA_PhaseCalibration default_calib = {
    .vrms_coeff = 21177.2051f, .irms_coeff = 53685.3828f, .vi_phase_correction = 0.0f, .p_coeff = 1136906368.0f};

static LMA_SystemEnergy system_energy = {
    .impulse = {.led_on_count = 39, .active_counter = 0, .reactive_counter = 0, .active_on = false, .reactive_on = false}};

void DriverThread(const simulation_params *sim_params)
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
      phase.ws.samples.voltage = sim_params->voltage_samples[sample];
      phase.ws.samples.current = sim_params->current_samples[sample];
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

void Simulation(const simulation_params *sim_params)
{
  g_sim_params = sim_params;

  LMA_Init(&config);
  LMA_EnergySet(&system_energy);
  LMA_PhaseRegister(&phase);
  LMA_PhaseLoadCalibration(&phase, &default_calib);
  LMA_Start();

  driver_thread = std::thread(DriverThread, g_sim_params);

  LMA_PhaseCalibArgs ca{.p_phase = &phase,
                        .vrms_tgt = static_cast<float>(sim_params->vrms),
                        .irms_tgt = static_cast<float>(sim_params->irms),
                        .line_cycles = 25};

  LMA_GlobalCalibArgs gca{.rtc_period = 1.0f, .rtc_cycles = 3};

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
