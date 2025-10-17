/**
 * @addtogroup Porting
 * @{
 *
 * @file LMA_Types.h
 * @brief Type definitions for LMA.
 *
 * @details This file defines/declares the type definitions of the LMA codebase.
 *
 * @}
 */

#ifndef _LMA_TYPES_H
#define _LMA_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @addtogroup Porting
 *  @{
 */

/** @brief Raw ADC sample type
 * @details This type should accomodate the raw ADC sample type.
 */
typedef int32_t spl_t;

/** @brief Accumulator type
 * @details This type should accomodate the accumulation of the product of raw ADC sample types.
 Generally a good idea to be double the bit width of spl_t.
 */
typedef int64_t acc_t;

/** @}*/

/** @addtogroup API
 *  @{
 */

/** @addtogroup Types
 * @brief LMA Types
 * @details The LMA API relies on a set of unique data structure, enum's and other defined types to operate, here these are
 * outlined.<br>
 * Checkout the \ref Storage topic to see what types are particularly useful for non-volatile storage.
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
 * @brief Structure for defining the inputs to a phase object in terms of samples.
 * @details Use this structure to load samples in a phase object before calling LMA_CB_ADC
 */
typedef struct LMA_PhaseInputs_str
{
  spl_t v_sample;   /**< Raw ADC Voltage Sample*/
  spl_t v90_sample; /**< 90 degreephase shifted ADC Voltage Sample*/
  spl_t i_sample;   /**< Raw ADC Current Sample*/
} LMA_PhaseInputs;

/**
 * @brief Structure for defining the inputs to a neutral object in terms of samples.
 * @details Use this structure to load samples in a neutral object before calling LMA_CB_ADC
 */
typedef struct LMA_NeutralInputs_str
{
  spl_t i_sample; /**< Raw ADC Current Sample*/
} LMA_NeutralInputs;

/**
 * @brief General Accumulator structure
 * @details Data structure containing all accumulators in a phase.
 */
typedef struct LMA_Accs_str
{
  acc_t v_acc;           /**< Voltage accumulator*/
  acc_t i_acc;           /**< Current accumulator*/
  acc_t p_acc;           /**< Active power accumulator*/
  acc_t q_acc;           /**< Reactive power accumulator*/
  uint32_t sample_count; /**< Sample counter, used to track number of samples during accumulation period.*/
} LMA_Accs;

/**
 * @brief Phase accumulators
 * @details Data structure containing all accumulators for use in phase computations.
 */
typedef struct LMA_PhaseAccs_str
{
  LMA_Accs temp;     /**< Object holding running accumulators*/
  LMA_Accs snapshot; /**< Object holding snapshot of accumulators after computation window finished*/
} LMA_PhaseAccs;

/**
 * @brief Neutral accumulators
 * @details Data structure containing all accumulators for use in neutral computations.
 */
typedef struct LMA_NeutralAccs_str
{
  acc_t i_acc_temp;     /**< Running current accumulator*/
  acc_t i_acc_snapshot; /**< Snapshot of current accumulator after computation window finished*/
} LMA_NeutralAccs;

/**
 * @brief Zero cross detection data
 * @details Data structure containing all parameters for use in zero-cross detection on AC voltage line.
 */
typedef struct LMA_ZeroCross_str
{
  uint32_t count;    /**< running counter to count the number of zero cross */
  spl_t last_sample; /**< Tracked/filtered voltage */
  bool debounce;     /**< zerocross debounce flag*/
  bool first_event;  /**< flag indicating we have already detected a zero cross (synch'd) */
} LMA_ZeroCross;

/**
 * @brief Measurement output
 * @details Convenience data structure to store snapshot of measurements.
 */
typedef struct LMA_Measurements_str
{
  float vrms;         /**< RMS Voltage */
  float irms;         /**< RMS Current */
  float irms_neutral; /**< RMS Current (Neutral Sensing) */
  float fline;        /**< Line Frequency */
  float p;            /**< Active Power */
  float q;            /**< Reactive Power */
  float s;            /**< Apparent Power */
} LMA_Measurements;

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

/** @addtogroup Storage
 * @brief LMA Storage Data
 * @details Some LMA API types are designed to be stored in non volatile memory for backup purposes, this top group outlines sub
 * groups containing types which may be useful to store.
 *  @{
 */

/** @addtogroup Calibration
 * @brief LMA Calibration Data
 * @details The calibration data is is subject to change on command during manufacture or maintenance, the types outlined here
 * contain calibration data.
 *  @{
 */

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
 * @brief Neutral calibration data
 * @details Data structure containing neutral calibration data
 */
typedef struct LMA_NeutralCalibration_str
{
  float irms_coeff; /**< Irms coefficient (Neutral channel)*/
} LMA_NeutralCalibration;

/** @} */

/** @} */

/**
 * @brief Phase-angle error calibration data
 * @details Data structure containing per phase phase-angle computation data.
 */
typedef struct LMA_PhaseAngleError_str
{
  LMA_CalibrationStatus status; /**< Status of the phase angle computation*/
  LMA_ZeroCross v_zero_cross;   /**< Zero cross structure for voltage signal*/
  LMA_ZeroCross i_zero_cross;   /**< Zero cross structure for current signal*/
  uint32_t sample_counter;      /**< counter to track the number of phase angle computations taken for averaging*/
  float v_fraction;             /**< the fractional component of the zero cross on the voltage */
  float i_fraction;             /**< the fractional component of the zero cross on the current */
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
 * @brief Neutral data
 * @details Data structure neutral only signal processing parameters.
 */
typedef struct LMA_Neutral_str
{
  LMA_NeutralInputs inputs;     /**< Area to load inputs (ADC Samples) for processing */
  LMA_NeutralAccs accs;         /**< Object holding accumulator data*/
  LMA_NeutralCalibration calib; /**< Instance of the neautrals calibration data block */
} LMA_Neutral;

/**
 * @brief Phase data
 * @details Data structure containing phase (V & I pair) signal processing parameters.
 */
typedef struct LMA_Phase_str
{
  struct LMA_Phase_str *p_next;  /**< Forms singly linked list of phases (null terminated) */
  LMA_PhaseInputs inputs;        /**< Area to load inputs (ADC Samples) for processing */
  LMA_PhaseAccs accs;            /**< Object holding accumulator data*/
  LMA_ZeroCross zero_cross;      /**< Zero cross tracking variables for voltage */
  LMA_PhaseCalibration calib;    /**< Instance of the phases calibration data block */
  LMA_PhaseAngleError pa_error;  /**< phase angle error data*/
  LMA_Measurements measurements; /**< Object holding measurements from last computation window update*/
  LMA_EnergyUnit energy_units;   /**< Energy processing block */
  LMA_Status status;             /**< Phase status */
  LMA_Signals sigs;              /**< Phase signals */
  LMA_Neutral *p_neutral;        /**< Pointer to neutral channel (if present)*/
  float (*p_computation_hook)(float *i, float *v,
                              float *f); /**< Hook to enable applying a compensation factor to power based on i, v and f args*/
  uint32_t phase_number;                 /**< zero indexed phase number for identification*/
} LMA_Phase;

/** @addtogroup Storage
 *  @{
 */

/** @addtogroup Logging
 * @brief LMA Logging Data
 * @details Logging data is data which may be required to be periodically stored in non-volatile memory, this group outlines
 * those types.
 *  @{
 */

/**
 * @brief Energy data
 * @details Data structure containing system energy computations and impulse control parameters.
 */
typedef struct LMA_SystemEnergy_str
{
  /** @brief Energy related data */
  struct energy
  {
    /** @brief Currently computed units of energy per ADC interval of whole system*/
    struct unit
    {
      float act;   /**< Currently computed unit of active energy per ADC interval*/
      float app;   /**< Currently computed unit of apparent energy per ADC interval*/
      float react; /**< Currently computed unit of reactive energy per ADC interval*/
    } unit;        /**< structure containing energy units per ADC*/

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
    } accumulator;          /**< structure containing energy accumulators*/

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
    } counter;              /**< structure containing energy counters*/
  } energy;                 /**< structure containing system energy data*/

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
  } impulse;                   /**< structure containing energy impulse controls*/
} LMA_SystemEnergy;

/** @} */

/** @} */

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
  float fline_target;  /**< Target line frequency */
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
  float fline_tol_low;          /**< Lower tolerance of system frequency*/
  float fline_tol_high;         /**< Upper tolerance of system frequency*/
  float meter_constant;         /**< Ws/imp ... translated Ws/imp = 3,600,000 / [imp/kwh]*/
  float no_load_i;              /**< No load current value */
  float no_load_p;              /**< No active/reactive power load value */
  float v_sag;                  /**< Voltage sag value */
  float v_swell;                /**< Voltage swell value */
} LMA_Config;

/** @} */

/** @} */

#endif /* _LMA_TYPES_H */
