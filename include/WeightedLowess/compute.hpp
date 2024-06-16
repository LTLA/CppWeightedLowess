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
 * Run the LOWESS smoother with precomputed windows. 
 * This avoids redundant window calculations when re-using the same x-coordinates across multiple y-coordinate vectors.
 *
 * @tparam Data_ Floating-point type for the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-values, sorted in increasing order.
 * (Consider using `SortBy` to permute `x` in-place.
 * Note that the same permutation should be applied to `y`, if present, and weights in `Options::weights`.)
 * @param windows Precomputed windows, created by calling `define_windows()` with `num_points`, `x` and `opt`.
 * @param[in] y Pointer to an array of `num_points` y-values. 
 * @param[out] fitted Pointer to an output array of length `num_points`, in which the fitted values of the smoother can be stored.
 * @param[out] robust_weights Pointer to an output array of length `num_points`, in which the robustness weights can be stored.
 * This may be `NULL` if the robustness weights are not needed.
 * @param opt Further options.
 * This should be the same object that is used in `define_windows()`.
 * Note that only a subset of options are actually used in this overload, namely `Options::weights` and `Options::iterations`.
 */
template<typename Data_>
void compute(size_t num_points, const Data_* x, const PrecomputedWindows<Data_>& windows, const Data_* y, Data_* fitted, Data_* robust_weights, const Options<Data_>& opt) {
    std::vector<Data_> rbuffer;
    if (robust_weights == NULL) {
        rbuffer.resize(num_points);
        robust_weights = rbuffer.data();
    }
    internal::fit_trend(num_points, x, windows, y, fitted, robust_weights, opt);
}

/**
 * LOWESS is a simple, efficient, general-purpose non-parametric smoothing algorithm. 
 * It perform weighted linear regressions on subsets of neighboring points, yielding a smooth curve fit to the data.
 *
 * @tparam Data_ Floating-point type for the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-values, sorted in increasing order.
 * (Consider using `SortBy` to permute `x` in-place.
 * Note that the same permutation should be applied to `y` and, if present, weights in `Options::weights`.)
 * @param[in] y Pointer to an array of `num_points` y-values.
 * @param[out] fitted Pointer to an output array of length `num_points`, in which the fitted values of the smoother can be stored.
 * @param[out] robust_weights Pointer to an output array of length `num_points`, in which the robustness weights can be stored.
 * This may be `NULL` if the robustness weights are not needed.
 * @param opt Further options.
 */
template<typename Data_>
void compute(size_t num_points, const Data_* x, const Data_* y, Data_* fitted, Data_* robust_weights, const Options<Data_>& opt) {
    auto win = define_windows(num_points, x, opt);
    compute(num_points, x, win, y, fitted, robust_weights, opt);
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
 * @param[in] x Pointer to an array of `num_points` x-values, sorted in increasing order.
 * (Consider using `SortBy` to permute `x` in-place.
 * Note that the same permutation should be applied to `y` and, if present, weights in `Options::weights`.)
 * @param[in] y Pointer to an array of `num_points` y-values.
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
