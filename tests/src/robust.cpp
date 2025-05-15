#include <gtest/gtest.h>
#include "WeightedLowess/robust.hpp"
#include "utils.h"

TEST(RobustTest, BasicMad) {
    std::vector<double> resids { 0.5, 0.2, -1, 1.5, -2 };
    std::vector<double> fitted(resids.size()), y(resids.size());
    for (size_t i = 0; i < y.size(); ++i) {
        fitted[i] = i;
        y[i] = i + resids[i];
    }

    std::vector<double> abs_dev;
    std::vector<size_t> perm;
    { 
        auto cmad = WeightedLowess::internal::compute_mad(y.size(), y.data(), fitted.data(), static_cast<double*>(NULL), static_cast<double>(resids.size()), abs_dev, perm);
        EXPECT_FLOAT_EQ(cmad, 1);
        for (size_t i = 0; i < resids.size(); ++i) {
            EXPECT_FLOAT_EQ(abs_dev[i], std::abs(resids[i]));
        }
    }

    // Even number of points now.
    resids.push_back(0.1);
    fitted.push_back(10);
    y.push_back(resids.back() + 10);
    { 
        auto cmad = WeightedLowess::internal::compute_mad(y.size(), y.data(), fitted.data(), static_cast<double*>(NULL), static_cast<double>(resids.size()), abs_dev, perm);
        EXPECT_FLOAT_EQ(cmad, 0.75);
        for (size_t i = 0; i < resids.size(); ++i) {
            EXPECT_FLOAT_EQ(abs_dev[i], std::abs(resids[i]));
        }
    }
}

TEST(RobustTest, WeightedMad) {
    std::vector<double> resids { 0.5, 0.2, -1, 1.5, -2 };
    std::vector<double> fitted(resids.size()), y(resids.size());
    for (size_t i = 0; i < y.size(); ++i) {
        fitted[i] = i;
        y[i] = i + resids[i];
    }

    std::vector<double> abs_dev;
    std::vector<size_t> perm;
    {
        std::vector<double> weights { 1, 5, 1, 1, 1 };
        auto total = std::accumulate(weights.begin(), weights.end(), 0.0);
        auto cmad = WeightedLowess::internal::compute_mad(y.size(), y.data(), fitted.data(), weights.data(), total, abs_dev, perm);
        EXPECT_FLOAT_EQ(cmad, 0.2);
    }

    {
        std::vector<double> weights { 2, 1, 1, 1, 1 };
        auto total = std::accumulate(weights.begin(), weights.end(), 0.0);
        auto cmad = WeightedLowess::internal::compute_mad(y.size(), y.data(), fitted.data(), weights.data(), total, abs_dev, perm);
        EXPECT_FLOAT_EQ(cmad, 0.75);
    }
}

TEST(RobustTest, ZeroWeights) {
    {
        std::vector<double> weights { 0, 1, 0, 0, 1 };
        std::vector<double> y { -10, 2, 10, 11, 5 };
        auto out = WeightedLowess::internal::compute_robust_range(5, y.data(), weights.data());
        EXPECT_EQ(out, 3);
    }

    {
        std::vector<double> weights { 1, 0, 1, 1, 0 };
        std::vector<double> y { -10, 2, 10, 11, 5 };
        auto out = WeightedLowess::internal::compute_robust_range(5, y.data(), weights.data());
        EXPECT_EQ(out, 21);
    }
}

TEST(RobustTest, PopulateWeights) {
    std::vector<double> abs_dev { 1, 5, 10, 0, 3 };
    std::vector<double> robust(abs_dev.size());

    WeightedLowess::internal::populate_robust_weights(abs_dev, 5.0, robust.data());
    EXPECT_TRUE(robust[0] > 0 && robust[0] < 1);
    EXPECT_EQ(robust[1], 0);
    EXPECT_EQ(robust[2], 0);
    EXPECT_FLOAT_EQ(robust[3], 1);
    EXPECT_TRUE(robust[4] > 0 && robust[4] < 1);
}
