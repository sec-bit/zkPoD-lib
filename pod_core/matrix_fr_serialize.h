#pragma once

#include "basic_types_serialize.h"
#include "matrix_fr.h"

namespace Eigen {
// save
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<Fr, _Rows, _Cols> const& t) {
  ar &t.rows();
  ar &t.cols();
  for (DenseIndex i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }
}

// load
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<Fr, _Rows, _Cols> &t) {
  DenseIndex rows, cols;
  ar &rows;
  ar &cols;
  t.resize(rows, cols);
  for (DenseIndex i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }  
}

// save
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<G1, _Rows, _Cols> const& t) {
  ar &t.rows();
  ar &t.cols();
  for (DenseIndex i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }
}

// load
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<G1, _Rows, _Cols> &t) {
  DenseIndex rows, cols;
  ar &rows;
  ar &cols;
  t.resize(rows, cols);
  for (DenseIndex i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }  
}

// save
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<G2, _Rows, _Cols> const& t) {
  ar &t.rows();
  ar &t.cols();
  for (DenseIndex i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }
}

// load
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<G2, _Rows, _Cols> &t) {
  DenseIndex rows, cols;
  ar &rows;
  ar &cols;
  t.resize(rows, cols);
  for (DenseIndex i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }  
}
}