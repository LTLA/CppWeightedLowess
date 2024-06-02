#ifndef WEIGHTEDLOWESS_COMPUTE_HPP
#define WEIGHTEDLOWESS_COMPUTE_HPP

#include <vector>

#include "fit.hpp"
#include "Options.hpp"

/**
 * @file compute.hpp
 * @brief Compute the LOWESS trend fit.
 */

namespace WeightedLowess {

/**
 * @cond
 */
namespace internal {

template<typename Data_>
void sort_and_run(size_t num_points, Data_* x, Data_* y, Data_* weights, Data_* fitted, Data_* robust_weights) {
    if (num_points == 0) {
        return;
    }

    std::vector<size_t> permutation(num_points);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::sort(permutation.begin(), permutation.end(), [&](size_t left, size_t right) -> bool {
        return x[left] < x[right];
    });

    // Reordering values in place.
    std::vector<uint8_t> used(num_points);
    std::fill(used.begin(), used.end(), 0);
    for (size_t i = 0; i < num_points; ++i) {
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
    Data_* rptr = robust_weights;
    std::vector<Data_> rbuffer;
    if (robust_weights == NULL) {
        rbuffer.resize(num_points);
        rptr = rbuffer.data();
    }
    fit_trend(num_points, x, y, weights, fitted, rptr);

    // Unpermuting the fitted values in place. 
    std::fill(used.begin(), used.end(), 0); 
    for (size_t i = 0; i < num_points; ++i) {
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
}

}
/**
 * @endcond
 */

/**
 * LOWESS is a simple, efficient, general-purpose non-parametric smoothing algorithm. 
 * It perform weighted linear regressions on subsets of neighboring points, yielding a smooth curve fit to the data.
 *
 * @tparam Data_ Floating-point type for the data.
 *
 * @param num_points Number of points.
 * @param x Pointer to an array of `num_points` x-values.
 * @param y Pointer to an array of `num_points` y-values.
 * @param[in] weights Pointer to an array of `n` positive weights.
 * Alternatively `NULL` if no weights are available.
 * @param[out] fitted Pointer to an output array of length `n`, in which the fitted values of the smoother can be stored.
 * @param[out] robust_weights Pointer to an output array of length `n`, in which the robustness weights can be stored.
 * @param Further options.
 */
template<typename Data_>
void compute(size_t num_points, const Data_* x, const Data_* y, const Data_* weights, Data_* fitted, Data_* robust_weights, const Options& opt) {
    if (opt.sorted) {
        std::vector<Data_> rbuffer;
        if (robust_weights == NULL) {
            rbuffer.resize(num_points);
            robust_weights = rbuffer.data();
        }
        fit_trend(num_points, x, y, weights, fitted, robust_weights, opt);

    } else {
        /* Sorts the observations by the means, applies the same permutation to the
         * variances. This makes downstream processing quite a lot easier.
         */
        std::vector<Data_> xbuffer(x, x + num_points);
        std::vector<Data_> ybuffer(y, y + num_points);

        std::vector<Data_> wbuffer;
        Data_* wptr = NULL;
        if (weights) {
            wbuffer.insert(wbuffer.end(), weights, weights + num_points);
            wptr = wbuffer.data();
        }

        fit_trend(num_points, xbuffer.data(), ybuffer.data(), wptr, fitted, robust_weights, opt);
    }
}

/**
 * Run the LOWESS smoother and fill output arrays with the smoothed values.
 * This differs from `compute()` in that it modifies `x` and `y` (and `weights`, if supplied) on output,
 * allowing us to avoid a copy if they are no longer needed after this method call. 
 *
 * @tparam Data_ Floating-point type for the data.
 *
 * @param n Number of points.
 * @param[in] x Pointer to an array of `n` x-values.
 * On output, this may be modified in an unspecified manner.
 * @param[in] y Pointer to an array of `n` y-values.
 * On output, this may be modified in an unspecified manner.
 * @param[in] weights Pointer to an array of `n` positive weights.
 * On output, this may be modified in an unspecified manner.
 * Alternatively `NULL` if no weights are available.
 * @param[out] fitted Pointer to an output array of length `n`, in which the fitted values of the smoother can be stored.
 * @param[out] robust_weights Pointer to an output array of length `n`, in which the robustness weights can be stored.
 * @param opt Further options.
 */
template<typename Data_>
void compute_in_place(size_t num_points, Data_* x, Data_* y, Data_* weights, Data_* fitted, Data_* robust_weights, const Options& opt) {
    if (opt.sorted) {
        std::vector<Data_> rbuffer;
        if (robust_weights == NULL) {
            rbuffer.resize(num_points);
            robust_weights = rbuffer.data();
        }
        fit_trend(num_points, x, y, weights, fitted, robust_weights, opt);

    } else {
        sort_and_run(num_points, x, y, weights, fitted, robust_weights, opt);
    }
}

/** 
 * @brief Store the smoothing results.
 * @tparam Data_ Floating-point type for the data.
 */
template<typename Data_>
struct Results {
    /**
     * @param n Number of points.
     */
    Results(size_t n) : fitted(n), robust_weights(n) {}

    /**
     * Fitted values from the LOWESS smoother. 
     */
    std::vector<Data_> fitted;

    /**
     * Robustness weight for each point.
     */
    std::vector<Data_> robust_weights;
};

/**
 * Run the LOWESS smoother and return a `Results` object.
 * This avoids the need to instantiate the various output arrays manually. 
 * 
 * @tparam Data_ Floating-point type for the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-values.
 * @param[in] y Pointer to an array of `num_points` y-values.
 * @param[in] weights Pointer to an array of `n` positive weights.
 * Alternatively `NULL` if no weights are available.
 * @param opt Further options.
 *
 * @return A `Results` object containing the fitted values and robustness weights.
 */
template<typename Data_>
Results<Data_> compute(size_t num_points, const Data_* x, const Data_* y, const Data_* weights, const Options& opt) {
    Results<Data_> output(num_points);
    compute(num_points, x, y, weights, output.fitted.data(), output.robust_weights.data(), opt);
    return output;
}

/**
 * Run the LOWESS smoother and return a `Results` object.
 * This differs from `compute()` in that `x` and `y` (and `weights`, if supplied) will be arbitrarily modified on output,
 * allowing us to avoid a copy if they are no longer needed after this method call. 
 * 
 * @tparam Data_ Floating-point type for the data.
 *
 * @param num_points Number of points.
 * @param[in] x Pointer to an array of `num_points` x-values.
 * @param[in] y Pointer to an array of `num_points` y-values.
 * @param[in] weights Pointer to an array of `n` positive weights.
 * Alternatively `NULL` if no weights are available.
 * @param opt Further options.
 *
 * @return A `Results` object containing the fitted values and robustness weights.
 */
template<typename Data_>
Results<Data_> compute_in_place(size_t num_points, Data_* x, Data_* y, Data_* weights, const Options& opt) {
    Results<Data_> output(num_points);
    compute_in_place(num_points, x, y, weights, output.fitted.data(), output.robust_weights.data(), opt);
    return output;
}

}

#endif
