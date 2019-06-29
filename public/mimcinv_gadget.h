#pragma once

#include <stdlib.h>
#include <iostream>

#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/gadgetlib1/gadget.hpp>
#include <libsnark/gadgetlib1/gadgets/basic_gadgets.hpp>
#include <libsnark/gadgetlib1/pb_variable.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

template <typename FieldT>
class MimcInvGadget : public libsnark::gadget<FieldT> {
 public:
  const std::vector<FieldT>& constants_;
  const libsnark::pb_linear_combination<FieldT> seed_;
  const libsnark::pb_variable<FieldT> digest_;
  libsnark::pb_variable_array<FieldT> rounds_x_;

  MimcInvGadget(libsnark::protoboard<FieldT>& pb,
                const std::vector<FieldT>& constants,
                const libsnark::pb_linear_combination<FieldT> seed,
                const libsnark::pb_variable<FieldT> digest,
                const std::string& annotation_prefix = "")
      : libsnark::gadget<FieldT>(pb, annotation_prefix),
        constants_(constants),
        seed_(seed),
        digest_(digest) {
    rounds_x_.allocate(pb, constants_.size(),
                       FMT(annotation_prefix, " rounds_x"));
  }

  libsnark::pb_variable<FieldT> result() {
    return rounds_x_[constants_.size() - 1];
  }

  void generate_r1cs_constraints() {
    // x[0] = FrInv(s + kConst[0]);
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        rounds_x_[0], seed_ + constants_[0], FieldT(1)));

    // x[1] = s + FrInv(x[0] + kConst[1]);
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        rounds_x_[1] - seed_, rounds_x_[0] + constants_[1], FieldT(1)));

    for (size_t i = 2; i < constants_.size(); ++i) {
      // x[i] = x[i - 2] + FrInv(x[i - 1] + kConst[i]);
      this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
          rounds_x_[i] - rounds_x_[i - 2], rounds_x_[i - 1] + constants_[i],
          FieldT(1)));
    }

    // digest_ == x.back()
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        1, rounds_x_[constants_.size() - 1] - digest_, 0));
  }

  void generate_r1cs_witness() {
    seed_.evaluate(this->pb);
    FieldT seed = this->pb.lc_val(seed_);
    FieldT temp;

    // x[0] = FrInv(s + kConst[0]);
    temp = seed + constants_[0];
    this->pb.val(rounds_x_[0]) = temp.inverse();

    // x[1] = s + FrInv(x[0] + kConst[1]);
    temp = this->pb.val(rounds_x_[0]) + constants_[1];
    this->pb.val(rounds_x_[1]) = seed + temp.inverse();

    for (size_t i = 2; i < constants_.size(); ++i) {
      // x[i] = x[i - 2] + FrInv(x[i - 1] + kConst[i]);
      temp = this->pb.val(rounds_x_[i - 1]) + constants_[i];
      this->pb.val(rounds_x_[i]) =
          this->pb.val(rounds_x_[i - 2]) + temp.inverse();
    }
  }
};