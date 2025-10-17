/*
 * Benchmark.h
 *
 *  Created on: 28 Jan 2025
 *      Author: a5126135
 */

#ifndef BENCHMARK_BENCHMARK_H_
#define BENCHMARK_BENCHMARK_H_

#include <stdbool.h>
#include <stdint.h>

/** @brief benchmarking object*/
typedef struct benchmark_t
{
  /** @brief worker struct*/
  struct worker
  {
    uint32_t idle_time;
    uint32_t working_time;
    uint32_t tick_count;
    uint32_t tick_count_reload;
    bool bm_running;
  } worker;
  float cpu_utilisation;
  float cpu_utilisation_pk;
} benchmark_t;

/** @brief intiialises the benchmarker
 * @param bm - pointer to benchmark worker.
 * @param ticks - number of ticks to average over.
 */
void Benchmark_init(benchmark_t *bm, uint32_t ticks);

/** @brief runs the benchmark - blocks until complete
 * @param bm - pointer to benchmark worker.
 */
void Benchmark_run(benchmark_t *bm);

/** @brief Marks the start of working code.
 * @param bm - pointer to benchmark worker.
 */
void Benchmark_work_begin(benchmark_t *bm);

/** @brief Marks the end of working code.
 * @param bm - pointer to benchmark worker.
 */
void Benchmark_work_end(benchmark_t *bm);

/** @brief times benchmarking periods - placed at a p0int which marks the elapsing of a benchmarking period.
 * One period is the period of time which to benchmark CPU utilisation.
 * Tupically placed in a timer ISR.
 * @param bm - pointer to benchmark worker.
 */
void Benchmark_cb_period(benchmark_t *bm);

#endif /* BENCHMARK_BENCHMARK_H_ */
