#ifndef SCRAN_WEIGHTED_LOWESS_H 
#define SCRAN_WEIGHTED_LOWESS_H 

#include <algorithm>
#include <vector>
#include <cstdint>

#define THRESHOLD 0.0000001

namespace scran {

class WeightedLowess {

private:
    double span = 0.3;
    int npts = 200;
    int iterations = 4;
    double delta = -1;

private:
    /* Sorts the observations by the means, applies the same permutation to the
     * variances. This makes downstream processing quite a lot easier.
     */
    void process_inputs(size_t n, const double* x, const double* y) {
        xbuffer = std::vector<double>(x, x + n);
        ybuffer = std::vector<double>(y, y + n);

        permutation.resize(n);
        std::iota(permutation.begin(), permutation.end(), 0);
        std::sort(permutation.begin(), permutation.end(), [&](size_t left, size_t right) -> bool {
            return xbuffer[left] < xbuffer[right];
        });

        // Reordering values in place.
        std::vector<uint8_t> used(n);
        for (size_t i = 0; i < permutation.size(); ++i) {
            if (used[i]) {
                continue;
            }
            used[i] = 1;

            size_t current = i, replacement = indices[i];
            while (replacement != i) {
                std::swap(xbuffer[current], xbuffer[replacement]);
                std::swap(ybuffer[current], ybuffer[replacement]);

                current = replacement;
                used[replacement] = 1;
                replacement = indices[replacement]; 
            } 
        }
        return;
    }

    std::vector<size_t> permutation;
    std::vector<double> xbuffer, ybuffer;

private:
    /* Determining the `delta`. For a seed point with x-coordinate `x`, we skip all
     * points in `[x, x + delta]` before finding the next seed point. 
     * We try to choose a `delta` that satisfies the constraints on the number
     * of seed points in `npts`. A naive approach would be to simply divide the
     * range of `x` by `npts - 1`. However, this may place seed points inside 
     * large gaps on the x-axis intervals where there are no actual observations. 
     * 
     * Instead, we try to distribute the seed points so that they don't fall
     * inside such large gaps. We do so by looking at the largest gaps and seeing
     * what happens if we were to shift the seed points to avoid such gaps. If we
     * jump across a gap, though, we need to "use up" a seed point to restart
     * the sequence of seed points on the other side of the gap. This requires some
     * iteration to find the compromise that minimizes the 'delta' (and thus the 
     * degree of approximation in the final lowess calculation).
     */
    double derive_delta() const {
        if (delta < 0) {
            return delta;
        }

        std::vector<double> diffs(xbuffer.size() - 1);
        for (size_t i = 0; i < diffs.size(); ++i) {
            diffs[i] = xbuffer[i + 1] - xbuffer[i];
        }

        std::sort(diffs.begin(), diffs.end());
        for (size_t i = 1; i < diffs.size(); ++i) {
            diffs[i] += diffs[i-1];            
        }

        double lowest_delta = diffs.back();
        for (size_t nskips = 0; nskips < npts - 1 && nskips < diffs.size() - 1; ++nskips) {
            double candidate_delta = diffs[diffs.size() - nskips - 1] / (npts - nskips);
            lowest_delta = std::min(candidate_delta, lowest_delta);
        }

        return lowest_delta;
    }

private:
    /* Finding the seed points, given the deltas. As previously mentioned, for
     * a seed point with x-coordinate `x`, we skip all points in `[x, x +
     * delta]` before finding the next seed point.
     */
    std::vector<size_t> find_seeds(double d) const {
        std::vector<size_t> indices;
        indices.push_back(0);
        size_t last_pt = 0;
        for (size_t pt = 1; pt < npts - 1; ++pt) {
            if (means[pt] - means[last_pt] > d) {
                indices.push_back(pt);
                last_pt=pt;
            }
        }
        indices.push_back(npoints - 1);
        return indices;
    }

private:
    /* This function identifies the start and end index in the span for each chosen sampling
     * point. It returns two arrays via reference containing said indices. It also returns
     * an array containing the maximum distance between points at each span.
     *
     * We don't use the update-based algorithm in Cleveland's paper, as it ceases to be
     * numerically stable once you throw in double-precision weights. It's not particularly
     * amenable to updating through cycles of addition and subtraction. At any rate, the
     * algorithm as a whole remains quadratic (as weights must be recomputed) so there's no
     * damage to scalability.
     */
    std::pair<std::vector<size_t>, std::vector<size_t> > find_limits(const double* weights) const {
        std::vector<size_t> left_out, right_out;
        const size_t npts = xbuffer.size();

        for (size_t curpt = 0; curpt < npts; ++curpt) {
            auto left=curpt, right=curpt;
            double curw = weights[curpt];
            int ende=(curpt==npts-1), ends=(curpt==0);
            double mdist=0, ldist, rdist;

            while (curw < spanweight && (!ende || !ends)) {
                if (ende) {
                    /* Can only extend backwards. */
                    --left;
                    curw+=wptr[left];
                    if (left==0) { ends=1; }
                    ldist=xptr[curpt]-xptr[left];
                    if (mdist < ldist) { mdist=ldist; }
                } else if (ends) {
                    /* Can only extend forwards. */
                    ++right;
                    curw+=wptr[right];
                    if (right==npts-1) { ende=1; }
                    rdist=xptr[right]-xptr[curpt];
                    if (mdist < rdist) { mdist=rdist; }
                } else {
                    /* Can do either; extending by the one that minimizes the curpt mdist. */
                    ldist=xptr[curpt]-xptr[left-1];
                    rdist=xptr[right+1]-xptr[curpt];
                    if (ldist < rdist) {
                        --left;
                        curw+=wptr[left];
                        if (left==0) { ends=1; }
                        if (mdist < ldist) { mdist=ldist; }
                    } else {
                        ++right;
                        curw+=wptr[right];
                        if (right==npts-1) { ende=1; }
                        if (mdist < rdist) { mdist=rdist; }
                    }
                }
            }

            /* Extending to ties. */
            while (left>0 && xptr[left]==xptr[left-1]) { --left; }
            while (right<npts-1 && xptr[right]==xptr[right+1]) { ++right; }

            /* Recording */
            spbegin[curx]=left;
            spend[curx]=right;
            spdist[curx]=mdist;
        }

        (*start)=spbegin;
        (*end)=spend;
        (*dist)=spdist;
        return;
    }

/* Computes the lowess fit at a given point using linear regression with a combination of tricube,
 * prior and robustness weighting. Some additional effort is put in to avoid numerical instability
 * and undefined values when divisors are near zero.
 */

double lowess_fit (const double* xptr, const double* yptr, const double* wptr, const double* rwptr,
		const int npts, const int curpt, const int left, const int right, const double dist, double* work) {
	double ymean=0, allweight=0;
	int pt;
	if (dist < THRESHOLD) {
		for (pt=left; pt<=right; ++pt) {
			work[pt]=wptr[pt]*rwptr[pt];
			ymean+=yptr[pt]*work[pt];
			allweight+=work[pt];
		}
		ymean/=allweight;
		return ymean;
	}
	double xmean=0;
	for (pt=left; pt<=right; ++pt) {
		work[pt]=pow(1-pow(fabs(xptr[curpt]-xptr[pt])/dist, 3.0), 3.0)*wptr[pt]*rwptr[pt];
		xmean+=work[pt]*xptr[pt];
		ymean+=work[pt]*yptr[pt];
		allweight+=work[pt];
	}
	xmean/=allweight;
	ymean/=allweight;

	double var=0, covar=0, temp;
	for (pt=left; pt<=right; ++pt) {
		temp=xptr[pt]-xmean;
		var+=temp*temp*work[pt];
		covar+=temp*(yptr[pt]-ymean)*work[pt];
	}
	if (var < THRESHOLD) { return ymean; }

	const double slope=covar/var;
	const double intercept=ymean-slope*xmean;
	return slope*xptr[curpt]+intercept;
}

/* This is a C version of the local weighted regression (lowess) trend fitting algorithm,
 * based on the Fortran code in lowess.f from http://www.netlib.org/go written by Cleveland.
 * Consideration of non-equal prior weights is added to the span calculations and linear
 * regression. These weights are intended to have the equivalent effect of frequency weights
 * (at least, in the integer case; extended by analogy to all non-negative values).
 */

SEXP weighted_lowess(SEXP covariate, SEXP response, SEXP weight, SEXP span, SEXP iter, SEXP delta) {
    if (!IS_NUMERIC(covariate)) { error("covariates must be double precision"); }
    if (!IS_NUMERIC(response)) { error("responses must be double precision"); }
    if (!IS_NUMERIC(weight)) { error("weights must be double precision"); }

	const int npts=LENGTH(covariate);
	if (npts!=LENGTH(response) || npts!=LENGTH(weight)) { error("weight, covariate and response vectors have unequal lengths"); }
	if (npts<2) { error("need at least two points"); }
	const double* covptr=NUMERIC_POINTER(covariate);
	const double* resptr=NUMERIC_POINTER(response);
	const double* weiptr=NUMERIC_POINTER(weight);

	if (!IS_NUMERIC(span) || LENGTH(span)!=1) { error("span should be a double-precision scalar"); }
	const double spv=NUMERIC_VALUE(span);
	if (!IS_INTEGER(iter) || LENGTH(iter)!=1) { error("number of robustness iterations should be an integer scalar"); }
	const int niter=INTEGER_VALUE(iter);
	if (niter<=0) { error("number of robustness iterations should be positive"); }
	if (!IS_NUMERIC(delta) || LENGTH(delta)!=1) { error("delta should be a double-precision scalar"); }
	const double dv=NUMERIC_VALUE(delta);

	/*** NO MORE ERRORS AT THIS POINT, MEMORY ASSIGNMENTS ARE ACTIVE. ***/

	/* Computing the span weight that each span must achieve. */
	double totalweight=0;
	int pt;
	for (pt=0; pt<npts; ++pt) { totalweight+=weiptr[pt]; }
	double spanweight=totalweight*spv;
	const double subrange=(covptr[npts-1]-covptr[0])/npts;

	/* Setting up the indices of points for sampling; the frame start and end for those indices, and the max dist. */
	int *seed_index;
	int nseeds;
	find_seeds(&seed_index, &nseeds, covptr, npts, dv);
   	int *frame_start, *frame_end;
	double* max_dist;
	find_limits (seed_index, nseeds, covptr, weiptr, npts, spanweight, &frame_start, &frame_end, &max_dist);

	/* Setting up arrays to hold the fitted values, residuals and robustness weights. */
	SEXP output=PROTECT(NEW_LIST(2));
	SET_VECTOR_ELT(output, 0, NEW_NUMERIC(npts));
	double* fitptr=NUMERIC_POINTER(VECTOR_ELT(output, 0));
	double* rsdptr=(double*)R_alloc(npts, sizeof(double));
	SET_VECTOR_ELT(output, 1, NEW_NUMERIC(npts));
	double* robptr=NUMERIC_POINTER(VECTOR_ELT(output, 1));
	int* rorptr=(int*)R_alloc(npts, sizeof(int));
	for (pt=0; pt<npts; ++pt) { robptr[pt]=1; }

	/* Robustness iterations. */
	int it=0;
	for (it=0; it<niter; ++it) {
		int cur_seed, last_pt=0, subpt;
		double current;

		/* Computing fitted values for seed points, and interpolating to the intervening points. */
		fitptr[0]=lowess_fit(covptr, resptr, weiptr, robptr, npts, 0, frame_start[0], frame_end[0], max_dist[0], rsdptr);
		for (cur_seed=1; cur_seed<nseeds; ++cur_seed) {
			pt=seed_index[cur_seed];
			fitptr[pt]=lowess_fit(covptr, resptr, weiptr, robptr, npts, pt, frame_start[cur_seed],
				frame_end[cur_seed], max_dist[cur_seed], rsdptr); /* using rsdptr as a holding cell. */

			if (pt-last_pt > 1) {
	 			/* Some protection is provided against infinite slopes. This shouldn't be
 				 * a problem for non-zero delta; the only concern is at the final point
 				 * where the covariate distance may be zero. Besides, if delta is not
 				 * positive, pt-last_pt could never be 1 so we'd never reach this point.
 				 */
				current = covptr[pt]-covptr[last_pt];
				if (current > THRESHOLD*subrange) {
					const double slope=(fitptr[pt]-fitptr[last_pt])/current;
					const double intercept=fitptr[pt] - slope*covptr[pt];
					for (subpt=last_pt+1; subpt<pt; ++subpt) { fitptr[subpt]=slope*covptr[subpt]+intercept; }
				} else {
					const double endave=0.5*(fitptr[pt]+fitptr[last_pt]);
					for (subpt=last_pt+1; subpt<pt; ++subpt) { fitptr[subpt]=endave; }
				}
			}
			last_pt=pt;
		}

		/* Computing the weighted MAD of the absolute values of the residuals. */
		double resid_scale=0;
		for (pt=0; pt<npts; ++pt) {
			rsdptr[pt]=fabs(resptr[pt]-fitptr[pt]);
			resid_scale+=rsdptr[pt];
			rorptr[pt]=pt;
		}
		resid_scale/=npts;
		rsort_with_index(rsdptr, rorptr, npts);

		current=0;
		double cmad=0;
		const double halfweight=totalweight/2;
		for (pt=0; pt<npts; ++pt) {
			current+=weiptr[rorptr[pt]];
			if (current==halfweight) {  /* In the unlikely event of an exact match. */
				cmad=3*(rsdptr[pt]+rsdptr[pt+1]);
				break;
			} else if (current>halfweight) {
				cmad=6*rsdptr[pt];
				break;
			}
		}

		/* If it's too small, then robustness weighting will have no further effect.
		 * Any points with large residuals would already be pretty lowly weighted.
		 * This is based on a similar step in lowess.c in the core R code.
		 */
		if (cmad <= THRESHOLD * resid_scale) { break; }

		/* Computing the robustness weights. */
		for (pt=0; pt<npts; ++pt) {
			if (rsdptr[pt]<cmad) {
				robptr[rorptr[pt]]=pow(1-pow(rsdptr[pt]/cmad, 2.0), 2.0);
			} else { robptr[rorptr[pt]]=0; }
		}
	}

	UNPROTECT(1);
	return output;
}
};

}

#endif
