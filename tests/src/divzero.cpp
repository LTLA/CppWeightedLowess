#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

TEST(DivisionByZeroTests, ZeroVariance) {
    // Just large enough that, with the default span, each window contains the
    // centered element and two elements on the boundaries (and thus have zero
    // weight). This provides a test for what happens when the variance is zero
    // in the local regression, which is not usually possible.
    std::vector<double> small{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    WeightedLowess::WeightedLowess wl;
    auto output = wl.run(small.size(), small.data(), small.data());
    compare_almost_equal(output.fitted, small);
}

TEST(DivisionByZeroTests, ZeroWeight) {
    // With robustness weights, it is possible to obtain a total weight of zero
    // in a window if all of the observations have large residuals from the
    // initial fit.  In such cases, we just ignore the robustness weights in an
    // attempt to obtain _something_, anything.
    std::vector<double> x{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 10 };
    std::vector<double> y{ 2, 3, 4, 5, 6, 7, 8, 9, 100, -90 };

    WeightedLowess::WeightedLowess wl;
    auto output = wl.run(x.size(), x.data(), y.data());

    // The last two should be computed by just averaging the values at x = 10.
    for (size_t i = 0; i < 2; ++i) {
        EXPECT_EQ(output.fitted[y.size()-i-1], 5.0);
        EXPECT_EQ(output.robust_weights[y.size()-i-1], 0);
    }
}

TEST(DivisionByZeroTests, ZeroWeight2) {
    // Same protection applies if the span is zero in the problematic region.
    std::vector<double> x{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10 };
    std::vector<double> y{ 2, 3, 4, 5, 6, 7, 8, 9, 100, -100, 100, -50 };

    WeightedLowess::WeightedLowess wl;
    auto output = wl.run(x.size(), x.data(), y.data());

    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(output.fitted[y.size()-i-1], 50.0/4);
        EXPECT_EQ(output.robust_weights[y.size()-i-1], 0);
    }
}
