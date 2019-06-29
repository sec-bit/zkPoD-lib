#include "bulletin_table.h"
#include "ecc_pub.h"
#include "misc.h"
#include "mkl_tree.h"
#include "public.h"

namespace scheme {
namespace table {
bool IsBulletinValid(Bulletin const& bulletin) {
  auto const& ecc_pub = GetEccPub();
  std::cout << bulletin.n << "," << bulletin.s << "," << ecc_pub.u1().size() << "\n";
  return bulletin.n && bulletin.s > 0 && bulletin.s <= ecc_pub.u1().size();
}

bool SaveBulletin(std::string const& output, Bulletin const& bulletin) {
  if (!IsBulletinValid(bulletin)) {
    assert(false);
    return false;
  }

  try {
    pt::ptree tree;
    tree.put("mode", "table");
    tree.put("n", bulletin.n);
    tree.put("s", bulletin.s);
    tree.put("sigma_mkl_root", misc::HexToStr(bulletin.sigma_mkl_root));
    tree.put("vrf_meta_digest", misc::HexToStr(bulletin.vrf_meta_digest));
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
    if (tree.get<std::string>("mode") != "table") throw std::exception();
    bulletin.n = tree.get<uint64_t>("n");
    bulletin.s = tree.get<uint64_t>("s");
    misc::HexStrToH256(tree.get<std::string>("sigma_mkl_root"),
                       bulletin.sigma_mkl_root);
    misc::HexStrToH256(tree.get<std::string>("vrf_meta_digest"),
                       bulletin.vrf_meta_digest);
    if (!IsBulletinValid(bulletin)) throw std::exception();
    return true;
  } catch (std::exception&) {
    std::cout << "bulletin invalid: " << input << "\n";
    assert(false);
    return false;
  }
}
}  // namespace table
}  // namespace scheme