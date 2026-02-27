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
 * This should be positive.
 * @param num_tasks Number of tasks.
 * This should be non-negative.
 * @param run_task_range Function to iterate over a range of tasks within a worker,
 * see the argument of the same name in `subpar::parallelize_range()`.
 *
 * By default, this function is an alias to `subpar::parallelize_range()`.
 * Its purpose is to enable **WeightedLowess**-specific customization to the parallelization scheme without affecting other libraries that use **subpar**.
 * If the `WEIGHTEDLOWESS_CUSTOM_PARALLEL` macro is defined, it will be used instead of `subpar::parallelize_range()` whenever `parallelize()` is called. 
 * Any user-defined macro should follow the same requirements as the `SUBPAR_CUSTOM_PARALLELIZE_RANGE` override for `subpar::parallelize_range()`.
 */
template<typename Task_, class Run_>
int parallelize(const int num_workers, const Task_ num_tasks, Run_ run_task_range) {
#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
    // Various methods don't allocate or throw, so nothrow_ = true is fine.
    return subpar::parallelize_range<true>(num_workers, num_tasks, std::move(run_task_range));
#else
    return WEIGHTEDLOWESS_CUSTOM_PARALLEL(num_workers, num_tasks, run_task_range);
#endif
}

}

#endif
