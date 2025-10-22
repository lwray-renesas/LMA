/*
 * Trap_integrator.c
 *
 *  Created on: 22 Oct 2025
 *      Author: a5126135
 */

#include "Trap_integrator.h"

Trap_integrator rogowski_integrator = {0LL, 0LL};

int32_t Trap_integrate(Trap_integrator *p_integrator, int32_t input)
{
  int32_t temp = p_integrator->prev_output + ((input + p_integrator->prev_input) / 2);
  p_integrator->prev_input = input;
  p_integrator->prev_output = temp;

  return temp;
}

void Trap_reset(Trap_integrator *p_integrator)
{
  p_integrator->prev_input = 0LL;
  p_integrator->prev_output = 0LL;
}
