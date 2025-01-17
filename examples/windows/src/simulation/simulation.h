#include "stdint.h"
#include "stdbool.h"

/** @brief interface param structure for simulation. */
typedef struct simulation_params
{
    const int32_t * const current_samples; /**< pointer to the current samples */
    const int32_t * const voltage_samples; /**< pointer to the coltage samples */
    const size_t sample_count; /**< number of sample pairs available */
    const size_t phase_count; /**< number of phases available */
    const double vrms; /**< target vrms for calibration */
    const double irms; /**< target vrms for calibration */
    const double fs; /**< sampling frequency */
    const bool calibrate; /**< flag to enable/disable calibration on startup */
}simulation_params;

/** @brief driver for our simulation
* @param[in] p_sim_params - pointer to the simulation parameters.
 */
void Simulation(const simulation_params * const p_sim_params);
