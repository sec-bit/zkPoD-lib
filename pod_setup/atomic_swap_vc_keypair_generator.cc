#include <array>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#endif
#include <gmp.h>
#include <gmpxx.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <libff/algebra/fields/field_utils.hpp>
#include <libff/algebra/fields/fp.hpp>
#include <libsnark/common/default_types/r1cs_gg_ppzksnark_pp.hpp>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/gadgetlib1/pb_variable.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

#include "atomic_swap_gadget.h"
#include "mimc.h"

typedef libff::Fr<libsnark::default_r1cs_ppzksnark_pp> ZkFr;

namespace {
ZkFr ConvertToZkFr(Fr const& mcl_fr) {
  mpz_class m = mcl_fr.getMpz();
  return ZkFr(libff::bigint<ZkFr::num_limbs>(m.get_mpz_t()));
}

std::vector<ZkFr> ConvertToZkFr(std::vector<Fr> const& mcl_frs) {
  std::vector<ZkFr> zk_frs(mcl_frs.size());
  for (size_t i = 0; i < zk_frs.size(); ++i) {
    zk_frs[i] = ConvertToZkFr(mcl_frs[i]);
  }
  return zk_frs;
}
}  // namespace

bool GenerateAtomicSwapKeyPair(std::string const& output_path, uint64_t count) {
  // Create protoboard
  libsnark::protoboard<ZkFr> pb;

  // Define variables
  libsnark::pb_variable<ZkFr> seed;
  libsnark::pb_variable<ZkFr> seed_rand;
  libsnark::pb_variable<ZkFr> digest;
  libsnark::pb_variable<ZkFr> result;
  libsnark::pb_variable_array<ZkFr> o;
  libsnark::pb_variable_array<ZkFr> w;

  // Allocate variables to protoboard
  // The strings (like "x") are only for debugging purposes
  o.allocate(pb, count, "o");           // public
  w.allocate(pb, count, "w");           // public
  digest.allocate(pb, "digest");        // public
  result.allocate(pb, "result");        // public
  seed.allocate(pb, "seed");            // witness
  seed_rand.allocate(pb, "seed_rand");  // witness

  // This sets up the protoboard variables
  // so that the first one (out) represents the public
  // input and the rest is private input
  pb.set_input_sizes(count * 2 + 2);

  // Add R1CS constraints to protoboard
  auto mimc3_const = ConvertToZkFr(Mimc3Const());
  auto mimcinv_const = ConvertToZkFr(MimcInvConst());
  AtomicSwapVcGadget<ZkFr> g(pb, mimc3_const, mimcinv_const, seed, seed_rand,
                             digest, result, o, w);
  g.generate_r1cs_constraints();

  // Trusted setup
  const libsnark::r1cs_constraint_system<ZkFr> constraint_system =
      pb.get_constraint_system();

  const libsnark::r1cs_gg_ppzksnark_keypair<
      libsnark::default_r1cs_gg_ppzksnark_pp>
      keypair = libsnark::r1cs_gg_ppzksnark_generator<
          libsnark::default_r1cs_gg_ppzksnark_pp>(constraint_system);

  try {
    std::ofstream ofs_vk_data;
    ofs_vk_data.open(output_path + "/atomic_swap_vc.vk",
                     std::ofstream::out | std::ofstream::binary);
    ofs_vk_data << keypair.vk;
    ofs_vk_data.close();

    std::ofstream ofs_pk_data;
    ofs_pk_data.open(output_path + "/atomic_swap_vc.pk",
                     std::ofstream::out | std::ofstream::binary);
    ofs_pk_data << keypair.pk;
    ofs_pk_data.close();

    libsnark::r1cs_gg_ppzksnark_keypair<libsnark::default_r1cs_gg_ppzksnark_pp>
        keypair_check;

    std::ifstream ifs_vk_data;
    ifs_vk_data.open(output_path + "/atomic_swap_vc.vk",
                     std::ifstream::in | std::ifstream::binary);
    ifs_vk_data >> keypair_check.vk;

    std::ifstream ifs_pk_data;
    ifs_pk_data.open(output_path + "/atomic_swap_vc.pk",
                     std::ifstream::in | std::ifstream::binary);
    ifs_pk_data >> keypair_check.pk;

    if (!(keypair.pk == keypair_check.pk) ||
        !(keypair.vk == keypair_check.vk)) {
      assert(false);
      throw std::runtime_error("oops");
    }
    std::cout << "Generate success\n";
    return true;
  } catch (std::exception& ex) {
    std::cerr << "Execption: " << ex.what() << "\n";
    return false;
  }
}