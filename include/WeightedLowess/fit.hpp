#ifndef WEIGHTEDLOWESS_FIT_HPP
#define WEIGHTEDLOWESS_FIT_HPP

#include <vector>
#include <algorithm>

#include "window.hpp"
#include "Options.hpp"
#include "robust.hpp"

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
#include "omp.h"
#endif
#endif

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

/* 
 * Computes the lowess fit at all anchor points. This is heavily macroed 
 * depending on the parallelization scheme being used, given that
 * the choice of scheme interacts in bespoke ways with the workspaces.
 */
template<typename Data_>
void fit_all_anchors(
    const std::vector<size_t>& anchors, 
    const std::vector<Window<Data_> >& limits, 
    const Data_* x, 
    const Data_* y, 
    const Data_* weights,
    Data_* fitted,
    Data_* robust_weights,
    std::vector<std::vector<Data_> >& workspaces,
    [[maybe_unused]] int nthreads) 
{
#if defined(WEIGHTEDLOWESS_CUSTOM_PARALLEL)
    WEIGHTEDLOWESS_CUSTOM_PARALLEL(anchors.size(), nthreads, [&](size_t t, size_t start, size_t length) {
        auto& workspace = workspaces[t];
        for (size_t s = start, end = start + length; s < end; ++s) {
            auto curpt = anchors[s];
            fitted[curpt] = fit_point(curpt, limits[s], x, y, weights, robust_weights, workspace);
        }
    });
#elif defined(_OPENMP)
    size_t num_anchors = anchors.size();
    #pragma omp parallel num_threads(nthreads)
    {
        auto& workspace = workspaces[omp_get_thread_num()];
        #pragma omp for
        for (size_t s = 0; s < num_anchors; ++s) { 
            auto curpt = anchors[s];
            fitted[curpt] = fit_point(curpt, limits[s], x, y, weights, robust_weights, workspace);
        }
    }
#else
    auto& workspace = workspaces.front();
    for (size_t s = 0, end = anchors.size(); s < end; ++s) { 
        auto curpt = anchors[s];
        fitted[curpt] = fit_point(curpt, limits[s], x, y, weights, robust_weights, workspace);
    }
#endif
}

/* 
 * Perform interpolation between anchor points. This assumes that the first
 * anchor is the first point and the last anchor is the last point (see
 * find_anchors() for an example). Note that we do this in a separate parallel
 * session from fit_all_anchors() to ensure that all 'fitted' values are
 * available for all anchors across all threads.
 */
template<typename Data_>
void interpolate_between_anchors(const std::vector<size_t>& anchors, const Data_* x, Data_* fitted, [[maybe_unused]] int nthreads) {
    size_t num_anchors = anchors.size();

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
    #pragma omp parallel for num_threads(nthreads)
#endif
    for (size_t s = 1; s < num_anchors; ++s) { 
#else
    WEIGHTEDLOWESS_CUSTOM_PARALLEL(num_anchors - 1, nthreads, [&](size_t, size_t start, size_t length) {
    auto start_p1 = start + 1;
    for (size_t s = start_p1, end = start_p1 + length; s < end; ++s) {
#endif

        auto curpt = anchors[s];
        auto last_anchor = anchors[s - 1];

        if (curpt - last_anchor > 1) { // only interpolate if there are points between anchors.
            Data_ current = x[curpt] - x[last_anchor];
            if (current > 0) {
                const Data_ slope = (fitted[curpt] - fitted[last_anchor])/current;
                const Data_ intercept = fitted[curpt] - slope * x[curpt];
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
                for (size_t subpt = last_anchor + 1; subpt < curpt; ++subpt) {
                    fitted[subpt] = ave;
                }
            }
        }

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
    }
#else
    }
    });
#endif
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

    size_t nthreads = opt.num_threads; // this better be positive.
    std::vector<std::vector<Data_> > workspaces(nthreads);
    for (auto& w : workspaces) {
        w.resize(num_points);
    }

    for (int it = 0; it <= opt.iterations; ++it) { // Robustness iterations.
        fit_all_anchors(anchors, limits, x, y, opt.weights, fitted, robust_weights, workspaces, nthreads);
        interpolate_between_anchors(anchors, x, fitted, nthreads);

        if (it < opt.iterations) {
            auto& abs_dev = workspaces.front();
            auto cmad = compute_mad(num_points, y, fitted, freq_weights, totalweight, abs_dev, residual_permutation, nthreads);
            cmad *= 6;
            cmad = std::max(cmad, min_mad); // avoid difficulties from numerical precision when all residuals are theoretically zero.
            populate_robust_weights(abs_dev, cmad, robust_weights, nthreads);

            /* Both limma::weightedLowess and the original Fortran code have an early
             * termination condition that stops the robustness iterations when the MAD
             * is small. We do not implement this and just allow the specified number of
             * iterations to run, as the termination can fail in pathological examples
             * where a minority of points are affected by an outlier. In such cases,
             * the MAD may indeed be very small as most residuals are fine, and we would
             * fail to robustify against the few outliers.
             */
        }
    }

    return;
}

}

}

#endif
