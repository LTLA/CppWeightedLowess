#ifndef WEIGHTEDLOWESS_COMPUTE_HPP
#define WEIGHTEDLOWESS_COMPUTE_HPP

#include <vector>

#include "fit.hpp"
#include "Options.hpp"

/**
 * @file compute.hpp
 * @brief Compute the LOWESS trend fit.
 */

namespace WeightedLowess {

/**
 * LOWESS is a simple, efficient, general-purpose non-parametric smoothing algorithm. 
 * It perform weighted linear regressions on subsets of neighboring points, yielding a smooth curve fit to the data.
 *
 * @tparam Data_ Floating-point type for the data.
 *
 * @param num_points Number of points.
 * @param x Pointer to an array of `num_points` x-values.
 * This should be sorted in increasing order, see `SortBy` for details.
 * @param y Pointer to an array of `num_points` y-values.
 * @param[out] fitted Pointer to an output array of length `n`, in which the fitted values of the smoother can be stored.
 * @param[out] robust_weights Pointer to an output array of length `n`, in which the robustness weights can be stored.
 * This may be `NULL` if the robustness weights are not needed.
 * @param Further options.
 */
template<typename Data_>
void compute(size_t num_points, const Data_* x, const Data_* y, Data_* fitted, Data_* robust_weights, const Options<Data_>& opt) {
    if (!std::is_sorted(x, x + num_points)) {
        throw std::runtime_error("'x' should be sorted");
    }

    std::vector<Data_> rbuffer;
    if (robust_weights == NULL) {
        rbuffer.resize(num_points);
        robust_weights = rbuffer.data();
    }

    internal::fit_trend(num_points, x, y, fitted, robust_weights, opt);
}

/** 
 * @brief Store the smoothing results.
 * @tparam Data_ Floating-point type for the data.
 */
template<typename Data_>
struct Results {
    /**
     * @param n Number of points.
     */
    Results(size_t n) : fitted(n), robust_weights(n) {}

    /**
     * Fitted values from the LOWESS smoother. 
     */
    std::vector<Data_> fitted;

    /**
     * Robustness weight for each point.
     */
    std::vector<Data_> robust_weights;
};

/**
 * Run the LOWESS smoother and return a `Results` object.
 * This avoids the need to instantiate the various output arrays manually. 
 * 
 * @tparam Data_ Floating-point type for the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-values.
 * @param[in] y Pointer to an array of `num_points` y-values.
 * @param[in] weights Pointer to an array of `n` positive weights.
 * Alternatively `NULL` if no weights are available.
 * @param opt Further options.
 *
 * @return A `Results` object containing the fitted values and robustness weights.
 */
template<typename Data_>
Results<Data_> compute(size_t num_points, const Data_* x, const Data_* y, const Options<Data_>& opt) {
    Results<Data_> output(num_points);
    compute(num_points, x, y, output.fitted.data(), output.robust_weights.data(), opt);
    return output;
}

}

#endif
