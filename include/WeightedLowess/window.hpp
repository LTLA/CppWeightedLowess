#ifndef WEIGHTEDLOWESS_WINDOW_HPP
#define WEIGHTEDLOWESS_WINDOW_HPP

#include <vector>
#include <algorithm>

namespace WeightedLowess {

namespace internal {

/* 
 * Determining the `delta`. For a anchor point with x-coordinate `x`, we skip all
 * points in `[x, x + delta]` before finding the next anchor point. 
 * We try to choose a `delta` that satisfies the constraints on the number
 * of anchor points in `num_anchors`. A naive approach would be to simply divide the
 * range of `x` by `num_anchors - 1`. However, this may place anchor points inside 
 * large gaps on the x-axis intervals where there are no actual observations. 
 * 
 * Instead, we try to distribute the anchor points so that they don't fall
 * inside such large gaps. We do so by looking at the largest gaps and seeing
 * what happens if we were to shift the anchor points to avoid such gaps. If we
 * jump across a gap, though, we need to "use up" a anchor point to restart
 * the sequence of anchor points on the other side of the gap. This requires some
 * iteration to find the compromise that minimizes the 'delta' (and thus the 
 * degree of approximation in the final lowess calculation).
 */
template<typename Data_>
Data_ derive_delta(size_t num_anchors, size_t num_points, const Data_* x) {
    size_t points_m1 = num_points - 1;
    std::vector<Data_> diffs(points_m1);
    for (size_t i = 0; i < points_m1; ++i) {
        diffs[i] = x[i + 1] - x[i];
    }

    std::sort(diffs.begin(), diffs.end());
    for (size_t i = 1; i < points_m1; ++i) {
        diffs[i] += diffs[i-1];            
    }

    Data_ lowest_delta = diffs.back();
    if (num_anchors > 1) {
        size_t max_skips = std::min(points_m1, num_anchors - 1);
        for (size_t nskips = 0; nskips < max_skips; ++nskips) {
            Data_ candidate_delta = diffs[points_m1 - nskips - 1] / (num_anchors - nskips);
            lowest_delta = std::min(candidate_delta, lowest_delta);
        }
    }

    return lowest_delta;
}

/* 
 * Finding the anchor points, given the deltas. As previously mentioned, for a
 * anchor point with x-coordinate `x`, we skip all points in `[x, x + delta]`
 * before finding the next anchor point. 
 *
 * We start at the first point (so it is always an anchor) and we do this
 * skipping up to but not including the last point; the last point itself is
 * always included as an anchor to ensure we have exactness at the ends.
 */
template<typename Data_>
void find_anchors(size_t num_points, const Data_* x, Data_ delta, std::vector<size_t>& anchors) {
    anchors.clear();
    anchors.push_back(0);

    size_t last_pt = 0;
    size_t points_m1 = num_points - 1;
    for (size_t pt = 1; pt < points_m1; ++pt) {
        if (x[pt] - x[last_pt] > delta) {
            anchors.push_back(pt);
            last_pt = pt;
        }
    }

    anchors.push_back(points_m1);
    return;
}

template<typename Data_>
struct Window {
    size_t left, right;
    Data_ distance;
};

/* This function identifies the start and end index in the span for each chosen sampling
 * point. It returns two arrays via reference containing said indices. It also returns
 * an array containing the maximum distance between points at each span.
 *
 * We don't use the update-based algorithm in Cleveland's paper, as it ceases to be
 * numerically stable once you throw in floating-point weights. It's not particularly
 * amenable to updating through cycles of addition and subtraction. At any rate, the
 * algorithm as a whole remains quadratic (as weights must be recomputed) so there's no
 * damage to scalability.
 */
template<typename Data_>
std::vector<Window<Data_> > find_limits(
    const std::vector<size_t>& anchors, 
    Data_ spanweight,
    size_t num_points,
    const Data_* x, 
    const Data_* weights,
    Data_ min_width,
    [[maybe_unused]] int nthreads = 1)
{
    const size_t nanchors = anchors.size();
    std::vector<Window<Data_> > limits(nanchors);
    auto half_min_width = min_width / 2;
    auto points_m1 = num_points - 1;

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
#ifdef _OPENMP
    #pragma omp parallel for num_threads(nthreads)
#endif
    for (size_t s = 0; s < nanchors; ++s) {
#else
    WEIGHTEDLOWESS_CUSTOM_PARALLEL(nanchors, nthreads, [&](size_t, size_t start, size_t length) {
    for (size_t s = start, end = start + length; s < end; ++s) {
#endif

        auto curpt = anchors[s], left = curpt, right = curpt;
        auto curx = x[curpt];
        Data_ curw = (weights == NULL ? 1 : weights[curpt]);

        // First expanding in both directions, choosing the one that
        // minimizes the increase in the window size.
        if (curpt > 0 && curpt < points_m1) {
            auto next_ldist = curx - x[left - 1];
            auto next_rdist = x[right + 1] - curx;

            while (curw < spanweight) {
                if (next_ldist < next_rdist) {
                    --left;
                    curw += (weights == NULL ? 1 : weights[left]);
                    if (left == 0) {
                        break;
                    }
                    next_ldist = curx - x[left - 1];

                } else if (next_ldist > next_rdist) {
                    ++right;
                    curw += (weights == NULL ? 1 : weights[right]);
                    if (right == points_m1) {
                        break;
                    }
                    next_rdist = x[right + 1] - curx;

                } else {
                    // In the very rare case that distances are equal, we do a
                    // simultaneous jump to ensure that both points are
                    // included.  Otherwise one of them is skipped if we break.
                    --left;
                    ++right;
                    curw += (weights == NULL ? 2 : weights[left] + weights[right]);
                    if (left == 0 || right == points_m1) {
                        break;
                    }
                    next_ldist = curx - x[left - 1];
                    next_rdist = x[right + 1] - curx;
                }
            }
        }

        // If we still need it, we expand in only one direction.
        while (left > 0 && curw < spanweight) {
            --left;
            curw += (weights == NULL ? 1 : weights[left]);
        }
 
        while (right < points_m1 && curw < spanweight) {
            ++right;
            curw += (weights == NULL ? 1 : weights[right]);
        }

        /* Once we've found the span, we stretch it out to include all ties. */
        while (left > 0 && x[left] == x[left - 1]) {
            --left; 
        }

        while (right < points_m1 && x[right] == x[right + 1]) { 
            ++right; 
        }

        /* Forcibly extending the span if it fails the min width.  We use
         * the existing 'left' and 'right' to truncate the search space.
         */
        auto mdist = std::max(curx - x[left], x[right] - curx);
        if (mdist < half_min_width) {
            left = std::lower_bound(x, x + left, curx - half_min_width) - x; 

            /* 'right' still refers to a point inside the window, and we
             * already know that the window is too small, so we shift it
             * forward by one to start searching outside. However,
             * upper_bound gives us the first element that is _outside_ the
             * window, so we need to subtract one to get to the last
             * element _inside_ the window.
             */
            right = std::upper_bound(x + right + 1, x + num_points, curx + half_min_width) - x;
            --right;

            mdist = std::max(curx - x[left], x[right] - curx);
        }

        limits[s].left = left;
        limits[s].right = right;
        limits[s].distance = mdist;

#ifndef WEIGHTEDLOWESS_CUSTOM_PARALLEL
    }
#else
    }
    });
#endif

    return limits;
}

}

}

#endif
