#include <gtest/gtest.h>

#include "WeightedLowess/WeightedLowess.hpp"

std::vector<double> x = { 
   0.506, 0.570, 0.043, 0.304, 0.343, 0.518, 0.180, 0.964, 0.491,
   0.755, 0.587, 0.590, 0.641, 0.921, 0.707, 0.420, 0.268, 0.164,
   0.654, 0.293, 0.898, 0.752, 0.614, 0.194, 0.677, 0.683, 0.257,
   0.667, 0.946, 0.678, 0.414, 0.472, 0.691, 0.511, 0.816, 0.365,
   0.748, 0.552, 0.990, 0.116, 0.733, 0.970, 0.765, 0.826, 0.741,  
   0.341, 0.447, 0.927, 0.313, 0.409    
};

std::vector<double> y = {
     0.214,  0.661, -0.564,  0.545,  0.964,  0.068, -1.706,  0.234,  0.940,
     2.209, -0.509, -0.988,  1.414,  0.074,  0.476,  0.529,  1.185,  0.465,
     0.347, -0.611, -2.884, -0.134,  0.097, -0.020, -0.620, -2.652,  0.955, 
    -0.929,  0.309, -0.775, -0.056,  0.616, -0.072, -1.110,  0.301,  0.010,
     1.191,  0.423, -0.426, -1.317, -0.448,  1.846,  2.264,  0.212, -1.076,
     0.206,  0.232,  0.426,  1.113,  0.785
};

void compare_almost_equal(const std::vector<double>& first, const std::vector<double>& second) {
    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        EXPECT_FLOAT_EQ(first[i], second[i]);
    }
    return;
}

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
    auto res2 = wl.set_points(10).run(x.size(), x.data(), alt.data());
    compare_almost_equal(res.fitted, res2.fitted);
    compare_to_zero(res2.residuals);

    // Only two points; start and end.
    res2 = wl.set_points(2).run(x.size(), x.data(), alt.data());
    compare_almost_equal(res.fitted, res2.fitted);
    compare_to_zero(res2.residuals);

    // Interpolation has some kind of effect.
    res = wl.set_points(200).run(x.size(), x.data(), y.data());
    res2 = wl.set_points(5).run(x.size(), x.data(), y.data());
    double sumdiff = 0;
    for (size_t i = 0; i < res.fitted.size(); ++i) {
        sumdiff += std::abs(res.fitted[i] - res2.fitted[i]);
    }
    EXPECT_TRUE(sumdiff > 0.01);
}

TEST(BasicTests, Weights) {
    WeightedLowess::WeightedLowess wl;
    wl.set_points(20);

    // No effect when dealing with a straight line.
    auto res = wl.run(x.size(), x.data(), x.data());
    auto wres = wl.run(x.size(), x.data(), x.data(), x.data()); // reusing 'x' as positive weights.
    compare_almost_equal(res.fitted, wres.fitted);

    // Weights have some kind of effect for non-trivial trends.
    res = wl.run(x.size(), x.data(), y.data());
    wres = wl.run(x.size(), x.data(), y.data(), x.data()); // reusing 'x' as positive weights.
    double sumdiff = 0;
    for (size_t i = 0; i < res.fitted.size(); ++i) {
        sumdiff += std::abs(res.fitted[i] - wres.fitted[i]);
    }
    EXPECT_TRUE(sumdiff > 0.01);

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

TEST(BasicTests, Ties) {
    WeightedLowess::WeightedLowess wl;
    wl.set_points(20).set_iterations(1);

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
