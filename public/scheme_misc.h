#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "bp.h"
#include "ecc.h"
#include "mpz.h"
#include "vrf.h"
#include "scheme_error.h"

namespace scheme {

enum Mode { kPlain, kTable };

enum Action {
  kVrfQuery,
  kOtVrfQuery,
  kVrfPod,
  kOtVrfPod,
  kComplaintPod,
  kOtComplaintPod,
  kAtomicSwapPod,
  kAtomicSwapPodVc,
};

void LoadMij(uint8_t const* data_start, uint8_t const* data_end, uint64_t i,
             uint64_t j, uint64_t s, Fr& mij);

bool CopyData(std::string const& src, std::string const& dst);

bool SaveMkl(std::string const& output, std::vector<h256_t> const& mkl_tree);

bool LoadMkl(std::string const& input, uint64_t n,
             std::vector<h256_t>& mkl_tree);

bool SaveSigma(std::string const& output, std::vector<G1> const& sigma);

bool LoadSigma(std::string const& input, uint64_t n, h256_t const* root,
               std::vector<G1>& sigmas);

bool SaveMatrix(std::string const& output, std::vector<Fr> const& m);

bool LoadMatrix(std::string const& input, uint64_t ns, std::vector<Fr>& m);

std::vector<G1> CalcSigma(std::vector<Fr> const& m, uint64_t n, uint64_t s);

std::vector<h256_t> BuildSigmaMklTree(std::vector<G1> const& sigmas);

bool GetBulletinMode(std::string const& file, Mode& mode);

bool IsElementUnique(std::vector<Fr> const v);

h256_t CalcRootOfK(std::vector<G1> const& k);

h256_t CalcPathOfK(std::vector<G1> const& k, uint64_t ij,
                   std::vector<h256_t>& path);

h256_t CalcRangesDigest(std::vector<Range> const& r);

h256_t CalcFrDataDigest(std::vector<Fr> const& m);

h256_t CalcG1DataDigest(std::vector<G1> const& d);

bool VerifyPathOfK(G1 const& kij, uint64_t ij, uint64_t ns, h256_t const& root,
                   std::vector<h256_t> const& path);

void BuildK(std::vector<Fr> const& v, std::vector<G1>& k, uint64_t s);

h256_t CalcSeed2(std::vector<h256_t> const& h);

bool CheckDemandPhantoms(uint64_t n, std::vector<Range> const& demands,
                         std::vector<Range> const& phantoms);

uint64_t GetRangesOffsetByIndexOfM(std::vector<Range> const& ranges,
                                   uint64_t index);

bool CheckPhantoms(uint64_t n, std::vector<Range> const& phantoms);

bool CheckDemands(uint64_t n, std::vector<Range> const& demands);
}  // namespace scheme

namespace std {

std::istream& operator>>(std::istream& in, scheme::Mode& t);

std::ostream& operator<<(std::ostream& os, scheme::Mode const& t);

std::istream& operator>>(std::istream& in, scheme::Action& t);

std::ostream& operator<<(std::ostream& os, scheme::Action const& t);

std::istream& operator>>(std::istream& in, Range& t);

std::ostream& operator<<(std::ostream& os, Range const& t);
}  // namespace std
