#include "scheme_plain.h"
#include "ecc.h"
#include "mkl_tree.h"
#include "public.h"
#include "scheme_misc.h"

namespace scheme::plain {

uint64_t GetDataBlockCount(uint64_t size, uint64_t column_num) {
  uint64_t slice_count = (size + 31 - 1) / 31;
  return (slice_count + column_num - 1) / column_num;
}

// pad a random fr in the front of every block
bool DataToM(std::string const& pathname, uint64_t size, uint64_t n,
             uint64_t column_num, std::vector<Fr>& m) {
  try {
    io::mapped_file_params params;
    params.path = pathname;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != size) return false;

    auto start = (uint8_t*)view.data();
    auto end = start + view.size();

    auto s = column_num + 1;
    m.resize(n * s);
    for (uint64_t i = 0; i < n; ++i) {
      m[i * s] = FrRand();  // pad random fr
      for (uint64_t j = 1; j < s; ++j) {
        LoadMij(start, end, i, j - 1, column_num, m[i * s + j]);
      }
    }
    return true;
  } catch (std::exception&) {
    return false;
  }
}

bool DecryptedRangeMToFile(std::string const& file, uint64_t size, uint64_t s,
                           uint64_t start, uint64_t count,
                           std::vector<Fr>::const_iterator m_begin,
                           std::vector<Fr>::const_iterator m_end) {
  if (s < 1 || !count || !size) return false;
  uint64_t column_num = s - 1;
  uint64_t n = GetDataBlockCount(size, column_num);
  if (!n || start >= n || (start + count) > n) return false;
  if ((size_t)std::distance(m_begin, m_end) != count * s) return false;

  fs::remove(file);
  io::mapped_file_params file_params;
  file_params.path = file;
  file_params.flags = io::mapped_file_base::readwrite;
  file_params.new_file_size = count * column_num * 31 + 1;
  io::mapped_file file_view = io::mapped_file(file_params);
  if (!file_view.data()) return false;

  uint8_t* p = (uint8_t*)file_view.data();
  for (uint64_t i = 0; i < count; ++i) {
    for (uint64_t j = 1; j < s; ++j) {
      Fr const& fr = m_begin[i * s + j];
      FrToBin(fr, p);
      if (p[31]) {
        assert(false);
        return false;
      }
      p += 31;
    }
  }

  file_view.close();

  uint64_t file_size = column_num * count * 31;
  if (start + count == n) {
    uint64_t tail_pad_len = n * column_num * 31 - size;
    assert(tail_pad_len < column_num * 31);
    file_size -= tail_pad_len;
  }

  fs::resize_file(file, file_size);
  return true;
}

}  // namespace scheme::plain