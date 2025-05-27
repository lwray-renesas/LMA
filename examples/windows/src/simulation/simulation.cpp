#include "simulation.hpp"
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <thread>
#include <vector>

/** @brief parameters passed to the driver thread*/
typedef struct DriverParams
{
  std::unique_ptr<std::vector<int32_t>> p_current_samples; /**< Pointer to the current samples */
  std::unique_ptr<std::vector<int32_t>> p_voltage_samples; /**< Pointer to the coltage samples */
  std::atomic<bool> stop_driver_thread;   /**< Pointer to the variable for stopping/cancelling the driver thread*/
  std::atomic<bool> driver_thread_running; /**< Pointer to the variable for indicating the driver thread is running*/
  std::unique_ptr<LMA_Phase> p_phase; /**< Pointer to the phase to work on*/
  double fs;                                                /**< sampling frequency*/
}DriverParams;


// Sine wave generator
static std::pair<std::unique_ptr<std::vector<int32_t>>, std::unique_ptr<std::vector<double>>>
                   GenerateSineWaveADC(size_t numSamples, double frequency, double phaseShift,
                                                                 double rmsValue, double gain, double divRatio,
                                                                 double sampleRate)
{
  auto res_adc = std::make_unique<std::vector<int32_t>>();
  auto res = std::make_unique<std::vector<double>>();

  if (sampleRate <= 0.0 || numSamples == 0 || divRatio == 0.0)
  {
    std::cerr << "\n\rInvalid waveform, potential memory issue, check args and try reducing samples\n\r";
  }

  const double amplitude = rmsValue * std::sqrt(2.0);
  const double omega = 2.0 * 3.14159265358979323846 * frequency;
  const double dt = 1.0 / sampleRate;

  for (size_t i = 0; i < numSamples; ++i)
  {
    double time = i * dt;
    double sample = amplitude * std::sin(omega * time + (phaseShift * 3.14159265358979323846 / 180.0));
    res->push_back(sample);
    res_adc->push_back(static_cast<int32_t>(std::round(sample * gain * divRatio * (1 << 23) / 0.5)));
  }

  return std::make_pair<std::unique_ptr<std::vector<int32_t>>, std::unique_ptr<std::vector<double>>>(std::move(res_adc), std::move(res));
}

static void Driver_thread(std::shared_ptr<DriverParams> drvr_params)
{
  size_t sample = 0;
  size_t rtc_counter = 0;
  size_t tmr_counter = 0;
  size_t sleep_counter = 0;

  const uint32_t one_sec = static_cast<uint32_t>(drvr_params->fs);
  const uint32_t ten_ms = one_sec / 10;

  drvr_params->driver_thread_running = true;

  while (sample < drvr_params->p_voltage_samples->size() && !(drvr_params->stop_driver_thread))
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
      drvr_params->p_phase->ws.samples.voltage = static_cast<spl_t>((*drvr_params->p_voltage_samples)[sample]);
      drvr_params->p_phase->ws.samples.current = static_cast<spl_t>((*drvr_params->p_current_samples)[sample]);
      drvr_params->p_phase->ws.samples.voltage90 = LMA_PhaseShift90(drvr_params->p_phase->ws.samples.voltage);

      LMA_CB_ADC();
      ++sample;
    }

    if (++sleep_counter >= 78)
    {
      sleep_counter = 0;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  }

  drvr_params->driver_thread_running = false;
}

std::shared_ptr<SimulationResults> Simulation(const SimulationParams *sim_params)
{
  auto results = std::make_shared<SimulationResults>();
  auto drv_params = std::make_shared<DriverParams>();

  results->voltage_signal = std::make_unique<std::vector<double>>();
  results->current_signal = std::make_unique<std::vector<double>>();

  // Construct waveforms
  auto v_pair =
      GenerateSineWaveADC(sim_params->sample_count, sim_params->fline, 0.0, sim_params->vrms, 1, 0.0012623, sim_params->fs);
  drv_params->p_voltage_samples = std::move(v_pair.first);
  results->voltage_signal = std::move(v_pair.second);

  if (sim_params->rogowski)
  {
    // TODO: Handle rogowski processing
  }
  else
  {
    auto i_pair =
        GenerateSineWaveADC(sim_params->sample_count, sim_params->fline, 0.0, sim_params->irms, 8, 0.0004, sim_params->fs);
    drv_params->p_current_samples = std::move(i_pair.first);
    results->current_signal = std::move(i_pair.second);
  }


  // Config
  auto p_config = std::make_unique<LMA_Config>();
  p_config->gcalib.fs = sim_params->fs;
  p_config->gcalib.fline_coeff = 97650.0f;
  p_config->gcalib.deg_per_sample = 4.608f;
  p_config->update_interval = 25;
  p_config->fline_target = sim_params->fline;
  p_config->fline_tol_low = sim_params->fline - (sim_params->fline / 2);
  p_config->fline_tol_high = sim_params->fline + (sim_params->fline / 2);
  p_config->meter_constant = 4500.0f;
  p_config->no_load_i = 0.01f;
  p_config->no_load_p = 2.0f;
  p_config->v_sag = sim_params->vrms * 0.25;
  p_config->v_swell = sim_params->vrms * 1.25;

  // PhaseCalibration
  auto p_default_calib = std::make_unique<LMA_PhaseCalibration>();
  p_default_calib->vrms_coeff = 21177.2051f;
  p_default_calib->irms_coeff = 53685.3828f;
  p_default_calib->vi_phase_correction = 0.0f;
  p_default_calib->p_coeff = 1136906368.0f;

  // SystemEnergy
  auto p_system_energy = std::make_unique<LMA_SystemEnergy>();
  p_system_energy->impulse.led_on_count = 0.01 / (1.0 / sim_params->fs); // 10ms
  p_system_energy->impulse.active_counter = 0;
  p_system_energy->impulse.reactive_counter = 0;
  p_system_energy->impulse.active_on = false;
  p_system_energy->impulse.reactive_on = false;

  drv_params->p_phase = std::make_unique<LMA_Phase>();
  LMA_Init(p_config.get());
  LMA_EnergySet(p_system_energy.get());
  LMA_PhaseRegister(drv_params->p_phase.get());
  LMA_PhaseLoadCalibration(drv_params->p_phase.get(), p_default_calib.get());
  LMA_Start();

  drv_params->driver_thread_running = false;
  drv_params->stop_driver_thread = false;
  std::thread driver_thread = std::thread(Driver_thread, drv_params);

  // Wait until the driver thread is running
  while (!drv_params->driver_thread_running)
  {
  }

  LMA_PhaseCalibArgs ca;
  LMA_GlobalCalibArgs gca;

  ca.p_phase = drv_params->p_phase.get();
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
              << "\t\tSampling Frequency:   " << p_config->gcalib.fs << "\n"
              << "\t\tDegrees Per Sample:   " << p_config->gcalib.deg_per_sample << "\n"
              << "\t\tFline Coefficient:    " << p_config->gcalib.fline_coeff << "\n"
              << std::endl;
  }

  std::cout << "\tLive Measurement Output...\n" << std::endl;

  int str_len = 0;

  while (drv_params->driver_thread_running)
  {
    /* Update simulation state from calling thread*/
    drv_params->stop_driver_thread.store(sim_params->stop_simulation.load());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    LMA_Measurements measurements;
    LMA_ConsumptionData energy;

    if (LMA_MeasurementsReady(drv_params->p_phase.get()))
    {
      LMA_MeasurementsGet(drv_params->p_phase.get(), &measurements);
      LMA_EnergyGet(p_system_energy.get());
      LMA_ConsumptionDataGet(p_system_energy.get(), &energy);

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

      results->measurements.push_back(measurements);

      str_len = 1;
    }
  }

  if (driver_thread.joinable())
  {
    driver_thread.join();
  }

  LMA_Stop();

  LMA_ConsumptionDataGet(p_system_energy.get(), &(results->final_energy));
  std::memcpy(&(results->calib_parameters), &(drv_params->p_phase->calib), sizeof(LMA_GlobalCalibration));

  LMA_Deinit();

  std::cout << "\nSimulation Complete!\n";

  results->raw_voltage_signal = std::move(drv_params->p_voltage_samples);
  results->raw_current_signal = std::move(drv_params->p_current_samples);

  return results;
}
