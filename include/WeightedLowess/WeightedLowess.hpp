#ifndef SCRAN_WEIGHTED_LOWESS_H 
#define SCRAN_WEIGHTED_LOWESS_H 

#include <algorithm>
#include <vector>
#include <cstdint>
#include <cmath>
#include <numeric>

#include <iostream>

namespace WeightedLowess {

class WeightedLowess {
public:
    WeightedLowess& set_span(double s = 0.3) {
        span = s;
        return *this;
    }

    WeightedLowess& set_points(int p = 200) {
        points = p;
        return *this;
    }

    WeightedLowess& set_iterations(int i = 4) {
        iterations = i;
        return *this;
    }

    WeightedLowess& set_delta(double d = -1) {
        delta = d;
        return *this;
    }
private:
    double span = 0.3;
    int points = 200;
    int iterations = 4;
    double delta = -1;

    std::vector<size_t> permutation;
    std::vector<double> xbuffer, ybuffer, wbuffer;

private:
    /* Determining the `delta`. For a seed point with x-coordinate `x`, we skip all
     * points in `[x, x + delta]` before finding the next seed point. 
     * We try to choose a `delta` that satisfies the constraints on the number
     * of seed points in `points`. A naive approach would be to simply divide the
     * range of `x` by `points - 1`. However, this may place seed points inside 
     * large gaps on the x-axis intervals where there are no actual observations. 
     * 
     * Instead, we try to distribute the seed points so that they don't fall
     * inside such large gaps. We do so by looking at the largest gaps and seeing
     * what happens if we were to shift the seed points to avoid such gaps. If we
     * jump across a gap, though, we need to "use up" a seed point to restart
     * the sequence of seed points on the other side of the gap. This requires some
     * iteration to find the compromise that minimizes the 'delta' (and thus the 
     * degree of approximation in the final lowess calculation).
     */
    double derive_delta() const {
        std::vector<double> diffs(xbuffer.size() - 1);
        for (size_t i = 0; i < diffs.size(); ++i) {
            diffs[i] = xbuffer[i + 1] - xbuffer[i];
        }

        std::sort(diffs.begin(), diffs.end());
        for (size_t i = 1; i < diffs.size(); ++i) {
            diffs[i] += diffs[i-1];            
        }

        double lowest_delta = diffs.back();
        for (size_t nskips = 0; nskips < points - 1 && nskips < diffs.size() - 1; ++nskips) {
            double candidate_delta = diffs[diffs.size() - nskips - 1] / (points - nskips);
            lowest_delta = std::min(candidate_delta, lowest_delta);
        }

        return lowest_delta;
    }

private:
    /* Finding the seed points, given the deltas. As previously mentioned, for
     * a seed point with x-coordinate `x`, we skip all points in `[x, x +
     * delta]` before finding the next seed point.
     */
    std::vector<size_t> find_seeds(double d) const {
        std::vector<size_t> indices;
        indices.push_back(0);
        size_t last_pt = 0;
        for (size_t pt = 1; pt < xbuffer.size() - 1; ++pt) {
            if (xbuffer[pt] - xbuffer[last_pt] > d) {
                indices.push_back(pt);
                last_pt=pt;
            }
        }
        indices.push_back(xbuffer.size() - 1);
        return indices;
    }

private:
    struct window {
        size_t left, right;
        double distance;
    };

    /* This function identifies the start and end index in the span for each chosen sampling
     * point. It returns two arrays via reference containing said indices. It also returns
     * an array containing the maximum distance between points at each span.
     *
     * We don't use the update-based algorithm in Cleveland's paper, as it ceases to be
     * numerically stable once you throw in double-precision weights. It's not particularly
     * amenable to updating through cycles of addition and subtraction. At any rate, the
     * algorithm as a whole remains quadratic (as weights must be recomputed) so there's no
     * damage to scalability.
     */
    std::vector<window> find_limits(double spanweight, const std::vector<size_t>& seeds) const {
        const size_t nobs = xbuffer.size(); 
        const size_t nseeds = seeds.size();
        std::vector<window> output(nseeds);

        for (size_t s = 0; s < nseeds; ++s) {
            auto curpt = seeds[s], left = curpt, right = curpt;
            double curw = (wbuffer.empty() ? 1 : wbuffer[curpt]);
            bool ende = (curpt == nobs - 1), ends = (curpt == 0);
            double mdist=0, ldist=0, rdist=0;

            while (curw < spanweight && (!ende || !ends)) {
                if (!ends) {
                    ldist = xbuffer[curpt] - xbuffer[left - 1];
                }
                if (!ende) {
                    rdist = xbuffer[right + 1] - xbuffer[curpt];
                }

                /* Move the span backwards. */
                if (!ends) {
                    if (ende || ldist <= rdist) {
                        --left;

                        if (!wbuffer.empty()) {
                            curw += wbuffer[left];
                        } else {
                            ++curw;
                        }

                        if (left==0) { 
                            ends=1;
                        }

                        if (mdist < ldist) { 
                            mdist=ldist; 
                        }
                    } 
                }
                
                /* Move the span forwards. */
                if (!ende) {
                    if (ends || ldist > rdist) {
                        ++right;

                        if (!wbuffer.empty()) {
                            curw += wbuffer[right];
                        } else {
                            ++curw;
                        }

                        if (right == nobs - 1) { 
                            ende=1; 
                        }

                        if (mdist < rdist) { 
                            mdist=rdist; 
                        }
                    }
                }
            }

            /* Once we've found the span, we stretch it out to include all ties. */
            while (left > 0 && xbuffer[left] == xbuffer[left-1]) {
                --left; 
            }

            while (right < nobs - 1 && xbuffer[right]==xbuffer[right+1]) { 
                ++right; 
            }

            output[s].left = left;
            output[s].right = right;
            output[s].distance = mdist;
        }
        return output;
    }

private:
    static double cube(double x) {
        return x*x*x;
    }

    /* Computes the lowess fit at a given point using linear regression with a
     * combination of tricube, prior and robustness weighting. 
     */
    double lowess_fit (const size_t curpt, const window& limits, const std::vector<double>& robustness, double* work) const {
        size_t left = limits.left, right = limits.right;
        double dist = limits.distance;

        if (dist <= 0) {
            double ymean = 0, allweight = 0;
            for (size_t pt = left; pt <= right; ++pt) {
                double curweight = robustness[pt];
                if (!wbuffer.empty()) {
                    curweight *= wbuffer[pt];
                }
                ymean += ybuffer[pt] * curweight;
                allweight += curweight;
            }
            ymean /= allweight;
            return ymean;
        }

        double xmean = 0, ymean = 0, allweight = 0;
        for (size_t pt = left; pt <= right; ++pt) {
            double& current = work[pt];
            current = cube(1 - cube(std::abs(xbuffer[curpt] - xbuffer[pt])/dist)) * robustness[pt];
            if (!wbuffer.empty()) {
                current *= wbuffer[pt];
            }
            xmean += current * xbuffer[pt];
            ymean += current * ybuffer[pt];
            allweight += current;
        }
        xmean /= allweight;
        ymean /= allweight;

        double var=0, covar=0;
        for (size_t pt = left; pt <= right; ++pt) {
            double temp = xbuffer[pt] - xmean;
            var += temp * temp * work[pt];
            covar += temp * (ybuffer[pt] - ymean) * work[pt];
        }
        if (var <= 0) { 
            return ymean; 
        }

        const double slope=covar/var;
        const double intercept=ymean-slope*xmean;
        return slope * xbuffer[curpt] + intercept;
    }

private:
    static double square (double x) {
        return x*x;
    }

    /* This is a C version of the local weighted regression (lowess) trend fitting algorithm,
     * based on the Fortran code in lowess.f from http://www.netlib.org/go written by Cleveland.
     * Consideration of non-equal prior weights is added to the span calculations and linear
     * regression. These weights are intended to have the equivalent effect of frequency weights
     * (at least, in the integer case; extended by analogy to all non-negative values).
     */
    void robust_lowess(double* fitted, double * residuals) {
        size_t nobs = xbuffer.size();

        /* Computing the span weight that each span must achieve. */
        double totalweight = 0;
        if (!wbuffer.empty()) {
            totalweight = std::accumulate(wbuffer.begin(), wbuffer.end(), 0.0); 
        } else {
            totalweight = nobs;
        }
        const double spanweight = totalweight * span;

        // Finding the seeds.
        std::vector<size_t> seeds;
        if (points < nobs || delta > 0) {
            double eff_delta = (delta < 0 ? derive_delta() : delta);
            seeds = find_seeds(eff_delta);
        } else {
            seeds.resize(nobs);
            std::iota(seeds.begin(), seeds.end(), 0);
        }

        auto limits = find_limits(spanweight, seeds);
        std::vector<size_t> residual_permutation(nobs);
        std::vector<double> robustness(nobs, 1);

        for (int it = 0; it < iterations; ++it) { // Robustness iterations.
            fitted[0] = lowess_fit(0, limits[0], robustness, residuals); // using `residuals` as the workspace.
            size_t last_seed = 0;

            for (size_t s = 1; s < seeds.size(); ++s) { // fitted values for seed points, interpolating the rest.
                auto curpt = seeds[s];
                fitted[curpt] = lowess_fit(curpt, limits[s], robustness, residuals); // using `residuals` as the workspace.

                if (seeds.size() < nobs) {
                    /* Some protection is provided against infinite slopes. This shouldn't be
                     * a problem for non-zero delta; the only concern is at the final point
                     * where the covariate distance may be zero.
                     */
                    double current = xbuffer[curpt] - xbuffer[last_seed];
                    if (current > 0) {
                        const double slope = (fitted[curpt] - fitted[last_seed])/current;
                        const double intercept = fitted[curpt] - slope * xbuffer[curpt];
                        for (size_t subpt = last_seed + 1; subpt < curpt; ++subpt) { 
                            fitted[subpt] = slope * xbuffer[subpt] + intercept; 
                        }
                    } else {
                        const double ave = (fitted[curpt] + fitted[last_seed]) / 2;
                        for (size_t subpt = last_seed + 1; subpt < curpt; ++subpt) {
                            fitted[subpt] = ave;
                        }
                    }

                    last_seed = curpt;
                }
            }

            /* Computing the weighted MAD of the absolute values of the residuals. */
            for (size_t pt = 0; pt < nobs; ++pt) {
                residuals[pt] = std::abs(ybuffer[pt] - fitted[pt]);
            }

            std::iota(residual_permutation.begin(), residual_permutation.end(), 0);
            std::sort(residual_permutation.begin(), residual_permutation.end(), 
                [&](size_t left, size_t right) -> bool {
                    return residuals[left] < residuals[right];
                }
            );

            double curweight = 0;
            double cmad = 0;
            const double halfweight = totalweight/2;
            for (size_t i = 0; i < nobs; ++i) {
                auto pt = residual_permutation[i];
                if (!wbuffer.empty()) {
                    curweight += wbuffer[pt];
                } else {
                    ++curweight;
                }

                if (curweight == halfweight) { // exact match, need to take the median.
                    cmad = 3 * (residuals[pt] + residuals[residual_permutation[i+1]]);
                    break;
                } else if (curweight > halfweight) {
                    cmad = 6 * residuals[pt];
                    break;
                }
            }

            /* If it's too small, then robustness weighting will have no further effect.
             * Any points with large residuals would already be pretty lowly weighted.
             * This is based on a similar step in lowess.c in the core R code.
             */
            double resid_scale = std::accumulate(residuals, residuals + nobs, 0.0)/nobs;
            if (cmad <= 0.0000001 * resid_scale) { break; }

            for (size_t i =0; i < nobs; ++i) {
                if (residuals[i] < cmad) {
                    robustness[i] = square(1 - square(residuals[i]/cmad));
                } else { 
                    robustness[i] = 0;
                }
            }
        }

        return;
    }

public:
    void run(size_t n, const double* x, const double* y, const double* weights, double* fitted, double* residuals) {
        /* Sorts the observations by the means, applies the same permutation to the
         * variances. This makes downstream processing quite a lot easier.
         */
        xbuffer = std::vector<double>(x, x + n);
        ybuffer = std::vector<double>(y, y + n);
        if (weights) {
            wbuffer = std::vector<double>(weights, weights + n);
        } else {
            wbuffer.clear();
        }

        permutation.resize(n);
        std::iota(permutation.begin(), permutation.end(), 0);
        std::sort(permutation.begin(), permutation.end(), [&](size_t left, size_t right) -> bool {
            return xbuffer[left] < xbuffer[right];
        });

        // Reordering values in place.
        std::vector<uint8_t> used(n);
        for (size_t i = 0; i < permutation.size(); ++i) {
            if (used[i]) {
                continue;
            }
            used[i] = 1;

            size_t current = i, replacement = permutation[i];
            while (replacement != i) {
                std::swap(xbuffer[current], xbuffer[replacement]);
                std::swap(ybuffer[current], ybuffer[replacement]);
                if (!wbuffer.empty()) {
                    std::swap(wbuffer[current], wbuffer[replacement]);
                }

                current = replacement;
                used[replacement] = 1;
                replacement = permutation[replacement]; 
            } 
        }

        // Computing the fitted values and residuals.
        robust_lowess(fitted, residuals); 

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
                std::swap(residuals[i], residuals[replacement]);
                used[replacement] = 1;
                replacement = permutation[replacement]; 
            } 
        }

        return;
    }
public:
    struct Results {
        Results(size_t n) : fitted(n), residuals(n) {}
        std::vector<double> fitted, residuals;
    };

    Results run(size_t n, const double* x, const double* y, const double* weights=NULL) {
        Results output(n);
        run(n, x, y, weights, output.fitted.data(), output.residuals.data());
        return output;
    }
};

}

#endif
