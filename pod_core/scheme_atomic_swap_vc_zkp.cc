#include "scheme_atomic_swap_vc_zkp.h"
#include <fstream>
#include <iostream>
#include "atomic_swap_gadget.h"
#include "mimc.h"

namespace scheme::atomic_swap_vc {

void GenerateZkProof(ZkProof& proof, ZkPk const& pk, ZkpItem const& item,
                     ZkVkPtr check_vk) {
  using namespace libsnark;
  auto const kCount = ZkpMimcCount();
  assert(item.o.size() == kCount);
  assert(item.w.size() == kCount);

  // Create protoboard
  protoboard<ZkFr> pb;

  // Define variables
  pb_variable<ZkFr> seed;
  pb_variable<ZkFr> seed_rand;
  pb_variable<ZkFr> digest;
  pb_variable<ZkFr> result;
  pb_variable_array<ZkFr> o;
  pb_variable_array<ZkFr> w;

  // Allocate variables to protoboard
  o.allocate(pb, kCount, "o");          // public
  w.allocate(pb, kCount, "w");          // public
  digest.allocate(pb, "digest");        // public
  result.allocate(pb, "result");        // public
  seed.allocate(pb, "seed");            // witness
  seed_rand.allocate(pb, "seed_rand");  // witness

  // This sets up the protoboard variables
  // so that the first one (out) represents the public
  // input and the rest is private input
  pb.set_input_sizes(kCount * 2 + 2);

  // Add R1CS constraints to protoboard
  auto mimc3_const = ConvertToZkFr(Mimc3Const());
  auto mimcinv_const = ConvertToZkFr(MimcInvConst());
  AtomicSwapVcGadget<ZkFr> g(pb, mimc3_const, mimcinv_const, seed, seed_rand,
                             digest, result, o, w);
  g.generate_r1cs_constraints();

  // public statement
  for (size_t i = 0; i < kCount; ++i) {
    pb.val(o[i]) = item.o[i];
    pb.val(w[i]) = item.w[i];
  }
  pb.val(digest) = item.seed_mimc3_digest;
  pb.val(result) = item.inner_product;

  // witness
  pb.val(seed) = item.seed;
  pb.val(seed_rand) = item.seed_rand;

  g.generate_r1cs_witness();

  assert(pb.is_satisfied());

  // Create proof
  proof = r1cs_gg_ppzksnark_prover<default_r1cs_gg_ppzksnark_pp>(
      pk, pb.primary_input(), pb.auxiliary_input());

  if (check_vk) {
    ZkvItem zkv_item;
    zkv_item.inner_product = item.inner_product;
    zkv_item.seed_mimc3_digest = item.seed_mimc3_digest;
    zkv_item.o = item.o;
    zkv_item.w = item.w;
    assert(VerifyZkProof(proof, *check_vk, zkv_item));
  }
}

bool VerifyZkProof(ZkProof const& proof, ZkVk const& vk, ZkvItem const& item) {
  using namespace libsnark;
  auto const kCount = ZkpMimcCount();
  assert(item.o.size() == kCount);
  assert(item.w.size() == kCount);

  // Create protoboard
  protoboard<ZkFr> pb;

  // Define variables
  pb_variable<ZkFr> digest;
  pb_variable<ZkFr> result;
  pb_variable_array<ZkFr> o;
  pb_variable_array<ZkFr> w;

  // Allocate variables to protoboard
  o.allocate(pb, kCount, "o");    // public
  w.allocate(pb, kCount, "w");    // public
  digest.allocate(pb, "digest");  // public
  result.allocate(pb, "result");  // public

  // This sets up the protoboard variables
  // so that the first one (out) represents the public
  // input and the rest is private input
  pb.set_input_sizes(kCount * 2 + 2);

  //// Add R1CS constraints to protoboard
  // auto mimc3_const = ConvertToZkFr(Mimc3Const());
  // auto mimcinv_const = ConvertToZkFr(MimcInvConst());
  // AtomicSwapVcGadget<ZkFr> g(pb, mimc3_const, mimcinv_const, seed,
  // seed_rand,
  //                           digest, result, o, w);
  // g.generate_r1cs_constraints();

  // public statement
  for (size_t i = 0; i < kCount; ++i) {
    pb.val(o[i]) = item.o[i];
    pb.val(w[i]) = item.w[i];
  }
  pb.val(digest) = item.seed_mimc3_digest;
  pb.val(result) = item.inner_product;

  // Verify proof
  bool verified =
      r1cs_gg_ppzksnark_verifier_strong_IC<default_r1cs_gg_ppzksnark_pp>(
          vk, pb.primary_input(), proof);
  assert(verified);
  return verified;
}
}  // namespace scheme::atomic_swap_vc