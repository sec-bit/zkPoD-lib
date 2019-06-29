#pragma once

#ifdef MULTICORE
#ifndef __clang__
#include <omp.h>
#endif
#endif
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4191)
#endif

#include <mcl/bn256.hpp>
#include <mcl/window_method.hpp>

#include "basic_types.h"

#ifdef _WIN32
#pragma warning(pop)
#endif

using mcl::bn256::Fp;
using mcl::bn256::Fp12;
using mcl::bn256::Fp2;
using mcl::bn256::Fp6;
using mcl::bn256::Fr;
using mcl::bn256::G1;
using mcl::bn256::G2;
typedef mcl::fp::WindowMethod<G1> G1WM;
typedef mcl::fp::WindowMethod<G2> G2WM;

void InitEcc();

void FpRand(Fp *f);

Fp FpRand();

Fp2 Fp2Rand();

void FrRand(Fr *f);

Fr FrRand();

void FrRand(Fr *r, size_t n);

void FrRand(std::vector<Fr *> &f);

Fr FrInv(Fr const &r);

void FrInv(Fr* begin, uint64_t count);

void FrInv(std::vector<Fr>& vec);

G1 G1Rand();

void G1Rand(G1 *r, size_t n);

G2 G2Rand();

Fr FrZero();

Fr FrOne();

G1 G1Zero();

void G1Zero(G1 *r, size_t n);

G1 G1One();

G2 G2Zero();

void G2Zero(G2 *r, size_t n);

G2 G2One();

G1 MapToG1(Fp t);

G1 MapToG1(void const *b, size_t n);

G1 MapToG1(std::string const &s);

G2 MapToG2(Fp2 t);

G2 MapToG2(void const *b, size_t n);

G2 MapToG2(std::string const &s);

Fr MapToFr(void const *b, size_t n);

Fr MapToFr(uint64_t b);

// inner product
G1 MultiExp(G1 const *g, Fr const *f, size_t n);

// a * g + b * g
G1 MultiExp(G1 const &g, Fr const &a, G1 const &h, Fr const &b);

G1 MultiExp(G1 const *g, Fr const *a, G1 const *h, Fr const *b, size_t n);

Fr InnerProduct(Fr const *a, Fr const *b, size_t n);

Fr InnerProduct(std::function<Fr(size_t)> const &get_a,
                std::function<Fr(size_t)> const &get_b, size_t n);

Fr StrHashToFr(std::string const &s);

// because the Fr is 254bits, so we only use 31bytes
Fr BinToFr31(void const *start, void const *end);

// the buffer of start must >= 32
bool BinToFr32(void const *start, Fr *fr);

Fr BinToFr32(void const *start); // may throw

// buf must 32 bytes, if the fr comes from BinToFr31(), the buf[31] will be 0
void FrToBin(Fr const &fr, uint8_t *buf);

h256_t FrToBin(Fr const &fr);

bool StrToFr(std::string const &s, Fr *fr);

// may throw exception
Fr StrToFr(std::string const &s);

std::string FrToStr(Fr const &fr);

// buf must 32 bytes
bool BinToG1(uint8_t const *buf, G1 *g);

G1 BinToG1(uint8_t const *buf);

// buf must 32 bytes
void G1ToBin(G1 const &g, uint8_t *buf);

h256_t G1ToBin(G1 const &g);

bool StrToG1(std::string const &s, G1 *g);

// may throw
G1 StrToG1(std::string const &s);

std::string G1ToStr(G1 const &g);

// buf must 64 bytes
bool BinToG2(uint8_t const *buf, G2 *g);

G2 BinToG2(uint8_t const *buf);

// buf must 64 bytes
void G2ToBin(G2 const &g, uint8_t *buf);

bool StrToG2(std::string const &s, G2 *g);

// may throw exception
G2 StrToG2(std::string const &s);

std::string G2ToStr(G2 const &g);

G1 MultiExpBosCoster(G1 const *pg, Fr const *pf, size_t n);

G1 MultiExpBosCoster(std::function<G1(size_t)> const &get_g,
                     std::function<Fr(size_t)> const &get_f, size_t n);

bool PairingMatch(G1 const &a, G1 const &c, G2 const &d);

bool PairingMatch(G1 const &a, G2 const &b, G1 const &c, G2 const &d);

Fr FrPower(Fr const& base, mpz_class const& exp);

inline bool operator==(G1WM const& a, G1WM const& b) {
  if (a.bitSize_ != b.bitSize_) return false;
  if (a.winSize_ != b.winSize_) return false;
  if (a.tbl_.size() != b.tbl_.size()) return false;
  for (size_t i = 0; i < a.tbl_.size(); ++i) {
    auto const& ga = a.tbl_[i];
    auto const& gb = b.tbl_[i];
    if (ga != gb) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(G1WM const &a, G1WM const &b) { return !(a == b); }

inline bool operator==(G2WM const &a, G2WM const &b)
{
  if (a.bitSize_ != b.bitSize_)
    return false;
  if (a.winSize_ != b.winSize_)
    return false;
  if (a.tbl_.size() != b.tbl_.size())
    return false;
  for (size_t i = 0; i < a.tbl_.size(); ++i)
  {
    if (a.tbl_[i] != b.tbl_[i])
      return false;
  }
  return true;
}

inline bool operator!=(G2WM const &a, G2WM const &b) { return !(a == b); }