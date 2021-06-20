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

    WeightedLowess& set_iterations(int i = 3) {
        iterations = i;
        return *this;
    }

    WeightedLowess& set_delta(double d = -1) {
        delta = d;
        return *this;
    }

    WeightedLowess& set_sorted(bool s = false) {
        sorted = s;
        return *this;
    }

private:
    double span = 0.3;
    int points = 200;
    int iterations = 3;
    double delta = -1;
    bool sorted = false;

private:
    std::vector<double> diffs;

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
    static double derive_delta(int points, size_t n, const double* x, std::vector<double>& diffs) {
        diffs.resize(n - 1);
        for (size_t i = 0; i < diffs.size(); ++i) {
            diffs[i] = x[i + 1] - x[i];
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
    std::vector<size_t> seeds;

    /* Finding the seed points, given the deltas. As previously mentioned, for
     * a seed point with x-coordinate `x`, we skip all points in `[x, x +
     * delta]` before finding the next seed point.
     */
    static void find_seeds(size_t n, const double* x, double d, std::vector<size_t>& seeds) {
        seeds.clear();
        seeds.push_back(0);

        size_t last_pt = 0;
        for (size_t pt = 1; pt < n - 1; ++pt) {
            if (x[pt] - x[last_pt] > d) {
                seeds.push_back(pt);
                last_pt = pt;
            }
        }

        seeds.push_back(n - 1);
        return;
    }

private:
    struct window {
        size_t left, right;
        double distance;
    };

    std::vector<window> limits;

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
    static void find_limits(const std::vector<size_t>& seeds, 
                            double spanweight,
                            size_t n,
                            const double* x, 
                            const double* weights,
                            std::vector<window>& limits)
    {
        const size_t nseeds = seeds.size();
        limits.resize(nseeds);

        for (size_t s = 0; s < nseeds; ++s) {
            auto curpt = seeds[s], left = curpt, right = curpt;
            double curw = (weights == NULL ? 1 : weights[curpt]);
            bool ende = (curpt == n - 1), ends = (curpt == 0);
            double mdist=0, ldist=0, rdist=0;

            while (curw < spanweight && (!ende || !ends)) {
                if (!ends) {
                    ldist = x[curpt] - x[left - 1];
                }
                if (!ende) {
                    rdist = x[right + 1] - x[curpt];
                }

                /* Move the span backwards. */
                if (!ends) {
                    if (ende || ldist <= rdist) {
                        --left;

                        if (weights != NULL) {
                            curw += weights[left];
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

                        if (weights != NULL) {
                            curw += weights[right];
                        } else {
                            ++curw;
                        }

                        if (right == n - 1) { 
                            ende=1; 
                        }

                        if (mdist < rdist) { 
                            mdist=rdist; 
                        }
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

            limits[s].left = left;
            limits[s].right = right;
            limits[s].distance = mdist;
        }
        return;
    }

private:
    static double cube(double x) {
        return x*x*x;
    }

    /* Computes the lowess fit at a given point using linear regression with a
     * combination of tricube, prior and robustness weighting. 
     */
    static double lowess_fit (const size_t curpt, const window& limits, 
                              size_t n,
                              const double* x,
                              const double* y,
                              const double* weights, 
                              const double* robust_weights, 
                              double* work) 
    {
        size_t left = limits.left, right = limits.right;
        double dist = limits.distance;

        if (dist <= 0) {
            double ymean = 0, allweight = 0;
            for (size_t pt = left; pt <= right; ++pt) {
                double curweight = robust_weights[pt];
                if (weights != NULL) {
                    curweight *= weights[pt];
                }
                ymean += y[pt] * curweight;
                allweight += curweight;
            }
            ymean /= allweight;
            return ymean;
        }

        double xmean = 0, ymean = 0, allweight = 0;
        for (size_t pt = left; pt <= right; ++pt) {
            double& current = work[pt];
            current = cube(1 - cube(std::abs(x[curpt] - x[pt])/dist)) * robust_weights[pt];
            if (weights != NULL) {
                current *= weights[pt];
            }
            xmean += current * x[pt];
            ymean += current * y[pt];
            allweight += current;
        }
        xmean /= allweight;
        ymean /= allweight;

        double var=0, covar=0;
        for (size_t pt = left; pt <= right; ++pt) {
            double temp = x[pt] - xmean;
            var += temp * temp * work[pt];
            covar += temp * (y[pt] - ymean) * work[pt];
        }

        // Impossible for var = 0 here, as this would imply dist = 0 above.
        const double slope = covar / var;
        const double intercept = ymean - slope * xmean;
        return slope * x[curpt] + intercept;
    }

private:
    static double square (double x) {
        return x*x;
    }

    std::vector<size_t> residual_permutation;

    /* This is a C version of the local weighted regression (lowess) trend fitting algorithm,
     * based on the Fortran code in lowess.f from http://www.netlib.org/go written by Cleveland.
     * Consideration of non-equal prior weights is added to the span calculations and linear
     * regression. These weights are intended to have the equivalent effect of frequency weights
     * (at least, in the integer case; extended by analogy to all non-negative values).
     */
    void robust_lowess(size_t n,
                       const double* x,
                       const double* y,
                       const double* weights,
                       double* fitted, 
                       double* residuals,
                       double* robust_weights)
    {
        /* Computing the span weight that each span must achieve. */
        double totalweight = 0;
        if (weights != NULL) {
            totalweight = std::accumulate(weights, weights + n, 0.0); 
        } else {
            totalweight = n;
        }
        const double spanweight = totalweight * span;

        // Finding the seeds.
        if (points < n || delta > 0) {
            double eff_delta = (delta < 0 ? derive_delta(points, n, x, diffs) : delta);
            find_seeds(n, x, eff_delta, seeds);
        } else {
            seeds.resize(n);
            std::iota(seeds.begin(), seeds.end(), 0);
        }

        find_limits(seeds, spanweight, n, x, weights, limits);
        std::fill(robust_weights, robust_weights + n, 1);
        residual_permutation.resize(n);
        auto workspace = residuals; // using `residuals` as the workspace.

        for (int it = 0; it <= iterations; ++it) { // Robustness iterations.
            fitted[0] = lowess_fit(0, limits[0], n, x, y, weights, robust_weights, workspace); 
            size_t last_seed = 0;

            for (size_t s = 1; s < seeds.size(); ++s) { // fitted values for seed points, interpolating the rest.
                auto curpt = seeds[s];
                fitted[curpt] = lowess_fit(curpt, limits[s], n, x, y, weights, robust_weights, workspace); 

                if (curpt - last_seed > 1) {
                    /* Some protection is provided against infinite slopes. This shouldn't be
                     * a problem for non-zero delta; the only concern is at the final point
                     * where the covariate distance may be zero.
                     */
                    double current = x[curpt] - x[last_seed];
                    if (current > 0) {
                        const double slope = (fitted[curpt] - fitted[last_seed])/current;
                        const double intercept = fitted[curpt] - slope * x[curpt];
                        for (size_t subpt = last_seed + 1; subpt < curpt; ++subpt) { 
                            fitted[subpt] = slope * x[subpt] + intercept; 
                        }
                    } else {
                        const double ave = (fitted[curpt] + fitted[last_seed]) / 2;
                        for (size_t subpt = last_seed + 1; subpt < curpt; ++subpt) {
                            fitted[subpt] = ave;
                        }
                    }
                }

                last_seed = curpt;
            }

            /* Computing the weighted MAD of the absolute values of the residuals. */
            for (size_t pt = 0; pt < n; ++pt) {
                residuals[pt] = std::abs(y[pt] - fitted[pt]);
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

            for (size_t i = 0; i < n; ++i) {
                auto pt = residual_permutation[i];
                if (weights != NULL) {
                    curweight += weights[pt];
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
            double resid_scale = std::accumulate(residuals, residuals + n, 0.0)/n;
            if (cmad <= 0.0000001 * resid_scale) { break; }

            for (size_t i =0; i < n; ++i) {
                if (residuals[i] < cmad) {
                    robust_weights[i] = square(1 - square(residuals[i]/cmad));
                } else { 
                    robust_weights[i] = 0;
                }
            }
        }

        return;
    }

public:
    std::vector<uint8_t> used;
    std::vector<double> rbuffer;

    void sort_and_run(size_t n, double* x, double* y, double* weights, double* fitted, double* residuals, double* robust_weights) {
        permutation.resize(n);
        std::iota(permutation.begin(), permutation.end(), 0);
        std::sort(permutation.begin(), permutation.end(), [&](size_t left, size_t right) -> bool {
            return x[left] < x[right];
        });

        // Reordering values in place.
        used.resize(n);
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

        // Computing the fitted values and residuals.
        double* rptr = robust_weights;
        if (robust_weights == NULL) {
            rbuffer.resize(n);
            rptr = rbuffer.data();
        }
        robust_lowess(n, x, y, weights, fitted, residuals, rptr);

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
    std::vector<size_t> permutation;
    std::vector<double> xbuffer, ybuffer, wbuffer;

    void run(size_t n, const double* x, const double* y, const double* weights, double* fitted, double* residuals, double* robust_weights) {
        if (sorted) {
            if (robust_weights == NULL) {
                rbuffer.resize(n);
                robust_weights = rbuffer.data();
            }
            robust_lowess(n, x, y, weights, fitted, residuals, robust_weights);
        } else {
            /* Sorts the observations by the means, applies the same permutation to the
             * variances. This makes downstream processing quite a lot easier.
             */
            xbuffer = std::vector<double>(x, x + n);
            ybuffer = std::vector<double>(y, y + n);
            double* wptr = NULL;
            if (weights) {
                wbuffer = std::vector<double>(weights, weights + n);
                wptr = wbuffer.data();
            }
            sort_and_run(n, xbuffer.data(), ybuffer.data(), wptr, fitted, residuals, robust_weights);
        }
        return;
    }

    void run_in_place(size_t n, double* x, double* y, double* weights, double* fitted, double* residuals, double* robust_weights) {
        if (sorted) {
            if (robust_weights == NULL) {
                rbuffer.resize(n);
                robust_weights = rbuffer.data();
            }
            robust_lowess(n, x, y, weights, fitted, residuals, robust_weights);
        } else {
            sort_and_run(n, x, y, weights, fitted, residuals, robust_weights);
        }
        return;
    }

public:
    struct Results {
        Results(size_t n) : fitted(n), residuals(n), robust_weights(n) {}
        std::vector<double> fitted, residuals, robust_weights;
    };

    Results run(size_t n, const double* x, const double* y, const double* weights=NULL) {
        Results output(n);
        run(n, x, y, weights, output.fitted.data(), output.residuals.data(), output.robust_weights.data());
        return output;
    }

    Results run_in_place(size_t n, double* x, double* y, double* weights=NULL) {
        Results output(n);
        run_in_place(n, x, y, weights, output.fitted.data(), output.residuals.data(), output.robust_weights.data());
        return output;
    }
};

}

#endif
