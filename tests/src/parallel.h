#ifndef PARALLEL_H
#define PARALLEL_H
#ifdef TEST_CUSTOM_PARALLEL
#include <thread>

template<class Function_>
void parallelize(size_t ntasks, size_t nthreads, Function_ fun) {
    size_t per_thread = (ntasks / nthreads) + (ntasks % nthreads > 0);
    size_t start = 0;
    std::vector<std::thread> jobs;
    jobs.reserve(nthreads);

    for (size_t t = 0; t < nthreads; ++t) {
        size_t len = std::min(ntasks - start, per_thread);
        jobs.emplace_back(fun, t, start, len);
        start += len;
    }

    for (auto& j : jobs) {
        j.join();
    }
}

#define WEIGHTEDLOWESS_CUSTOM_PARALLEL parallelize
#endif
#endif
