#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

std::vector<int> order(const std::vector<double>& x) {
    std::vector<int> perm(x.size());
    std::iota(perm.begin(), perm.end(), 0);
    std::sort(perm.begin(), perm.end(), [&](int left, int right) -> bool { return x[left] < x[right]; });
    return perm;
}

std::vector<double> permute(const std::vector<double>& vals, const std::vector<int>& index) {
    auto copy = vals;
    auto it = copy.begin();
    for (auto p : index) {
        *it = vals[p];
        ++it;
    }
    return copy;
}

TEST(RunnerTest, Presorted) {
    auto perm = order(x);
    auto x1 = permute(x, perm);
    auto y1 = permute(y, perm);

    WeightedLowess::WeightedLowess wl;
    auto res = wl.run(x.size(), x.data(), y.data());
    auto res1 = wl.set_sorted(true).run(x.size(), x1.data(), y1.data());
    EXPECT_EQ(permute(res.fitted, perm), res1.fitted);

    // Weighting is handled correctly by the sorting mechanism.
    auto w = x;
    for (auto& i : w) {
        i *= i; // square weights, for a bit of variety.
    }
    res = wl.set_sorted(false).run(x.size(), x.data(), y.data(), w.data());

    auto w1 = permute(w, perm);
    res1 = wl.set_sorted(true).run(x.size(), x1.data(), y1.data(), w1.data());
    EXPECT_EQ(permute(res.fitted, perm), res1.fitted);
}

TEST(RunnerTest, InPlace) {
    WeightedLowess::WeightedLowess wl;

    auto x1 = x;
    auto y1 = y;

    // Permutes elements in place.
    auto res = wl.run(x.size(), x.data(), y.data());
    auto res1 = wl.run_in_place(x.size(), x1.data(), y1.data());
    EXPECT_EQ(res.fitted, res1.fitted);
    EXPECT_NE(x, x1);

    // Unless they're already sorted, in which case there's no need to modify the values.
    auto perm = order(x);
    x1 = permute(x, perm);
    y1 = permute(y, perm);
    auto x1copy = x1;

    auto res2 = wl.set_sorted(true).run_in_place(x.size(), x1.data(), y1.data());
    EXPECT_EQ(permute(res.fitted, perm), res2.fitted);
    EXPECT_EQ(x1copy, x1);
}
