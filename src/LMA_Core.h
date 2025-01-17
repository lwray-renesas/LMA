#ifndef _LMA_CORE_H
#define _LMA_CORE_H

#include "LMA_Types.h"

/*****************************
* Control API
*****************************/
/** @brief Initalises the Light-Weight Metrology for AC Framework according to the config.
* @param[in] p_config - pointer to the configuration structure.
*/
void LMA_Init(const LMA_Config *const p_config);

/** @brief Initalises a phase ready for computations.
* @param[in] p_phase - pointer to the phase
*/
void LMA_PhaseInit(LMA_Phase *const p_phase);

/** @brief Starts the metering state machine and all associated drivers.
*/
void LMA_Start(void);

/** @brief Starts the metering state machine and all associated drivers.
*/
void LMA_Stop(void);

/** @brief Performs a calibration command according to the flags passed.
* @param[in] calib_args - Arguments and data structure use for a calibration.
*/
void LMA_Calibrate(Calibration_args * const calib_args);


/*****************************
* Measurement API
*****************************/
/** @brief Returns VRMS of particular phase
* @param[inout] p_phase - pointer to the phase block on which to get vrms from.
* @return phase vrms
*/
param_t LMA_VRMS_Get(const LMA_Phase *const p_phase);

/** @brief Returns IRMS of particular phase
* @param[inout] p_phase - pointer to the phase block on which to get irms from.
* @return phase irms
*/
param_t LMA_IRMS_Get(const LMA_Phase *const p_phase);

/** @brief Returns Line Frequency of particular phase
* @param[inout] p_phase - pointer to the phase block on which to get fline from.
* @return phase line frequency
*/
param_t LMA_FLine_Get(const LMA_Phase *const p_phase);

/** @brief Returns Active Power of particular phase
* @param[inout] p_phase - pointer to the phase block on which to get active power from.
* @return phase active power
*/
param_t LMA_ActivePower_Get(const LMA_Phase *const p_phase);

/** @brief Returns Reactive Power of particular phase
* @param[inout] p_phase - pointer to the phase block on which to get reactive power from.
* @return phase reactive power
*/
param_t LMA_ReactivePower_Get(const LMA_Phase *const p_phase);

/** @brief Returns Apparent Power of particular phase
* @param[inout] p_phase - pointer to the phase block on which to get apparent power from.
* @return phase apparent power
*/
param_t LMA_ApparentPower_Get(const LMA_Phase *const p_phase);

/** @brief Outputs current sna shot of measurement set
* @param[inout] p_phase - pointer to the phase block on which to get measurements from.
* @param[out] p_measurements - pointer to the measurement structure to populate.
*/
void LMA_Measurements_Get(LMA_Phase *const p_phase, LMA_Measurements *const p_measurements);

/*****************************
* DRIVER CALLBACKS
*****************************/
/** @brief ADC CALLBACK - Processes the phase pair computations according to the new samples.
* @param[inout] p_phase - pointer to the phase block on which to operate the samples on.
* @param[in] p_adc_samples - pointer to the adc samples.
*/
void LMA_CB_ADC_Phase(LMA_Phase *const p_phase, const Samples * p_adc_samples);

/** @brief RTC CALLBACK - Process periodic rtc interrupt.
* @details The RTC isr callling this should deally have nested interrupts enabled in which the ADC can interrupt us.
*/
void LMA_CB_RTC(void);

#endif /* _LMA_CORE_H */