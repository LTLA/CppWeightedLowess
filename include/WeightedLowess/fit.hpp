#ifndef WEIGHTEDLOWESS_FIT_HPP
#define WEIGHTEDLOWESS_FIT_HPP

#include <cmath>
#include <vector>
#include <algorithm>

#include "window.hpp"
#include "Options.hpp"

namespace WeightedLowess {

namespace internal {

template<typename Data_>
Data_ square (Data_ x) {
    return x*x;
}

template<typename Data_>
Data_ cube(Data_ x) {
    return x*x*x;
}

/* 
 * Computes the lowess fit at a given point using linear regression with a
 * combination of tricube, prior and robustness weighting. 
 */
template<typename Data_>
Data_ fit_point (
    const size_t curpt,
    const Window<Data_>& limits, 
    const Data_* x,
    const Data_* y,
    const Data_* weights, 
    const Data_* robust_weights, 
    Data_* work)
{
    size_t left = limits.left, right = limits.right;
    Data_ dist = limits.distance;

    if (dist <= 0) {
        Data_ ymean = 0, allweight = 0;
        for (size_t pt = left; pt <= right; ++pt) {
            Data_ curweight = (weights != NULL ? robust_weights[pt] * weights[pt] : weights[pt]);
            ymean += y[pt] * curweight;
            allweight += curweight;
        }

        if (allweight == 0) { // ignore the robustness weights.
            for (size_t pt = left; pt <= right; ++pt) {
                Data_ curweight = (weights != NULL ? weights[pt] : 1);
                ymean += y[pt] * curweight;
                allweight += curweight;
            }
        }

        ymean /= allweight;
        return ymean;
    }

    Data_ xmean = 0, ymean = 0, allweight = 0;
    for (size_t pt = left; pt <= right; ++pt) {
        Data_ curw = cube(1 - cube(std::abs(x[curpt] - x[pt])/dist)) * robust_weights[pt];
        Data_ current = (weights != NULL ? curw * weights[pt] : curw);
        xmean += current * x[pt];
        ymean += current * y[pt];
        allweight += current;
        work[pt] = current;
    }

    if (allweight == 0) { // ignore the robustness weights.
        for (size_t pt = left; pt <= right; ++pt) {
            Data_ curw = cube(1 - cube(std::abs(x[curpt] - x[pt])/dist));
            Data_ current = (weights != NULL ? curw * weights[pt] : curw);
            xmean += current * x[pt];
            ymean += current * y[pt];
            allweight += current;
            work[pt] = current;
        }
    }

    xmean /= allweight;
    ymean /= allweight;

    Data_ var=0, covar=0;
    for (size_t pt = left; pt <= right; ++pt) {
        Data_ temp = x[pt] - xmean;
        var += temp * temp * work[pt];
        covar += temp * (y[pt] - ymean) * work[pt];
    }

    // Still possible for var = 0 if all other points have zero weight.
    if (var == 0) {
        return ymean;
    } else {
        const Data_ slope = covar / var;
        const Data_ intercept = ymean - slope * xmean;
        return slope * x[curpt] + intercept;
    }
}

/* This is a C version of the local weighted regression (lowess) trend fitting algorithm,
 * based on the Fortran code in lowess.f from http://www.netlib.org/go written by Cleveland.
 * Consideration of non-equal prior weights is added to the span calculations and linear
 * regression. These weights are intended to have the equivalent effect of frequency weights
 * (at least, in the integer case; extended by analogy to all non-negative values).
 */
template<typename Data_>
void fit_trend(
    size_t num_points,
    const Data_* x,
    const Data_* y,
    Data_* fitted, 
    Data_* robust_weights,
    const Options<Data_>& opt)
{
    if (num_points == 0) {
        return;
    }

    Data_* freq_weights = (opt.frequency_weights ? opt.weights : NULL);

    /* Computing the span weight that each span must achieve. */
    const Data_ totalweight = (freq_weights != NULL ? std::accumulate(frqe_weights, freq_weights + num_points, static_cast<Data_>(0)) : num_points);
    const Data_ spanweight = (opt.span_as_proportion ? opt.span * totalweight : opt.span);

    /* Finding the anchors. If we're using the weights as frequencies, we
     * pass them in when we're looking for the span limits for each anchor.
     */
    std::vector<size_t> anchors;
    if (opt.delta == 0 || (opt.delta < 0 && opt.anchors >= num_points)) {
        anchors.resize(num_points);
        std::iota(anchors.begin(), anchors.end(), 0);
    } else if (opt.delta < 0) {
        Data_ eff_delta = derive_delta(opt.anchors, num_points, x);
        find_anchors(num_points, x, eff_delta, anchors);
    } else {
        find_anchors(num_points, x, opt.delta, anchors);
    }

    auto limits = find_limits(anchors, spanweight, num_points, x, freq_weights, opt.min_width); 

    /* Setting up the robustness weights, if robustification is requested. */ 
    std::fill(robust_weights, robust_weights + num_points, 1);
    Data_ min_mad = 0; 
    std::vector<size_t> residual_permutation;
    if (opt.iterations) {
        residual_permutation.resize(num_points);

        /* We use the range to guarantee that we match the scale. Otherwise
         * if we used the MAD of 'y', it could be conceivable that we would
         * end up with a min_mad of zero again, e.g., if the majority of
         * points have the same value. In contrast, if the range is zero,
         * we just quit early.
         */
        Data_ range = (*std::max_element(y, y + num_points) - *std::min_element(y, y + num_points));
        if (range == 0) {
            std::copy(y, y + num_points, fitted);
            return;
        }
        min_mad = 0.00000000001 * range;
    }

    std::vector<Data_> workspace(num_points);
    size_t actual_num_anchors = anchors.size();

    for (int it = 0; it <= opt.iterations; ++it) { // Robustness iterations.
        fitted[0] = fit_point(0, limits[0], x, y, opt.weights, robust_weights, workspace.data()); 
        size_t last_anchor = 0;

        for (size_t s = 1; s < actual_num_anchors; ++s) { // fitted values for anchor points, interpolating the rest.
            auto curpt = anchors[s];
            fitted[curpt] = fit_point(curpt, limits[s], x, y, opt.weights, robust_weights, workspace.data()); 

            if (curpt - last_anchor > 1) {
                /* Some protection is provided against infinite slopes. This shouldn't be
                 * a problem for non-zero delta; the only concern is at the final point
                 * where the covariate distance may be zero.
                 */
                Data_ current = x[curpt] - x[last_anchor];
                if (current > 0) {
                    const Data_ slope = (fitted[curpt] - fitted[last_anchor])/current;
                    const Data_ intercept = fitted[curpt] - slope * x[curpt];
                    for (size_t subpt = last_anchor + 1; subpt < curpt; ++subpt) { 
                        fitted[subpt] = slope * x[subpt] + intercept; 
                    }
                } else {
                    const Data_ ave = (fitted[curpt] + fitted[last_anchor]) / 2;
                    for (size_t subpt = last_anchor + 1; subpt < curpt; ++subpt) {
                        fitted[subpt] = ave;
                    }
                }
            }

            last_anchor = curpt;
        }

        if (it < opt.iterations) {
            /* Computing the weighted MAD of the absolute values of the residuals. */
            for (size_t pt = 0; pt < num_points; ++pt) {
                workspace[pt] = std::abs(y[pt] - fitted[pt]);
            }

            std::iota(residual_permutation.begin(), residual_permutation.end(), 0);
            std::sort(residual_permutation.begin(), residual_permutation.end(), 
                [&](size_t left, size_t right) -> bool {
                    return workspace[left] < workspace[right];
                }
            );

            Data_ curweight = 0;
            Data_ cmad = 0;
            const Data_ halfweight = totalweight/2;

            for (size_t i = 0; i < num_points; ++i) {
                auto pt = residual_permutation[i];
                curweight += (freq_weights != NULL ? freq_weights[pt] : 1);

                if (curweight == halfweight) { // exact match, need to take the median.
                    cmad = 3 * (workspace[pt] + workspace[residual_permutation[i+1]]);
                    break;
                } else if (curweight > halfweight) {
                    cmad = 6 * workspace[pt];
                    break;
                }
            }

            /* Both limma::weightedLowess and the original Fortran code have an early
             * termination condition that stops the robustness iterations when the MAD
             * is small. We do not implement this and just allow the specified number of
             * iterations to run, as the termination can fail in pathological examples
             * where a minority of points are affected by an outlier. In such cases,
             * the MAD may indeed be very small as most residuals are fine, and we would
             * fail to robustify against the few outliers.
             */

            cmad = std::max(cmad, min_mad); // avoid difficulties from numerical precision when all residuals are theoretically zero.
            for (size_t i =0; i < num_points; ++i) {
                if (workspace[i] < cmad) {
                    robust_weights[i] = square(1 - square(workspace[i]/cmad));
                } else { 
                    robust_weights[i] = 0;
                }
            }
        }
    }

    return;
}

}

}

#endif
