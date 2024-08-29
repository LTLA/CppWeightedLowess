#ifndef WEIGHTEDLOWESS_PARALLELIZE_HPP
#define WEIGHTEDLOWESS_PARALLELIZE_HPP

/**
 * @file parallelize.hpp
 * @brief Definitions for parallelization.
 */

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#include "subpar/subpar.hpp"

/**
 * Function-like macro implementing the parallelization scheme for the **WeightedLowess** library.
 * If undefined by the user, it defaults to `subpar::parallelize()`.
 * Any user-defined macro should accept the same arguments as `subpar::parallelize()`.
 */ 
#define WEIGHTEDLOWESS_CUSTOM_PARALLEL ::subpar::parallelize
#endif

#endif
