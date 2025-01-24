#ifndef _LMA_PORT_H
#define _LMA_PORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hal_data.h"

/** @brief macros used to red current interrupt state */
#define LMA_CRITICAL_SECTION_PREPARE() uint32_t interrupt_save =  __get_PRIMASK()
/** @brief macro used to disable interrupts*/
#define LMA_CRITICAL_SECTION_ENTER() __disable_irq()
/** @brief macro used to restore interrupt state*/
#define LMA_CRITICAL_SECTION_EXIT() __set_PRIMASK(interrupt_save)

/** @brief Defines the raw ADC sample type */
typedef int32_t spl_t;
/** @brief Defines the accumulator type (generally double the bit-width of the sample) */
typedef int64_t acc_t;
/** @brief Defines extended accumulator type for apparent power*/
typedef struct acc_ext_t
{
    uint64_t upper; /**< Upper part of extended accumulator*/
    uint64_t lower; /**< Lower part of extended accumulator*/
}acc_ext_t;

/** @brief dedicated struct for a sample set */
typedef struct LMA_Samples
{
  spl_t voltage;   /**< Voltage ADC sample */
  spl_t current;   /**< Current ADC sample */
  spl_t voltage90; /**< Sample used to contain the phase shifted voltage signal */
} LMA_Samples;

/** @brief Some glue logic between the phases and porting layer to enable work on a phase between systems*/
typedef struct LMA_Workspace
{
    LMA_Samples *p_samples; /**< Pointer to current sample set*/
    acc_t * p_vacc; /**< Pointer to voltage accumulator*/
    acc_t * p_iacc; /**< Pointer to current accumulator*/
    acc_t * p_pacc; /**< Pointer to power accumulator*/
    acc_t * p_qacc; /**< Pointer to reactive accumulator*/
}LMA_Workspace;

/** Converts an accumulator type to a floating point type.
 * @param acc - accumulator for conversion.
 * @return floating point representation.
 */
float LMA_AccToFloat(acc_t acc);

/** Multiplies two floating ppint numbers.
 * @details - does not handle special cases ! NaN, denorm and norm etc.
 * @param a - first operand.
 * @param b - second operand.
 * @return a * b
 */
float LMA_Fast_FPMul(float a, float b);

/** Divides two floating ppint numbers.
 * @details - does not handle special cases ! NaN, denorm and norm etc.
 * @param a - first operand.
 * @param b - second operand.
 * @return a / b
 */
float LMA_Fast_FPDiv(float a, float b);

/** Retusn the sqaure root of the float passed
 * @param a - number to be sqrt.
 * @return sqrt(a)
 */
float LMA_Fast_FPSqrt(float a);

/** Retusn the absolute of the float passed
 * @param a - number to be absoluted.
 * @return abs(a)
 */
float LMA_Fast_FPAbs(float a);

/** @brief handles sample reset between cycles.
 * @details Performs:
 * vacc = v_sample ^ 2
 * iacc = i_sample ^ 2
 * pacc = i_sample * v_sample
 * qacc = i_sample * v90_sample
 * @param[inout] pointer to data to work with.
 */
void LMA_AccReset(LMA_Workspace *const p_ws);

/** @brief handles sample accumulation
 * @details Performs:
 * vacc += v_sample ^ 2
 * iacc += i_sample ^ 2
 * pacc += i_sample * v_sample
 * qacc += i_sample * v90_sample
 * @param[inout] pointer to data to work with.
 */
void LMA_AccRun(LMA_Workspace *const p_ws);

/** @brief Gets internal accumulators into workspace variables.
 * @param[inout] pointer to data to work with.
 */
void LMA_AccGet(LMA_Workspace *const p_ws);

/** @brief Multiples a and b and stores the result in result
 * @details - MUST BE POSITIVE A AND B.
 * @param res - pointer to location to store result.
 * @param a - first operand
 * @param b - second operand.
 */
void LMA_AccMultiply(acc_ext_t *res, acc_t a, acc_t b);

/** @brief Fast square root algorithm
 * @details We make this available here for potential device/platform optimisations.
 * @details https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
 * @param[in] val - value to be square rooted
 * @return square root of val
 */
acc_t LMA_AccSqrt(acc_t val);


/******************
* DRIVERS
******************/
/** @brief Initialises ADC but doesn't start it */
void LMA_ADC_Init(void);
/** @brief Starts the ADC running */
void LMA_ADC_Start(void);
/** @brief Stops the ADC running */
void LMA_ADC_Stop(void);


/** @brief Initialises RTC but doesn't start it */
void LMA_RTC_Init(void);
/** @brief Starts the RTC running */
void LMA_RTC_Start(void);
/** @brief Stops the RTC running */
void LMA_RTC_Stop(void);


/** @brief Callback to turn on active impulse LED */
void LMA_IMP_ActiveOn(void);
/** @brief Callback to turn off active impulse LED */
void LMA_IMP_ActiveOff(void);
/** @brief Callback to turn on reactive impulse LED */
void LMA_IMP_ReactiveOn(void);
/** @brief Callback to turn off reactive impulse LED */
void LMA_IMP_ReactiveOff(void);
#endif /* _LMA_PORT_H */
