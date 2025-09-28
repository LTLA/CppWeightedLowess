#ifndef WEIGHTEDLOWESS_OPTIONS_HPP
#define WEIGHTEDLOWESS_OPTIONS_HPP

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
     * By default, this is interpreted as a proportion of the total number of points used in `compute()` and should be between 0 and 1.
     * Each window is defined as the smallest interval centered on the current point that covers the specified proportion.
     *
     * If `weights` are provided to `compute()` and `Options::frequency_weights = true`, the span is instead defined from the proportion of the total weight across all points.
     * This interprets the weights on each observation as relative frequencies.
     *
     * See also `Options::span_as_proportion`, which changes the interpretation of this option.
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
     * This is useful for forcing the creation of a larger window in highly dense regions of the covariate range.
     * Otherwise, overly small windows can lead to overfitting and a "bumpy" trend.
     */
    Data_ minimum_width = 0;

    /**
     * The number of points that can be used as "anchors".
     * LOWESS smoothing is performed exactly for each anchor, while the fitted values for all intervening points are computed by linear interpolation.
     * A higher number of anchor points improves accuracy at the cost of computational work.
     *
     * Note that this number is only used as a guideline by our LOWESS implementation.
     * The actual number of selected anchors depends on the distribution of x-coordinates; in addition, the first and last points are always used as the anchors,.
     * If the specified number of anchors is greater than the number of points, LOWESS smoothing is performed directly for each point.
     *
     * This setting is ignored if `Options::delta` is non-negative.
     */
    std::size_t anchors = 200;

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
     * Otherwise, the chosen `delta` should have similar magnitude to the range of the x-coordinates.
     */
    Data_ delta = -1;

    /**
     * Pointer to an array of length equal to the number of points used in `compute()`.
     * Each element should be a positive weight for the corresponding point in `x` and `y`.
     * Alternatively, this may be `NULL` in which case all points are equally weighted.
     */
    const Data_* weights = NULL;

    /** 
     * Whether the weights (if provided in `Options::weights`) should be interpreted as frequency weights.
     * This means that they will be involved in both the span calculations for the smoothing window around each point, as well as in the LOWESS calculations themselves.
     * If `false`, the weights will only be used for the latter.
     */
    bool frequency_weights = true;

    /**
     * Number of threads to use for various steps.
     * This should be a positive integer.
     * The parallelization scheme is determined by the #WEIGHTEDLOWESS_CUSTOM_PARALLEL macro. 
     */
    int num_threads = 1;
};

}

#endif
