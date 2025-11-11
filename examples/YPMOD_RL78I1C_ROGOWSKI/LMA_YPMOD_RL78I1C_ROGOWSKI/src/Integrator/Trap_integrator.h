/*
 * Trap_integrator.h
 *
 *  Created on: 22 Oct 2025
 *      Author: a5126135
 */

#ifndef INTEGRATOR_TRAP_INTEGRATOR_H_
#define INTEGRATOR_TRAP_INTEGRATOR_H_

#include "stdbool.h"
#include "stdint.h"

/** @brief Trapezoidal integration structure*/
typedef struct Trap_integrator_str
{
  int32_t prev_input;
  int32_t prev_output;
  int32_t prev_input_hpf;
  int32_t prev_output_hpf;
  bool run_already;
} Trap_integrator;

extern Trap_integrator rogowski_integrator;

/**
 * @brief Performs Integration iteration.
 * @param p_integrator - pointer to integrator struct to work on.
 * @param input - latest input signal.
 * @return output of integration.
 */
int32_t Trap_integrate(Trap_integrator *p_integrator, int32_t input);

/**
 * @brief Resets integrator
 * @param p_integrator - pointer to integrator struct to work on.
 * @param input - latest input signal.
 */
void Trap_reset(Trap_integrator *p_integrator);

#endif /* INTEGRATOR_TRAP_INTEGRATOR_H_ */
