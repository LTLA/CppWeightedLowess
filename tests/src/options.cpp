#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

TEST(OptionsTest, IterationsEffect) {
    WeightedLowess::WeightedLowess wl;
    wl.set_points(20);

    auto res = wl.run(x.size(), x.data(), y.data());

    auto res1 = wl.set_iterations(1).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res2 = wl.set_iterations(2).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);
}

TEST(OptionsTest, PointsEffect) {
    WeightedLowess::WeightedLowess wl;

    auto res = wl.set_points(200).run(x.size(), x.data(), y.data());
    auto res1 = wl.set_points(10).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res2 = wl.set_points(5).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);
}

TEST(OptionsTest, SpanEffect) {
    WeightedLowess::WeightedLowess wl;

    auto res = wl.run(x.size(), x.data(), y.data());
    auto res1 = wl.set_span(0.6).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res2 = wl.set_span(0.1).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);
}

TEST(OptionsTest, DeltaTest) {
    WeightedLowess::WeightedLowess wl;

    auto res = wl.set_delta(0.5).run(x.size(), x.data(), y.data());
    auto res1 = wl.set_delta(0.1).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res0 = wl.set_delta(0).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res0.fitted) > 0.01);

    // Should be equal to the case where each point is a seed.
    auto res_full  = wl.set_delta(-1).set_points(100).run(x.size(), x.data(), y.data());
    EXPECT_EQ(res0.fitted, res_full.fitted);

    // Points has no effect if delta is set.
    auto res0_2 = wl.set_delta(0).set_points(5).run(x.size(), x.data(), y.data());
    EXPECT_EQ(res0.fitted, res0_2.fitted);
}
