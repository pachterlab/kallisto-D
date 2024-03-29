#ifndef KALLISTO_BOOTSTRAP_H
#define KALLISTO_BOOTSTRAP_H

#include <mutex>
#include <thread>

#include "KmerIndex.h"
#include "MinCollector.h"
#include "weights.h"
#include "EMAlgorithm.h"
#include "Multinomial.hpp"

class Bootstrap {
    // needs:
    // - "true" counts
    // - ecmap
    // - target_names
    // - eff_lens
public:
  Bootstrap(const std::vector<uint32_t>& true_counts,
            const KmerIndex& index,
            const MinCollector& tc,
            const std::vector<double>& eff_lens,
            size_t seed,
            const std::vector<double>& mean_fls,
            const ProgramOptions& opt) :
    index_(index),
    tc_(tc),
    eff_lens_(eff_lens),
    seed_(seed),
    mult_(true_counts, seed_),
    mean_fls_(mean_fls),
    opt(opt)
    {}

  // EM Algorithm generates a sample from the Multinomial, then returns
  // an "EMAlgorithm" that has already run the EM as well as compute the
        // rho values
  EMAlgorithm run_em();

private:
  const KmerIndex& index_;
  const MinCollector& tc_;
  const std::vector<double>& eff_lens_;
  size_t seed_;
  Multinomial mult_;
  const std::vector<double>& mean_fls_;
  const ProgramOptions& opt;
};

class BootstrapWriter {
  public:
    virtual ~BootstrapWriter() {};

    virtual void init(const std::string& fname, int num_bootstrap, int num_processed,
      const std::vector<int>& fld, const std::vector<int>& preBias, const std::vector<double>& postBias, uint compression, size_t index_version,
      const std::string& shell_call, const std::string& start_time) = 0;

    virtual void write_main(const EMAlgorithm& em,
        const std::vector<std::string>& targ_ids,
        const std::vector<int>& lengths) = 0;

    virtual void write_bootstrap(const EMAlgorithm& em, int bs_id) = 0;
};

class BootstrapThreadPool {
  friend class BootstrapWorker;

  public:
    BootstrapThreadPool(
        size_t n_threads,
        std::vector<size_t> seeds,
        const std::vector<uint32_t>& true_counts,
        const KmerIndex& index,
        const MinCollector& tc,
        const std::vector<double>& eff_lens,
        const ProgramOptions& p_opts,
        BootstrapWriter *bswriter,
        const std::vector<double>& mean_fls
        );

    size_t num_threads() {return n_threads_;}

    ~BootstrapThreadPool();
  private:
    std::vector<size_t> seeds_;
    size_t n_threads_;

    std::vector<std::thread> threads_;
    std::mutex seeds_mutex_;
    std::mutex write_lock_;

    size_t n_complete_;

    // things to run bootstrap
    const std::vector<uint32_t> true_counts_;
    const KmerIndex& index_;
    const MinCollector& tc_;
    const std::vector<double>& eff_lens_;
    const ProgramOptions& opt_;
    BootstrapWriter *writer_;
    const std::vector<double>& mean_fls_;
};

class BootstrapWorker {
  public:
    BootstrapWorker(BootstrapThreadPool& pool, size_t thread_id) :
      pool_(pool),
      thread_id_(thread_id)
    {}

  void operator()();

  private:
    BootstrapThreadPool& pool_;
    size_t thread_id_;
};

#endif
