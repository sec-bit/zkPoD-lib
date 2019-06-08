#include "scheme_otbatch3_misc.h"
#include "misc.h"
#include "tick.h"

namespace scheme::otbatch3::details {

void ComputeChallenge(h256_t const& seed, RomChallenge& challenge) {
  static const std::string suffix_c = "challenge_c";
  static const std::string suffix_e1 = "challenge_e1";
  static const std::string suffix_e2 = "challenge_e2";

  h256_t digest;
  CryptoPP::Keccak_256 hash;

  hash.Update(seed.data(), seed.size());
  hash.Update((uint8_t*)suffix_c.data(), suffix_c.size());
  hash.Final(digest.data());
  challenge.c.setArrayMaskMod(digest.data(), digest.size());
  // std::cout << "session c_: " << c_ << "\n";

  hash.Update(seed.data(), seed.size());
  hash.Update((uint8_t*)suffix_e1.data(), suffix_e1.size());
  hash.Final(digest.data());
  challenge.e1.setArrayMaskMod(digest.data(), digest.size());
  // std::cout << "session e1_: " << e1_ << "\n";

  hash.Update(seed.data(), seed.size());
  hash.Update((uint8_t*)suffix_e2.data(), suffix_e2.size());
  hash.Final(digest.data());
  challenge.e2.setArrayMaskMod(digest.data(), digest.size());
  // std::cout << "session e2_: " << e2_ << "\n";

  challenge.e1_square = challenge.e1 * challenge.e1;
  challenge.e2_square = challenge.e2 * challenge.e2;
  challenge.e1_e2_inverse = FrInv(challenge.e1 * challenge.e2_square -
                                  challenge.e2 * challenge.e1_square);
}

}  // namespace scheme::otbatch3::details
