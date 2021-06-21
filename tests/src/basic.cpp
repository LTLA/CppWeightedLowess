#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

TEST(BasicTests, Exact) {
    WeightedLowess::WeightedLowess wl;

    // y = x
    auto res = wl.run(x.size(), x.data(), x.data());
    compare_almost_equal(res.fitted, x);

    // y = 2x + 1
    std::vector<double> alt(x);
    for (auto& i : alt) {
        i = 2*i + 1;
    }
    res = wl.run(x.size(), x.data(), alt.data());
    compare_almost_equal(res.fitted, alt);

    // y = 2.5 - x/4
    alt = x;
    for (auto& i : alt) {
        i = 2.5 - i/4;
    }
    res = wl.run(x.size(), x.data(), alt.data());
    compare_almost_equal(res.fitted, alt);
}

TEST(BasicTests, Interpolation) {
    WeightedLowess::WeightedLowess wl;

    // y = 2x + 1
    std::vector<double> alt(x);
    for (auto& i : alt) {
        i = 2*i + 1;
    }

    auto res = wl.run(x.size(), x.data(), alt.data());
    auto res2 = wl.set_anchors(10).run(x.size(), x.data(), alt.data());
    compare_almost_equal(res.fitted, res2.fitted);

    // Only two points; start and end.
    res2 = wl.set_anchors(2).run(x.size(), x.data(), alt.data());
    compare_almost_equal(res.fitted, res2.fitted);
}

TEST(BasicTests, Weights) {
    WeightedLowess::WeightedLowess wl;
    wl.set_anchors(20);

    // No effect when dealing with a straight line.
    auto res = wl.run(x.size(), x.data(), x.data());
    auto wres = wl.run(x.size(), x.data(), x.data(), x.data()); // reusing 'x' as positive weights.
    compare_almost_equal(res.fitted, wres.fitted);

    // Weighting has a frequency interpretation.
    std::vector<double> fweights(x.size());
    std::vector<double> expanded_x, expanded_y;
    for (size_t i = 0; i < x.size(); ++i) {
        fweights[i] = std::max(1.0, std::round(x[i] * 5));
        expanded_x.insert(expanded_x.end(), fweights[i], x[i]);
        expanded_y.insert(expanded_y.end(), fweights[i], y[i]);
    }
    auto eres = wl.run(expanded_x.size(), expanded_x.data(), expanded_y.data());

    std::vector<double> ref;
    wres = wl.run(x.size(), x.data(), y.data(), fweights.data()); 
    for (size_t i =0; i < x.size(); ++i) { 
        ref.insert(ref.end(), fweights[i], wres.fitted[i]);
    }
    compare_almost_equal(ref, eres.fitted);
}

TEST(BasicTests, Robustness) {
    // Spiking in a crazy thing.
    auto x1 = x;
    auto y1 = y;
    x1.push_back(0.5);
    y1.push_back(100);

    WeightedLowess::WeightedLowess wl;
    auto wres = wl.run(x1.size(), x1.data(), y1.data());
    EXPECT_EQ(wres.robust_weights[x1.size()-1], 0);

    // Comparing to a straight line.
    auto x2 = x;
    x2.push_back(10);
    auto res = wl.run(x.size(), x.data(), x.data());
    wres = wl.run(x1.size(), x1.data(), x2.data());

    std::vector<double> rest(wres.fitted.begin(), wres.fitted.begin() + x.size());
    compare_almost_equal(res.fitted, rest);
}
