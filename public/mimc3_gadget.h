#pragma once

#include <stdlib.h>
#include <iostream>

#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/gadgetlib1/gadget.hpp>
#include <libsnark/gadgetlib1/gadgets/basic_gadgets.hpp>
#include <libsnark/gadgetlib1/pb_variable.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

template <typename FieldT>
class Mimc3Gadget : public libsnark::gadget<FieldT> {
 public:
  const std::vector<FieldT>& constants_;
  const libsnark::pb_variable<FieldT> left_;
  const libsnark::pb_variable<FieldT> right_;
  const libsnark::pb_variable<FieldT> digest_;
  libsnark::pb_variable_array<FieldT> rounds_x_;
  libsnark::pb_variable_array<FieldT> rounds_y_;

  Mimc3Gadget(libsnark::protoboard<FieldT>& pb,
              const std::vector<FieldT>& constants,
              const libsnark::pb_variable<FieldT> left,
              const libsnark::pb_variable<FieldT> right,
              const libsnark::pb_variable<FieldT> digest,
              const std::string& annotation_prefix = "")
      : libsnark::gadget<FieldT>(pb, annotation_prefix),
        constants_(constants),
        left_(left),
        right_(right),
        digest_(digest) {
    rounds_x_.allocate(pb, constants_.size(),
                       FMT(annotation_prefix, " rounds_x"));
    rounds_y_.allocate(pb, constants_.size(),
                       FMT(annotation_prefix, " rounds_y"));
  }

  libsnark::pb_variable<FieldT> result() {
    return rounds_y_[constants_.size() - 1];
  }

  void generate_r1cs_constraints() {
    // x[0] = (left + kConst[0]) * (left + kConst[0]);
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        left_ + constants_[0], left_ + constants_[0], rounds_x_[0]));

    // y[0] = right + (left + kConst[0]) * x[0];
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        left_ + constants_[0], rounds_x_[0], rounds_y_[0] - right_));

    // x[1] = (y[0] + kConst[1]) * ((y[0] + kConst[1]));
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        rounds_y_[0] + constants_[1], rounds_y_[0] + constants_[1],
        rounds_x_[1]));

    // y[1] = left + ((y[0] + kConst[1])) * x[1];
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        rounds_y_[0] + constants_[1], rounds_x_[1], rounds_y_[1] - left_));

    for (size_t i = 2; i < constants_.size(); ++i) {
      // x[i] = (y[i - 1] + kConst[i]) * (y[i - 1] + kConst[i]);
      this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
          rounds_y_[i - 1] + constants_[i], rounds_y_[i - 1] + constants_[i],
          rounds_x_[i]));

      // y[i] = y[i - 2] + (y[i - 1] + kConst[i]) * x[i];
      this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
          rounds_y_[i - 1] + constants_[i], rounds_x_[i],
          rounds_y_[i] - rounds_y_[i - 2]));
    }

    // digest_ == y.back()
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        1, rounds_y_[constants_.size() - 1] - digest_, 0));
  }

  void generate_r1cs_witness() {
    FieldT left = this->pb.val(left_);
    FieldT right = this->pb.val(right_);
    FieldT temp;

    // x[0] = (left + kConst[0]) * (left + kConst[0]);
    temp = left + constants_[0];
    this->pb.val(rounds_x_[0]) = temp * temp;

    // y[0] = right + (left + kConst[0]) * x[0];
    this->pb.val(rounds_y_[0]) = right + temp * this->pb.val(rounds_x_[0]);

    // x[1] = (y[0] + kConst[1]) * ((y[0] + kConst[1]));
    temp = this->pb.val(rounds_y_[0]) + constants_[1];
    this->pb.val(rounds_x_[1]) = temp * temp;

    // y[1] = left + ((y[0] + kConst[1])) * x[1];
    this->pb.val(rounds_y_[1]) = left + temp * this->pb.val(rounds_x_[1]);

    for (size_t i = 2; i < constants_.size(); ++i) {
      // x[i] = (y[i - 1] + kConst[i]) * (y[i - 1] + kConst[i]);
      temp = this->pb.val(rounds_y_[i - 1]) + constants_[i];
      this->pb.val(rounds_x_[i]) = temp * temp;

      // y[i] = y[i - 2] + (y[i - 1] + kConst[i]) * x[i];
      this->pb.val(rounds_y_[i]) =
          this->pb.val(rounds_y_[i - 2]) + temp * this->pb.val(rounds_x_[i]);
    }
    // std::cout << "mimc3 result: " << this->pb.val(result()) << "\n";
  }
};