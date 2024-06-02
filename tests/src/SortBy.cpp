#include <gtest/gtest.h>
#include "WeightedLowess/SortBy.hpp"
#include "utils.h"

TEST(SortBy, Basic) {
    auto sim = simulate(800, /* sorted = */ false);
    const auto& x1 = sim.first;

    auto ref = sim.first; 
    std::sort(ref.begin(), ref.end());

    WeightedLowess::SortBy sorter(x1.size(), x1.data());
    std::vector<uint8_t> work;

    auto test = sim.first;
    sorter.permute(test.data(), work);
    EXPECT_EQ(test, ref);
    sorter.unpermute(test.data(), work);
    EXPECT_EQ(test, sim.first);

    auto y1 = sim.second;
    sorter.permute({ test.data(), y1.data() }, work);
    EXPECT_EQ(test, ref);
    sorter.unpermute({ test.data(), y1.data() }, work);
    EXPECT_EQ(test, sim.first);
    EXPECT_EQ(y1, sim.second);
}

TEST(SortBy, AlreadySorted) {
    auto sim = simulate(800, /* sorted = */ true);
    auto x1 = sim.first;
    EXPECT_TRUE(std::is_sorted(x1.begin(), x1.end()));

    WeightedLowess::SortBy sorter(x1.size(), x1.data());
    std::vector<uint8_t> work;

    auto test = sim.first;
    sorter.permute(test.data(), work);
    EXPECT_EQ(test, sim.first);
    sorter.unpermute(test.data(), work);
    EXPECT_EQ(test, sim.first);
}
