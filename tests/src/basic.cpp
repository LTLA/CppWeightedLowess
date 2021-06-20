#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

void compare_to_zero(const std::vector<double>& resids) {
    for (size_t i = 0; i < resids.size(); ++i) {
        EXPECT_TRUE(resids[i] < 0.00000000001);
    }
    return;
}

TEST(BasicTests, Exact) {
    WeightedLowess::WeightedLowess wl;

    // y = x
    auto res = wl.run(x.size(), x.data(), x.data());
    compare_almost_equal(res.fitted, x);
    compare_to_zero(res.residuals);

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
    compare_to_zero(res2.residuals);

    // Only two points; start and end.
    res2 = wl.set_anchors(2).run(x.size(), x.data(), alt.data());
    compare_almost_equal(res.fitted, res2.fitted);
    compare_to_zero(res2.residuals);
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

