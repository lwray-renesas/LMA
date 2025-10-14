/** \addtogroup Porting
 * @brief LMA Porting Layer
 * @details The LMA porting layer is used to perform most (ideally all) porting activities between platforms to enable LMA to
 * run across different different cores. To perform a port, ensure you fulfill the functional requirements of the API in the
 * porting files - using an existing port as reference may help.
 *
 * @note The documentation here is based on empty "skeleton" porting files and should be used to aid with creating a new
 * port or navigating existing ports.
 *  @{
 *
 * @file LMA_Port.h
 * @brief Porting file declarations for the LMA codebase.
 *
 * @details This file provides declarations of the LMA porting requirements - everything in this file must be considered when
 * porting between platforms.
 */

/**@} */

#ifndef _LMA_PORT_H
#define _LMA_PORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Prepares for disabling interrupts/enterring critical section
 * @details Reads current interrupt state for restoring later in function.
 * @warning Typically declares a variables so depending on language standard, should be called at beginning of block scope for
 * C90 compatiblity.
 */
#define LMA_CRITICAL_SECTION_PREPARE()

/** @brief Disables interrupts/enters critical section
 * @details Platform dependent mechanism for enterring critical section/disabling interrupts.
 */
#define LMA_CRITICAL_SECTION_ENTER()

/** @brief Enables interrupts/exits critical section
 * @details Platform dependent mechanism for exiting critical section/enabling interrupts.
 */
#define LMA_CRITICAL_SECTION_EXIT()

/** @brief Defines the raw ADC sample type */
typedef int32_t spl_t;
/** @brief Defines the accumulator type (generally double the bit-width of the sample) */
typedef int64_t acc_t;

/**
 * @brief Accumulators
 * @details Data structure containing accumulators for accumulating samples.
 */
typedef struct LMA_Accumulators
{
  acc_t vacc;            /**< Voltage accumulator*/
  acc_t iacc;            /**< Current accumulator*/
  acc_t pacc;            /**< Power accumulator*/
  acc_t qacc;            /**< Reactive accumulator*/
  uint32_t sample_count; /**< line frequency accumulator*/
} LMA_Accumulators;

/**
 * @brief Workspace
 * @details Data structure containing some glue logic between the phases and porting layer to enable work on a phase between
 * systems
 */
typedef struct LMA_Workspace
{
  /** @brief Dedicated struct for a sample set */
  struct LMA_Samples
  {
    spl_t voltage;   /**< Voltage ADC sample */
    spl_t current;   /**< Current ADC sample */
    spl_t voltage90; /**< Sample used to contain the phase shifted voltage signal */
  } samples;         /**< structure containing samples to work on*/

  LMA_Accumulators accs; /**< Block of accumulators to work with*/
} LMA_Workspace;

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
 * @param[inout] p_ws - pointer to data to work with.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccReset(LMA_Workspace *const p_ws, const uint32_t phase_id);

/** @brief handles sample accumulation
 * @details Performs:
 * vacc += v_sample ^ 2
 * iacc += i_sample ^ 2
 * pacc += i_sample * v_sample
 * qacc += i_sample * v90_sample
 * @param[inout] p_ws - pointer to data to work with.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccRun(LMA_Workspace *const p_ws, const uint32_t phase_id);

/** @brief Loads the accumulators into the accumulator structure.
 * @param[inout] p_ws - pointer to data to work with.
 * @param[inout] p_accs - pointer to accumulators to copy results to.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccLoad(LMA_Workspace *const p_ws, LMA_Accumulators *const p_accs, const uint32_t phase_id);

/** @brief Phase shifts voltage signal
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
/** @brief Initialises ADC
 * @details Doesn't start it.
 */
void LMA_ADC_Init(void);

/** @brief Starts the ADC running
 * @details Must begin ADC sampling interrupt that will call LMA_CB_ADC.
 */
void LMA_ADC_Start(void);

/** @brief Stops the ADC running
 * @details Must stop ADC sampling interrupt that will call LMA_CB_ADC.
 */
void LMA_ADC_Stop(void);

/** @brief Initialises TMR
 * @details Doesn't start it.
 */
void LMA_TMR_Init(void);

/** @brief Starts the TMR running
 * @details Must start TMR that will periodically trigger the interrupt that will call LMA_CB_TMR.
 */
void LMA_TMR_Start(void);

/** @brief Stops the TMR running
 * @details Must stop TMR that will periodically trigger the interrupt that will call LMA_CB_TMR.
 */
void LMA_TMR_Stop(void);

/** @brief Initialises RTC
 * @details Doesn't start it.
 */
void LMA_RTC_Init(void);

/** @brief Starts the RTC running
 * @details Must start RTC that will periodically trigger the interrupt that will call LMA_CB_RTC.
 */
void LMA_RTC_Start(void);

/** @brief Stops the RTC running
 * @details Must stop RTC that will periodically trigger the interrupt that will call LMA_CB_RTC.
 */
void LMA_RTC_Stop(void);

/** @brief Callback to turn on active impulse LED
 * @details Must turn on the active LED (leave blank if unused).
 */
void LMA_IMP_ActiveOn(void);

/** @brief Callback to turn off active impulse LED
 * @details Must turn off the active LED (leave blank if unused).
 */
void LMA_IMP_ActiveOff(void);

/** @brief Callback to turn on reactive impulse LED
 * @details Must turn on the reactive LED (leave blank if unused).
 */
void LMA_IMP_ReactiveOn(void);

/** @brief Callback to turn off reactive impulse LED
 * @details Must turn off the reactive LED (leave blank if unused).
 */
void LMA_IMP_ReactiveOff(void);

/** @brief Callback to turn on apparent impulse LED
 * @details Must turn on the apparent LED (leave blank if unused).
 */
void LMA_IMP_ApparentOn(void);

/** @brief Callback to turn off apparent impulse LED
 * @details Must turn off the apparent LED (leave blank if unused).
 */
void LMA_IMP_ApparentOff(void);

#endif /* _LMA_PORT_H */
