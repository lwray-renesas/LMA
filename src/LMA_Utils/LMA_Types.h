/**
 * @file LMA_Types.h
 * @brief Type definitions for LMA.
 *
 * @details This file defines/declares the type definitions of the LMA codebase.
 */

#ifndef _LMA_TYPES_H
#define _LMA_TYPES_H

#include "LMA_Port.h"

/** @addtogroup API
 *  @{
 */

/** @addtogroup Types
 * @brief LMA Types
 * @details The LMA API relies on a set of unique data structure, enum's and other defined types to operate, here these are
 * outlined.
 *  @{
 */

/**
 * @brief Phase status
 * @details Enumerated type to indicate events detected per phase
 */
typedef enum LMA_Status_e
{
  LMA_OK = 0,               /**< No Problems */
  LMA_NO_ACTIVE_LOAD = 1,   /**< No Active Power (P < LMA_Config.no_load_p) */
  LMA_NO_REACTIVE_LOAD = 2, /**< No Reactive Power (Q < LMA_Config.no_load_p) */
  LMA_NO_APPARENT_LOAD = 4, /**< No Apparent Power (S < LMA_Config.no_load_p) */
  LMA_VOLTAGE_SAG = 8,      /**< Vrms Sagged (Vrms < LMA_Config.v_sag) */
  LMA_VOLTAGE_SWELL = 16    /**< Vrms Swelled (Vrms > LMA_Config.v_swell) */
} LMA_Status;

/**
 * @brief Calibration status
 * @details Enumerated type to indicate calibration status.
 */
typedef enum LMA_CalibrationStatus_e
{
  LMA_CALIB_OK = 0,            /**< Calibration ended sucessfully*/
  LMA_CALIB_PHASE_ANGLE_ERROR, /**< The phase difference between the current and voltage is too large. Suspect non UPF load*/
} LMA_CalibrationStatus;

/**
 * @brief Zero cross detection data
 * @details Data structure containing all parameters for use in zero-cross detection on AC voltage line.
 */
typedef struct LMA_ZeroCross_str
{
  uint32_t zero_cross_counter; /**< running counter to count the number of zero cross */
  spl_t last_sample;           /**< Last voltage sample to check for zero cross */
  bool zero_cross_debounce;    /**< zerocross debounce flag*/
  bool sync_zc;                /**< flag indicating we have already detected a zero cross (synch'd) */
} LMA_ZeroCross;

/**
 * @brief Voltage computation data
 * @details Data structure containing all parameters for use in computing/computed AC voltage parameters.
 */
typedef struct LMA_Voltage_str
{
  float v_rms;      /**< Temporary value to store voltage computation [V] */
  float fline;      /**< Computed line frequency [Hz]*/
  LMA_ZeroCross zc; /**< Zero cross tracking variables for voltage */
} LMA_Voltage;

/**
 * @brief Current computation data
 * @details Data structure containing all parameters for use in computing/computed AC current parameters.
 */
typedef struct LMA_Current_str
{
  float i_rms; /**< Temporary value to store current computation */
} LMA_Current;

/**
 * @brief Power computation data
 * @details Data structure containing all parameters for use in computing/computed AC power parameters (P, Q & S).
 */
typedef struct LMA_Power_str
{
  float p; /**< Variable to store active power consumption */
  float q; /**< Variable to store reactive power consumption */
  float s; /**< Variable to store apparent power consumption */
} LMA_Power;

/**
 * @brief Energy computation data
 * @details Data structure containing all parameters for use in computing/computed AC energy consumption parameters.
 */
typedef struct LMA_ConsumptionData_str
{
  float act_imp_energy_wh; /**< active energy imported in Wh*/
  float act_exp_energy_wh; /**< active energy exported in Wh*/
  float app_imp_energy_wh; /**< apparent energy imported in Wh*/
  float app_exp_energy_wh; /**< apparent energy exported in Wh*/
  float c_imp_energy_wh;   /**< reactive (capacitive) energy imported in Wh*/
  float c_exp_energy_wh;   /**< reactive (capacitive) energy exported in Wh*/
  float l_imp_energy_wh;   /**< reactive (inductive) energy imported in Wh*/
  float l_exp_energy_wh;   /**< reactive (inductive) energy exported in Wh*/
} LMA_ConsumptionData;

/**
 * @brief Energy unit data
 * @details Data structure containing all computed energy units per ADC interval (as of last computation update).
 */
typedef struct LMA_EnergyUnit_str
{
  float act;   /**< Currently computed unit of active energy per ADC interval*/
  float app;   /**< Currently computed unit of apparent energy per ADC interval*/
  float react; /**< Currently computed unit of reactive energy per ADC interval*/
} LMA_EnergyUnit;

/**
 * @brief Global/System calibration data
 * @details Data structure containing system wide calibration parameters (i.e., not per phase).
 */
typedef struct LMA_GlobalCalibration_str
{
  float fs;             /**< Sampling frequency*/
  float fline_coeff;    /**< Line Frequency Coefficient*/
  float deg_per_sample; /**< degrees per sample*/
} LMA_GlobalCalibration;

/**
 * @brief Phase calibration data
 * @details Data structure containing per phase calibration data.
 */
typedef struct LMA_PhaseCalibration_str
{
  float vrms_coeff;          /**< Vrms coefficient*/
  float irms_coeff;          /**< Irms coefficient*/
  float vi_phase_correction; /**< V-I Phase Correction (I relative to V i.e., I lags V = negative, I leads V = positive)*/
  float p_coeff;             /**< Power coefficient*/
} LMA_PhaseCalibration;

/**
 * @brief Phase-angle error calibration data
 * @details Data structure containing per phase phase-angle computation data.
 */
typedef struct LMA_PhaseAngleError_str
{
  LMA_CalibrationStatus status; /**< Status of the phase angle computation*/
  spl_t v_last_sample;          /**< Storing previous samples of voltage for simplified zero cross*/
  spl_t i_last_sample;          /**< Storing previous samples of current for simplified zero cross*/
  uint32_t sample_counter;      /**< counter to track the number of phase angle computations taken for averaging*/
  float v_fraction;             /**< the fractional component of the zero cross on the voltage (Q32.32 format)*/
  float i_fraction;             /**< the fractional component of the zero cross on the current (Q32.32 format)*/
} LMA_PhaseAngleError;

/**
 * @brief Signal/flag data
 * @details Data structure containing signals/flags for indicating events between LMA components.
 */
typedef struct LMA_Signals_str
{
  bool accumulators_ready;    /**< Flag to indicate our accumulators are ready for update */
  bool measurements_ready;    /**< Flag to indicate a new measurement set is ready */
  bool calibrate_angle_error; /**< Flag to indicate we are calibrating the phase angle error*/
} LMA_Signals;

/**
 * @brief Phase data
 * @details Data structure containing phase (V & I pair) signal processing parameters.
 */
typedef struct LMA_Phase_str
{
  struct LMA_Phase_str *p_next; /**< Forms singly linked list of phases (null terminated) */
  LMA_PhaseCalibration calib;   /**< Instance of the phases calibration data block */
  LMA_PhaseAngleError pa_error; /**< phase angle error data*/
  LMA_Voltage voltage;          /**< Voltage data processing block */
  LMA_Current current;          /**< Current data processing block */
  LMA_Power power;              /**< Power data processing block */
  LMA_EnergyUnit energy_units;  /**< Energy processing block */
  LMA_Workspace ws;             /**< Porting glue - contains samples!*/
  LMA_Accumulators accs;        /**< snapshot of last updated accumulators*/
  LMA_Status status;            /**< Phase status */
  LMA_Signals sigs;             /**< Phase signals */
  uint32_t phase_number;        /**< zero indexed phase number for identification*/
} LMA_Phase;

/**
 * @brief Energy data
 * @details Data structure containing system energy computations and impulse control parameters.
 */
typedef struct LMA_SystemEnergy_str
{
  /** @brief Energy related data */
  struct energy
  {
    /** @brief Currently computed unit of energy per ADC interval of whole system*/
    struct unit
    {
      float act;   /**< Currently computed unit of active energy per ADC interval*/
      float app;   /**< Currently computed unit of apparent energy per ADC interval*/
      float react; /**< Currently computed unit of reactive energy per ADC interval*/
    } unit;

    /** @brief Running energy accumulators for counting energy between pulses - Ws (Watt second)*/
    struct accumulator
    {
      float act_imp_ws;     /**< Variable used to accumulate the active import (from grid) energy*/
      float act_exp_ws;     /**< Variable used to accumulate the active export (to grid) energy*/
      float app_imp_ws;     /**< Variable used to accumulate the apparent import (from grid) energy*/
      float app_exp_ws;     /**< Variable used to accumulate the apparent export (to grid) energy*/
      float c_react_imp_ws; /**< Variable used to accumulate the C reactive import (from grid) energy*/
      float c_react_exp_ws; /**< Variable used to accumulate the C reactive export (to grid) energy*/
      float l_react_imp_ws; /**< Variable used to accumulate the L reactive import (from grid) energy*/
      float l_react_exp_ws; /**< Variable used to accumulate the L reactive export (to grid) energy*/
    } accumulator;

    /** @brief Total energy measured by meter in units of energy (pulses or kwh/imp)*/
    struct counter
    {
      uint64_t act_imp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (active import ... from
                           grid) */
      uint64_t
          act_exp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (active export ... to grid) */
      uint64_t app_imp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (apparent import ... from
                           grid) */
      uint64_t app_exp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (apparent export ... to
                           grid) */
      uint64_t c_react_imp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (C reactive import ...
                               from grid) */
      uint64_t c_react_exp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (C reactive export ...
                               to grid) */
      uint64_t l_react_imp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (L reactive import ...
                               from grid) */
      uint64_t l_react_exp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (L reactive export ...
                               to grid) */
    } counter;
  } energy;

  /** @brief Data related to impulse controls */
  struct impulse
  {
    uint32_t led_on_count;     /**< Number of ADC intervals for the LED on count */
    uint32_t active_counter;   /**< Counter used to track on time of the active LED. */
    uint32_t apparent_counter; /**< Counter used to track on time of the apparent LED. */
    uint32_t reactive_counter; /**< Counter used to track on time of the reactive LED. */
    bool active_on;            /**< Flag indicating the active led is on */
    bool apparent_on;          /**< Flag indicating the apparent led is on */
    bool reactive_on;          /**< Flag indicating the reactive led is on */
  } impulse;
} LMA_SystemEnergy;

/**
 * @brief Calibration arguments (per phase)
 * @details Data structure containing calibration invocation arguments/parameters for per phase calibration.
 */
typedef struct LMA_PhaseCalibArgs_str
{
  LMA_Phase *p_phase;   /**< Pointer to a phases targetting calibration */
  float vrms_tgt;       /**< Vrms target calibration voltage */
  float irms_tgt;       /**< Irms target calibration current */
  uint32_t line_cycles; /**< Line cycles to stabilise readings over */
} LMA_PhaseCalibArgs;

/**
 * @brief Calibration arguments (global)
 * @details Data structure containing calibration invocation arguments/parameters for global calibration.
 */
typedef struct LMA_GlobalCalibArgs_str
{
  float rtc_period;    /**< period of the RTC for fs calibration */
  uint32_t rtc_cycles; /**< number of RTC periods to accumulate for frequency calibration */
} LMA_GlobalCalibArgs;

/**
 * @brief Runtime LMA Configuration
 * @details Data structure containing the runtime configuration of LMA system wide operation.
 */
typedef struct LMA_Config_str
{
  LMA_GlobalCalibration gcalib; /**< Global calibration data block */
  uint32_t update_interval;     /**< Number of V line cycles to between computation updates. */
  float fline_target;           /**< Frequency of the target system */
  float fline_tol_low;          /**< Lower tolerance of system frequency*/
  float fline_tol_high;         /**< Upper tolerance of system frequency*/
  float meter_constant;         /**< Ws/imp ... translated Ws/imp = 3,600,000 / [imp/kwh]*/
  float no_load_i;              /**< No load current value */
  float no_load_p;              /**< No active/reactive power load value */
  float v_sag;                  /**< Voltage sag value */
  float v_swell;                /**< Voltage swell value */
} LMA_Config;

/** @brief  */

/**
 * @brief Measurement output
 * @details Convenience data structure enabling dump of all measurement parameters.
 */
typedef struct LMA_Measurements_str
{
  float vrms;  /**< RMS Voltage */
  float irms;  /**< RMS Current */
  float fline; /**< Line Frequency */
  float p;     /**< Active Power */
  float q;     /**< Reactive Power */
  float s;     /**< Apparent Power */
} LMA_Measurements;

/** @} */

/** @} */

#endif /* _LMA_TYPES_H */
