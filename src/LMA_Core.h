#ifndef _LMA_CORE_H
#define _LMA_CORE_H

#include "LMA_Types.h"

/*****************************
* Control API
*****************************/
/** @brief Initalises the Light-Weight Metrology for AC Framework according to the config.
* @param[in] p_config_arg - pointer to the configuration structure.
*/
void LMA_Init(LMA_Config *const p_config_arg);

/** @brief Registers a phse to the library - once on power up.
* @param[in] p_phase - pointer to the phase
*/
void LMA_PhaseRegister(LMA_Phase *const p_phase);

/** @brief Loads calibration data to a phase.
* @param[inout] p_phase - pointer to the phase
* @param[in] p_calib - pointer to the calibration data to load.
*/
void LMA_PhaseLoadCalibration(LMA_Phase *const p_phase, const LMA_PhaseCalibration *const p_calib);

/** @brief Starts the metering state machine and all associated drivers.
*/
void LMA_Start(void);

/** @brief Starts the metering state machine and all associated drivers.
*/
void LMA_Stop(void);

/** @brief Performs a calibration command according to the flags passed.
* @param[in] calib_args - Arguments and data structure use for a calibration.
*/
void LMA_PhaseCalibrate(LMA_PhaseCalibArgs * const calib_args);

/** @brief Performs a calibration command according to the flags passed.
* @details - results stored in LMA_Config.gcalib
*/
void LMA_GlobalCalibrate(LMA_GlobalCalibArgs * const calib_args);

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

/*****************************
* Measurement API
*****************************/
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

/** @brief Outputs current sna shot of measurement set
* @param[inout] p_phase - pointer to the phase block on which to get measurements from.
* @param[out] p_measurements - pointer to the measurement structure to populate.
*/
void LMA_MeasurementsGet(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements);

/*****************************
* DRIVER CALLBACKS
*****************************/
/** @brief ADC CALLBACK - Processes the ADC samples (assumes a single phase system)
*/
void LMA_CB_ADC_SinglePhase(void);

/** @brief ADC CALLBACK - Processes the ADC samples acording to the number of phases registered (multi phase system)
*/
void LMA_CB_ADC_PolyPhase(void);

/** @brief RTC CALLBACK - Process periodic rtc interrupt.
* @details The RTC isr callling this should deally have nested interrupts enabled in which the ADC can interrupt us.
*/
void LMA_CB_RTC(void);

#endif /* _LMA_CORE_H */
