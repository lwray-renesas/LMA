#ifndef _LMA_PORT_H
#define _LMA_PORT_H

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

#include "LMA_Types.h"

/* LLVM Toolchain*/
#if defined(__clang__) && defined(__RL78__)

  #define LMA_CRITICAL_SECTION_PREPARE() char _cs_ie_bit = __builtin_rl78_pswie()
  #define LMA_CRITICAL_SECTION_ENTER() asm("di")
  #define LMA_CRITICAL_SECTION_EXIT()                                                                                          \
    if (_cs_ie_bit != 0)                                                                                                       \
    {                                                                                                                          \
      asm("ei");                                                                                                               \
    }

/* CCRL Toolchain*/
#elif defined(__CCRL__)

  #define LMA_CRITICAL_SECTION_PREPARE() unsigned char _psw_ie_masked = __get_psw() & 0x80
  #define LMA_CRITICAL_SECTION_ENTER() __DI()
  #define LMA_CRITICAL_SECTION_EXIT()                                                                                          \
    if (_psw_ie_masked != 0)                                                                                                   \
    {                                                                                                                          \
      __EI();                                                                                                                  \
    }

/* IAR Toolcahin*/
#elif define(__ICCRL78__)

  #include <intrinsics.h>
  #define LMA_CRITICAL_SECTION_PREPARE() __istate_t _cs_is = __get_interrupt_state()
  #define LMA_CRITICAL_SECTION_ENTER() __disable_interrupt()
  #define LMA_CRITICAL_SECTION_EXIT() __set_interrupt_state(_cs_is)

#else
  #error "Unsupported compiler!"
#endif

/** @brief handles sample accumulation for a phase
 * @details Performs:
 * vacc += v_sample ^ 2
 * iacc += i_sample ^ 2
 * pacc += i_sample * v_sample
 * qacc += i_sample * v90_sample
 * and where applicable iacc_neutral += i_neutral_sample ^ 2
 * @param[inout] p_phase - pointer to the phase we are working with.
 */
void LMA_AccPhaseRun(LMA_Phase *const p_phase);

/** @brief handles sample reset between cycles for a phase
 * @details Performs:
 * vacc = v_sample ^ 2
 * iacc = i_sample ^ 2
 * pacc = i_sample * v_sample
 * qacc = i_sample * v90_sample
 * and where applicable iacc_neutral = i_neutral_sample ^ 2
 * @param[inout] p_phase - pointer to the phase we are working with.
 */
void LMA_AccPhaseReset(LMA_Phase *const p_phase);

/** @brief Move accumulators from temp to snapshot.
 * @details This could be done directly in LMA_Core, however with devices supporting hardware accumulator
 * buffers, the temp values might not be stored where expected for efficiency.
 * So we leave it to the porting layer to manage the accumulators in this way.
 * @param[inout] p_phase - pointer to the phase we are working with.
 */
void LMA_AccPhaseLoad(LMA_Phase *const p_phase);

/******************
 * DRIVERS
 ******************/
/** @brief Initialises ADC
 * @details Doesn't start it, just prepares it.
 */
void LMA_ADC_Init(void);

/** @brief Starts the ADC running
 * @details This function should start the ADC in a such a way that it results in a periodic "sampling complete" interrupt which
 * enters the ISR that calls LMA_CB_ADC.
 */
void LMA_ADC_Start(void);

/** @brief Stops the ADC running
 * @details This function should stop the ADC in a such a way that it stops the periodic "sampling complete" interrupt which
 * enters the ISR that calls LMA_CB_ADC.
 */
void LMA_ADC_Stop(void);

/** @brief Initialises TMR
 * @details Doesn't start it, just prepares it.
 */
void LMA_TMR_Init(void);

/** @brief Starts the TMR running
 * @details This function should start the TMR in a such a way that it results in a periodic interrupt which enters the ISR
 * that calls LMA_CB_TMR.
 */
void LMA_TMR_Start(void);

/** @brief Stops the TMR running
 * @details This function should stop the TMR in a such a way that it stops the periodic interrupt which enters the ISR
 * that calls LMA_CB_TMR.
 */
void LMA_TMR_Stop(void);

/** @brief Initialises RTC
 * @details Doesn't start it, just prepares it.
 */
void LMA_RTC_Init(void);

/** @brief Starts the RTC running
 * @details This function should start the RTC in a such a way that it results in a periodic interrupt which enters the ISR
 * that calls LMA_CB_RTC.
 */
void LMA_RTC_Start(void);

/** @brief Stops the RTC running
 * @details This function should stop the RTC in a such a way that it stops the periodic interrupt which enters the ISR
 * that calls LMA_CB_RTC.
 */
void LMA_RTC_Stop(void);

/** @brief Callback to turn on active impulse LED
 * @details This function is called by the library to turn on the active impulse LED.
 */
void LMA_IMP_ActiveOn(void);

/** @brief Callback to turn off active impulse LED
 * @details This function is called by the library to turn off the active impulse LED.
 */
void LMA_IMP_ActiveOff(void);

/** @brief Callback to turn on reactive impulse LED
 * @details This function is called by the library to turn on the reactive impulse LED.
 */
void LMA_IMP_ReactiveOn(void);

/** @brief Callback to turn off reactive impulse LED
 * @details This function is called by the library to turn off the reactive impulse LED.
 */
void LMA_IMP_ReactiveOff(void);

/** @brief Callback to turn on apparent impulse LED
 * @details This function is called by the library to turn on the apparent impulse LED.
 */
void LMA_IMP_ApparentOn(void);

/** @brief Callback to turn off apparent impulse LED
 * @details This function is called by the library to turn off the apparent impulse LED.
 */
void LMA_IMP_ApparentOff(void);

/**@} */

#endif /* _LMA_PORT_H */
