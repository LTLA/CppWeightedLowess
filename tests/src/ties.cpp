#include <gtest/gtest.h>
#include "WeightedLowess/compute.hpp"
#include "utils.h"

TEST(TiesTest, DuplicatedTies) {
    auto simulated = simulate(2000);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

    // Every element is duplicated evenly, so the fit should be the same.
    std::vector<double> x2, y2;
    for (size_t i = 0; i < x.size(); ++i) {
        x2.insert(x2.end(), 2, x[i]);
        y2.insert(y2.end(), 2, y[i]);
    }
    auto res2 = WeightedLowess::compute(x2.size(), x2.data(), y2.data(), opt);

    std::vector<double> obsfit;
    for (size_t i = 0; i < res.fitted.size(); ++i) {
        obsfit.insert(obsfit.end(), 2, res.fitted[i]);
    }
    compare_almost_equal(obsfit, res2.fitted);
}

TEST(TiesTest, StartTies) {
    auto simulated = simulate(2001);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    // Inserting a stack of observations at the _start_ of the ordered sequence.
    // We insert enough so that the default span will be zero for many elements.
    auto x2 = x;
    size_t ninserts = x.size();
    x2.insert(x2.begin(), ninserts, x[0]);
    auto y2 = y;
    y2.insert(y2.begin(), y.begin(), y.begin() + ninserts);

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x2.size(), x2.data(), y2.data(), opt);

    // No one is NA or Inf'd.
    for (auto x : res.fitted) {
        EXPECT_TRUE(std::isfinite(x));
    }

    // All inserted elements have the same value.
    double last = res.fitted.front();
    for (size_t i = 0; i < ninserts; ++i) {
        EXPECT_FLOAT_EQ(last, res.fitted[i]);
    }
}

TEST(TiesTest, EndTies) {
    auto simulated = simulate(2002);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    // Inserting a stack of observations at the _end_ of the ordered sequence.
    // We insert enough so that the default span will be zero for many elements.
    auto x2 = x;
    size_t ninserts = x.size();
    x2.insert(x2.end(), ninserts, x.back());
    auto y2 = y;
    y2.insert(y2.end(), y.begin(), y.begin() + ninserts);

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x2.size(), x2.data(), y2.data(), opt);

    // No one is NA or Inf'd.
    for (auto x : res.fitted) {
        EXPECT_TRUE(std::isfinite(x));
    }

    // All inserted elements have the same value.
    double last = res.fitted.back();
    for (size_t i = 0; i < ninserts; ++i) {
        EXPECT_FLOAT_EQ(last, res.fitted[res.fitted.size() - i - 1]);
    }
}

TEST(TiesTest, TiedYValues) {
    auto simulated = simulate(2003);
    const auto& x = simulated.first;
    std::vector<double> constant(x.size(), 1.2); // Behaves properly when all y-values are tied.

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x.size(), x.data(), constant.data(), opt);

    EXPECT_EQ(res.fitted, constant);
    EXPECT_EQ(res.robust_weights, std::vector<double>(x.size(), 1));
}
