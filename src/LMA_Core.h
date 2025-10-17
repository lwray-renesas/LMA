/**
 * @file LMA_Core.h
 * @brief Core functionality declarations for LMA.
 *
 * @details This file includes LMA_Utils and provides declarations of the LMA core functionality.
 */

#ifndef _LMA_CORE_H
#define _LMA_CORE_H

#include "LMA_Port.h"

/** @addtogroup API
 * @brief LMA API
 * @details The LMA API consists of control and measurement, both of which are used together to form the base software defined
 * functionality of AC metrology.
 *  @{
 */

/** @addtogroup Control
 * @brief LMA Control API
 * @details The LMA Control API is used to control the functionality of LMA, that is to say it defines LMA's behaviour. For
 * example starting and stopping operation or performing a calibration.
 *  @{
 */

/** @brief Initalises the Light-Weight Metrology for AC Framework according to the config.
 * @param[in] p_config_arg - pointer to the configuration structure.
 */
void LMA_Init(LMA_Config *const p_config_arg);

/** @brief Deinitialises LMA
 * @details Deconstructs the phase linked list so callbacks have no structures to work on.
 * @warning Does NOT stop drivers operating - call LMA_Stop first if this is the desired behaviour.
 */
void LMA_Deinit(void);

/** @brief Registers a phase to the library
 * @details Do once on power up.
 * This function also initialises the phase, so should be called BEFORE
 * LMA_NeutralRegister
 * LMA_ComputationHookRegister
 * @param[in] p_phase - pointer to the phase
 */
void LMA_PhaseRegister(LMA_Phase *const p_phase);

/** @brief Registers the systems neutral line to the library (if used)
 * @warning Must be performed AFTER a phase is registered - registering a phase nullifys this.
 * @param[in] p_phase - pointer to the phase structure to link to
 * @param[in] p_neutral - pointer to the neutral structure
 */
void LMA_NeutralRegister(LMA_Phase *const p_phase, LMA_Neutral *const p_neutral);

/** @brief Registers a hook to be called during paramter computations.
 * @warning Must be performed AFTER a phase is registered - registering a phase nullifys this.
 * @details provides a function which is called after voltage and current are computed.
 * This function can modify the voltage and current and also return a compensation factor which is applied
 * to the computed power values, from which the energy values are derived.
 * This is useful for linearisation and compensation techniques used for accuracy improvements at run time.
 * @param[in] p_phase - pointer to the phase structure to link to
 * @param[in] comp_hook - function pointer that should point to a function which accepts two pointers to floats and returns a
 * float. the first argument is a pointer to the computed current, the second to the voltage and the third to the frequency it
 * returns a compensation factor which is multiplied by ALL power values and propogates to the energy values. If no compensation
 * is desired, return 1.00f - if no computation hook is set, no compensation is applied.
 */
void LMA_ComputationHookRegister(LMA_Phase *const p_phase, float (*comp_hook)(float *i, float *v, float *f));

/** @brief Loads calibration data to a system (and config).
 * @param[in] p_calib - pointer to the calibration data to load.
 */
void LMA_GlobalLoadCalibration(const LMA_GlobalCalibration *const p_calib);

/** @brief Loads calibration data to a phase.
 * @param[inout] p_phase - pointer to the phase
 * @param[in] p_calib - pointer to the calibration data to load.
 */
void LMA_PhaseLoadCalibration(LMA_Phase *const p_phase, const LMA_PhaseCalibration *const p_calib);

/** @brief Loads calibration data to a neutral.
 * @param[inout] p_neutral - pointer to the neutral object.
 * @param[in] p_calib - pointer to the calibration data to load.
 */
void LMA_NeutralLoadCalibration(LMA_Neutral *const p_neutral, const LMA_NeutralCalibration *const p_calib);

/** @brief Starts LMA Operation
 * @details Starts the metering state machine and all associated drivers.
 */
void LMA_Start(void);

/** @brief Stops LMA Operation
 * @details Stops the metering state machine and all associated drivers.
 */
void LMA_Stop(void);

/** @brief Performs a calibration command according to the flags passed.
 * @details - for calibration of the phase angle error - the result is stored in each phases
 * phase.calib.vi_phase_correction.
 * Because it depends on whether your ADC supports phase correction in hardware, it is left to the programmer
 * to use this parameter as they see fit.
 * @param[in] calib_args - Arguments and data structure use for a calibration.
 */
void LMA_PhaseCalibrate(LMA_PhaseCalibArgs *const calib_args);

/** @brief Performs a calibration command according to the flags passed.
 * @details - results stored in LMA_Config.gcalib
 */
void LMA_GlobalCalibrate(LMA_GlobalCalibArgs *const calib_args);

/** @brief Sets the energy data
 * @param[in] p_energy - pointer to the energy data structure to work on
 */
void LMA_EnergySet(LMA_SystemEnergy *const p_energy);

/** @brief Gets the energy data
 * @param[in] p_energy - pointer to the energy data structure to work on
 */
void LMA_EnergyGet(LMA_SystemEnergy *const p_energy);

/** @brief Gets copy of the current phase status using critical secrtions
 * @param[inout] p_phase - pointer to the phase block on which to get status from.
 * @return LMA_Status of phase
 */
LMA_Status LMA_StatusGet(const LMA_Phase *const p_phase);

/** @} */

/** @addtogroup Measurement
 * @brief LMA Measurement API
 * @details The LMA Measurement API is used to retrieve measurement metrics from LMA at runtime.
 *  @{
 */

/** @brief Outputs current snap shot of measurement set
 * @param[inout] p_phase - pointer to the phase block on which to get measurements from.
 * @param[out] p_measurements - pointer to the measurement structure to populate.
 */
void LMA_MeasurementsGet(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements);

/** @brief Converts current snap shot of energy consumed by the meter in Wh.
 * @details Must call LMA_EnergyGet first.
 * @param[in] p_se - pointer to the system energy structure (containing system energy counters)
 * @param[out] p_ec - pointer to the energy consumption structure to populate.
 */
void LMA_ConsumptionDataGet(const LMA_SystemEnergy *const p_se, LMA_ConsumptionData *const p_ec);

/** @brief Checks whether measurements are ready.
 * @param p_phase - pointer to the phase to check.
 * @return true if new measurements are ready, false otherwise.
 */
bool LMA_MeasurementsReady(LMA_Phase *const p_phase);

/** @} */

/** @} */

/** @addtogroup Callbacks
 * @brief LMA Interrupt Callbacks
 * @details The LMA Callbacks are the foundation of LMA's operation, these callbacks are installed in the appropraite interrupts
 * service routines (ISR's) and invoke LMA's core operation.
 *  @{
 */

/** @brief ADC CALLBACK - Processes the ADC samples according to the number of phases registered.
 */
void LMA_CB_ADC(void);

/** @brief TMR CALLBACK - 10ms periodic timer - processes the accumulated ADC values as accumulated by the ADC CB and computes
 * the measured parameters.
 */
void LMA_CB_TMR(void);

/** @brief RTC CALLBACK - Process periodic rtc interrupt.
 */
void LMA_CB_RTC(void);

/** @} */

#endif /* _LMA_CORE_H */
