#ifndef WEIGHTEDLOWESS_OPTIONS_HPP
#define WEIGHTEDLOWESS_OPTIONS_HPP

#include <algorithm>
#include <vector>
#include <cstdint>
#include <cmath>
#include <numeric>

#include "window.hpp"

/**
 * @file Options.hpp
 *
 * @brief Options for `compute()`.
 */

namespace WeightedLowess {

/**
 * @brief Options for `compute()`.
 * @tparam Data_ Floating-point type for the data.
 */
template<typename Data_ = double>
struct Options {
    /**
     * Span of the smoothing window around each point.
     * This is parametrized as a proportion of the total number of points and should be between 0 and 1.
     * Each window is defined as the smallest interval centered on the current point that covers the specified proportion.
     *
     * If `weights` are provided to `compute()` and `Options::frequency_weights = true`, the span is instead defined from the proportion of the total weight across all points.
     * This interprets the weights on each observation as relative frequencies.
     */
    Data_ span = 0.3;

    /**
     * Whether the span should be interpreted as a proportion of the total number of points.
     * If `false`, the value in `Options::span` is directly interpreted as the number of points that must fall inside the window.
     * If `false` and `weights` are provided to `compute()` and `Options::frequency_weights = true`, the value in `Options::span` is interpreted as the sum of weights inside the window.
     */
    bool span_as_proportion = true;

    /**
     * Minimum width of the window centered around each point.
     * This is useful for forcing the use of a larger window in highly dense regions of the covariate range.
     */
    Data_ minimum_width = 0;

    /**
     * The number of points that can be used as "anchors".
     * LOWESS smoothing is performed exactly for each anchor, while the fitted values for all intervening points are computed by linear interpolation.
     * A higher number of anchor points improves accuracy at the cost of computational work.
     *
     * Note that this number is only used as a guideline by our LOWESS implementation.
     * The actual number of selected anchors depends on the distribution of x-values; in addition, the first and last points are always used as the anchors,.
     * If the specified number of anchors is greater than the number of points, LOWESS smoothing is performed directly for each point.
     *
     * This setting is ignored if `Options::delta` is non-negative.
     */
    size_t anchors = 200;

    /**
     * The number of robustness iterations.
     * At each iteration, each point is weighted according to its difference from the smoothed value, and the smoothing is repeated with these weights. 
     * More iterations increase robustness to outliers at the cost of computational work.
     */
    int iterations = 3;

    /**
     * Delta value used to identify anchors.
     * Seeds are identified greedily, by walking through the ordered x-coordinate values and marking a point `y` as a anchor if there are no anchors in `[y - delta, y]`.
     * If set to zero, all unique points are used as anchors.
     * If set to a negative value, an appropriate delta is determined from the number of points specified in `set_points()`.
     * Otherwise, the chosen `delta` should have similar magnitude to the range of the x-values.
     */
    Data_ delta = -1;

    /**
     * Pointer to an array of length equal to the number of points used in `compute()`.
     * Each element should be a positive weight for the corresponding point in `x` and `y`.
     * Alternatively, this may be `NULL` in which case all points are equally weighted.
     */
    Data_* weights = NULL;

    /** 
     * Whether the weights (if provided in `Options::weights`) should be interpreted as frequency weights.
     * This means that they will be involved in both the span calculations for the smoothing window around each point, as well as in the LOWESS calculations themselves.
     * If `false`, the weights will only be used for the latter.
     */
    bool frequency_weights = true;

    /**
     * Number of threads to use for various steps.
     * This should be a positive integer.
     *
     * By default, `compute()` uses OpenMP for parallelization.
     * Applications can override this by setting the `WEIGHTEDLOWESS_CUSTOM_PARALLEL` function-like macro, e.g., in environments where OpenMP is not available.
     * This macro should accept three arguments:
     * - `num_jobs`, a `size_t` specifying the number of tasks to perform.
     * - `num_threads`, an `int` specifying the number of threads to use.
     * - `fun`, a function that accepts three `size_t` values.
     * .
     * The macro should partition the tasks into blocks, assign each block to a thread,
     * and call `fun()` in each thread with the thread number (an ID in `[0, num_threads)`), start position and length of its block.
     */
    int num_threads = 1;
};

}

#endif
