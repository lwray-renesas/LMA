/*
 * SysTick_Benchmark.c
 *
 *  Created on: 28 Jan 2025
 *      Author: a5126135
 */

#include <Benchmark/Benchmark.h>

/** @brief macros used to red current interrupt state */
#define BM_CRITICAL_SECTION_PREPARE() uint32_t interrupt_save = __get_PRIMASK()
/** @brief macro used to disable interrupts*/
#define BM_CRITICAL_SECTION_ENTER() __disable_irq()
/** @brief macro used to restore interrupt state*/
#define BM_CRITICAL_SECTION_EXIT() __set_PRIMASK(interrupt_save)

static void Benchmark_start_run(benchmark_t *bm)
{
  BM_CRITICAL_SECTION_PREPARE();
  BM_CRITICAL_SECTION_ENTER();

  R_AGT_Stop(&g_idle_timer_ctrl);
  R_AGT_Stop(&g_working_timer_ctrl);

  R_AGT_Reset(&g_working_timer_ctrl);
  R_AGT_Reset(&g_idle_timer_ctrl);

  bm->worker.tick_count = bm->worker.tick_count_reload;
  bm->worker.idle_time = 0;
  bm->worker.working_time = 0;
  bm->worker.bm_running = true;

  R_AGT_Start(&g_idle_timer_ctrl);

  BM_CRITICAL_SECTION_EXIT();
}

static bool Benchmark_running(benchmark_t *bm)
{
  return bm->worker.bm_running;
}

static void Benchmark_compute(benchmark_t *bm)
{
  bm->cpu_utilisation =
      100.00f * ((float)bm->worker.working_time / ((float)bm->worker.idle_time + (float)bm->worker.working_time));

  if (bm->cpu_utilisation > bm->cpu_utilisation_pk)
  {
    bm->cpu_utilisation_pk = bm->cpu_utilisation;
  }
}

void Benchmark_init(benchmark_t *bm, uint32_t ticks)
{
  bm->worker.idle_time = 0;
  bm->worker.working_time = 0;
  bm->worker.tick_count = 0;
  bm->worker.tick_count_reload = ticks;
  bm->worker.bm_running = false;
  bm->cpu_utilisation = 0.0f;
  bm->cpu_utilisation_pk = 0.0f;

  R_AGT_Open(&g_idle_timer_ctrl, &g_idle_timer_cfg);
  R_AGT_Open(&g_working_timer_ctrl, &g_working_timer_cfg);
}

void Benchmark_run(benchmark_t *bm)
{
  Benchmark_start_run(bm);

  while (Benchmark_running(bm))
  {
    __NOP();
  }

  Benchmark_compute(bm);
}

void Benchmark_work_begin(benchmark_t *bm)
{
  BM_CRITICAL_SECTION_PREPARE();
  BM_CRITICAL_SECTION_ENTER();

  if (true == bm->worker.bm_running)
  {
    R_AGT0->AGT16.CTRL.AGTCR &= (uint8_t) ~(0xF0); /* Clear interrupt flags*/
    R_AGT_Stop(&g_idle_timer_ctrl);
    bm->worker.idle_time += (R_AGT0->AGT16.AGTCMA - R_AGT0->AGT16.AGT); /* Store idle timer*/

    R_AGT_Reset(&g_working_timer_ctrl);
    R_AGT_Start(&g_working_timer_ctrl);
  }

  BM_CRITICAL_SECTION_EXIT();
}

void Benchmark_work_end(benchmark_t *bm)
{
  BM_CRITICAL_SECTION_PREPARE();
  BM_CRITICAL_SECTION_ENTER();

  if (true == bm->worker.bm_running)
  {
    R_AGT1->AGT16.CTRL.AGTCR &= (uint8_t) ~(0xF0); /* Clear interrupt flags*/
    R_AGT_Stop(&g_working_timer_ctrl);
    bm->worker.working_time += (R_AGT1->AGT16.AGTCMA - R_AGT1->AGT16.AGT); /* Store working timer*/

    R_AGT_Reset(&g_idle_timer_ctrl);
    R_AGT_Start(&g_idle_timer_ctrl);
  }

  BM_CRITICAL_SECTION_EXIT();
}

void Benchmark_cb_period(benchmark_t *bm)
{
  BM_CRITICAL_SECTION_PREPARE();
  BM_CRITICAL_SECTION_ENTER();

  if (true == bm->worker.bm_running)
  {
    --bm->worker.tick_count;
    if (0 == bm->worker.tick_count)
    {

      R_AGT_Stop(&g_idle_timer_ctrl);
      R_AGT_Stop(&g_working_timer_ctrl);

      bm->worker.bm_running = false;
    }
  }

  BM_CRITICAL_SECTION_EXIT();
}
