#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

TEST(BasicTests, Exact) {
    WeightedLowess::Options opt;

    // y = x
    auto res = WeightedLowess::compute(x.size(), x.data(), x.data(), NULL, opt);
    compare_almost_equal(res.fitted, x);

    // y = 2x + 1
    std::vector<double> alt(x);
    for (auto& i : alt) {
        i = 2*i + 1;
    }
    res = WeightedLowess::compute(x.size(), x.data(), alt.data(), NULL, opt);
    compare_almost_equal(res.fitted, alt);

    // y = 2.5 - x/4
    alt = x;
    for (auto& i : alt) {
        i = 2.5 - i/4;
    }
    res = WeightedLowess::compute(x.size(), x.data(), alt.data(), NULL, opt);
    compare_almost_equal(res.fitted, alt);
}

TEST(BasicTests, Interpolation) {
    WeightedLowess::Options opt;

    // y = 2x + 1
    std::vector<double> alt(x);
    for (auto& i : alt) {
        i = 2*i + 1;
    }

    auto res = WeightedLowess::compute(x.size(), x.data(), alt.data(), NULL, opt);
    auto res2 = wl.set_anchors(10).run(x.size(), x.data(), alt.data(), NULL, opt);
    compare_almost_equal(res.fitted, res2.fitted);

    // Only two points; start and end.
    res2 = wl.set_anchors(2).run(x.size(), x.data(), alt.data(), NULL, opt);
    compare_almost_equal(res.fitted, res2.fitted);
}

TEST(BasicTests, Weights) {
    WeightedLowess::Options opt;
    opt.anchors = 20;

    // No effect when dealing with a straight line.
    auto res = WeightedLowess::compute(x.size(), x.data(), x.data(), NULL, opt);
    auto wres = WeightedLowess::compute(x.size(), x.data(), x.data(), x.data(), opt); // reusing 'x' as positive weights.
    compare_almost_equal(res.fitted, wres.fitted);

    // Weighting has a frequency interpretation.
    std::vector<double> fweights(x.size());
    std::vector<double> expanded_x, expanded_y;
    for (size_t i = 0; i < x.size(); ++i) {
        fweights[i] = std::max(1.0, std::round(x[i] * 5));
        expanded_x.insert(expanded_x.end(), fweights[i], x[i]);
        expanded_y.insert(expanded_y.end(), fweights[i], y[i]);
    }
    auto eres = WeightedLowess::compute(expanded_x.size(), expanded_x.data(), expanded_y.data(), opt);

    std::vector<double> ref;
    wres = WeightedLowess::compute(x.size(), x.data(), y.data(), fweights.data(), opt); 
    for (size_t i =0; i < x.size(); ++i) { 
        ref.insert(ref.end(), fweights[i], wres.fitted[i]);
    }
    compare_almost_equal(ref, eres.fitted);
}

TEST(BasicTests, Robustness) {
    WeightedLowess::Options opt;

    // Spiking in a crazy thing.
    auto x1 = x;
    auto y1 = y;
    x1.push_back(0.5);
    y1.push_back(100);

    auto wres = WeightedLowess::compute(x1.size(), x1.data(), y1.data(), NULL, opt);
    EXPECT_EQ(wres.robust_weights[x1.size()-1], 0);

    // Comparing to a straight line.
    auto x2 = x;
    x2.push_back(10);
    auto res = WeightedLowess::compute(x.size(), x.data(), x.data(), NULL, opt);
    wres = WeightedLowess::compute(x1.size(), x1.data(), x2.data(), NULL, opt);

    std::vector<double> rest(wres.fitted.begin(), wres.fitted.begin() + x.size());
    compare_almost_equal(res.fitted, rest);
}
