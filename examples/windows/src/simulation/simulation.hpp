#ifndef _SIMULTAION_H_
#define _SIMULTAION_H_

#include <cstdint>
#include <atomic>

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
    std::atomic<bool> running;   /**< simulation running*/
}SimulationParams;

/** @brief driver for our simulation
* @param[in] p_sim_params - pointer to the simulation parameters.
 */
void Simulation(const SimulationParams * const p_sim_params);

#endif /* _SIMULTAION_H_*/
