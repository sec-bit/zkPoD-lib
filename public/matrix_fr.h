#pragma once

#include <Eigen/Dense>
#include "ecc.h"

namespace Eigen {
template <>
struct NumTraits<Fr> : NumTraits<mpz_class> {
  typedef Fr Real;
  typedef Fr NonInteger;
  typedef Fr Nested;
  enum {
    IsComplex = 0,
    IsInteger = 0,
    IsSigned = 1,
    RequireInitialization = 1,
    ReadCost = 1,
    AddCost = 3,
    MulCost = 3
  };
};

typedef Eigen::Matrix<Fr, Eigen::Dynamic, Eigen::Dynamic> MatrixXFr;
typedef Eigen::Matrix<Fr, 2, 2> Matrix2Fr;

typedef Eigen::Matrix<Fr, 1, Eigen::Dynamic> RowVectorXFr;
typedef Eigen::Matrix<Fr, Eigen::Dynamic, 1> ColVectorXFr;

typedef Eigen::Matrix<G1, Eigen::Dynamic, Eigen::Dynamic> MatrixXG1;
typedef Eigen::Matrix<G1, 1, Eigen::Dynamic> RowVectorXG1;
typedef Eigen::Matrix<G1, Eigen::Dynamic, 1> ColVectorXG1;

typedef Eigen::Matrix<G2, Eigen::Dynamic, Eigen::Dynamic> MatrixXG2;
typedef Eigen::Matrix<G2, 1, Eigen::Dynamic> RowVectorXG2;
typedef Eigen::Matrix<G2, Eigen::Dynamic, 1> ColVectorXG2;

}  // namespace Eigen
