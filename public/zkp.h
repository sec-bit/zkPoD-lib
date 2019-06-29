#pragma once

#include <libff/algebra/fields/field_utils.hpp>
#include <libff/algebra/fields/fp.hpp>
#include <libsnark/common/default_types/r1cs_gg_ppzksnark_pp.hpp>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/gadgetlib1/pb_variable.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

#include "ecc.h"

typedef libsnark::r1cs_gg_ppzksnark_proof<
    libsnark::default_r1cs_gg_ppzksnark_pp>
    ZkProof;

typedef libff::Fr<libsnark::default_r1cs_gg_ppzksnark_pp> ZkFr;

typedef libsnark::r1cs_gg_ppzksnark_proving_key<
    libsnark::default_r1cs_gg_ppzksnark_pp>
    ZkPk;
typedef std::shared_ptr<ZkPk> ZkPkPtr;

typedef libsnark::r1cs_gg_ppzksnark_verification_key<
    libsnark::default_r1cs_gg_ppzksnark_pp>
    ZkVk;
typedef std::shared_ptr<ZkVk> ZkVkPtr;

enum
{
    // check the ZkProof operator<<()
    kZkProofSerializeSize = 2 * (32 + 2) + 1 * (64 + 2),
};

void InitZkp(bool disable_log);

ZkFr ConvertToZkFr(Fr const &mcl_fr);

std::vector<ZkFr> ConvertToZkFr(std::vector<Fr> const &mcl_frs);

std::vector<ZkFr> ConvertToZkFr(std::vector<uint64_t> const &o);

ZkPkPtr LoadZkPk(std::string const &file);

ZkVkPtr LoadZkVk(std::string const &file);

void ZkProofToBin(ZkProof const &proof,
                  std::array<uint8_t, kZkProofSerializeSize> &bin);

void ZkProofFromBin(ZkProof &proof,
                    std::array<uint8_t, kZkProofSerializeSize> const &bin);