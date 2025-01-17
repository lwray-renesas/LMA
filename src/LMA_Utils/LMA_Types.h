#ifndef _LMA_TYPES_H
#define _LMA_TYPES_H

#include "LMA_Port.h"

/** @brief enumerated tye for signalling the calibration functions to be performed.
 * @details can be OR'd together to perform multiple at a time
 */
typedef enum Calibration_flags
{
  SAMPLING_FREQUENCY = 0x00000001ul, /**< Calibrates sampling frequency */
  PHASE_PAIR = 0x00000002ul,         /**< Calibration of LMA_PhaseCalibration struct members */
  NEUTRAL_IRMS = 0x00000004ul, /**< Calibration of IRMS Coefficient for global neutral channel (useful for 3PH 4W systems) */
} Calibration_flags;

/** @brief dedicated struct for a sample set */
typedef struct Samples
{
  spl_t voltage;   /**< Voltage ADC sample */
  spl_t current;   /**< Current ADC sample */
  spl_t voltage90; /**< Sample used to contain the phase shifted voltage signal */
} Samples;

/** @brief Runtime configuration data of the library*/
typedef struct LMA_Config
{
  uint32_t update_interval;        /**< Number of V line cycles to between comutation updates. */
  param_t target_system_frequency; /**< Frequency of the target system */
  param_t meter_constant;          /**< Ws/imp ... translated Ws/imp = 3,600,000 / [imp/kwh]*/
} LMA_Config;

/** @brief zero cross detection struct */
typedef struct LMA_ZeroCross
{
  uint32_t zero_cross_counter; /**< running counter to count the number of zero cross */
  spl_t last_voltage_sample;   /**< Last voltage sample to check for zero cross */
  bool zero_cross_debounce;    /**< zerocross debounce flag - disables zerocross after one has been detected for a single
                                  sample */
  bool v_sync_zc;              /**< flag indicating we have already detected a zero cross (synch'd) */

} LMA_ZeroCross;

/** @brief Data related to voltage signal processing */
typedef struct LMA_Voltage
{
  acc_t v_acc;               /**< Accumulated voltage signal */
  param_t v_rms;             /**< Temporary value to store voltage computation [V] */
  uint32_t fline_acc;        /**< accumulating counter to compute the line frequency */
  param_t fline;             /**< Computed line frequency [Hz]*/
  uint32_t v_sample_counter; /**< counter to count the number of samples on the voltage*/
  LMA_ZeroCross v_zc;        /**< Zero cross tracking variables for voltage */
} LMA_Voltage;

/** @brief Data related to current signal processing */
typedef struct LMA_Current
{
  acc_t i_acc;   /**< Accumulated current signal */
  param_t i_rms; /**< Temporary value to store current computation */
} LMA_Current;

/** @brief Data related to power (P, Q & S) signal processing */
typedef struct LMA_Power
{
  acc_t p_acc; /**< Accumulated active power signal */
  param_t p;   /**< Temporary value to store active power consumption */
  acc_t q_acc; /**< Accumulated reactive power signal */
  param_t q;   /**< Temporary value to store reactive power consumption */
  acc_t s_acc; /**< Accumulated apparent power signal [UNUSED]*/
  param_t s;   /**< Temporary value to store apparent power consumption */
} LMA_Power;

/** @brief Data related to energy computations */
typedef struct LMA_Energy
{
  /** @brief Variables holding currently computed unit of enery per ADC interval */
  struct unit
  {
    param_t act;
    param_t react;
    param_t app;
  } unit;

  /** @brief Running energy accumulator for counting energy between pulses - Ws (Watt second)*/
  struct accumulator
  {
    param_t act_imp_ws;     /**< Variable used to accumulate the active import (from grid) energy over meter lifetime*/
    param_t act_exp_ws;     /**< Variable used to accumulate the active export (to grid) energy over meter lifetime*/
    param_t c_react_imp_ws; /**< Variable used to accumulate the C reactive import (from grid) energy over meter lifetime*/
    param_t c_react_exp_ws; /**< Variable used to accumulate the C reactive export (to grid) energy over meter lifetime*/
    param_t l_react_imp_ws; /**< Variable used to accumulate the L reactive import (from grid) energy over meter lifetime*/
    param_t l_react_exp_ws; /**< Variable used to accumulate the L reactive export (to grid) energy over meter lifetime*/
    param_t app_imp_ws;     /**< Variable used to accumulate the apparent import (from grid) energy over meter lifetime*/
    param_t app_exp_ws;     /**< Variable used to accumulate the apparent export (to grid) energy over meter lifetime*/
  } accumulator;

  /** @brief Total energy measured by meter in units of energy (pulses or kwh/imp)*/
  struct counter
  {
    uint64_t act_imp;     /**< Variable used to accumulate units of energy (pulses) (active import ... from grid) */
    uint64_t act_exp;     /**< Variable used to accumulate units of energy (pulses) (active export ... to grid) */
    uint64_t c_react_imp; /**< Variable used to accumulate units of energy (pulses) (C reactive import ... from grid) */
    uint64_t c_react_exp; /**< Variable used to accumulate units of energy (pulses) (C reactive export ... to grid) */
    uint64_t l_react_imp; /**< Variable used to accumulate units of energy (pulses) (L reactive import ... from grid) */
    uint64_t l_react_exp; /**< Variable used to accumulate units of energy (pulses) (L reactive export ... to grid) */
    uint64_t app_imp;     /**< Variable used to accumulate units of energy (pulses) (active import ... from grid) */
    uint64_t app_exp;     /**< Variable used to accumulate units of energy (pulses) (active export ... to grid) */
  } counter;
} LMA_Energy;

/** @brief Data related to impulse controls */
typedef struct LMA_Impulse
{
  void (*Active_imp_on)(void);    /**< Callback to turn on active impulse LED */
  void (*Active_imp_off)(void);   /**< Callback to turn off active impulse LED */
  void (*Reactive_imp_on)(void);  /**< Callback to turn on reactive impulse LED */
  void (*Reactive_imp_off)(void); /**< Callback to turn off reactive impulse LED */
  void (*Apparent_imp_on)(void);  /**< Callback to turn on apparent impulse LED */
  void (*Apparent_imp_off)(void); /**< Callback to turn off apparent impulse LED */
} LMA_Impulse;

/** @brief Data related to the calibration of the project */
typedef struct LMA_GlobalCalibration
{
  param_t fs;          /**< Sampling frequency*/
  param_t fline_coeff; /**< Line Frequency Coefficient*/
} LMA_GlobalCalibration;

/** @brief Data related to the calibration of a phase pair */
typedef struct LMA_PhaseCalibration
{
  param_t vrms_coeff;          /**< Vrms coeffcicent*/
  param_t irms_coeff;          /**< Irms coeffcicent*/
  param_t vi_phase_correction; /**< V-I Phase Correction*/
  param_t p_coeff;             /**< Power coeffcicent*/
} LMA_PhaseCalibration;

/** @brief Data related to the calibration of neutral only curernt channel (3PH4W connection only) */
typedef struct LMA_NeutralCalibration
{
  param_t irms_coeff; /**< Irms coeffcicent*/
} LMA_NeutralCalibration;

/** @brief Data related to phase (V & I pair) signal processing */
typedef struct LMA_Phase
{
  LMA_PhaseCalibration calib;            /**< Instance of the phases calibration data block */
  LMA_Voltage voltage;                   /**< Voltage data processing block */
  LMA_Current current;                   /**< Current data processing block */
  LMA_Power power;                       /**< Power data processing block */
  LMA_Energy energy;                     /**< Energy processing block */
  LMA_Impulse impulse;                   /**< Impulse output control block */
  LMA_GlobalCalibration *const p_gcalib; /**< Pointer to the global calibration data block */
  bool calibrating;                      /**< Flag to indicate we are in calibration mode */
  bool calibrating_fs;                   /**< Flag to indicate we are in calibration mode */
  bool disable_acc;                      /**< Flag to disable accumulation*/
} LMA_Phase;

/** @brief Data related to neutral only signal processing (3PH4W connection only) */
typedef struct LMA_Neutral
{
  LMA_NeutralCalibration calib;          /**< Instance of the neutral calibration data block */
  LMA_GlobalCalibration *const p_gcalib; /**< Pointer to the global calibration data block */
  LMA_Current *const current;            /**< Pointer to current data processing block */
  LMA_Voltage *const voltage;            /**< Pointer to voltage data processing block (for synch to zero cross)*/
} LMA_Neutral;

/** @brief dedicated struct for calibration arguments */
typedef struct Calibration_args
{
  Calibration_flags flags;         /**< flags describing the calibration to perform */
  LMA_GlobalCalibration *p_gcalib; /**< Pointer to the global calibation data structure */
  LMA_Phase *p_phase;              /**< Pointer to a phases targetting calibration */
  param_t vrms_tgt;                /**< Vrms target calibration voltage */
  param_t irms_tgt;                /**< Irms target calibration current */
  param_t p_tgt;                   /**< Active target calibration power*/
  param_t rtc_period;              /**< period of the RTC for fs calibration */
  uint32_t rtc_cycles;             /**< number of RTC periods to accumulate for frequency calibration */
  uint32_t line_cycles;            /**< Line cycles to stabilise readings over */
} Calibration_args;

/** @brief Convenience type to get a full measurement dump */
typedef struct LMA_Measurements
{
  param_t vrms;  /**< RMS Voltage */
  param_t irms;  /**< RMS Current */
  param_t fline; /**< Line Frequency */
  param_t p;     /**< Active Power */
  param_t q;     /**< Rective Power */
  param_t s;     /**< Apparent Power */
} LMA_Measurements;

#endif /* _LMA_TYPES_H */
