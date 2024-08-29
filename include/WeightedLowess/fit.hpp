#ifndef WEIGHTEDLOWESS_FIT_HPP
#define WEIGHTEDLOWESS_FIT_HPP

#include <vector>
#include <algorithm>
#include <cmath>

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
    const size_t curpt,
    const Window<Data_>& limits, 
    const Data_* x,
    const Data_* y,
    const Data_* weights, 
    const Data_* robust_weights, 
    std::vector<Data_>& work)
{
    size_t left = limits.left, right = limits.right;
    Data_ dist = limits.distance;

    if (dist <= 0) {
        Data_ ymean = 0, allweight = 0;
        for (size_t pt = left; pt <= right; ++pt) {
            Data_ curweight = (weights != NULL ? robust_weights[pt] * weights[pt] : robust_weights[pt]);
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
void fit_trend(size_t num_points, const Data_* x, const PrecomputedWindows<Data_>& windows, const Data_* y, Data_* fitted, Data_* robust_weights, const Options<Data_>& opt) {
    if (num_points == 0) {
        return;
    }

    const auto& anchors = windows.anchors;
    const Data_* freq_weights = windows.freq_weights;
    const Data_ totalweight = windows.total_weight;
    const auto& limits = windows.limits;

    // Setting up the robustness weights, if robustification is requested.
    std::fill_n(robust_weights, num_points, 1);
    Data_ min_threshold = 0; 
    std::vector<size_t> residual_permutation;
    constexpr Data_ threshold_multiplier = 1e-8;

    if (opt.iterations) {
        residual_permutation.resize(num_points);

        /* If the range of 'y' is zero, we just quit early. Otherwise, we use
         * the range to set a lower bound on the robustness threshold to avoid
         * problems with divide-by-zero. We don't use the MAD of 'y' as it
         * could be conceivable that we would end up with a threshold of zero
         * again, e.g., if the majority of points have the same value.
         */
        Data_ range = (*std::max_element(y, y + num_points) - *std::min_element(y, y + num_points));
        if (range == 0) {
            std::copy_n(y, num_points, fitted);
            return;
        }
        min_threshold = range * threshold_multiplier;
    }

    size_t nthreads = opt.num_threads; // this better be positive.
    std::vector<std::vector<Data_> > workspaces(nthreads);
    for (auto& w : workspaces) {
        w.resize(num_points);
    }

    size_t num_anchors = anchors.size();
    for (int it = 0; it <= opt.iterations; ++it) { // Robustness iterations.
        WEIGHTEDLOWESS_CUSTOM_PARALLEL(nthreads, num_anchors, [&](int t, size_t start, size_t length) {
            auto& workspace = workspaces[t];
            for (size_t s = start, end = start + length; s < end; ++s) {
                auto curpt = anchors[s];
                fitted[curpt] = fit_point(curpt, limits[s], x, y, opt.weights, robust_weights, workspace);
            }
        });

        /* Perform interpolation between anchor points. This assumes that the first
         * anchor is the first point and the last anchor is the last point (see
         * find_anchors() for an example). Note that we do this in a separate parallel
         * session from the anchor fitting ensure that all 'fitted' values are
         * available for all anchors across all threads.
         */
        WEIGHTEDLOWESS_CUSTOM_PARALLEL(nthreads, num_anchors - 1, [&](size_t, size_t start, size_t length) {
            auto start_p1 = start + 1;
            for (size_t s = start_p1, end = start_p1 + length; s < end; ++s) {
                auto curpt = anchors[s];
                auto last_anchor = anchors[s - 1];

                if (curpt - last_anchor > 1) { // only interpolate if there are points between anchors.
                    Data_ current = x[curpt] - x[last_anchor];
                    if (current > 0) {
                        const Data_ slope = (fitted[curpt] - fitted[last_anchor])/current;
                        const Data_ intercept = fitted[curpt] - slope * x[curpt];
#ifdef _OPENMP
                        #pragma omp simd
#endif
                        for (size_t subpt = last_anchor + 1; subpt < curpt; ++subpt) { 
                            fitted[subpt] = slope * x[subpt] + intercept; 
                        }
                    } else {
                        /* Some protection is provided against infinite slopes.
                         * This shouldn't be a problem for non-zero delta; the only
                         * concern is at the final point where the covariate
                         * distance may be zero.
                         */
                        const Data_ ave = (fitted[curpt] + fitted[last_anchor]) / 2;
                        std::fill(fitted + last_anchor + 1, fitted + curpt, ave);
                    }
                }
            }
        });

        if (it < opt.iterations) {
            /* Both limma::weightedLowess and the original Fortran code have an early
             * termination condition that stops the robustness iterations when the MAD
             * is "small". We do not implement this and just allow the specified number of
             * iterations to run, as the termination can fail in pathological examples
             * where a minority of points are affected by an outlier. In such cases,
             * the MAD may indeed be very small as most residuals are fine, and we would
             * fail to robustify against the few outliers.
             *
             * That said, we do quit if the range of the existing (non-outlier) points 
             * is exactly zero, because that implies that we should have a perfect fit
             * among all the remaining points. We also use this range to refine the minimum
             * threshold. This ensures that a massive outlier at the start does not 
             * continue to inflate the 'min_threshold', even after it has been rendered 
             * irrelevant by the robustness weighting.
             */
            if (it > 0) {
                auto range = compute_robust_range(num_points, y, robust_weights);
                if (range == 0) {
                    break;
                }
                min_threshold = range * threshold_multiplier;
            }

            auto& abs_dev = workspaces.front(); // just using the first workspace as a spare buffer, not using any values therein.
            auto cmad = compute_mad(num_points, y, fitted, freq_weights, totalweight, abs_dev, residual_permutation, nthreads);
            cmad *= 6;
            cmad = std::max(cmad, min_threshold); // avoid difficulties from numerical precision when all residuals are theoretically zero.
            populate_robust_weights(abs_dev, cmad, robust_weights);
        }
    }

    return;
}

}

}

#endif
