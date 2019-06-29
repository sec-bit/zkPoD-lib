#pragma once

#include <stdint.h>
#include <vector>

#include "ecc.h"
#include "vrf.h"
#include "vrf_meta.h"

namespace scheme::table {
typedef std::vector<std::string> Record;

typedef std::vector<Record> Table;

enum Type {
  kCsv,  // now only support csv format
};

std::istream& operator>>(std::istream& in, Type& t);

std::ostream& operator<<(std::ostream& os, Type const& t);

void UniqueRecords(Table& table, std::vector<uint64_t> const& vrf_key_colnums);

uint64_t GetMaxRecordSize(Table const& table);

Fr GetPadFr(uint32_t len);

void RecordToBin(Record const& record, std::vector<uint8_t>& bin);

bool BinToRecord(std::vector<uint8_t> const& bin, Record& record,
                 uint64_t max_item);

void DataToM(Table const& table, std::vector<uint64_t> columens_index,
             uint64_t s, vrf::Sk<> const& vrf_sk, std::vector<Fr>& m);

VrfKeyMeta const* GetKeyMetaByName(VrfMeta const& vrf_meta,
                                   std::string const& name);

bool DecryptedRangeMToFile(std::string const& file, uint64_t s,
                           VrfMeta const& vrf_meta,
                           std::vector<Range> const& demands,
                           std::vector<Fr> const& part_m);
}  // namespace scheme::table