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
#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
    #pragma omp parallel for num_threads(nthreads)
#endif
    for (size_t pt = 0; pt < num_points; ++pt) {
#else
    WEIGHTEDLOWESS_CUSTOM_PARALLEL(num_points, nthreads, [&](size_t, size_t start, size_t length) {
    for (size_t pt = start, end = start + length; pt < end; ++pt) {
#endif

        abs_dev[pt] = std::abs(y[pt] - fitted[pt]);

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
    }
#else
    }
    });
#endif

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
Data_ square (Data_ x) {
    return x * x;
}

template<typename Data_>
void populate_robust_weights(const std::vector<Data_>& abs_dev, Data_ threshold, Data_* robust_weights, [[maybe_unused]] int nthreads) {
    size_t num_points = abs_dev.size();

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
    #pragma omp parallel for num_threads(nthreads)
#endif
    for (size_t i = 0; i < num_points; ++i) {
#else
    WEIGHTEDLOWESS_CUSTOM_PARALLEL(num_points, nthreads, [&](size_t, size_t start, size_t length) {
    for (size_t i = start, end = start + length; i < end; ++i) {
#endif

        if (abs_dev[i] < threshold) {
            robust_weights[i] = square(1 - square(abs_dev[i]/threshold));
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

}

#endif
