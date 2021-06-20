# Weighted LOWESS for C++

## Overview

A header-only C++ library for weighted LOWESS calculations, directly ported from the `weightedLowess()` function in the [**limma**](https://bioconductor.org/packages/limma/) package.
This implements the method described by Cleveland (1979, 1981):

1. Define a set of anchor points, more-or-less evenly spaced throughout the range of x-values.
2. Perform a local linear regression around each anchor, using only points within a window centered at each anchor.
In this regression, points inside the windoware weighted based on their distances from the anchor on the x-axis.
Each anchor's window must be wide enough to contain a given proportion of all points.
4. Interpolate to obtain smoothed values for all points in between two anchors.
5. Weight each point based on their residual, i.e., difference from the fitted value.
Points with very large residuals are assigned zero weight.
6. Repeat steps 1-5 using the computed weights in each anchor's local linear regression.
This is iterated several times to eliminate the effect of outliers on the fit.

In here and `limma::weightedLowess`, we implement some (optional) modifications from the original FORTRAN code:

- We allow weights to be interpreted as frequencies, in which case they are used in determining the width of the smoothing window around each point.
This is in addition to their usual role in the local linear regression.
- The `delta` value can be automatically determined from a pre-specified number of anchor points.
This provides a convenient way of controlling the approximation fidelity.

In this library, we also implement some modifications from `limma::weightedLowess`:

- The number of robustness iterations in this library refers to additional iterations beyond the first fit,
while the number of robustness iterations in `limma:;weightedLowess` includes the first fit.
- We omit the early termination condition when the MAD of the residuals is much lower than the mean.
This avoids inappropriate termination of the robustness iterations in pathological scenarios.

See the [`?weightedLowess` documentation](https://rdrr.io/bioc/limma/man/weightedLowess.html) for more details.

## Usage

Using this library is as simple as including the header file in your source code:

```cpp
#include "WeightedLowess/WeightedLowess.hpp"

WeightedLowess::WeightedLowess smoother;

// Optional: set parameters.
smoother.set_span(0.5).set_anchors(100);

// Run on some input values.
auto results = smoother.run(num_points, x, y);
results.fitted;
```

See the [reference documentation](https://ltla.github.io/CppWeightedLowess) for more details.

If you're already using CMake, then you just need to add something like this to your `CMakeLists.txt`:

```
include(FetchContent)

FetchContent_Declare(
  WeightedLowess 
  GIT_REPOSITORY https://github.com/LTLA/CppWeightedLowess
  GIT_TAG master # or any version of interest
)

FetchContent_MakeAvailable(tatami)
```

And then:

```
# For executables:
target_link_libraries(myexe WeightedLowess)

# For libaries
target_link_libraries(mylib INTERFACE WeightedLowess)
```

Otherwise, you can just copy the header file into some location that is visible to your compiler.

## References 

Cleveland, W.S. (1979).
Robust locally weighted regression and smoothing scatterplots. 
_Journal of the American Statistical Association_ 74(368), 829-836.

Cleveland, W.S. (1981). 
LOWESS: A program for smoothing scatterplots by robust locally weighted regression. 
_The American Statistician_ 35(1), 54.

