#pragma once

//#include <libsnark/gadgetlib1/gadgets/hashes/sha256/sha256_gadget.hpp>
#include "mimc3_gadget.h"
#include "mimcinv_gadget.h"

template <typename FieldT>
class AtomicSwapVcGadget : public libsnark::gadget<FieldT> {
 public:
  const std::vector<FieldT>& mimc3_constants_;
  const std::vector<FieldT>& mimcinv_constants_;
  const libsnark::pb_variable<FieldT> seed_;
  const libsnark::pb_variable<FieldT> seed_rand_;
  const libsnark::pb_variable<FieldT> digest_;
  const libsnark::pb_variable<FieldT> result_;
  const libsnark::pb_variable_array<FieldT> o_;
  const libsnark::pb_variable_array<FieldT> w_;
  libsnark::pb_variable_array<FieldT> v_;
  libsnark::pb_linear_combination_array<FieldT> seed_o_;
  std::shared_ptr<libsnark::inner_product_gadget<FieldT>> ip_gadget_;
  std::shared_ptr<Mimc3Gadget<FieldT>> mimc3_gadget_;
  std::vector<std::shared_ptr<MimcInvGadget<FieldT>>> mimcinv_gadgets_;

  AtomicSwapVcGadget(libsnark::protoboard<FieldT>& pb,
                     const std::vector<FieldT>& mimc3_constants,
                     const std::vector<FieldT>& mimcinv_constants,
                     const libsnark::pb_variable<FieldT> seed,
                     const libsnark::pb_variable<FieldT> seed_rand,
                     const libsnark::pb_variable<FieldT> digest,
                     const libsnark::pb_variable<FieldT> result,
                     const libsnark::pb_variable_array<FieldT> o,
                     const libsnark::pb_variable_array<FieldT> w,
                     const std::string& annotation_prefix = "")
      : libsnark::gadget<FieldT>(pb, annotation_prefix),
        mimc3_constants_(mimc3_constants),
        mimcinv_constants_(mimcinv_constants),
        seed_(seed),
        seed_rand_(seed_rand),
        digest_(digest),
        result_(result),
        o_(o),
        w_(w),
        seed_o_(w_.size()) {
    v_.allocate(pb, w_.size(), FMT(annotation_prefix, " v"));

    ip_gadget_.reset(new libsnark::inner_product_gadget<FieldT>(
        pb, v_, w_, result_, FMT(this->annotation_prefix, " ip")));

    mimc3_gadget_.reset(
        new Mimc3Gadget<FieldT>(pb, mimc3_constants, seed_, seed_rand_, digest_,
                                FMT(this->annotation_prefix, " mimc3")));

    mimcinv_gadgets_.resize(w_.size());
    for (size_t i = 0; i < mimcinv_gadgets_.size(); ++i) {
      libsnark::linear_combination<FieldT> lc_seed_o = seed_ + o_[i];
      seed_o_[i].assign(pb, lc_seed_o);

      mimcinv_gadgets_[i].reset(
          new MimcInvGadget<FieldT>(pb, mimcinv_constants, seed_o_[i], v_[i],
                                    FMT(this->annotation_prefix, " mimcinv")));
    }
  }

  void generate_r1cs_constraints() {
    mimc3_gadget_->generate_r1cs_constraints();

    for (size_t i = 0; i < mimcinv_gadgets_.size(); ++i) {
      mimcinv_gadgets_[i]->generate_r1cs_constraints();
    }

    ip_gadget_->generate_r1cs_constraints();
  }

  void generate_r1cs_witness() {
    mimc3_gadget_->generate_r1cs_witness();

    for (size_t i = 0; i < mimcinv_gadgets_.size(); ++i) {
      mimcinv_gadgets_[i]->generate_r1cs_witness();
      this->pb.val(v_[i]) = this->pb.val(mimcinv_gadgets_[i]->result());
    }

    ip_gadget_->generate_r1cs_witness();

    // std::cout << "ip result: " << this->pb.val(ip_gadget_->result) << "\n";
  }
};