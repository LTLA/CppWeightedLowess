#include <gtest/gtest.h>
#include "WeightedLowess/compute.hpp"
#include "utils.h"

TEST(BasicTests, Exact) {
    auto simulated = simulate(1000);
    const auto& x = simulated.first;
    WeightedLowess::Options opt;

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

TEST(BasicTests, Unsorted) {
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

TEST(BasicTests, Interpolation) {
    auto simulated = simulate(1001);
    const auto& x = simulated.first;

    // y = 2x + 1
    std::vector<double> alt(x);
    for (auto& i : alt) {
        i = 2*i + 1;
    }

    WeightedLowess::Options opt;
    auto res = WeightedLowess::compute(x.size(), x.data(), alt.data(), opt);

    opt.anchors = 10;
    auto res2 = WeightedLowess::compute(x.size(), x.data(), alt.data(), opt);
    compare_almost_equal(res.fitted, res2.fitted);

    // Only two points; start and end.
    opt.anchors = 2;
    res2 = WeightedLowess::compute(x.size(), x.data(), alt.data(), opt);
    compare_almost_equal(res.fitted, res2.fitted);

    // Hell, trying only one point; the start!
    opt.anchors = 1;
    res2 = WeightedLowess::compute(x.size(), x.data(), alt.data(), opt);
    compare_almost_equal(res.fitted, res2.fitted);
}

TEST(BasicTests, Weights) {
    auto simulated = simulate(1002);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    // No effect when dealing with a straight line.
    {
        WeightedLowess::Options opt;
        auto res = WeightedLowess::compute(x.size(), x.data(), x.data(), opt);

        std::vector<double> mock_weights(x.size());
        std::iota(mock_weights.begin(), mock_weights.end(), 0.1);
        opt.weights = mock_weights.data();

        auto wres = WeightedLowess::compute(x.size(), x.data(), x.data(), opt); 
        compare_almost_equal(res.fitted, wres.fitted);
    }

    // No effect when weights are all equal.
    {
        WeightedLowess::Options opt;
        auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);

        std::vector<double> mock_weights(x.size(), 1);
        opt.weights = mock_weights.data(); 

        auto wres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt); 
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

        WeightedLowess::Options opt;
        auto eres = WeightedLowess::compute(expanded_x.size(), expanded_x.data(), expanded_y.data(), opt);

        opt.weights = fweights.data();
        auto wres = WeightedLowess::compute(x.size(), x.data(), y.data(), opt); 

        std::vector<double> ref;
        for (size_t i =0; i < x.size(); ++i) { 
            ref.insert(ref.end(), fweights[i], wres.fitted[i]);
        }
        compare_almost_equal(ref, eres.fitted);
    }
}

TEST(BasicTests, Robustness) {
    auto simulated = simulate(1003);
    const auto& x = simulated.first;
    const auto& y = simulated.second;

    // Spiking in a crazy thing.
    {
        std::vector<double> x1 = x, y1 = y;
        x1.insert(x1.begin() + 25, x[25]); // injecting a value to remain sorted.
        y1.insert(y1.begin() + 25, 100);

        WeightedLowess::Options opt;
        auto wres = WeightedLowess::compute(x1.size(), x1.data(), y1.data(), opt);
        EXPECT_EQ(wres.robust_weights[25], 0);
    }

    // Same results for a straight line.
    {
        std::vector<double> x1 = x;
        x1.insert(x1.begin() + 10, (x[9] + x[10]) / 2); // injecting a value to remain sorted.
        auto x2 = x1;
        x2[10] = 100;

        WeightedLowess::Options opt;
        auto ref = WeightedLowess::compute(x.size(), x.data(), x.data(), opt);
        auto wres = WeightedLowess::compute(x1.size(), x1.data(), x2.data(), opt);

        std::vector<double> obsfitted = wres.fitted;
        obsfitted.erase(obsfitted.begin() + 10);
        compare_almost_equal(ref.fitted, obsfitted);
    }
}

TEST(BasicTests, Empty) {
    WeightedLowess::Options opt;
    std::vector<double> x, y;
    auto res = WeightedLowess::compute(x.size(), x.data(), y.data(), opt);
    EXPECT_EQ(res.fitted.size(), 0);
    EXPECT_EQ(res.robust_weights.size(), 0);
}
