#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

TEST(TiesTest, DuplicatedTies) {
    WeightedLowess::WeightedLowess wl;
    wl.set_anchors(20);

    // Handles ties with elegance and grace.
    auto res = wl.run(x.size(), x.data(), y.data());

    auto x2 = x;
    x2.insert(x2.end(), x.begin(), x.end());
    auto y2 = y;
    y2.insert(y2.end(), y.begin(), y.end());
    auto res2 = wl.run(x2.size(), x2.data(), y2.data());

    {
        std::vector<double> sub(res2.fitted.begin(), res2.fitted.begin() + x.size());
        compare_almost_equal(sub, res.fitted);
    }
    {
        std::vector<double> sub(res2.fitted.begin() + x.size(), res2.fitted.begin() + 2*x.size());
        compare_almost_equal(sub, res.fitted);
    }
}

TEST(TiesTest, StartTies) {
    // Inserting a stack of observations at the _start_ of the ordered sequence.
    // We insert enough so that the default span will be zero for many elements.
    auto x2 = x;
    size_t ninserts = x.size();
    x2.insert(x2.end(), ninserts, *std::min_element(x.begin(), x.end()));
    auto y2 = y;
    y2.insert(y2.end(), y.begin(), y.begin() + ninserts);

    WeightedLowess::WeightedLowess wl;
    auto res = wl.set_anchors(20).run(x2.size(), x2.data(), y2.data());

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

TEST(TiesTest, EndTies) {
    // Inserting a stack of observations at the _end_ of the ordered sequence.
    // We insert enough so that the default span will be zero for many elements.
    auto x2 = x;
    size_t ninserts = x.size();
    x2.insert(x2.end(), ninserts, *std::max_element(x.begin(), x.end()));
    auto y2 = y;
    y2.insert(y2.end(), y.begin(), y.begin() + ninserts);

    WeightedLowess::WeightedLowess wl;
    auto res = wl.set_anchors(20).run(x2.size(), x2.data(), y2.data());

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
    // Behaves properly when all y-values are tied.
    std::vector<double> constant(x.size(), 1.2);
    WeightedLowess::WeightedLowess wl;
    auto res = wl.run(x.size(), x.data(), constant.data());
    EXPECT_EQ(res.fitted, constant);
    EXPECT_EQ(res.robust_weights, std::vector<double>(x.size(), 1));
}
