#include <gtest/gtest.h>
#include "parallel.h"
#include "WeightedLowess/window.hpp"
#include "utils.h"

TEST(WindowTest, DeriveDelta) {
    std::vector<double> pts { 1, 2.5, 5, 6.2, 9, 10 };

    std::vector<double> cumulative_gaps { 
       (10 - 1), // Only anchor is at 1.
       (6.2 - 1 + 10 - 9), // add an anchor at 9.
       (2.5 - 1 + 6.2 - 5 + 10 - 9), // add an anchor at 5.
       (6.2 - 5 + 10 -9 )  // add an anchor at 2.5.
   };

    EXPECT_FLOAT_EQ(WeightedLowess::internal::derive_delta(1, pts.size(), pts.data()), cumulative_gaps[0]);

    {
        std::vector<double> choices { cumulative_gaps[0] / 3, cumulative_gaps[1] / 2, cumulative_gaps[2] };
        EXPECT_FLOAT_EQ(WeightedLowess::internal::derive_delta(3, pts.size(), pts.data()), *std::min_element(choices.begin(), choices.end()));
    }

    {
        std::vector<double> choices { cumulative_gaps[0] / 2, cumulative_gaps[1] };
        EXPECT_FLOAT_EQ(WeightedLowess::internal::derive_delta(2, pts.size(), pts.data()), *std::min_element(choices.begin(), choices.end()));
    }

    {
        std::vector<double> choices { cumulative_gaps[0] / 4, cumulative_gaps[1] / 3, cumulative_gaps[2] / 2, cumulative_gaps[3] };
        EXPECT_FLOAT_EQ(WeightedLowess::internal::derive_delta(4, pts.size(), pts.data()), *std::min_element(choices.begin(), choices.end()));
    }
}

TEST(WindowTest, FindAnchors) {
    std::vector<double> pts { 1, 2.5, 5, 6.2, 9, 10 };

    {
        std::vector<size_t> anchors;
        WeightedLowess::internal::find_anchors(pts.size(), pts.data(), 1.3, anchors);
        std::vector<size_t> expected{ 0, 1, 2, 4, 5 };
        EXPECT_EQ(anchors, expected);
    }

    {
        std::vector<size_t> anchors;
        WeightedLowess::internal::find_anchors(pts.size(), pts.data(), 2.0, anchors);
        std::vector<size_t> expected{ 0, 2, 4, 5 };
        EXPECT_EQ(anchors, expected);
    }

    {
        std::vector<size_t> anchors;
        WeightedLowess::internal::find_anchors(pts.size(), pts.data(), 4.0, anchors);
        std::vector<size_t> expected{ 0, 3, 5 };
        EXPECT_EQ(anchors, expected);
    }

    {
        std::vector<size_t> anchors;
        WeightedLowess::internal::find_anchors(pts.size(), pts.data(), 0.0, anchors);
        std::vector<size_t> expected{ 0, 1, 2, 3, 4, 5 };
        EXPECT_EQ(anchors, expected);
    }
}

TEST(WindowTest, FindLimitsBasic) {
    std::vector<double> pts { 1, 2.5, 5, 6.2, 9, 10 };
    auto limiters = WeightedLowess::internal::find_limits({ 0, 1, 2, 3, 4, 5 }, 4.0, pts.size(), pts.data(), static_cast<double*>(NULL), 0.0);

    EXPECT_EQ(limiters[0].left, 0);
    EXPECT_EQ(limiters[0].right, 3);
    EXPECT_FLOAT_EQ(limiters[0].distance, 5.2);

    EXPECT_EQ(limiters[1].left, 0);
    EXPECT_EQ(limiters[1].right, 3);
    EXPECT_FLOAT_EQ(limiters[1].distance, 3.7);

    EXPECT_EQ(limiters[2].left, 0);
    EXPECT_EQ(limiters[2].right, 4); // 9 and 1 are equi-distant!
    EXPECT_FLOAT_EQ(limiters[2].distance, 4);

    EXPECT_EQ(limiters[3].left, 1);
    EXPECT_EQ(limiters[3].right, 4);
    EXPECT_FLOAT_EQ(limiters[3].distance, 3.7);

    EXPECT_EQ(limiters[4].left, 2);
    EXPECT_EQ(limiters[4].right, 5); 
    EXPECT_FLOAT_EQ(limiters[4].distance, 4);

    EXPECT_EQ(limiters[5].left, 2);
    EXPECT_EQ(limiters[5].right, 5); 
    EXPECT_FLOAT_EQ(limiters[5].distance, 5);
}

TEST(WindowTest, FindLimitsWeights) {
    std::vector<double> pts { 1, 2.5, 5, 6.2, 9, 10 };
    std::vector<double> weights { 1, 2, 1, 2, 1, 2 };
    auto limiters = WeightedLowess::internal::find_limits({ 0, 1, 2, 3, 4, 5 }, 4.0, pts.size(), pts.data(), weights.data(), 0.0);

    EXPECT_EQ(limiters[0].left, 0);
    EXPECT_EQ(limiters[0].right, 2);
    EXPECT_FLOAT_EQ(limiters[0].distance, 4);

    EXPECT_EQ(limiters[1].left, 0);
    EXPECT_EQ(limiters[1].right, 2);
    EXPECT_FLOAT_EQ(limiters[1].distance, 2.5);

    EXPECT_EQ(limiters[2].left, 1);
    EXPECT_EQ(limiters[2].right, 3); 
    EXPECT_FLOAT_EQ(limiters[2].distance, 2.5);

    EXPECT_EQ(limiters[3].left, 2);
    EXPECT_EQ(limiters[3].right, 4);
    EXPECT_FLOAT_EQ(limiters[3].distance, 2.8);

    EXPECT_EQ(limiters[4].left, 3);
    EXPECT_EQ(limiters[4].right, 5); 
    EXPECT_FLOAT_EQ(limiters[4].distance, 2.8);

    EXPECT_EQ(limiters[5].left, 3);
    EXPECT_EQ(limiters[5].right, 5); 
    EXPECT_FLOAT_EQ(limiters[5].distance, 3.8);
}

TEST(WindowTest, FindLimitsTies) {
    std::vector<double> pts { 1, 2.5, 2.5, 4, 4, 5, 6, 6, 6.2, 6.2, 9 };
    auto limiters = WeightedLowess::internal::find_limits({ 0, 5, 10 }, 2.0, pts.size(), pts.data(), static_cast<double*>(NULL), 0.0);

    EXPECT_EQ(limiters[0].left, 0);
    EXPECT_EQ(limiters[0].right, 2); 
    EXPECT_FLOAT_EQ(limiters[0].distance, 1.5);

    EXPECT_EQ(limiters[1].left, 3);
    EXPECT_EQ(limiters[1].right, 7); // as distances of 5->4 and 5->6 are tied.
    EXPECT_FLOAT_EQ(limiters[1].distance, 1);

    EXPECT_EQ(limiters[2].left, 8);
    EXPECT_EQ(limiters[2].right, 10); 
    EXPECT_FLOAT_EQ(limiters[2].distance, 2.8);
}

TEST(WindowTest, FindLimitsMoreTies) {
    std::vector<double> pts { 1, 2.5, 2.5, 4, 4, 5, 6, 6, 6.2, 9 };

    // Checking correct handling of tied left/right distances;
    // we ask for a larger span around '5', which forces the
    // algorithm to correctly consider all 4's and all 6's before
    // jumping to the next element (i.e., 6.2).
    {
        auto limiters = WeightedLowess::internal::find_limits({ 5 }, 6.0, pts.size(), pts.data(), static_cast<double*>(NULL), 0.0);
        EXPECT_EQ(limiters[0].left, 3);
        EXPECT_EQ(limiters[0].right, 8); 
        EXPECT_FLOAT_EQ(limiters[0].distance, 1.2);
    }

    // Does the right thing with weights.
    {
        std::vector<double> weights(pts.size(), 10);
        auto limiters = WeightedLowess::internal::find_limits({ 5 }, 60.0, pts.size(), pts.data(), weights.data(), 0.0);
        EXPECT_EQ(limiters[0].left, 3);
        EXPECT_EQ(limiters[0].right, 8); 
        EXPECT_FLOAT_EQ(limiters[0].distance, 1.2);
    }

    // Checking that it interacts properly with the tie handling at the window edges.
    {
        auto limiters = WeightedLowess::internal::find_limits({ 5 }, 7.0, pts.size(), pts.data(), static_cast<double*>(NULL), 0.0);
        EXPECT_EQ(limiters[0].left, 1);
        EXPECT_EQ(limiters[0].right, 8); 
        EXPECT_FLOAT_EQ(limiters[0].distance, 2.5);
    }
}

TEST(WindowTest, MinimumWidth) {
    std::vector<double> pts { 1, 2.5, 5, 6.2, 9, 10 };
    auto limiters = WeightedLowess::internal::find_limits({ 0, 1, 2, 3, 4 }, 2.0, pts.size(), pts.data(), static_cast<double*>(NULL), 5.0);

    EXPECT_EQ(limiters[0].left, 0);
    EXPECT_EQ(limiters[0].right, 1); 
    EXPECT_FLOAT_EQ(limiters[0].distance, 1.5);

    EXPECT_EQ(limiters[1].left, 0);
    EXPECT_EQ(limiters[1].right, 2); 
    EXPECT_FLOAT_EQ(limiters[1].distance, 2.5);

    EXPECT_EQ(limiters[2].left, 1);
    EXPECT_EQ(limiters[2].right, 3); 
    EXPECT_FLOAT_EQ(limiters[2].distance, 2.5);

    EXPECT_EQ(limiters[3].left, 2);
    EXPECT_EQ(limiters[3].right, 3); // it doesn't extend all the way to 2.8, as that's beyond the minimum width requirement.
    EXPECT_FLOAT_EQ(limiters[3].distance, 1.2);

    EXPECT_EQ(limiters[4].left, 4);
    EXPECT_EQ(limiters[4].right, 5); // ditto
    EXPECT_FLOAT_EQ(limiters[4].distance, 1);
}

TEST(WindowTest, Parallelized) {
    auto sim = simulate(1000);
    const auto& pts = sim.first;

    std::vector<size_t> anchors(1000);
    std::iota(anchors.begin(), anchors.end(), 0);

    auto limiters = WeightedLowess::internal::find_limits(anchors, 0.5, pts.size(), pts.data(), static_cast<double*>(NULL), 0.0);
    auto plimiters = WeightedLowess::internal::find_limits(anchors, 0.5, pts.size(), pts.data(), static_cast<double*>(NULL), 0.0, 3);

    for (size_t i = 0; i < anchors.size(); ++i) {
        const auto& val = limiters[i];
        const auto& pval = plimiters[i];
        EXPECT_EQ(val.left, pval.left);
        EXPECT_EQ(val.right, pval.right);
        EXPECT_EQ(val.distance, pval.distance);
    }
}
