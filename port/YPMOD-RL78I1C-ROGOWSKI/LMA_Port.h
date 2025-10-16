#ifndef _LMA_PORT_H
#define _LMA_PORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
typedef struct LMA_TempData
{
  /** @brief dedicated struct for a sample set */
  struct LMA_Samples
  {
    spl_t voltage;   /**< Voltage ADC sample */
    spl_t current;   /**< Current ADC sample */
    spl_t voltage90; /**< Sample used to contain the phase shifted voltage signal */
  } samples;

  LMA_Accumulators accs; /**< Block of accumulators to work with*/
} LMA_TempData;

/** @brief handles sample reset between cycles.
 * @details Performs:
 * vacc = v_sample ^ 2
 * iacc = i_sample ^ 2
 * pacc = i_sample * v_sample
 * qacc = i_sample * v90_sample
 * @param[inout] pointer to data to work with.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccReset(LMA_TempData *const p_ws, const uint32_t phase_id);

/** @brief handles sample accumulation
 * @details Performs:
 * vacc += v_sample ^ 2
 * iacc += i_sample ^ 2
 * pacc += i_sample * v_sample
 * qacc += i_sample * v90_sample
 * @param[inout] pointer to data to work with.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccRun(LMA_TempData *const p_ws, const uint32_t phase_id);

/** @brief Loads the accumulators into the accumulator structure.
 * @param[inout] pointer to data to work with.
 * @param[inout] pointer to accumulators to copy results to.
 * @param[in] phase_id - phase identifier for managing porting resources.
 */
void LMA_AccLoad(LMA_TempData *const p_ws, LMA_Accumulators *const p_accs, const uint32_t phase_id);

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
