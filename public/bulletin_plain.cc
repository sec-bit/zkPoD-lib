#include "bulletin_plain.h"

#include "ecc_pub.h"
#include "misc.h"
#include "mkl_tree.h"
#include "public.h"
#include "scheme_plain.h"

namespace scheme {
namespace plain {
bool IsBulletinValid(Bulletin const& bulletin) {
  auto const& ecc_pub = GetEccPub();
  if (!bulletin.size || !bulletin.s) return false;
  if (bulletin.s > ecc_pub.u1().size()) return false;
  if (bulletin.s == 1) return false;
  auto column_size = bulletin.s - 1;
  auto nn = GetDataBlockCount(bulletin.size, column_size);
  if (bulletin.n != nn) return false;
  return true;
}

bool SaveBulletin(std::string const& output, Bulletin const& bulletin) {
  try {
    pt::ptree tree;
    tree.put("mode", "plain");
    tree.put("size", bulletin.size);
    tree.put("s", bulletin.s);
    tree.put("n", bulletin.n);
    tree.put("sigma_mkl_root", misc::HexToStr(bulletin.sigma_mkl_root));
    pt::write_json(output, tree);
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadBulletin(std::string const& input, Bulletin& bulletin) {
  try {
    pt::ptree tree;
    pt::read_json(input, tree);
    if (tree.get<std::string>("mode") != "plain") return false;
    bulletin.size = tree.get<uint64_t>("size");
    bulletin.s = tree.get<uint64_t>("s");
    bulletin.n = tree.get<uint64_t>("n");
    misc::HexStrToH256(tree.get<std::string>("sigma_mkl_root"),
                       bulletin.sigma_mkl_root);
    return IsBulletinValid(bulletin);
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}
}  // namespace plain
}  // namespace scheme