#include <gtest/gtest.h>
#include "parallel.h"
#include "WeightedLowess/compute.hpp"
#include "utils.h"

TEST(OptionsTest, IterationsEffect) {
    auto simulated = simulate(900);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

    opt.iterations = 1;
    auto res1 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    opt.iterations = 2;
    auto res2 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);
}

TEST(OptionsTest, AnchorsEffect) {
    auto simulated = simulate(901);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

    opt.anchors = 10;
    auto res1 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    opt.anchors = 5;
    auto res2 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);

    // Make sure this doesn't crash; we should always pick at least the first point.
    opt.anchors = 0;
    auto res3 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res3.fitted) > 0.01);
}

TEST(OptionsTest, WeightsEffect) {
    auto simulated = simulate(902);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

    // Weights have some kind of effect for non-trivial trends.
    std::vector<double> weights(x.size());
    std::mt19937_64 rng(9999);
    std::uniform_real_distribution udist;
    for (auto& w : weights) {
        w = udist(rng);
    }

    opt.weights = weights.data();
    auto wres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, wres.fitted) > 0.01);

    // Weighting can be treated as non-frequency weights.
    opt.frequency_weights = false;
    auto wres2 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(wres.fitted, wres2.fitted) > 0.01);
}

TEST(OptionsTest, SpanEffect) {
    auto simulated = simulate(903);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

    opt.span = 0.1;
    auto res1 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    opt.span = 0.6;
    auto res2 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res2.fitted) > 0.01);

    // We can interpret the span as the number of points inside the window.
    {
        WeightedLowess::Options opt2;
        opt2.span *= x.size();
        opt2.span_as_proportion = false;
        auto res3 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt2);
        EXPECT_EQ(res.fitted, res3.fitted);
    }

    // Setting a minimum width does something.
    {
        WeightedLowess::Options opt2;
        opt2.minimum_width = 1;
        auto res3a = WeightedLowess::compute(x.size(), x.data(), y.data(), opt2);
        EXPECT_TRUE(sum_abs_diff(res.fitted, res3a.fitted) > 0.01);

        // Forcing a width to cover all points.
        opt2.minimum_width = 100;
        auto res3b = WeightedLowess::compute(x.size(), x.data(), y.data(), opt2);
        EXPECT_TRUE(sum_abs_diff(res.fitted, res3b.fitted) > 0.01);
        EXPECT_TRUE(sum_abs_diff(res3a.fitted, res3b.fitted) > 0.01);

        // This should be the same as actually covering all points.
        opt2.minimum_width = 0;
        opt2.span = 1;
        auto ref = WeightedLowess::compute(x.size(), x.data(), y.data(), opt2);
        EXPECT_EQ(ref.fitted, res3b.fitted);
    }
}

TEST(OptionsTest, DeltaTest) {
    auto simulated = simulate(904);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;

    opt.delta = 0.5;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

    opt.delta = 0.1;
    auto res1 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res1.fitted) > 0.01);

    opt.delta = 0;
    auto res0 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_TRUE(sum_abs_diff(res.fitted, res0.fitted) > 0.01);

    // Should be equal to the case where each point is a seed.
    opt.delta = -1;
    opt.anchors = x.size();
    auto res_full = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_EQ(res0.fitted, res_full.fitted);

    // Points has no effect if delta is set.
    opt.delta = 0;
    opt.anchors = 5;
    auto res0_2 = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_EQ(res0.fitted, res0_2.fitted);
}

TEST(OptionsTest, Parallelized) {
    auto simulated = simulate(905);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    // Span parallelization under various circumstances:
    {
        WeightedLowess::Options opt;
        auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
        opt.num_threads = 3;
        auto pres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

        EXPECT_EQ(res.fitted, pres.fitted);
        EXPECT_EQ(res.robust_weights, pres.robust_weights);
    }

    {
        WeightedLowess::Options opt;
        opt.anchors = 50;
        auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
        opt.num_threads = 3;
        auto pres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

        EXPECT_EQ(res.fitted, pres.fitted);
        EXPECT_EQ(res.robust_weights, pres.robust_weights);
    }

    {
        WeightedLowess::Options opt;
        opt.span = 1;
        auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
        opt.num_threads = 3;
        auto pres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

        EXPECT_EQ(res.fitted, pres.fitted);
        EXPECT_EQ(res.robust_weights, pres.robust_weights);
    }

    {
        WeightedLowess::Options opt;
        opt.iterations = 10;
        auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
        opt.num_threads = 3;
        auto pres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

        EXPECT_EQ(res.fitted, pres.fitted);
        EXPECT_EQ(res.robust_weights, pres.robust_weights);
    }
}
