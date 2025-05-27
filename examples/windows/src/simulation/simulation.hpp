#ifndef _SIMULTAION_H_
#define _SIMULTAION_H_

#include <cstdint>
#include <atomic>
#include <memory>
#include <vector>

extern "C"
{
#include "LMA_Core.h"
extern bool tmr_running;
extern bool adc_running;
extern bool rtc_running;
}

/** @brief interface param structure for simulation. */
typedef struct SimulationParams
{
    size_t sample_count; /**< number of sample pairs available */
    double ps;           /**< Phase shift between current and coltage in degrees*/
    double vrms; /**< target vrms for calibration */
    double irms; /**< target vrms for calibration */
    double fs; /**< sampling frequency */
    double fline;   /**< line frequency */
    bool calibrate; /**< flag to enable/disable calibration on startup */
    bool rogowski; /**< flag to enable/disable rogowski on startup */
    std::atomic<bool> stop_simulation;   /**< signal to stop the simulation*/
}SimulationParams;

/** @brief results of smiulation*/
typedef struct SimulationResults
{
  std::unique_ptr<std::vector<int32_t>> raw_voltage_signal; /**< Generated raw voltage signal*/
  std::unique_ptr<std::vector<int32_t>> raw_current_signal; /**< Generated raw current signal*/
  std::vector<LMA_Measurements> measurements; /**< Computed measurment results*/
  LMA_ConsumptionData final_energy;              /**< Final measured energy*/
} SimulationResults;

/** @brief driver for our simulation
* @param[in] p_sim_params - pointer to the simulation parameters.
* @return shared pointer to simulation results.
 */
std::shared_ptr<SimulationResults> Simulation(const SimulationParams *const p_sim_params);

#endif /* _SIMULTAION_H_*/
