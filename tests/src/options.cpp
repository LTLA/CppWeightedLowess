#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

TEST(OptionsTest, IterationsEffect) {
    WeightedLowess::WeightedLowess wl;
    wl.set_anchors(20);

    auto res = wl.run(x.size(), x.data(), y.data());

    auto res1 = wl.set_iterations(1).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res2 = wl.set_iterations(2).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);
}

TEST(OptionsTest, PointsEffect) {
    WeightedLowess::WeightedLowess wl;

    auto res = wl.set_anchors(200).run(x.size(), x.data(), y.data());
    auto res1 = wl.set_anchors(10).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res2 = wl.set_anchors(5).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);
}

TEST(OptionsTest, WeightsEffect) {
    WeightedLowess::WeightedLowess wl;

    // Weights have some kind of effect for non-trivial trends.
    auto res = wl.run(x.size(), x.data(), y.data());
    auto wres = wl.run(x.size(), x.data(), y.data(), x.data()); // reusing 'x' as positive weights.
    EXPECT_TRUE(sum_abs_diff(res.fitted, wres.fitted) > 0.01);

    // Weighting can be treated as non-frequency weights.
    auto wres2 = wl.set_as_frequency_weights(false).run(x.size(), x.data(), y.data(), x.data()); 
    EXPECT_TRUE(sum_abs_diff(wres.fitted, wres2.fitted) > 0.01);
}

TEST(OptionsTest, SpanEffect) {
    WeightedLowess::WeightedLowess wl;

    auto res = wl.run(x.size(), x.data(), y.data());
    auto res1 = wl.set_span(0.6).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res2 = wl.set_span(0.1).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);

    // We can interpret the span as the weight inside the window.
    {
        WeightedLowess::WeightedLowess wl2;
        auto default_span = WeightedLowess::WeightedLowess<>::Defaults::span;
        auto res3 = wl2.set_span(default_span * static_cast<double>(x.size())).set_span_as_proportion(false).run(x.size(), x.data(), y.data());
        EXPECT_EQ(res.fitted, res3.fitted);
    }

    // Setting a minimum width does something.
    {
        WeightedLowess::WeightedLowess wl2;
        auto res3a = wl2.set_min_width(0.5).run(x.size(), x.data(), y.data());
        EXPECT_TRUE(sum_abs_diff(res.fitted, res3a.fitted) > 0.01);

        auto res3b = wl2.set_min_width(10).run(x.size(), x.data(), y.data());
        EXPECT_TRUE(sum_abs_diff(res.fitted, res3b.fitted) > 0.01);
        EXPECT_TRUE(sum_abs_diff(res3a.fitted, res3b.fitted) > 0.01);

        auto ref = wl2.set_min_width(0).set_span(1).run(x.size(), x.data(), y.data());
        EXPECT_EQ(ref.fitted, res3b.fitted);
    }
}

TEST(OptionsTest, DeltaTest) {
    WeightedLowess::WeightedLowess wl;

    auto res = wl.set_delta(0.5).run(x.size(), x.data(), y.data());
    auto res1 = wl.set_delta(0.1).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    auto res0 = wl.set_delta(0).run(x.size(), x.data(), y.data());
    EXPECT_TRUE(sum_abs_diff(res.fitted, res0.fitted) > 0.01);

    // Should be equal to the case where each point is a seed.
    auto res_full  = wl.set_delta(-1).set_anchors(100).run(x.size(), x.data(), y.data());
    EXPECT_EQ(res0.fitted, res_full.fitted);

    // Points has no effect if delta is set.
    auto res0_2 = wl.set_delta(0).set_anchors(5).run(x.size(), x.data(), y.data());
    EXPECT_EQ(res0.fitted, res0_2.fitted);
}
