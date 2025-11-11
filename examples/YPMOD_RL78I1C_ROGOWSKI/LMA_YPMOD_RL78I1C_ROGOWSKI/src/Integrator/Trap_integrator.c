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
  int32_t integrated;
  int64_t filtered_temp;

  if (!p_integrator->run_already)
  {
    p_integrator->prev_input = input;
  }

  integrated = (p_integrator->prev_output + ((input + p_integrator->prev_input) / 2));

  if (!p_integrator->run_already)
  {
    p_integrator->run_already = true;
    p_integrator->prev_input_hpf = integrated;
  }

  filtered_temp = ((int64_t)(p_integrator->prev_output_hpf * 63493LL) / 0xFFFFLL) +
                  (((int64_t)(p_integrator->prev_input_hpf - integrated) * 63493LL) / 0xFFFFLL);

  p_integrator->prev_input = input;
  p_integrator->prev_output = integrated;
  p_integrator->prev_input_hpf = integrated;
  p_integrator->prev_output_hpf = (int32_t)filtered_temp;

  return (int32_t)filtered_temp;
}

void Trap_reset(Trap_integrator *p_integrator)
{
  p_integrator->prev_input = 0L;
  p_integrator->prev_output = 0L;
  p_integrator->prev_input_hpf = 0L;
  p_integrator->prev_output_hpf = 0L;
  p_integrator->run_already = false;
}
