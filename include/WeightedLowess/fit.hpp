#ifndef WEIGHTEDLOWESS_FIT_HPP
#define WEIGHTEDLOWESS_FIT_HPP

#include <cmath>
#include <vector>
#include <algorithm>

#include "window.hpp"
#include "Options.hpp"

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
#include "omp.h"
#endif
#endif

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
    const Data_ totalweight = (freq_weights != NULL ? std::accumulate(freq_weights, freq_weights + num_points, static_cast<Data_>(0)) : num_points);
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

    size_t actual_num_anchors = anchors.size(); // Note that opt.anchors may not equal actual_num_anchors, the former is more of a guideline. 
    auto limits = find_limits(anchors, spanweight, num_points, x, freq_weights, opt.minimum_width, opt.num_threads); 

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

#if defined(WEIGHTEDLOWESS_CUSTOM_PARALLEL) || defined(_OPENMP)
    size_t nthreads = opt.num_threads; // this better be positive.
    std::vector<std::vector<Data_> > workspaces(nthreads);
    for (auto& w : workspaces) {
        w.resize(num_points);
    }
#else
    std::vector<Data_> workspace(num_points);
#endif

    for (int it = 0; it <= opt.iterations; ++it) { // Robustness iterations.
        // Computing the fitted values for anchor points. Some shenanigans are 
        // required for parallelization with thread-specific workspaces.
#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
        #pragma omp parallel num_threads(nthreads)
        {
        auto& workspace = workspaces[omp_get_thread_num()];
        #pragma omp for
#endif
        for (size_t s = 0; s < actual_num_anchors; ++s) { 
#else
        WEIGHTEDLOWESS_CUSTOM_PARALLEL(actual_num_anchors, nthreads, [&](size_t t, size_t start, size_t length) {
        auto& workspace = workspaces[t];
        for (size_t s = start, end = start + length; s < end; ++s) {
#endif

            auto curpt = anchors[s];
            fitted[curpt] = fit_point(curpt, limits[s], x, y, opt.weights, robust_weights, workspace);

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
        }
#endif
        }
#else
        }
        });
#endif

        // Now computing the interpolation, under the assumption that the first
        // anchor is the first point and the last anchor is the last point. We
        // do this in a separate parallel session to ensure that all 'fitted'
        // values are available for all anchors across all threads.
#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
        #pragma omp parallel for num_threads(nthreads)
#endif
        for (size_t s = 1; s < actual_num_anchors; ++s) { 
#else
        WEIGHTEDLOWESS_CUSTOM_PARALLEL(actual_num_anchors - 1, nthreads, [&](size_t, size_t start, size_t length) {
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

        if (it < opt.iterations) {
#if defined(WEIGHTEDLOWESS_CUSTOM_PARALLEL) || defined(_OPENMP)
            auto& workspace = workspaces.front();
#endif

            // Computing the weighted MAD of the absolute values of the residuals. 
#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
            #pragma omp parallel for num_threads(nthreads)
#endif
            for (size_t pt = 0; pt < num_points; ++pt) {
#else
            WEIGHTEDLOWESS_CUSTOM_PARALLEL(num_points, nthreads, [&](size_t, size_t start, size_t length) {
            for (size_t pt = start, end = start + length; pt < end; ++pt) {
#endif

                workspace[pt] = std::abs(y[pt] - fitted[pt]);

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
            }
#else
            }
            });
#endif

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

            cmad = std::max(cmad, min_mad); // avoid difficulties from numerical precision when all residuals are theoretically zero.

            /* Both limma::weightedLowess and the original Fortran code have an early
             * termination condition that stops the robustness iterations when the MAD
             * is small. We do not implement this and just allow the specified number of
             * iterations to run, as the termination can fail in pathological examples
             * where a minority of points are affected by an outlier. In such cases,
             * the MAD may indeed be very small as most residuals are fine, and we would
             * fail to robustify against the few outliers.
             */

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
            #pragma omp parallel for num_threads(nthreads)
#endif
            for (size_t i = 0; i < num_points; ++i) {
#else
            WEIGHTEDLOWESS_CUSTOM_PARALLEL(num_points, nthreads, [&](size_t, size_t start, size_t length) {
            for (size_t i = start, end = start + length; i < end; ++i) {
#endif

                if (workspace[i] < cmad) {
                    robust_weights[i] = square(1 - square(workspace[i]/cmad));
                } else { 
                    robust_weights[i] = 0;
                }

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
            }
#else
            }
            });
#endif

        }
    }

    return;
}

}

}

#endif
