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

/** @brief accumulators for accumulating samples*/
typedef struct LMA_Accumulators
{
    acc_t vacc; /**< Voltage accumulator*/
    acc_t iacc; /**< Current accumulator*/
    acc_t pacc; /**< Power accumulator*/
    acc_t qacc; /**< Reactive accumulator*/
    uint32_t sample_count; /**< line frequency accumulator*/
} LMA_Accumulators;

/** @brief Some glue logic between the phases and porting layer to enable work on a phase between systems*/
typedef struct LMA_Workspace
{
    /** @brief dedicated struct for a sample set */
    struct LMA_Samples
    {
      spl_t voltage;   /**< Voltage ADC sample */
      spl_t current;   /**< Current ADC sample */
      spl_t voltage90; /**< Sample used to contain the phase shifted voltage signal */
    } samples;

    LMA_Accumulators accs; /**< Block of accumulators to work with*/
}LMA_Workspace;

/** @brief Converts an accumulator type to a floating point type.
 * @param acc - accumulator for conversion.
 * @return floating point representation.
 */
float LMA_AccToFloat(acc_t acc);

/** @brief Multiplies two floating point numbers.
 * @details - does not handle special cases ! NaN, denorm and norm etc.
 * @param a - first operand.
 * @param b - second operand.
 * @return a * b
 */
float LMA_FPMul_Fast(float a, float b);

/** @brief Divides two floating point numbers.
 * @details - does not handle special cases ! NaN, denorm and norm etc.
 * @param a - first operand.
 * @param b - second operand.
 * @return a / b
 */
float LMA_FPDiv_Fast(float a, float b);

/** @brief Returns the square root of the float passed.
 * @param a - number to be sqrt.
 * @return sqrt(a)
 */
float LMA_FPSqrt_Fast(float a);

/** @brief Returns the absolute of the float passed
 * @param a - number to be absoluted.
 * @return abs(a)
 */
float LMA_FPAbs_Fast(float a);

/** @brief handles sample reset between cycles.
 * @details Performs:
 * vacc = v_sample ^ 2
 * iacc = i_sample ^ 2
 * pacc = i_sample * v_sample
 * qacc = i_sample * v90_sample
 * @param[inout] pointer to data to work with.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccReset(LMA_Workspace *const p_ws, const uint32_t phase_id);

/** @brief handles sample accumulation
 * @details Performs:
 * vacc += v_sample ^ 2
 * iacc += i_sample ^ 2
 * pacc += i_sample * v_sample
 * qacc += i_sample * v90_sample
 * @param[inout] pointer to data to work with.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccRun(LMA_Workspace *const p_ws, const uint32_t phase_id);

/** @brief Loads the accumulators into the accumulator structure.
 * @param[inout] pointer to data to work with.
 * @param[inout] pointer to accumulators to copy results to.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccLoad(LMA_Workspace *const p_ws, LMA_Accumulators *const p_accs, const uint32_t phase_id);

/** @brief phase shifts voltage signal
 * @details
 * - 50Hz signal is 20ms.
 * - 50Hz signal being 360degree of period, to get 90degree we divide by 4.
 * - 20ms divided by 4 = 5ms.
 * - to delay 5ms with a 3906Hz clock we can do 0.005/(1/3906) = 19.53 samples - so we do 20
 * samples.
 *
 * @param[in] new_voltage - new voltage to store in the buffer
 * @return voltage sample 90degree (20 samples) ago.
 */
spl_t LMA_PhaseShift90(spl_t new_voltage);


/******************
* DRIVERS
******************/
/** @brief Initialises ADC but doesn't start it */
void LMA_ADC_Init(void);
/** @brief Starts the ADC running */
void LMA_ADC_Start(void);
/** @brief Stops the ADC running */
void LMA_ADC_Stop(void);

/** @brief Initialises TMR but doesn't start it */
void LMA_TMR_Init(void);
/** @brief Starts the TMR running */
void LMA_TMR_Start(void);
/** @brief Stops the TMR running */
void LMA_TMR_Stop(void);

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
/** @brief Callback to turn on apparent impulse LED */
void LMA_IMP_ApparentOn(void);
/** @brief Callback to turn off apparent impulse LED */
void LMA_IMP_ApparentOff(void);

#endif /* _LMA_PORT_H */
