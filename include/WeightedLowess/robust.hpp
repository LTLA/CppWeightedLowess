#ifndef WEIGHTEDLOWESS_ROBUST_HPP
#define WEIGHTEDLOWESS_ROBUST_HPP

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstddef>

#include "sanisizer/sanisizer.hpp"

#include "utils.hpp"

namespace WeightedLowess {

namespace internal {

template<typename Data_>
Data_ compute_mad(
    const std::size_t num_points, 
    const Data_* const y, 
    const Data_* const fitted, 
    const Data_* const freq_weights, 
    Data_ total_weight, 
    std::vector<Data_>& abs_dev,
    std::vector<std::size_t>& permutation
) {
    sanisizer::resize(abs_dev, num_points); // resizing here for safety, even though it would be more performant to resize once outside the robustness loop in fit().
    for (I<decltype(num_points)> i = 0; i < num_points; ++i) {
        abs_dev[i] = std::abs(y[i] - fitted[i]);
    }

    sanisizer::resize(permutation, num_points);
    std::iota(permutation.begin(), permutation.end(), static_cast<std::size_t>(0));
    std::sort(permutation.begin(), permutation.end(), [&](std::size_t left, std::size_t right) -> bool { return abs_dev[left] < abs_dev[right]; });

    Data_ curweight = 0;
    const Data_ halfweight = total_weight / 2;
    for (I<decltype(num_points)> i = 0; i < num_points; ++i) {
        const auto pt = permutation[i];
        curweight += (freq_weights != NULL ? freq_weights[pt] : 1);

        if (curweight == halfweight) { 
            const auto next_pt = permutation[i + 1]; // increment is safe as 'i + 1 <= num_points'.
            return abs_dev[pt] + (abs_dev[next_pt] - abs_dev[pt]) / 2.0; // reduce risk of overflow.
        } else if (curweight > halfweight) {
            return abs_dev[pt];
        }
    }

    return 0;
}

template<typename Data_>
Data_ compute_robust_range(const std::size_t num_points, const Data_* const y, const Data_* const robust_weights) {
    Data_ first = 0;
    I<decltype(num_points)> i = 0;
    for (; i < num_points; ++i) {
        if (robust_weights[i]) {
            first = y[i];
            ++i;
            break;
        }
    }

    Data_ min = first, max = first;
    for (; i < num_points; ++i) {
        if (robust_weights[i]) {
            const auto val = y[i];
            min = std::min(val, min);
            max = std::max(val, max);
        }
    }

    return max - min;
}

template<typename Data_>
Data_ square (const Data_ x) {
    return x * x;
}

template<typename Data_>
void populate_robust_weights(const std::vector<Data_>& abs_dev, const Data_ threshold, Data_* const robust_weights) {
    const auto num_points = abs_dev.size();
    for (I<decltype(num_points)> i = 0; i < num_points; ++i) {
        const auto ad = abs_dev[i];
        // Effectively a branchless if/else, which should help auto-vectorization.
        // This assumes that threshold > 0, which should be true from fit_trend().
        robust_weights[i] = (ad < threshold) * square(static_cast<Data_>(1) - square(ad/threshold));
    }
}

}

}

#endif
