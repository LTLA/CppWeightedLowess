#ifndef WEIGHTEDLOWESS_COMPUTE_HPP
#define WEIGHTEDLOWESS_COMPUTE_HPP

#include <vector>
#include <cstddef>

#include "sanisizer/sanisizer.hpp"

#include "fit.hpp"
#include "Options.hpp"
#include "utils.hpp"

/**
 * @file compute.hpp
 * @brief Compute the LOWESS trend fit.
 */

namespace WeightedLowess {

/**
 * Run the LOWESS algorithm for non-parametric smoothing. 
 *
 * First, we identify anchor points that have (roughly) evenly-spaced x-coordinates.
 * For each anchor point, we identify a window of neighboring points and compute a weight for each neighbor based on its distance to the anchor.
 * We perform a weighted linear regression to obtain a fitted value for the anchor.
 * For all non-anchor points, we compute a fitted value via linear interpolation of the surrounding anchor points.
 * We then compute robustness weights for each point based on their deviation from the fitted value;
 * the regressions are then repeated with robustness weights for the specified number of iterations.
 *
 * @tparam Data_ Floating-point type of the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-coordinates, sorted in increasing order.
 * (Consider using `SortBy` to permute `x` in-place.
 * Note that the same permutation should be applied to `y` and, if present, weights in `Options::weights`.)
 * @param windows Precomputed windows around the anchor points, created by calling `define_windows()` with `num_points`, `x` and `opt`.
 * This can be re-used across multiple calls to `compute()` with different `y`.
 * @param[in] y Pointer to an array of `num_points` y-coordinates. 
 * @param[out] fitted Pointer to an output array of length `num_points`, in which the fitted values of the smoother can be stored.
 * @param[out] robust_weights Pointer to an output array of length `num_points`, in which the robustness weights can be stored.
 * This may be `NULL` if the robustness weights are not needed.
 * @param opt Further options.
 * This should be the same object that is used in `define_windows()`.
 * Note that only a subset of options are actually used in this overload, namely `Options::weights` and `Options::iterations`.
 */
template<typename Data_>
void compute(
    const std::size_t num_points,
    const Data_* const x,
    const PrecomputedWindows<Data_>& windows,
    const Data_* const y,
    Data_* const fitted,
    Data_* robust_weights,
    const Options<Data_>& opt
) {
    std::vector<Data_> rbuffer;
    if (robust_weights == NULL) {
        sanisizer::resize(rbuffer, num_points);
        robust_weights = rbuffer.data();
    }
    internal::fit_trend(num_points, x, windows, y, fitted, robust_weights, opt);
}

/**
 * Overload of `compute()` that computes the windows around each anchor point.
 *
 * @tparam Data_ Floating-point type of the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-coordinates, sorted in increasing order.
 * (Consider using `SortBy` to permute `x` in-place.
 * Note that the same permutation should be applied to `y` and, if present, weights in `Options::weights`.)
 * @param[in] y Pointer to an array of `num_points` y-coordinates.
 * @param[out] fitted Pointer to an output array of length `num_points`, in which the fitted values of the smoother can be stored.
 * @param[out] robust_weights Pointer to an output array of length `num_points`, in which the robustness weights can be stored.
 * This may be `NULL` if the robustness weights are not needed.
 * @param opt Further options.
 */
template<typename Data_>
void compute(
    const std::size_t num_points,
    const Data_* const x,
    const Data_* const y,
    Data_* const fitted,
    Data_* const robust_weights,
    const Options<Data_>& opt
) {
    const auto win = define_windows(num_points, x, opt);
    compute(num_points, x, win, y, fitted, robust_weights, opt);
}

/** 
 * @brief Results of the LOWESS smoother.
 * @tparam Data_ Floating-point type of the data.
 *
 * Instances of this class are usually created by `compute()`.
 */
template<typename Data_>
struct Results {
    /**
     * @param n Number of points.
     */
    Results(const std::size_t n) :
        fitted(sanisizer::cast<I<decltype(fitted.size())> >(n)),
        robust_weights(sanisizer::cast<I<decltype(robust_weights.size())> >(n))
    {}

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
 * Overload of `compute()` that allocates storage for the results of the smoothing.
 * 
 * @tparam Data_ Floating-point type of the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-coordinates, sorted in increasing order.
 * (Consider using `SortBy` to permute `x` in-place.
 * Note that the same permutation should be applied to `y` and, if present, weights in `Options::weights`.)
 * @param[in] y Pointer to an array of `num_points` y-coordinates.
 * @param opt Further options.
 *
 * @return A `Results` object containing the fitted values and robustness weights.
 */
template<typename Data_>
Results<Data_> compute(
    const std::size_t num_points,
    const Data_* const x,
    const Data_* const y,
    const Options<Data_>& opt
) {
    Results<Data_> output(num_points);
    compute(num_points, x, y, output.fitted.data(), output.robust_weights.data(), opt);
    return output;
}

}

#endif
