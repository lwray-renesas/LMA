/**
 * @file LMA_Core.h
 * @brief Core functionality declarations for LMA.
 *
 * @details This file includes LMA_Utils and provides declarations of the LMA core functionality.
 */

#ifndef _LMA_CORE_H
#define _LMA_CORE_H

#include "LMA_Utils.h"

/** @addtogroup API
 * @brief
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

/** @brief Registers a phase to the library - once on power up.
 * @param[in] p_phase - pointer to the phase
 */
void LMA_PhaseRegister(LMA_Phase *const p_phase);

/** @brief Loads calibration data to a phase.
 * @param[inout] p_phase - pointer to the phase
 * @param[in] p_calib - pointer to the calibration data to load.
 */
void LMA_PhaseLoadCalibration(LMA_Phase *const p_phase, const LMA_PhaseCalibration *const p_calib);

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

/** @brief Returns VRMS of particular phase
 * @param[inout] p_phase - pointer to the phase block on which to get vrms from.
 * @return phase vrms
 */
float LMA_VrmsGet(LMA_Phase *const p_phase);

/** @brief Returns IRMS of particular phase
 * @param[inout] p_phase - pointer to the phase block on which to get irms from.
 * @return phase irms
 */
float LMA_IrmsGet(LMA_Phase *const p_phase);

/** @brief Returns Line Frequency of particular phase
 * @param[inout] p_phase - pointer to the phase block on which to get fline from.
 * @return phase line frequency
 */
float LMA_FLineGet(LMA_Phase *const p_phase);

/** @brief Returns Active Power of particular phase
 * @param[inout] p_phase - pointer to the phase block on which to get active power from.
 * @return phase active power
 */
float LMA_ActivePowerGet(const LMA_Phase *const p_phase);

/** @brief Returns Reactive Power of particular phase
 * @param[inout] p_phase - pointer to the phase block on which to get reactive power from.
 * @return phase reactive power
 */
float LMA_ReactivePowerGet(const LMA_Phase *const p_phase);

/** @brief Returns Apparent Power of particular phase
 * @param[inout] p_phase - pointer to the phase block on which to get reactive power from.
 * @return phase apparent power
 */
float LMA_ApparentPowerGet(const LMA_Phase *const p_phase);

/** @brief Outputs current snap shot of measurement set
 * @param[inout] p_phase - pointer to the phase block on which to get measurements from.
 * @param[out] p_measurements - pointer to the measurement structure to populate.
 */
void LMA_MeasurementsGet(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements);

/** @brief Converts current snap shot of energy consumed by the meter in Wh.
 * @details Must call LMA_EnergyGet first.
 * @param[in] p_ec - pointer to the energy consumption structure to populate.
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
 * @details - The ADC Callback handles:
 *  1. Sample processing and accumulation.
 *  2. Energy Impulse Management.
 *
 *  It does not update the measured parameters - this is done in the TMR callback.
 */
void LMA_CB_ADC(void);

/** @brief TMR CALLBACK - 10ms periodic timer - processes the accumulated ADC values as accumulated by the ADC CB and computes
 * the measured parameters.
 * @details - The TMR Callback computes and updates:
 *  1. Energy consumption in units (how much energy is consumed in Ws/VARs/VAs per ADC interval)
 *  2. Power
 *  3. Voltage
 *  4. Current
 *  5. Frequency
 *
 *  For each phase.
 */
void LMA_CB_TMR(void);

/** @brief RTC CALLBACK - Process periodic rtc interrupt.
 * @details The RTC isr calling this should ideally have nested interrupts enabled in which the ADC can interrupt us.
 * This callback allows us to calibrate sampling frequency.
 */
void LMA_CB_RTC(void);

/** @} */

#endif /* _LMA_CORE_H */
