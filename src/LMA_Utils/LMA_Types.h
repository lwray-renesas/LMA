#ifndef _LMA_TYPES_H
#define _LMA_TYPES_H

#include "LMA_Port.h"

/** @brief enumerated type to indicate events detected per phase*/
typedef enum LMA_Status
{
  LMA_OK = 0,               /**< No Problems */
  LMA_NO_ACTIVE_LOAD = 1,   /**< No Active Power (P < LMA_Config.no_load_p) */
  LMA_NO_REACTIVE_LOAD = 2, /**< No Reactive Power (Q < LMA_Config.no_load_p) */
  LMA_NO_APPARENT_LOAD = 4, /**< No Apparent Power (S < LMA_Config.no_load_p) */
  LMA_VOLTAGE_SAG = 8,      /**< Vrms Sagged (Vrms < LMA_Config.v_sag) */
  LMA_VOLTAGE_SWELL = 16    /**< Vrms Swelled (Vrms > LMA_Config.v_swell) */
} LMA_Status;

/** @brief zero cross detection struct */
typedef struct LMA_ZeroCross
{
  uint32_t zero_cross_counter; /**< running counter to count the number of zero cross */
  spl_t last_voltage_sample;   /**< Last voltage sample to check for zero cross */
  bool zero_cross_debounce;    /**< zerocross debounce flag*/
  bool v_sync_zc;              /**< flag indicating we have already detected a zero cross (synch'd) */
} LMA_ZeroCross;

/** @brief Data related to voltage signal processing */
typedef struct LMA_Voltage
{
  acc_t v_acc;               /**< Accumulated voltage signal */
  float v_rms;             /**< Temporary value to store voltage computation [V] */
  uint32_t fline_acc;        /**< accumulating counter to compute line frequency */
  uint32_t fline_latch;        /**< accumulating counter to compute line frequency */
  float fline;             /**< Computed line frequency [Hz]*/
  uint32_t v_sample_counter; /**< counter to count the number of samples on the voltage*/
  float v_sample_counter_fp; /**< counter to count the number of samples on the voltage converted to float for computations*/
  LMA_ZeroCross v_zc;        /**< Zero cross tracking variables for voltage */
} LMA_Voltage;

/** @brief Data related to current signal processing */
typedef struct LMA_Current
{
  acc_t i_acc;   /**< Accumulated current signal */
  float i_rms; /**< Temporary value to store current computation */
} LMA_Current;

/** @brief Data related to power (P, Q & S) signal processing */
typedef struct LMA_Power
{
  acc_t p_acc; /**< Accumulated active power signal */
  float p;   /**< Variable to store active power consumption */
  acc_t q_acc; /**< Accumulated reactive power signal */
  float q;   /**< Variable to store reactive power consumption */
  acc_ext_t s_acc; /**< Accumulated apparent power signal (unused currently) */
  float s;   /**< Variable to store apparent power consumption */
} LMA_Power;

/** @brief Data related to energy accumulation per phase */
typedef struct LMA_Energy
{
  float act_unit;   /**< Currently computed unit of active energy per ADC interval*/
  float app_unit; /**< Currently computed unit of apparent energy per ADC interval*/
  float react_unit; /**< Currently computed unit of reactive energy per ADC interval*/
} LMA_Energy;

/** @brief Data related to the calibration of the project */
typedef struct LMA_GlobalCalibration
{
  float fs;          /**< Sampling frequency*/
  float deltat;      /**< 1 / Sampling frequency*/
  float fline_coeff; /**< Line Frequency Coefficient*/
  uint32_t fline_acc_tol_low; /**< Low tolerance for valid input AC signal according to accumulator (default = ((fs/fline) * line_count) / 2 )*/
  uint32_t fline_acc_tol_high; /**< High tolerance for valid input AC signal according to accumulator (default = ((fs/fline) * line_count) + ((fs/fline) * line_count) / 2) )*/
} LMA_GlobalCalibration;

/** @brief Data related to the calibration of a phase pair */
typedef struct LMA_PhaseCalibration
{
  float vrms_coeff;          /**< Vrms coeffcicent*/
  float irms_coeff;          /**< Irms coeffcicent*/
  float vi_phase_correction; /**< V-I Phase Correction*/
  float p_coeff;             /**< Power coeffcicent*/
} LMA_PhaseCalibration;

/** @brief Data related to phase (V & I pair) signal processing */
typedef struct LMA_Phase
{
  struct LMA_Phase * p_next;             /**< Forms singly linked list of phases (null terminated) */
  LMA_PhaseCalibration calib;            /**< Instance of the phases calibration data block */
  LMA_Voltage voltage;                   /**< Voltage data processing block */
  LMA_Current current;                   /**< Current data processing block */
  LMA_Power power;                       /**< Power data processing block */
  LMA_Energy energy;                     /**< Energy processing block */
  LMA_Samples samples;                   /**< Sample structure*/
  LMA_Workspace ws;                      /**< Porting glue*/
  LMA_Status status;                     /**< Phase status */
  bool calibrating;                      /**< Flag to indicate we are in calibration mode */
  bool disable_acc;                      /**< Flag to disable accumulation*/
} LMA_Phase;

/** @brief Data related to energy computations and impulse control */
typedef struct LMA_SystemEnergy
{
  /** @brief struct containing all energy related data */
  struct energy
  {
    /** @brief Variables holding currently computed unit of energy per ADC interval of whole system*/
    struct unit
    {
      float act;   /**< Currently computed unit of active energy per ADC interval*/
      float app;   /**< Currently computed unit of apparent energy per ADC interval*/
      float react; /**< Currently computed unit of reactive energy per ADC interval*/
    } unit;

    /** @brief Running energy accumulator for counting energy between pulses - Ws (Watt second)*/
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
      uint64_t act_imp;     /**< Variable used to accumulate units of energy (pulses) over meter lifetime (active import ... from grid) */
      uint64_t act_exp;     /**< Variable used to accumulate units of energy (pulses) over meter lifetime (active export ... to grid) */
      uint64_t app_imp;     /**< Variable used to accumulate units of energy (pulses) over meter lifetime (apparent import ... from grid) */
      uint64_t app_exp;     /**< Variable used to accumulate units of energy (pulses) over meter lifetime (apparent export ... to grid) */
      uint64_t c_react_imp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (C reactive import ... from grid) */
      uint64_t c_react_exp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (C reactive export ... to grid) */
      uint64_t l_react_imp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (L reactive import ... from grid) */
      uint64_t l_react_exp; /**< Variable used to accumulate units of energy (pulses) over meter lifetime (L reactive export ... to grid) */
    } counter;
  } energy;

  /** @brief Data related to impulse controls */
  struct impulse
  {
    uint32_t led_on_count;          /**< Number of ADC intervals for the LED on count */
    uint32_t active_counter; /**< Counter used to track on time of the active LED. */
    uint32_t apparent_counter; /**< Counter used to track on time of the apparent LED. */
    uint32_t reactive_counter; /**< Counter used to track on time of the reactive LED. */
    bool active_on; /**< Flag indicating the active led is on */
    bool apparent_on; /**< Flag indicating the apparent led is on */
    bool reactive_on; /**< Flag indicating the reactive led is on */
  } impulse;
} LMA_SystemEnergy;

/** @brief dedicated struct for calibration arguments */
typedef struct LMA_PhaseCalibArgs
{
  LMA_Phase *p_phase;              /**< Pointer to a phases targetting calibration */
  float vrms_tgt;                /**< Vrms target calibration voltage */
  float irms_tgt;                /**< Irms target calibration current */
  uint32_t line_cycles;            /**< Line cycles to stabilise readings over */
} LMA_PhaseCalibArgs;

/** @brief dedicated struct for calibration arguments */
typedef struct LMA_GlobalCalibArgs
{
  float rtc_period;              /**< period of the RTC for fs calibration */
  uint32_t rtc_cycles;             /**< number of RTC periods to accumulate for frequency calibration */
} LMA_GlobalCalibArgs;

/** @brief Runtime configuration data of the library*/
typedef struct LMA_Config
{
  LMA_GlobalCalibration gcalib;    /**< Global calibration data block */
  uint32_t update_interval;        /**< Number of V line cycles to between computation updates. */
  float target_system_frequency; /**< Frequency of the target system */
  float meter_constant;          /**< Ws/imp ... translated Ws/imp = 3,600,000 / [imp/kwh]*/
  float no_load_i;  /**< No load current value */
  float no_load_p;  /**< No active/reactive power load value */
  float v_sag;      /**< Votlage sag value */
  float v_swell;    /**< Votlage swell value */
} LMA_Config;

/** @brief Convenience type to get a full measurement dump (per phase) */
typedef struct LMA_Measurements
{
  float vrms;  /**< RMS Voltage */
  float irms;  /**< RMS Current */
  float fline; /**< Line Frequency */
  float p;     /**< Active Power */
  float q;     /**< Reactive Power */
  float s;     /**< Apparent Power */
} LMA_Measurements;

#endif /* _LMA_TYPES_H */
