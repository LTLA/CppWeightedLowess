#ifndef UTILS_H
#define UTILS_H

#include "gtest/gtest.h"

#include <vector>
#include <random>

inline std::pair<std::vector<double>, std::vector<double> > simulate(size_t n, bool sorted = true) {
    std::mt19937_64 rng(n);
    std::normal_distribution ndist;

    std::pair<std::vector<double>, std::vector<double> > output;
    output.first.reserve(n);
    output.second.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        output.first.push_back(ndist(rng));
        output.second.push_back(ndist(rng));
    }

    if (sorted) {
        std::sort(output.first.begin(), output.first.end());
    }
    return output;
}

inline void compare_almost_equal(const std::vector<double>& first, const std::vector<double>& second) {
    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        EXPECT_FLOAT_EQ(first[i], second[i]);
    }
    return;
}

inline double sum_abs_diff(const std::vector<double>& first, const std::vector<double>& second) {
    EXPECT_EQ(first.size(), second.size());
    if (first.size() == second.size()) {
        double sumdiff = 0;
        for (size_t i = 0; i < first.size(); ++i) {
            sumdiff += std::abs(first[i] - second[i]);
        }
        return sumdiff;
    } else {
        return 0;
    }
}

#endif
