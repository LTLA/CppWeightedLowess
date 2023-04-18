# Building the function.
library(Rcpp)
if (!file.exists("WeightedLowess")) {
    file.symlink("../../include/WeightedLowess", "WeightedLowess")
}
sourceCpp("test.cpp")

# Running it against the reference.
set.seed(10)
x <- runif(100)
y <- rnorm(100)

library(limma)
library(testthat)

test_that("default", {
    output <- weightedLowess(x, y)
    values <- run_weighted_lowess(x, y, w=numeric(0), span=0.3, npts=200, iterations=3)
    expect_equal(output$fitted, values[[1]])
    expect_true(output$delta == 0)

    # Checking weights are correctly computed. Note that limma::weightedLowess
    # recomputes the weights _after_ the last iteration, while
    # WeightedLowess::WeightedLowess reports the weights used _during_ the last
    # iteration. Thus, we need to add an extra iteration to get the same
    # matching weights.
    output <- weightedLowess(x, y, iterations=3)
    expect_equal(output$weights, values[[2]])

    # Ramping up the number of points to force delta calculations.
    x2 <- runif(10000)
    y2 <- runif(10000)
    output2 <- weightedLowess(x2, y2)
    expect_true(output2$delta > 0)
    values2 <- run_weighted_lowess(x2, y2, w=numeric(0), span=0.3, npts=200, iterations=3)
    expect_equal(output2$fitted, values2[[1]])
})

test_that("approximations", {
    output <- weightedLowess(x, y, npts=10)
    values <- run_weighted_lowess(x, y, w=numeric(0), span=0.3, npts=10, iterations=3)
    expect_equal(output$fitted, values[[1]])
})

test_that("odd number of points", {
    output <- weightedLowess(x[-1], y[-1])
    values <- run_weighted_lowess(x[-1], y[-1], w=numeric(0), span=0.3, npts=200, iterations=3)
    expect_equal(output$fitted, values[[1]])
})

test_that("weighting", {
    w <- runif(100)
    output <- weightedLowess(x, y, w)
    values <- run_weighted_lowess(x, y, w=w, span=0.3, npts=200, iterations=3)
    expect_equal(output$fitted, values[[1]])
})
