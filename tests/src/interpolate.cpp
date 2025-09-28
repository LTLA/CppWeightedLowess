#include <gtest/gtest.h>
#include "WeightedLowess/interpolate.hpp"
#include "WeightedLowess/compute.hpp"
#include "utils.h"

TEST(Interpolate, Assignment) {
    std::vector<double> x{ 0.1, 0.2, 0.3, 0.4, 0.5 };
    const auto windows = WeightedLowess::define_windows(x.size(), x.data(), WeightedLowess::Options<double>());

    {
        std::vector<double> x2{ 0.15, 0.33, 0.37, 0.44 };
        const auto assigned = WeightedLowess::assign_to_segments(x.data(), windows, x2.size(), x2.data());
        std::vector<std::size_t> expected{ 0, 1, 1, 3, 4 };
        EXPECT_EQ(assigned.boundaries, expected);
        const auto bounds = WeightedLowess::get_interpolation_boundaries(assigned);
        EXPECT_EQ(bounds.first, 0);
        EXPECT_EQ(bounds.second, 4);
    }

    // Works with points that are tied with the anchors.
    {
        std::vector<double> x2{ 0.05, 0.08, 0.1, 0.22, 0.25, 0.28, 0.4, 0.45, 0.5, 0.6 };
        const auto assigned = WeightedLowess::assign_to_segments(x.data(), windows, x2.size(), x2.data());
        std::vector<std::size_t> expected{ 2, 3, 6, 6, 9 };
        EXPECT_EQ(assigned.boundaries, expected);
        const auto bounds = WeightedLowess::get_interpolation_boundaries(assigned);
        EXPECT_EQ(bounds.first, 2);
        EXPECT_EQ(bounds.second, 9);
    }
}

class InterpolateTest : public ::testing::TestWithParam<int> {};

TEST_P(InterpolateTest, Consistency) {
    const int N = 1000;
    auto simulated = simulate(N);
    const auto& x = simulated.first;
    WeightedLowess::Options opt;
    opt.anchors = GetParam();

    const auto win = WeightedLowess::define_windows(x.size(), x.data(), opt);
    std::vector<double> fitted(N);
    WeightedLowess::compute(x.size(), x.data(), win, simulated.second.data(), fitted.data(), static_cast<double*>(NULL), opt);

    std::vector<double> interpolated(N);
    std::vector<double> subx;
    std::vector<double> subfit, subfitp;
    constexpr int div = 7;
    for (int d = 0; d < div; ++d) {
        subx.clear();
        for (int n = d; n < N; n += div) {
            subx.push_back(x[n]);
        }

        subfit.resize(subx.size());
        auto bounds = WeightedLowess::interpolate(x.data(), win, fitted.data(), subx.size(), subx.data(), subfit.data(), 1);
        EXPECT_EQ(bounds.first, 0);
        EXPECT_EQ(bounds.second, subx.size());
        auto sfIt = subfit.begin();
        for (int n = d; n < N; n += div) {
            interpolated[n] = *sfIt;
            ++sfIt;
        }

        subfitp.resize(subx.size());
        WeightedLowess::interpolate(x.data(), win, fitted.data(), subx.size(), subx.data(), subfitp.data(), 3);
        EXPECT_EQ(subfit, subfitp);
    }

    compare_almost_equal(interpolated, fitted);
}

INSTANTIATE_TEST_SUITE_P(
    Interpolate,
    InterpolateTest,
    ::testing::Values(10, 50, 200, 1000)
);

TEST(Interpolate, OutOfBounds) {
    const int N = 1000;
    auto simulated = simulate(N);
    const auto& x = simulated.first;
    WeightedLowess::Options opt;

    const auto win = WeightedLowess::define_windows(x.size(), x.data(), opt);
    std::vector<double> fitted(N);
    WeightedLowess::compute(x.size(), x.data(), win, simulated.second.data(), fitted.data(), static_cast<double*>(NULL), opt);

    std::vector<double> newx(N + 2);
    std::copy(x.begin(), x.end(), newx.begin() + 1);
    newx.front() = x.front() - 1;
    newx.back() = x.back() + 1;

    constexpr double dummy = std::numeric_limits<double>::lowest();
    std::vector<double> interpolated(newx.size(), dummy);
    const auto bounds = WeightedLowess::interpolate(x.data(), win, fitted.data(), newx.size(), newx.data(), interpolated.data(), 1);
    EXPECT_EQ(bounds.first, 1);
    EXPECT_EQ(bounds.second, newx.size() - 1);

    EXPECT_EQ(interpolated.front(), dummy);
    EXPECT_EQ(interpolated.back(), dummy);
    interpolated.pop_back();
    interpolated.erase(interpolated.begin());
    compare_almost_equal(interpolated, fitted);
}

TEST(Interpolate, Ties) {
    constexpr int N = 100;
    auto simulated = simulate(N);

    // Duplicating each element to get some coverage on the averaging.
    // Duplicates are shifted up and down to introduce some differences.
    std::vector<double> x2, y2;
    for (int i = 0; i < N; ++i) {
        x2.insert(x2.end(), 2, simulated.first[i]);
        y2.push_back(simulated.second[i] - 0.01);
        y2.push_back(simulated.second[i] + 0.01);
    }

    WeightedLowess::Options opt;
    opt.anchors = x2.size();
    auto res = WeightedLowess::compute(x2.size(), x2.data(), y2.data(), opt);

    const auto win = WeightedLowess::define_windows(x2.size(), x2.data(), opt);
    std::vector<double> fitted(x2.size());
    WeightedLowess::compute(x2.size(), x2.data(), win, y2.data(), fitted.data(), static_cast<double*>(NULL), opt);

    std::vector<double> interpolated(x2.size());
    WeightedLowess::interpolate(x2.data(), win, fitted.data(), x2.size(), x2.data(), interpolated.data(), 1);
    compare_almost_equal(interpolated, fitted);
}
