#ifndef WEIGHTEDLOWESS_ROBUST_HPP
#define WEIGHTEDLOWESS_ROBUST_HPP

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace WeightedLowess {

namespace internal {

template<typename Data_>
Data_ compute_mad(
    size_t num_points, 
    const Data_* y, 
    const Data_* fitted, 
    const Data_* freq_weights, 
    Data_ total_weight, 
    std::vector<Data_>& abs_dev, 
    std::vector<size_t>& permutation, 
    [[maybe_unused]] int nthreads) 
{
#ifdef _OPENMP
    #pragma omp simd
#endif
    for (size_t i = 0; i < num_points; ++i) {
        abs_dev[i] = std::abs(y[i] - fitted[i]);
    }

    std::iota(permutation.begin(), permutation.end(), 0);
    std::sort(permutation.begin(), permutation.end(), [&](size_t left, size_t right) -> bool { return abs_dev[left] < abs_dev[right]; });

    Data_ curweight = 0;
    const Data_ halfweight = total_weight/2;
    for (size_t i = 0; i < num_points; ++i) {
        auto pt = permutation[i];
        curweight += (freq_weights != NULL ? freq_weights[pt] : 1);

        if (curweight == halfweight) { 
            auto next_pt = permutation[i + 1];
            return (abs_dev[pt] + abs_dev[next_pt]) / 2.0;
        } else if (curweight > halfweight) {
            return abs_dev[pt];
        }
    }

    return 0;
}

template<typename Data_>
Data_ compute_robust_range(size_t num_points, const Data_* y, const Data_* robust_weights) {
    Data_ first = 0;
    size_t i = 0;
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
            auto val = y[i];
            min = std::min(val, min);
            max = std::max(val, max);
        }
    }

    return max - min;
}

template<typename Data_>
Data_ square (Data_ x) {
    return x * x;
}

template<typename Data_>
void populate_robust_weights(const std::vector<Data_>& abs_dev, Data_ threshold, Data_* robust_weights) {
    size_t num_points = abs_dev.size();

#ifdef _OPENMP
    #pragma omp simd
#endif
    for (size_t i = 0; i < num_points; ++i) {
        auto ad = abs_dev[i];
        // Effectively a branchless if/else, which should help auto-vectorization.
        robust_weights[i] = (ad < threshold) * square(1 - square(ad/threshold));
    }
}

}

}

#endif
