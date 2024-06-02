#include "Rcpp.h"
#include "WeightedLowess/WeightedLowess.hpp"

// [[Rcpp::export(rng=false)]]
Rcpp::List run_weighted_lowess(Rcpp::NumericVector x, Rcpp::NumericVector y, Rcpp::NumericVector w, double span, int npts, int iterations) {
    WeightedLowess::Options opt;
    opt.span = span;
    opt.anchors = npts;
    opt.iterations = iterations;

    double* weights=NULL;
    if (w.size()) {
        opt.weights = w.begin();
    }

    Rcpp::NumericVector fitted(x.size()), resids(x.size());
    WeightedLowess::compute<double>(x.size(), x.begin(), y.begin(), fitted.begin(), resids.begin(), opt);

    return Rcpp::List::create(fitted, resids);
}
