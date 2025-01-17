#ifndef _LMA_PORT_H
#define _LMA_PORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** @brief Enables use of hardware FPU (relies on compiler to generate hw instructions) */
#define FPU_SUPPORT 1U
/** @brief Enables use of software FPU (C code to perform necessary bit manipulations) */
#define SOFT_FPU_SUPPORT 0U
/** @brief Enables use of software fixed point library (uses fixed point library implementation) */
#define FIXED_POINT_SUPPORT 0U

/** @brief Outlines the raw ADC sample type */
typedef int32_t spl_t;
/** @brief Outlines the accumulator type (generally double the bit-width of the sample) */
typedef int64_t acc_t;

#if ((FPU_SUPPORT == 1U) || (SOFT_FPU_SUPPORT == 1U)) && (FIXED_POINT_SUPPORT == 0U)
/** @brief Define the parameter type as floating point (assumes IEEE754) */
typedef float param_t;
#elif (FPU_SUPPORT == 0U) && (SOFT_FPU_SUPPORT == 0U) && (FIXED_POINT_SUPPORT == 1U)
/** @brief define a fixed point implementation without FPU - ideally double the acc bit width */
typedef int64_t param_t;
#else
#error "Parameter type left undefined!"
#endif

/** @brief macros used to red current interrupt state */
#define LMA_CRITICAL_SECTION_PREPARE()
/** @brief macro used to disable interrupts*/
#define LMA_CRITICAL_SECTION_ENTER()
/** @brief macro used to restore interrupt state*/
#define LMA_CRITICAL_SECTION_EXIT()

/**
 * @brief Performs MAC operation on samples & accumulators
 * @details We make this available here for potential device/platform optimisations.
 * @param a - first operand
 * @param b - second operand
 * @param b - third operand
 * @return the result: a + (b * c)
 */
static inline acc_t Accumulate_sample(const acc_t a, const spl_t b, const spl_t c)
{
  return (a + (acc_t)((acc_t)b * (acc_t)c));
}

/** @brief Fast square root algorithm
 * @details We make this available here for potential device/platform optimisations.
 * @details https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
 * @param[in] val - value to be square rooted
 * @return square root of val
 */
static inline acc_t Acc_sqrt(acc_t val)
{
  acc_t x = val, c = 0;
  if (val > 0)
  {
    acc_t d = (acc_t)1 << (acc_t)62;

    while (d > val)
    {
      d >>= 2;
    }

    while (d != 0)
    {
      if (x >= c + d)
      {
        x -= c + d;
        c = (c >> 1) + d;
      }
      else
      {
        c >>= 1;
      }
      d >>= 2;
    }
  }
  return c;
}
/* END OF FUNCTION*/

#if (FPU_SUPPORT == 1U)
/** @brief Converts an accumulator type to a parameter type */
#define PARAM_FROM_ACC(x) ((param_t)x)
/** @brief Converts an integer type to a parameter type */
#define PARAM_FROM_INT(x) ((param_t)x)

/**
 * @brief Multiplies two numbers together and returns the result
 * @param a - first operand
 * @param b - second operand
 * @return the result: a * b
 */
static inline param_t Param_mul(const param_t a, const param_t b)
{
  return a * b;
}

/**
 * @brief Divides two numbers and returns the result.
 * @param a - first operand
 * @param b - second operand
 * @return the result: a / b
 */
static inline param_t Param_div(const param_t a, const param_t b)
{
  return a / b;
}

#elif (SOFT_FPU_SUPPORT == 1U) /* By default we trust the compilers soft implementation*/
/** @brief Converts an accumulator type to a parameter type */
#define PARAM_FROM_ACC(x) ((param_t)x)
/** @brief Converts an integer type to a parameter type */
#define PARAM_FROM_INT(x) ((param_t)x)

/**
 * @brief Multiplies two numbers together and returns the result
 * @param a - first operand
 * @param b - second operand
 * @return the result: a * b
 */
static inline param_t Param_mul(const param_t a, const param_t b)
{
  return a * b;
}

/**
 * @brief Divides two numbers and returns the result.
 * @param a - first operand
 * @param b - second operand
 * @return the result: a / b
 */
static inline param_t Param_div(const param_t a, const param_t b)
{
  return a / b;
}
#elif (FIXED_POINT_SUPPORT == 1U)
/** @brief Converts an accumulator type to a parameter type */
#define PARAM_FROM_ACC(x) (x)

/** @brief Number of bits to shift to support the fractional bits in the 64b type
* @details 21 bits leaves room for 21 bit integer, 21 bit fractional and 21bit shifting room
*/
#define PARAM_SHIFT (21)
/** @brief Number "1" in in the fractional representation*/
#define PARAM_ONE (1ULL << PARAM_SHIFT)
/** @brief Turns number from float representation to param (capable for use at compile time)*/
#define PARAM_FROM_FLOAT(x) ((param_t)(x * PARAM_ONE))
/** @brief Turns number from param to float representation (capable for use at compile time)*/
#define PARAM_TO_FLOAT(x) ((float)(x) / PARAM_ONE)
/** @brief Converts an integer type to a parameter type */
#define PARAM_FROM_INT(x) ((param_t)(x * PARAM_ONE))

/**
 * @brief Multiplies two numbers together and returns the result
 * @param a - first operand
 * @param b - second operand
 * @return the result: a * b
 */
static inline param_t Param_mul(const param_t a, const param_t b)
{
  return ((a * b) >> PARAM_SHIFT);
}

/**
 * @brief Multiplies two numbers together and accumulates and returns the result.
 * @param a - first operand
 * @param b - second operand
 * @return the result: a / b
 */
static inline param_t Param_div(const param_t a, const param_t b)
{
  return ((a << PARAM_SHIFT) + ((b > 0) ? (b >> 1) : -(b >> 1))) / b;
}

#else
#error "Parameter type left undefined!"
#endif /* (FPU_SUPPORT) == 1U */



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

/** @brief Starts the RTC running */
void RTC_Stop(void);

#endif /* _LMA_PORT_H */
