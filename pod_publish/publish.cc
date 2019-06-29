#include "publish.h"

#include "bp.h"
#include "bulletin_plain.h"
#include "bulletin_table.h"
#include "chain.h"
#include "csv.hpp"
#include "ecc.h"
#include "ecc_pub.h"
#include "misc.h"
#include "mkl_tree.h"
#include "multiexp.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_table.h"
#include "vrf_meta.h"

namespace {

using namespace scheme;
bool LoadCsvTable(std::string const& file, std::vector<std::string>& col_names,
                  table::Table& table) {
  Tick _tick_(__FUNCTION__);
  using namespace csv;
  try {
    CSVReader reader(file);
    col_names = reader.get_col_names();

    for (CSVRow& row : reader) {  // Input iterator
      table::Record record;
      for (CSVField& field : row) {
        record.push_back(std::string(field.get<>()));
      }
      assert(record.size() == col_names.size());
      table.emplace_back(std::move(record));
    }
    reader.close();
    return true;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool LoadTable(std::string const& file, table::Type table_type,
               std::vector<std::string>& col_names, table::Table& table) {
  if (table_type != table::Type::kCsv) {
    // TBD: support more db file types
    return false;
  }
  return LoadCsvTable(file, col_names, table);
}

void PadRubbishRow(table::Table& table) {
  Tick _tick_(__FUNCTION__);
  table::Record record(table[0].size());
  for (auto& i : record) {
    i = "PAD";
  }
  table.emplace_back(std::move(record));
}
}  // namespace

bool PublishTable(std::string publish_file, std::string output_path,
                  scheme::table::Type table_type,
                  std::vector<uint64_t> vrf_colnums_index,
                  std::vector<bool> unique_key) {
  using namespace scheme;
  using namespace scheme::table;
  using namespace misc;

  assert(unique_key.size() == vrf_colnums_index.size());

  auto& ecc_pub = GetEccPub();
  boost::system::error_code err;
  std::string public_path = output_path + "/public";
  if (!fs::is_directory(public_path, err) &&
      !fs::create_directories(public_path, err)) {
    assert(false);
    return false;
  }
  std::string private_path = output_path + "/private";
  if (!fs::is_directory(private_path, err) &&
      !fs::create_directories(private_path, err)) {
    assert(false);
    return false;
  }

  std::string bulletin_file = output_path + "/bulletin";
  std::string original_file = private_path + "/original";
  std::string matrix_file = private_path + "/matrix";
  std::string sigma_file = public_path + "/sigma";
  std::string sigma_mkl_tree_file = public_path + "/sigma_mkl_tree";
  std::string vrf_pk_file = public_path + "/vrf_pk";
  std::string vrf_sk_file = private_path + "/vrf_sk";
  std::string vrf_meta_file = public_path + "/vrf_meta";
  std::vector<std::string> key_bp_files(vrf_colnums_index.size());
  std::vector<std::string> key_m_files(vrf_colnums_index.size());
  for (size_t i = 0; i < key_bp_files.size(); ++i) {
    std::string str_i = std::to_string(i);
    key_bp_files[i] = public_path + "/key_bp_" + str_i;
    key_m_files[i] = public_path + "/key_m_" + str_i;
  }

  if (!CopyData(publish_file, original_file)) {
    assert(false);
    return false;
  }

  vrf::Pk<> vrf_pk;
  vrf::Sk<> vrf_sk;

  vrf::Generate<>(vrf_pk, vrf_sk);

  Table table;
  VrfMeta vrf_meta;
  if (!LoadTable(original_file, table_type, vrf_meta.column_names, table)) {
    assert(false);
    return false;
  }

  vrf_meta.keys.resize(vrf_colnums_index.size());
  for (uint64_t i = 0; i < vrf_colnums_index.size(); ++i) {
    vrf_meta.keys[i].column_index = vrf_colnums_index[i];
    vrf_meta.keys[i].j = i;
    vrf_meta.keys[i].unique = unique_key[i];
  }

  std::vector<uint64_t> unique_index;
  for (uint64_t i = 0; i < vrf_colnums_index.size(); ++i) {
    if (unique_key[i]) unique_index.push_back(vrf_colnums_index[i]);
  }
  UniqueRecords(table, unique_index);

  PadRubbishRow(table);

  auto max_record_size = GetMaxRecordSize(table);

  Bulletin bulletin;
  bulletin.n = table.size();
  auto record_fr_num = (max_record_size + 30) / 31;
  bulletin.s = vrf_colnums_index.size() + 1 + record_fr_num;
  auto max_s = ecc_pub.u1().size();
  if (bulletin.s > max_s) {
    assert(false);
    return false;
  }

  std::vector<Fr> m(bulletin.n * bulletin.s);
  DataToM(table, vrf_colnums_index, bulletin.s, vrf_sk, m);
  if (!SaveMatrix(matrix_file, m)) {
    assert(false);
    return false;
  }

  // sigma
  std::vector<G1> sigmas = CalcSigma(m, bulletin.n, bulletin.s);
  if (!SaveSigma(sigma_file, sigmas)) {
    assert(false);
    return false;
  }

  // build sigma mkl tree
  auto sigma_mkl_tree = BuildSigmaMklTree(sigmas);
  if (!SaveMkl(sigma_mkl_tree_file, sigma_mkl_tree)) {
    assert(false);
    return false;
  }
  bulletin.sigma_mkl_root = sigma_mkl_tree.back();

  // vrf pk
  if (!SaveVrfPk(vrf_pk_file, vrf_pk)) {
    assert(false);
    return false;
  }

  if (!GetFileSha256(vrf_pk_file, vrf_meta.pk_digest)) {
    assert(false);
    return false;
  }

  // vrf sk
  if (!SaveVrfSk(vrf_sk_file, vrf_sk)) {
    assert(false);
    return false;
  }

  // key and mkl_root
  for (size_t j = 0; j < vrf_colnums_index.size(); ++j) {
    std::vector<Fr> km(bulletin.n);
    for (size_t i = 0; i < bulletin.n; ++i) {
      km[i] = m[i * bulletin.s + j];
    }

    // NOTE: After UniqueRecords(), all of the keys are difference. But there
    // still has very small probability that the km is not unique (two
    // difference key have same digest).
    // Here just simply not supporting such data.
    if (vrf_meta.keys[j].unique && !IsElementUnique(km)) {
      assert(false);
      return false;
    }

    // save key_m_files
    if (!SaveMatrix(key_m_files[j], km)) {
      assert(false);
      return false;
    }

    auto get_item = [&km](uint64_t i) -> h256_t {
      if (i < km.size()) {
        h256_t h;
        FrToBin(km[i], h.data());
        return h;
      } else {
        return h256_t();
      }
    };
    vrf_meta.keys[j].mj_mkl_root = mkl::CalcRoot(get_item, bulletin.n);
  }

  // key bp proof: bp about relation about mi_key with sigma_i
  for (size_t i = 0; i < vrf_colnums_index.size(); ++i) {
    auto& key = vrf_meta.keys[i];
    bp::P1Proof bp_p1_proof;
    BuildKeyBp(bulletin.n, bulletin.s, m, bulletin.sigma_mkl_root,
               key.column_index, key.mj_mkl_root, bp_p1_proof);

#ifdef _DEBUG
    std::vector<Fr> dummy_km(bulletin.n);
    for (uint64_t col = 0; col < bulletin.n; ++col) {
      dummy_km[col] = m[col * bulletin.s + key.column_index];
    }
    assert(VerifyKeyBp(bulletin.n, bulletin.s, dummy_km, sigmas,
                       key.column_index, bulletin.sigma_mkl_root,
                       key.mj_mkl_root, bp_p1_proof));
#endif

    if (!SaveBpP1Proof(key_bp_files[i], bp_p1_proof)) {
      assert(false);
      return false;
    }
    if (!GetFileSha256(key_bp_files[i], key.bp_digest)) {
      assert(false);
      return false;
    }
  }

  if (!SaveVrfMeta(vrf_meta_file, vrf_meta)) {
    assert(false);
    return false;
  }

  if (!GetFileSha256(vrf_meta_file, bulletin.vrf_meta_digest)) {
    assert(false);
    return false;
  }

  if (!SaveBulletin(bulletin_file, bulletin)) {
    assert(false);
    return false;
  }

  std::cout << "n: " << bulletin.n << ", s: " << bulletin.s << "\n";

#ifdef _DEBUG
  std::string debug_data_file = original_file + ".debug";
  std::vector<Range> debug_demands(1);
  debug_demands[0] = Range(0, bulletin.n);
  if (!DecryptedRangeMToFile(debug_data_file, bulletin.s, vrf_meta,
                             debug_demands, m)) {
    assert(false);
    return false;
  }
  Table debug_table;
  VrfMeta debug_vrf_meta;
  if (!LoadTable(debug_data_file, table_type, debug_vrf_meta.column_names,
                 debug_table)) {
    assert(false);
    return false;
  }
  assert(debug_vrf_meta.column_names == vrf_meta.column_names);
  assert(table == debug_table);

  fs::remove(debug_data_file);
#endif

  return true;
}

bool PublishPlain(std::string publish_file, std::string output_path,
                  uint64_t column_num) {
  using namespace scheme;
  using namespace scheme::plain;
  using namespace misc;

  auto& ecc_pub = GetEccPub();
  auto max_s = ecc_pub.u1().size();
  if (column_num > max_s) {
    std::cerr << "column_num too large! The upper bound is " << max_s
              << std::endl;
    return false;
  }
  boost::system::error_code err;
  std::string public_path = output_path + "/public";
  if (!fs::is_directory(public_path, err) &&
      !fs::create_directories(public_path, err)) {
    assert(false);
    return false;
  }
  std::string private_path = output_path + "/private";
  if (!fs::is_directory(private_path, err) &&
      !fs::create_directories(private_path, err)) {
    assert(false);
    return false;
  }

  Bulletin bulletin;
  bulletin.size = fs::file_size(publish_file);
  if (!bulletin.size) return false;
  bulletin.s = column_num + 1;
  bulletin.n = GetDataBlockCount(bulletin.size, column_num);

  std::string bulletin_file = output_path + "/bulletin";
  std::string original_file = private_path + "/original";
  std::string matrix_file = private_path + "/matrix";
  std::string sigma_file = public_path + "/sigma";
  std::string sigma_mkl_file = public_path + "/sigma_mkl_tree";

  if (!CopyData(publish_file, original_file)) {
    assert(false);
    return false;
  }

  std::vector<Fr> m;
  if (!DataToM(original_file, bulletin.size, bulletin.n, column_num, m)) {
    assert(false);
    return false;
  }

  if (!SaveMatrix(matrix_file, m)) {
    assert(false);
    return false;
  }

  std::vector<G1> sigmas = CalcSigma(m, bulletin.n, bulletin.s);

  if (!SaveSigma(sigma_file, sigmas)) {
    assert(false);
    return false;
  }

  // mkl
  auto sigma_mkl_tree = BuildSigmaMklTree(sigmas);
  if (!SaveMkl(sigma_mkl_file, sigma_mkl_tree)) {
    assert(false);
    return false;
  }
  bulletin.sigma_mkl_root = sigma_mkl_tree.back();

  // meta
  if (!SaveBulletin(bulletin_file, bulletin)) {
    assert(false);
    return false;
  }

#ifdef _DEBUG
  std::string debug_data_file = original_file + ".debug";
  if (!DecryptedRangeMToFile(debug_data_file, bulletin.size, bulletin.s, 0,
                             bulletin.n, m.begin(), m.end())) {
    assert(false);
    return false;
  }

  if (!IsSameFile(debug_data_file, original_file)) {
    assert(false);
    return false;
  }
  fs::remove(debug_data_file);
#endif

  std::cout << "file size: " << bulletin.size << "\n";
  std::cout << "n: " << bulletin.n << ", s: " << bulletin.s << "\n";

  return true;
}