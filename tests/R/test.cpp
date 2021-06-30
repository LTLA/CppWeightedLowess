#include "Rcpp.h"
#include "WeightedLowess/WeightedLowess.hpp"

// [[Rcpp::export(rng=false)]]
Rcpp::List run_weighted_lowess(Rcpp::NumericVector x, Rcpp::NumericVector y, Rcpp::NumericVector w, double span, int npts, int iterations) {
    WeightedLowess::WeightedLowess smoother;

    smoother.set_span(span).set_anchors(npts).set_iterations(iterations);

    double* weights=NULL;
    if (w.size()) {
        weights = w.begin();
    }

    Rcpp::NumericVector fitted(x.size()), resids(x.size());
    smoother.run(x.size(), x.begin(), y.begin(), weights, fitted.begin(), resids.begin());

    return Rcpp::List::create(fitted, resids);
}
