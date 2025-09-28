#ifndef WEIGHTEDLOWESS_PARALLELIZE_HPP
#define WEIGHTEDLOWESS_PARALLELIZE_HPP

/**
 * @file parallelize.hpp
 * @brief Definitions for parallelization.
 */

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#include "subpar/subpar.hpp"
#endif

namespace WeightedLowess {

/**
 * @tparam Task_ Integer type of the number of tasks.
 * @tparam Run_ Function to execute a range of tasks.
 *
 * @param num_workers Number of workers.
 * @param num_tasks Number of tasks.
 * @param run_task_range Function to iterate over a range of tasks within a worker.
 *
 * By default, this is an alias to `subpar::parallelize_range()`.
 * However, if the `WEIGHTEDLOWESS_CUSTOM_PARALLEL` function-like macro is defined, it is called instead. 
 * Any user-defined macro should accept the same arguments as `subpar::parallelize_range()`.
 */
template<typename Task_, class Run_>
void parallelize(const int num_workers, const Task_ num_tasks, Run_ run_task_range) {
#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
    // Various methods don't allocate or throw, so nothrow_ = true is fine.
    subpar::parallelize_range<true>(num_workers, num_tasks, std::move(run_task_range));
#else
    WEIGHTEDLOWESS_CUSTOM_PARALLEL(num_workers, num_tasks, run_task_range);
#endif
}

}

#endif
