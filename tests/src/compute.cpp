#include <gtest/gtest.h>
#include "WeightedLowess/compute.hpp"
#include "utils.h"

class ComputeTest : public ::testing::TestWithParam<int> {};

TEST_P(ComputeTest, Exact) {
    auto simulated = simulate(1000);
    const auto& x = simulated.first;
    WeightedLowess::Options opt;
    opt.anchors = GetParam(); // should work under all numbers of anchors, as it's a straight line.

    // y = x
    auto res = WeightedLowess::compute(x.size(), x.data(), x.data(), opt);
    compare_almost_equal(res.fitted, x);

    // y = 2x + 1
    std::vector<double> alt(x);
    for (auto& i : alt) {
        i = 2*i + 1;
    }
    res = WeightedLowess::compute(x.size(), x.data(), alt.data(), opt);
    compare_almost_equal(res.fitted, alt);

    // y = 2.5 - x/4
    alt = x;
    for (auto& i : alt) {
        i = 2.5 - i/4;
    }
    res = WeightedLowess::compute(x.size(), x.data(), alt.data(), opt);
    compare_almost_equal(res.fitted, alt);
}

TEST_P(ComputeTest, Weights) {
    auto simulated = simulate(888);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;
    opt.anchors = GetParam();

    // No effect when dealing with a straight line.
    {
        auto res = WeightedLowess::compute(x.size(), x.data(), x.data(), opt);

        std::vector<double> mock_weights(x.size());
        std::iota(mock_weights.begin(), mock_weights.end(), 0.1);

        auto wopt = opt;
        wopt.weights = mock_weights.data();
        auto wres = WeightedLowess::compute(x.size(), x.data(), x.data(), wopt); 
        compare_almost_equal(res.fitted, wres.fitted);
    }

    // No effect when weights are all equal.
    {
        auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

        std::vector<double> mock_weights(x.size(), 1);
        auto wopt = opt;
        wopt.weights = mock_weights.data(); 
        auto wres = WeightedLowess::compute(x.size(), x.data(), y.data(), wopt); 
        compare_almost_equal(res.fitted, wres.fitted);
    }

    // Weighting has a frequency interpretation.
    {
        std::vector<double> fweights(x.size());
        std::vector<double> expanded_x, expanded_y;
        for (size_t i = 0; i < x.size(); ++i) {
            fweights[i] = std::max(1.0, std::round(x[i] * 5));
            expanded_x.insert(expanded_x.end(), fweights[i], x[i]);
            expanded_y.insert(expanded_y.end(), fweights[i], y[i]);
        }

        auto eres = WeightedLowess::compute(expanded_x.size(), expanded_x.data(), expanded_y.data(), opt);

        auto wopt = opt;
        wopt.weights = fweights.data();
        auto wres = WeightedLowess::compute(x.size(), x.data(), y.data(), wopt); 

        std::vector<double> ref;
        for (size_t i =0; i < x.size(); ++i) { 
            ref.insert(ref.end(), fweights[i], wres.fitted[i]);
        }
        compare_almost_equal(ref, eres.fitted);
    }
}

TEST_P(ComputeTest, Robustness) {
    auto simulated = simulate(500);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    WeightedLowess::Options opt;
    opt.anchors = GetParam();

    // Spiking in a crazy thing.
    {
        std::vector<double> x1 = x, y1 = y;
        x1.insert(x1.begin() + 25, x[25]); // injecting a value to remain sorted.
        y1.insert(y1.begin() + 25, 100);

        auto wres = WeightedLowess::compute(x1.size(), x1.data(), y1.data(), opt);
        EXPECT_EQ(wres.robust_weights[25], 0);
    }

    // Same results for a straight line.
    {
        std::vector<double> x1 = x;
        x1.insert(x1.begin() + 10, (x[9] + x[10]) / 2); // injecting a value to remain sorted.
        auto x2 = x1;
        x2[10] = 100;

        auto ref = WeightedLowess::compute(x.size(), x.data(), x.data(), opt);
        auto res = WeightedLowess::compute(x1.size(), x1.data(), x2.data(), opt);

        std::vector<double> obsfitted = res.fitted;
        obsfitted.erase(obsfitted.begin() + 10);
        compare_almost_equal(ref.fitted, obsfitted);
    }
}

INSTANTIATE_TEST_SUITE_P(
    Compute,
    ComputeTest,
    ::testing::Values(10, 50, 200, 1000)
);

TEST(Compute, Unsorted) {
    std::vector<double> unsorted(10);
    unsorted[0] = 1;
    WeightedLowess::Options opt;

    std::string msg;
    try {
        WeightedLowess::compute(unsorted.size(), unsorted.data(), unsorted.data(), opt);
    } catch (std::exception& e) {
        msg = e.what();
    }

    EXPECT_TRUE(msg.find("sorted") != std::string::npos);
}

TEST(Compute, TiedInterpolation) {
    // Basically getting some coverage of the interpolation when the anchors
    // have tied coordinates. This requires manual specification of the anchors
    // as define_anchors() would never give us tied anchors.
    std::vector<double> x { 0., 0., 0., 0., 0., 0. };
    std::vector<double> y { 1., 2., 3., 4., 5., 6. };

    WeightedLowess::PrecomputedWindows<double> win;
    win.anchors.push_back(0);
    win.anchors.push_back(x.size() - 1);
    win.freq_weights = NULL;
    win.total_weight = x.size();

    win.limits.resize(2);
    win.limits.front().left = 0;
    win.limits.front().right = 2;
    win.limits.front().distance = 0;
    win.limits.back().left = 3;
    win.limits.back().right = 5;
    win.limits.back().distance = 0;

    WeightedLowess::Options opt;
    opt.iterations = 0;
    std::vector<double> fitted(x.size());
    WeightedLowess::compute(x.size(), x.data(), win, y.data(), fitted.data(), static_cast<double*>(NULL), opt);

    EXPECT_FLOAT_EQ(fitted[0], 2.0);
    for (int i = 1; i < 4; ++i) {
        EXPECT_FLOAT_EQ(fitted[i], 3.5);
    }
    EXPECT_FLOAT_EQ(fitted[5], 5.0);
}

TEST(ComputeTests, QuitEarly) {
    auto simulated = simulate(1004);
    const auto& x = simulated.first;
    std::vector<double> y(x.size(), 101);

    // Quits early before any iterations.
    {
        WeightedLowess::Options opt;
        auto wres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
        EXPECT_EQ(wres.robust_weights, std::vector<double>(x.size(), 1));
        EXPECT_EQ(wres.fitted, std::vector<double>(x.size(), 101));
    }

    // Also quits early after one iteration. To simplify the test, we balance
    // the outliers so that the fitted values are preserved correctly.
    // (Otherwise, the shift in the fitted values would cause neighbor points
    // to be considered outliers and get robustness weights of zero.)
    {
        auto xcopy = x;
        auto ycopy = y;
        xcopy[0] = xcopy[1];
        ycopy[0] = 0;
        ycopy[1] = 202;

        WeightedLowess::Options opt;
        auto wres = WeightedLowess::compute(xcopy.size(), xcopy.data(), ycopy.data(), opt);
        compare_almost_equal(wres.fitted, std::vector<double>(xcopy.size(), 101));

        std::vector<double> expected_weights(xcopy.size(), 1);
        expected_weights[0] = 0;
        expected_weights[1] = 0;
        compare_almost_equal(wres.robust_weights, expected_weights);
    }
}

TEST(ComputeTests, Empty) {
    WeightedLowess::Options opt;
    std::vector<double> x, y;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_EQ(res.fitted.size(), 0);
    EXPECT_EQ(res.robust_weights.size(), 0);
}
