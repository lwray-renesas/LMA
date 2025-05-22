#include "args_parser.hpp"
#include "simulation.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum class AppStatus
{
  SUCCESS = 0,
  ERROR_ARGS,
  ERROR_SIGNALS,
  FINISHED
};

enum class SimulationState
{
  PARSE_ARGS = 0,
  CONSTRUCT_SIGNALS,
  SIMULATE
};

// Argument setup
arg_t args[] = {
    {"ns=", nullptr, sizeof("ns=") - 1},       {"fs=", nullptr, sizeof("fs=") - 1},
    {"fline=", nullptr, sizeof("fline=") - 1}, {"vrms=", nullptr, sizeof("vrms=") - 1},
    {"irms=", nullptr, sizeof("irms=") - 1},   {"ps=", nullptr, sizeof("ps=") - 1},
    {"calib=", nullptr, sizeof("calib=") - 1},
};

arg_data_t arg_data = {args, sizeof(args) / sizeof(arg_t)};

// Graceful exit
int GracefulExit(AppStatus status)
{
  switch (status)
  {
  case AppStatus::FINISHED:
    std::cout << "\n\r\n\rSimulation Finished!\n\r\n\r";
    return 0;
  case AppStatus::ERROR_ARGS:
    std::cout << "\n\r\n\rError in arguments, please verify!\n\r\n\r";
    return 1;
  case AppStatus::ERROR_SIGNALS:
    std::cout << "\n\r\n\rError in constructing signals, typically means not enough memory\n\r\n\r";
    return 1;
  default:
    std::cout << "\n\r\n\rUnknown App Status!\n\r\n\r";
    return 1;
  }
}

// Sine wave generator
std::unique_ptr<int32_t[]> GenerateSineWaveADC(size_t numSamples, double frequency, double phaseShift, double rmsValue,
                                               double gain, double divRatio, double sampleRate)
{
  if (sampleRate <= 0.0 || numSamples == 0 || divRatio == 0.0)
  {
    std::cerr << "\n\rInvalid waveform, potential memory issue, check args and try reducing samples\n\r";
    return nullptr;
  }

  auto waveform = std::make_unique<int32_t[]>(numSamples);
  const double amplitude = rmsValue * std::sqrt(2.0) * gain * divRatio;
  const double omega = 2.0 * 3.14159265358979323846 * frequency;
  const double dt = 1.0 / sampleRate;

  for (size_t i = 0; i < numSamples; ++i)
  {
    double time = i * dt;
    double sample = amplitude * std::sin(omega * time + (phaseShift * 3.14159265358979323846 / 180.0));
    waveform[i] = static_cast<int32_t>(std::round(sample * (1 << 23) / 0.5));
  }

  return waveform;
}

int main(int argc, const char *argv[])
{
  AppStatus status = AppStatus::SUCCESS;
  SimulationState state = SimulationState::PARSE_ARGS;

  std::cout << "\n\rLWM Windows Simulation Application!\n\r\n\r";

  size_t ns = 0;
  double fs = 0.0, fline = 0.0, vrms = 0.0, irms = 0.0, ps = 0.0;
  bool calib = false;

  std::unique_ptr<int32_t[]> voltageSamples;
  std::unique_ptr<int32_t[]> currentSamples;

  while (true)
  {
    switch (state)
    {
    case SimulationState::PARSE_ARGS:
    {
      const size_t numArgs = sizeof(args) / sizeof(arg_t);
      status = (Args_Parse(argc, argv, &arg_data) == numArgs) ? AppStatus::SUCCESS : AppStatus::ERROR_ARGS;

      if (status == AppStatus::SUCCESS)
      {
        try
        {
          ns = std::stoull(args[0].info);
          fs = std::stod(args[1].info);
          fline = std::stod(args[2].info);
          vrms = std::stod(args[3].info);
          irms = std::stod(args[4].info);
          ps = std::stod(args[5].info);
          calib = std::stoull(args[6].info) != 0;

          if (ns && fs && fline && vrms && irms)
          {
            std::cout << "Number of samples:  " << ns << "\n\r"
                      << "Sampling Frequency: " << fs << " [Hz]\n\r"
                      << "Line Frequency:     " << fline << " [Hz]\n\r"
                      << "Vrms:               " << vrms << " [V]\n\r"
                      << "Irms:               " << irms << " [A]\n\r"
                      << "Phase Shift:        " << ps << " [degrees]\n\r"
                      << "Calibration:        " << (calib ? "Enabled" : "Disabled") << "\n\r";

            state = SimulationState::CONSTRUCT_SIGNALS;
          }
          else
          {
            status = AppStatus::ERROR_ARGS;
          }
        }
        catch (...)
        {
          status = AppStatus::ERROR_ARGS;
        }
      }

      if (status == AppStatus::ERROR_ARGS)
      {
        std::cout << "\n\rUsage:\n\r"
                  << "\tns:    Number of samples (e.g., ns=39060)\n\r"
                  << "\tfs:    Sampling frequency (e.g., fs=3609.25)\n\r"
                  << "\tfline: Line frequency (e.g., fline=50.0)\n\r"
                  << "\tvrms:  RMS Voltage (e.g., vrms=230.0)\n\r"
                  << "\tirms:  RMS Current (e.g., irms=10.0)\n\r"
                  << "\tps:    Phase shift (e.g., ps=10.0)\n\r"
                  << "\tcalib: 0 = disable, non-0 = enable\n\r";
      }
      break;
    }

    case SimulationState::CONSTRUCT_SIGNALS:
    {
      voltageSamples = GenerateSineWaveADC(ns, fline, 0.0, vrms, 1, 0.0012623, fs);
      currentSamples = GenerateSineWaveADC(ns, fline, ps, irms, 8, 0.0004, fs);

      if (voltageSamples && currentSamples)
      {
        state = SimulationState::SIMULATE;
      }
      else
      {
        status = AppStatus::ERROR_SIGNALS;
      }
      break;
    }

    case SimulationState::SIMULATE:
    {
      std::cout << "\n\rSimulation Beginning!\n\r\n\r";

      simulation_params sp{
          .current_samples = currentSamples.get(),
          .voltage_samples = voltageSamples.get(),
          .sample_count = ns,
          .phase_count = 1,
          .vrms = vrms,
          .irms = irms,
          .fs = fs,
          .calibrate = calib,
      };

      Simulation(&sp);
      status = AppStatus::FINISHED;
      break;
    }

    default:
      break;
    }

    if (status != AppStatus::SUCCESS)
    {
      break;
    }
  }

  return GracefulExit(status);
}
