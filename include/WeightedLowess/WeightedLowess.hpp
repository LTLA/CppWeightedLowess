#ifndef SCRAN_WEIGHTED_LOWESS_H 
#define SCRAN_WEIGHTED_LOWESS_H 

#include <algorithm>
#include <vector>
#include <cstdint>
#include <cmath>
#include <numeric>

#include <iostream>

/**
 * @file WeightedLowess.hpp
 *
 * @brief LOWESS implementation generalized for frequency weights.
 */

namespace WeightedLowess {

/**
 * @brief Run a (weighted) LOWESS algorithm from vectors of x- and y-coordinates.
 *
 * LOWESS is a simple, efficient, general-purpose non-parametric smoothing algorithm. 
 * As the name suggests, the idea is to perform linear regressions on subsets of neighboring points, yielding a smooth curve fit to the data.
 * The `run()` method performs the smoothing while the various setter methods (e.g., `set_span()`) allow users to easily modify optional parameters.
 * 
 * @tparam Data_t Floating-point type for the data.
 */
template<typename Data_t = double>
class WeightedLowess {
public:
    /**
     * @brief Default parameter settings.
     */
    struct Defaults {
        /**
         * See `set_span()` for more details.
         */
        static constexpr Data_t span = 0.3;

        /**
         * See `set_span_as_proportion()` for more details.
         */
        static constexpr bool span_as_proportion = true;

        /**
         * See `set_min_width()` for more details.
         */
        static constexpr Data_t min_width = 0;

        /**
         * See `set_anchors()` for more details.
         */
        static constexpr int anchors = 200;

        /**
         * See `set_iterations()` for more details.
         */
        static constexpr int iterations = 3;

        /**
         * See `set_delta()` for more details.
         */
        static constexpr Data_t delta = -1;

        /**
         * See `set_sorted()` for more details.
         */
        static constexpr bool sorted = false;

        /**
         * See `set_frequency_weights()` for more details.
         */
        static constexpr bool frequency_weights = true;
    };

public:
    /**
     * Set the width of the smoothing window around each point, parametrized as a proportion of the total number of points.
     * Each window is defined as the smallest interval centered on the current point that covers the specified proportion.
     * If `weights` are provided to `run()` and `set_as_frequency_weights()` is set to `true`, the span is defined from the proportion of the total weight across all points.
     * This interprets the weights on each observation as relative frequencies.
     *
     * @param s Span value, between 0 and 1.
     *
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_span(Data_t s = Defaults::span) {
        span = s;
        return *this;
    }

    /**
     * Specify whether the span should be interpreted as a proportion of the total number of points.
     * If `false`, the value used in `set_span()` is directly interpreted as the number of points that must fall inside the window.
     * If `false` and `weights` are provided to `run()` and `set_as_frequency_weights()` is set to `true`, the value in `set_span()` is interpreted as the sum of weights inside the window.
     *
     * @param s Whether to interpret the span as a proportion.
     * 
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_span_as_proportion(bool s = Defaults::span_as_proportion) {
        span_as_proportion = s;
        return *this;
    }

    /**
     * Set the minimum width of the window centered around each point.
     * This is useful for forcing a larger window in highly dense regions of the covariate range.
     *
     * @param m Minimum width of the window.
     *
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_min_width(Data_t m = Defaults::min_width) {
        min_width = m;
        return *this;
    }

    /**
     * Set the number of points that can be used as "anchors".
     * LOWESS smoothing is performed exactly for each anchor, while the fitted values for all intervening points are computed by linear interpolation.
     * A higher number of anchor points improves accuracy at the cost of computational work.
     * If the specified number of anchors is greater than the number of points, LOWESS smoothing is performed directly for each point.
     * This setting is ignored if `set_delta()` is set to a non-negative value.
     *
     * @param p Number of anchors.
     *
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_anchors(int p = Defaults::anchors) {
        points = p;
        return *this;
    }

    /**
     * Set the number of robustness iterations.
     * At each iteration, each point is weighted according to its difference from the smoothed value, and the smoothing is repeated with these weights. 
     * More iterations increase robustness to outliers at the cost of computational work.
     *
     * @param i Number of robustness iterations.
     *
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_iterations(int i = Defaults::iterations) {
        iterations = i;
        return *this;
    }

    /**
     * Set the delta value used to identify anchors.
     * Seeds are identified greedily, by walking through the ordered x-coordinate values and marking a point `y` as a anchor if there are no anchors in `[y - delta, y]`.
     * If set to zero, all unique points are used as anchors.
     * If set to a negative value, an appropriate delta is determined from the number of points specified in `set_points()`.
     *
     * @param d The delta value.
     * This should have similar magnitude to the range of the x-values.
     *
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_delta(Data_t d = Defaults::delta) {
        delta = d;
        return *this;
    }

    /** 
     * Specify that the input data are already sorted on the x-values.
     * This will cause `run()` to skip the sorting step.
     *
     * @param s Whether the input data are already sorted.
     *
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_sorted(bool s = Defaults::sorted) {
        sorted = s;
        return *this;
    }

    /** 
     * Specify that the weights should be interpreted as frequency weights.
     * This means that they will be involved in both the span calculations for the smoothing window around each point, as well as in the LOWESS calculations themselves.
     * If `false`, the weights will only be used for the latter.
     *
     * @param f Whether the weights are frequency weights.
     *
     * @return A reference to the modified `WeightedLowess` object is returned.
     */
    WeightedLowess& set_as_frequency_weights(bool f = Defaults::frequency_weights) {
        freqweights = f;
        return *this;
    }

private:
    Data_t span = Defaults::span;
    bool span_as_proportion = Defaults::span_as_proportion;
    Data_t min_width = Defaults::min_width;
    int points = Defaults::anchors;
    int iterations = Defaults::iterations;
    Data_t delta = Defaults::delta;
    bool sorted = Defaults::sorted;
    bool freqweights = Defaults::frequency_weights;

private:
    /* Determining the `delta`. For a anchor point with x-coordinate `x`, we skip all
     * points in `[x, x + delta]` before finding the next anchor point. 
     * We try to choose a `delta` that satisfies the constraints on the number
     * of anchor points in `points`. A naive approach would be to simply divide the
     * range of `x` by `points - 1`. However, this may place anchor points inside 
     * large gaps on the x-axis intervals where there are no actual observations. 
     * 
     * Instead, we try to distribute the anchor points so that they don't fall
     * inside such large gaps. We do so by looking at the largest gaps and seeing
     * what happens if we were to shift the anchor points to avoid such gaps. If we
     * jump across a gap, though, we need to "use up" a anchor point to restart
     * the sequence of anchor points on the other side of the gap. This requires some
     * iteration to find the compromise that minimizes the 'delta' (and thus the 
     * degree of approximation in the final lowess calculation).
     */
    static Data_t derive_delta(int points, size_t n, const Data_t* x) {
        std::vector<Data_t> diffs(n - 1);
        for (size_t i = 0; i < diffs.size(); ++i) {
            diffs[i] = x[i + 1] - x[i];
        }

        std::sort(diffs.begin(), diffs.end());
        for (size_t i = 1; i < diffs.size(); ++i) {
            diffs[i] += diffs[i-1];            
        }

        Data_t lowest_delta = diffs.back();
        for (size_t nskips = 0; nskips < points - 1 && nskips < diffs.size() - 1; ++nskips) {
            Data_t candidate_delta = diffs[diffs.size() - nskips - 1] / (points - nskips);
            lowest_delta = std::min(candidate_delta, lowest_delta);
        }

        return lowest_delta;
    }

private:
    /* Finding the anchor points, given the deltas. As previously mentioned, for
     * a anchor point with x-coordinate `x`, we skip all points in `[x, x +
     * delta]` before finding the next anchor point.
     */
    static void find_anchors(size_t n, const Data_t* x, Data_t d, std::vector<size_t>& anchors) {
        anchors.clear();
        anchors.push_back(0);

        size_t last_pt = 0;
        for (size_t pt = 1; pt < n - 1; ++pt) {
            if (x[pt] - x[last_pt] > d) {
                anchors.push_back(pt);
                last_pt = pt;
            }
        }

        anchors.push_back(n - 1);
        return;
    }

private:
    struct window {
        size_t left, right;
        Data_t distance;
    };

    /* This function identifies the start and end index in the span for each chosen sampling
     * point. It returns two arrays via reference containing said indices. It also returns
     * an array containing the maximum distance between points at each span.
     *
     * We don't use the update-based algorithm in Cleveland's paper, as it ceases to be
     * numerically stable once you throw in Data_t-precision weights. It's not particularly
     * amenable to updating through cycles of addition and subtraction. At any rate, the
     * algorithm as a whole remains quadratic (as weights must be recomputed) so there's no
     * damage to scalability.
     */
    static std::vector<window> find_limits(const std::vector<size_t>& anchors, 
        Data_t spanweight,
        size_t n,
        const Data_t* x, 
        const Data_t* weights,
        Data_t min_width)
    {
        const size_t nanchors = anchors.size();
        std::vector<window> limits(nanchors);
        auto half_min_width = min_width / 2;

        for (size_t s = 0; s < nanchors; ++s) {
            auto curpt = anchors[s], left = curpt, right = curpt;
            auto curx = x[curpt];
            Data_t curw = (weights == NULL ? 1 : weights[curpt]);
            bool ende = (curpt == n - 1), ends = (curpt == 0);
            Data_t mdist=0, ldist=0, rdist=0;

            while (curw < spanweight && (!ende || !ends)) {
                if (!ends) {
                    ldist = curx - x[left - 1];
                }
                if (!ende) {
                    rdist = x[right + 1] - curx;
                }

                // Go backwards if the left edge is not at the start already
                // and the left distance is lower (or the right edge is at the end). 
                bool go_back = false;
                if (!ends) {
                    if (ende || ldist <= rdist) {
                        go_back = true;
                    }
                } 

                if (go_back)  {
                    /* Move the span backwards. */
                    --left;

                    if (weights != NULL) {
                        curw += weights[left];
                    } else {
                        ++curw;
                    }

                    if (left==0) { 
                        ends = true;
                    }

                    if (mdist < ldist) { 
                        mdist = ldist; 
                    }
                }  else {
                    /* Move the span forwards. */
                    ++right;

                    if (weights != NULL) {
                        curw += weights[right];
                    } else {
                        ++curw;
                    }

                    if (right == n - 1) { 
                        ende = true; 
                    }

                    if (mdist < rdist) { 
                        mdist = rdist; 
                    }
                }
            }

            /* Once we've found the span, we stretch it out to include all ties. */
            while (left > 0 && x[left] == x[left-1]) {
                --left; 
            }

            while (right < n - 1 && x[right]==x[right+1]) { 
                ++right; 
            }

            /* Forcibly extending the span if it fails the min width.  We use
             * the existing 'left' and 'right' to truncate the search space.
             */
            if (mdist < half_min_width) {
                left = std::lower_bound(x, x + left, curx - half_min_width) - x; 

                /* 'right' still refers to a point inside the window, and we
                 * already know that the window is too small, so we shift it
                 * forward by one to start searching outside. However,
                 * upper_bound gives us the first element that is _outside_ the
                 * window, so we need to subtract one to get to the last
                 * element _inside_ the window.
                 */
                right = std::upper_bound(x + right + 1, x + n, curx + half_min_width) - x;
                --right;

                mdist = std::max(curx - x[left], x[right] - curx);
            }

            limits[s].left = left;
            limits[s].right = right;
            limits[s].distance = mdist;
        }

        return limits;
    }

private:
    static Data_t cube(Data_t x) {
        return x*x*x;
    }

    /* Computes the lowess fit at a given point using linear regression with a
     * combination of tricube, prior and robustness weighting. 
     */
    static Data_t lowess_fit (const size_t curpt, const window& limits, 
                              size_t n,
                              const Data_t* x,
                              const Data_t* y,
                              const Data_t* weights, 
                              const Data_t* robust_weights, 
                              Data_t* work) 
    {
        size_t left = limits.left, right = limits.right;
        Data_t dist = limits.distance;

        if (dist <= 0) {
            Data_t ymean = 0, allweight = 0;
            for (size_t pt = left; pt <= right; ++pt) {
                Data_t curweight = robust_weights[pt];
                if (weights != NULL) {
                    curweight *= weights[pt];
                }
                ymean += y[pt] * curweight;
                allweight += curweight;
            }

            if (allweight == 0) { // ignore the robustness weights.
                for (size_t pt = left; pt <= right; ++pt) {
                    Data_t curweight = 1;
                    if (weights != NULL) {
                        curweight = weights[pt];
                    }
                    ymean += y[pt] * curweight;
                    allweight += curweight;
                }
            }

            ymean /= allweight;
            return ymean;
        }

        Data_t xmean = 0, ymean = 0, allweight = 0;
        for (size_t pt = left; pt <= right; ++pt) {
            Data_t& current = work[pt];
            current = cube(1 - cube(std::abs(x[curpt] - x[pt])/dist)) * robust_weights[pt];
            if (weights != NULL) {
                current *= weights[pt];
            }
            xmean += current * x[pt];
            ymean += current * y[pt];
            allweight += current;
        }

        if (allweight == 0) { // ignore the robustness weights.
            for (size_t pt = left; pt <= right; ++pt) {
                Data_t& current = work[pt];
                current = cube(1 - cube(std::abs(x[curpt] - x[pt])/dist));
                if (weights != NULL) {
                    current *= weights[pt];
                }
                xmean += current * x[pt];
                ymean += current * y[pt];
                allweight += current;
            }
        }

        xmean /= allweight;
        ymean /= allweight;

        Data_t var=0, covar=0;
        for (size_t pt = left; pt <= right; ++pt) {
            Data_t temp = x[pt] - xmean;
            var += temp * temp * work[pt];
            covar += temp * (y[pt] - ymean) * work[pt];
        }

        // Still possible for var = 0 if all other points have zero weight.
        if (var == 0) {
            return ymean;
        } else {
            const Data_t slope = covar / var;
            const Data_t intercept = ymean - slope * xmean;
            return slope * x[curpt] + intercept;
        }
    }

private:
    static Data_t square (Data_t x) {
        return x*x;
    }

    /* This is a C version of the local weighted regression (lowess) trend fitting algorithm,
     * based on the Fortran code in lowess.f from http://www.netlib.org/go written by Cleveland.
     * Consideration of non-equal prior weights is added to the span calculations and linear
     * regression. These weights are intended to have the equivalent effect of frequency weights
     * (at least, in the integer case; extended by analogy to all non-negative values).
     */
    void robust_lowess(size_t n,
                       const Data_t* x,
                       const Data_t* y,
                       const Data_t* weights,
                       Data_t* fitted, 
                       Data_t* robust_weights)
    const {
        std::vector<Data_t> workspace(n);

        /* Computing the span weight that each span must achieve. */
        const Data_t totalweight = (weights != NULL ? std::accumulate(weights, weights + n, 0.0) : n);
        const Data_t spanweight = (span_as_proportion ? span * totalweight : span);

        /* Finding the anchors. If we're using the weights as frequencies, we
         * pass them in when we're looking for the span limits for each anchor.
         */
        std::vector<size_t> anchors;
        if (points < n || delta > 0) {
            Data_t eff_delta = (delta < 0 ? derive_delta(points, n, x) : delta);
            find_anchors(n, x, eff_delta, anchors);
        } else {
            anchors.resize(n);
            std::iota(anchors.begin(), anchors.end(), 0);
        }

        auto limits = find_limits(anchors, spanweight, n, x, (freqweights ? weights : NULL), min_width); 

        /* Setting up the robustness weights, if robustification is requested.
         */ 
        std::fill(robust_weights, robust_weights + n, 1);
        Data_t min_mad = 0; 
        std::vector<size_t> residual_permutation;
        if (iterations) {
            residual_permutation.resize(n);

            /* We use the range so guarantee that we catch the scale. Otherwise
             * if we used the MAD of 'y', it could be conceivable that we would
             * end up with a min_mad of zero again, e.g., if the majority of
             * points have the same value. In contrast, if the range is zero,
             * we just quit early.
             */
            Data_t range = (*std::max_element(y, y + n) - *std::min_element(y, y + n));
            if (range == 0) {
                std::copy(y, y + n, fitted);
                return;
            }
            min_mad = 0.00000000001 * range;
        }

        for (int it = 0; it <= iterations; ++it) { // Robustness iterations.
            fitted[0] = lowess_fit(0, limits[0], n, x, y, weights, robust_weights, workspace.data()); 
            size_t last_anchor = 0;

            for (size_t s = 1; s < anchors.size(); ++s) { // fitted values for anchor points, interpolating the rest.
                auto curpt = anchors[s];
                fitted[curpt] = lowess_fit(curpt, limits[s], n, x, y, weights, robust_weights, workspace.data()); 

                if (curpt - last_anchor > 1) {
                    /* Some protection is provided against infinite slopes. This shouldn't be
                     * a problem for non-zero delta; the only concern is at the final point
                     * where the covariate distance may be zero.
                     */
                    Data_t current = x[curpt] - x[last_anchor];
                    if (current > 0) {
                        const Data_t slope = (fitted[curpt] - fitted[last_anchor])/current;
                        const Data_t intercept = fitted[curpt] - slope * x[curpt];
                        for (size_t subpt = last_anchor + 1; subpt < curpt; ++subpt) { 
                            fitted[subpt] = slope * x[subpt] + intercept; 
                        }
                    } else {
                        const Data_t ave = (fitted[curpt] + fitted[last_anchor]) / 2;
                        for (size_t subpt = last_anchor + 1; subpt < curpt; ++subpt) {
                            fitted[subpt] = ave;
                        }
                    }
                }

                last_anchor = curpt;
            }

            if (it < iterations) {
                /* Computing the weighted MAD of the absolute values of the residuals. */
                for (size_t pt = 0; pt < n; ++pt) {
                    workspace[pt] = std::abs(y[pt] - fitted[pt]);
                }

                std::iota(residual_permutation.begin(), residual_permutation.end(), 0);
                std::sort(residual_permutation.begin(), residual_permutation.end(), 
                    [&](size_t left, size_t right) -> bool {
                        return workspace[left] < workspace[right];
                    }
                );

                Data_t curweight = 0;
                Data_t cmad = 100;
                const Data_t halfweight = totalweight/2;

                for (size_t i = 0; i < n; ++i) {
                    auto pt = residual_permutation[i];
                    if (weights != NULL) {
                        curweight += weights[pt];
                    } else {
                        ++curweight;
                    }

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
                for (size_t i =0; i < n; ++i) {
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

private:
    void sort_and_run(size_t n, Data_t* x, Data_t* y, Data_t* weights, Data_t* fitted, Data_t* robust_weights) const {
        std::vector<size_t> permutation(n);
        std::iota(permutation.begin(), permutation.end(), 0);
        std::sort(permutation.begin(), permutation.end(), [&](size_t left, size_t right) -> bool {
            return x[left] < x[right];
        });

        // Reordering values in place.
        std::vector<uint8_t> used(n);
        std::fill(used.begin(), used.end(), 0);
        for (size_t i = 0; i < permutation.size(); ++i) {
            if (used[i]) {
                continue;
            }
            used[i] = 1;

            size_t current = i, replacement = permutation[i];
            while (replacement != i) {
                std::swap(x[current], x[replacement]);
                std::swap(y[current], y[replacement]);
                if (weights != NULL) {
                    std::swap(weights[current], weights[replacement]);
                }

                current = replacement;
                used[replacement] = 1;
                replacement = permutation[replacement]; 
            } 
        }

        // Computing the fitted values and robustness weights.
        Data_t* rptr = robust_weights;
        std::vector<Data_t> rbuffer;
        if (robust_weights == NULL) {
            rbuffer.resize(n);
            rptr = rbuffer.data();
        }
        robust_lowess(n, x, y, weights, fitted, rptr);

        // Unpermuting the fitted values in place. 
        std::fill(used.begin(), used.end(), 0); 
        for (size_t i = 0; i < permutation.size(); ++i) {
            if (used[i]) {
                continue;
            }
            used[i] = 1;

            size_t replacement = permutation[i];
            while (replacement != i) {
                std::swap(fitted[i], fitted[replacement]);
                if (robust_weights) {
                    std::swap(robust_weights[i], robust_weights[replacement]);
                }

                used[replacement] = 1;
                replacement = permutation[replacement]; 
            } 
        }

        return;
    }

public:
    /**
     * Run the LOWESS smoother and fill output arrays with the smoothed values.
     *
     * @param n Number of points.
     * @param x Pointer to an array of `n` x-values.
     * @param y Pointer to an array of `n` y-values.
     * @param weights Pointer to an array of `n` positive weights.
     * Alternatively `NULL` if no weights are available.
     * @param fitted Pointer to an output array of length `n`, in which the fitted values of the smoother can be stored.
     * @param robust_weights Pointer to an output array of length `n`, in which the robustness weights can be stored.
     */
   void run(size_t n, const Data_t* x, const Data_t* y, const Data_t* weights, Data_t* fitted, Data_t* robust_weights) const {
        if (sorted) {
            std::vector<Data_t> rbuffer;
            if (robust_weights == NULL) {
                rbuffer.resize(n);
                robust_weights = rbuffer.data();
            }
            robust_lowess(n, x, y, weights, fitted, robust_weights);
        } else {
            /* Sorts the observations by the means, applies the same permutation to the
             * variances. This makes downstream processing quite a lot easier.
             */
            std::vector<Data_t> xbuffer(x, x+ n);
            std::vector<Data_t> ybuffer(y, y + n);

            std::vector<Data_t> wbuffer;
            Data_t* wptr = NULL;
            if (weights) {
                wbuffer.insert(wbuffer.end(), weights, weights + n);
                wptr = wbuffer.data();
            }

            sort_and_run(n, xbuffer.data(), ybuffer.data(), wptr, fitted, robust_weights);
        }
        return;
    }

    /**
     * Run the LOWESS smoother and fill output arrays with the smoothed values.
     * This differs from `run()` in that `x` and `y` (and `weights`, if supplied) will be arbitrarily modified on output,
     * allowing us to avoid a copy if they are no longer needed after this method call. 
     *
     * @param n Number of points.
     * @param x Pointer to an array of `n` x-values.
     * @param y Pointer to an array of `n` y-values.
     * @param weights Pointer to an array of `n` positive weights.
     * Alternatively `NULL` if no weights are available.
     * @param fitted Pointer to an output array of length `n`, in which the fitted values of the smoother can be stored.
     * @param robust_weights Pointer to an output array of length `n`, in which the robustness weights can be stored.
     */
    void run_in_place(size_t n, Data_t* x, Data_t* y, Data_t* weights, Data_t* fitted, Data_t* robust_weights) const {
        if (sorted) {
            std::vector<Data_t> rbuffer;
            if (robust_weights == NULL) {
                rbuffer.resize(n);
                robust_weights = rbuffer.data();
            }
            robust_lowess(n, x, y, weights, fitted, robust_weights);
        } else {
            sort_and_run(n, x, y, weights, fitted, robust_weights);
        }
        return;
    }

public:
    /** 
     * @brief Store the smoothing results.
     */
    struct Results {
        /**
         * @param n Number of points.
         */
        Results(size_t n) : fitted(n), robust_weights(n) {}

        /**
         * Fitted values from the LOWESS smoother. 
         */
        std::vector<Data_t> fitted;

        /**
         * Robustness weight for each point.
         */
        std::vector<Data_t> robust_weights;
    };

    /**
     * Run the LOWESS smoother and return a `Results` object.
     * This avoids the need to instantiate the various output arrays manually. 
     *
     * @param n Number of points.
     * @param x Pointer to an array of `n` x-values.
     * @param y Pointer to an array of `n` y-values.
     * @param weights Pointer to an array of `n` positive weights.
     * Alternatively `NULL` if no weights are available.
     *
     * @return A `Results` object containing the fitted values and robustness weights.
     */
    Results run(size_t n, const Data_t* x, const Data_t* y, const Data_t* weights=NULL) const {
        Results output(n);
        run(n, x, y, weights, output.fitted.data(), output.robust_weights.data());
        return output;
    }

    /**
     * Run the LOWESS smoother and return a `Results` object.
     * This differs from `run()` in that `x` and `y` (and `weights`, if supplied) will be arbitrarily modified on output,
     * allowing us to avoid a copy if they are no longer needed after this method call. 
     *
     * @param n Number of points.
     * @param x Pointer to an array of `n` x-values.
     * @param y Pointer to an array of `n` y-values.
     * @param weights Pointer to an array of `n` positive weights.
     * Alternatively `NULL` if no weights are available.
     *
     * @return A `Results` object containing the fitted values and robustness weights.
     */
    Results run_in_place(size_t n, Data_t* x, Data_t* y, Data_t* weights=NULL) const {
        Results output(n);
        run_in_place(n, x, y, weights, output.fitted.data(), output.robust_weights.data());
        return output;
    }
};

}

#endif
