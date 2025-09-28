#ifndef WEIGHTEDLOWESS_FIT_HPP
#define WEIGHTEDLOWESS_FIT_HPP

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstddef>

#include "subpar/subpar.hpp"
#include "sanisizer/sanisizer.hpp"

#include "window.hpp"
#include "Options.hpp"
#include "robust.hpp"
#include "parallelize.hpp"

namespace WeightedLowess {

namespace internal {

template<typename Data_>
Data_ cube(Data_ x) {
    return x * x * x;
}

/* 
 * Computes the lowess fit at a given point using linear regression with a
 * combination of tricube, prior and robustness weighting. 
 */
template<typename Data_>
Data_ fit_point (
    const std::size_t curpt,
    const Window<Data_>& limits, 
    const Data_* const x,
    const Data_* const y,
    const Data_* const weights, 
    const Data_* const robust_weights, 
    std::vector<Data_>& work)
{
    const auto left = limits.left, right = limits.right;
    const Data_ dist = limits.distance;

    if (dist <= 0) {
        Data_ ymean = 0, allweight = 0;
        for (auto pt = left; pt <= right; ++pt) {
            Data_ curweight = (weights != NULL ? robust_weights[pt] * weights[pt] : robust_weights[pt]);
            ymean += y[pt] * curweight;
            allweight += curweight;
        }

        if (allweight == 0) { // ignore the robustness weights.
            for (auto pt = left; pt <= right; ++pt) {
                Data_ curweight = (weights != NULL ? weights[pt] : static_cast<Data_>(1));
                ymean += y[pt] * curweight;
                allweight += curweight;
            }
        }

        ymean /= allweight;
        return ymean;
    }

    Data_ xmean = 0, ymean = 0, allweight = 0;
    for (auto pt = left; pt <= right; ++pt) {
        const Data_ curw = cube(static_cast<Data_>(1) - cube(std::abs(x[curpt] - x[pt])/dist)) * robust_weights[pt];
        const Data_ current = (weights != NULL ? curw * weights[pt] : curw);
        xmean += current * x[pt];
        ymean += current * y[pt];
        allweight += current;
        work[pt] = current;
    }

    if (allweight == 0) { // ignore the robustness weights.
        for (auto pt = left; pt <= right; ++pt) {
            const Data_ curw = cube(static_cast<Data_>(1) - cube(std::abs(x[curpt] - x[pt])/dist));
            const Data_ current = (weights != NULL ? curw * weights[pt] : curw);
            xmean += current * x[pt];
            ymean += current * y[pt];
            allweight += current;
            work[pt] = current;
        }
    }

    xmean /= allweight;
    ymean /= allweight;

    Data_ var = 0, covar = 0;
    for (auto pt = left; pt <= right; ++pt) {
        const Data_ temp = x[pt] - xmean;
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

/* This is a C++ version of the local weighted regression (lowess) trend fitting algorithm,
 * based on the Fortran code in lowess.f from http://www.netlib.org/go written by Cleveland.
 * Consideration of non-equal prior weights is added to the span calculations and linear
 * regression. These weights are intended to have the equivalent effect of frequency weights
 * (at least, in the integer case; extended by analogy to all non-negative values).
 */
template<typename Data_>
void fit_trend(
    const std::size_t num_points,
    const Data_* const x,
    const PrecomputedWindows<Data_>& windows,
    const Data_* const y,
    Data_* const fitted,
    Data_* const robust_weights,
    const Options<Data_>& opt
) {
    if (num_points == 0) {
        return;
    }

    const auto& anchors = windows.anchors;
    const auto freq_weights = windows.freq_weights;
    const Data_ totalweight = windows.total_weight;
    const auto& limits = windows.limits;

    // Setting up the robustness weights, if robustification is requested.
    std::fill_n(robust_weights, num_points, 1);
    Data_ min_threshold = 0; 
    std::vector<std::size_t> residual_permutation;
    constexpr Data_ threshold_multiplier = 1e-8;

    if (opt.iterations) {
        /* If the range of 'y' is zero, we just quit early. Otherwise, we use
         * the range to set a lower bound on the robustness threshold to avoid
         * problems with divide-by-zero. We don't use the MAD of 'y' as it
         * could be conceivable that we would end up with a threshold of zero
         * again, e.g., if the majority of points have the same value.
         */
        const Data_ range = (*std::max_element(y, y + num_points) - *std::min_element(y, y + num_points));
        if (range == 0) {
            std::copy_n(y, num_points, fitted);
            return;
        }
        min_threshold = range * threshold_multiplier;
    }

    const auto num_anchors = anchors.size();
    auto workspaces = sanisizer::create<std::vector<std::vector<Data_> > >(opt.num_threads);

    I<decltype(opt.iterations)> it = 0;
    while (1) { // Robustness iterations.
        parallelize(opt.num_threads, num_anchors, [&](const int t, const I<decltype(num_anchors)> start, const I<decltype(num_anchors)> length) {
            auto& workspace = workspaces[t];
            sanisizer::resize(workspace, num_points); // resizing inside the thread to encourage allocations to a thread-specific heap to avoid false sharing.
            for (I<decltype(start)> s = start, end = start + length; s < end; ++s) {
                const auto curpt = anchors[s];
                fitted[curpt] = fit_point(curpt, limits[s], x, y, opt.weights, robust_weights, workspace);
            }
        });

        /* Perform interpolation between anchor points. This assumes that the first
         * anchor is the first point and the last anchor is the last point (see
         * find_anchors() for an example). Note that we do this in a separate parallel
         * session from the anchor fitting ensure that all 'fitted' values are
         * available for all anchors across all threads.
         *
         * One would think that we should parallelize across x instead of the anchors,
         * as this has better worksharing when x is not evenly distributed across anchor segments.
         * However, if we did so, we'd have to store the slope and intercept for the anchor segments first,
         * then look up the slope and intercept for each element of x.
         * This involves an extra memory access and is not SIMD-able.
         */
        const auto num_anchors_m1 = num_anchors - 1;
        parallelize(opt.num_threads, num_anchors_m1, [&](const int, const I<decltype(num_anchors_m1)> start, const I<decltype(num_anchors_m1)> length) {
            for (I<decltype(start)> s = start, end = start + length; s < end; ++s) {
                const auto left_anchor = anchors[s];
                const auto right_anchor = anchors[s + 1];
                if (right_anchor - left_anchor <= 1) { // only interpolate if there are points between anchors.
                    continue;
                }

                const Data_ xdiff = x[right_anchor] - x[left_anchor];
                const Data_ ydiff = fitted[right_anchor] - fitted[left_anchor];
                if (xdiff > 0) {
                    const Data_ slope = ydiff / xdiff;
                    const Data_ intercept = fitted[right_anchor] - slope * x[right_anchor];
                    for (I<decltype(right_anchor)> subpt = left_anchor + 1; subpt < right_anchor; ++subpt) { 
                        fitted[subpt] = slope * x[subpt] + intercept; 
                    }
                } else {
                    /* Some protection is provided against infinite slopes.
                     * This shouldn't be a problem for non-zero delta; the only
                     * concern is at the final point where the covariate
                     * distance may be zero.
                     */
                    const Data_ ave = fitted[left_anchor] + ydiff / 2;
                    std::fill(fitted + left_anchor + 1, fitted + right_anchor, ave);
                }
            }
        });

        // Using a manual break to avoid overflow of 'it' in a for loop that requires
        // one last iteration at 'it == opt.iterations'.
        if (it == opt.iterations) {
            break;
        }

        /* Both limma::weightedLowess and the original Fortran code have an
         * early termination condition that stops the robustness iterations
         * when the MAD is "small" (relative to the sum of the absolute
         * deviations). We do not implement this and just allow the specified
         * number of iterations to run, as the termination can fail in
         * pathological examples where a minority of points are affected by a
         * neighboring outlier. In such cases, the MAD may indeed be very small
         * as most residuals are fine, and we would terminate early and fail to
         * robustify against the few outliers.
         */
        if (it > 0) {
            /* That said, we do quit if the range of the non-outlier points
             * is exactly zero, because that implies that we should already
             * have a perfect fit among all of these points.
             */
            const auto range = compute_robust_range(num_points, y, robust_weights);
            if (range == 0) {
                break;
            }

            /* We redefine the minimum threshold from the non-outlier
             * points. This ensures that a massive outlier at the start
             * does not continue to inflate the 'min_threshold', even after
             * it has been rendered irrelevant by the robustness weighting.
             */
            min_threshold = range * threshold_multiplier;
        }

        auto& abs_dev = workspaces.front(); // just using the first workspace as a spare buffer, not using any values therein.
        auto cmad = compute_mad(num_points, y, fitted, freq_weights, totalweight, abs_dev, residual_permutation);
        cmad *= 6;
        cmad = std::max(cmad, min_threshold); // avoid difficulties from numerical precision when all residuals are theoretically zero.
        populate_robust_weights(abs_dev, cmad, robust_weights);
        ++it;
    }

    return;
}

}

}

#endif
