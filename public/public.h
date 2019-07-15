#pragma once

#ifdef _WIN32
#pragma warning(disable : 4503)
#pragma warning(disable : 4996)
#endif

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
//#if defined(_MSC_VER)
//#include <execution>
//#else
//#include <parallel/algorithm>
//#endif
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
//#include <boost/asio.hpp>
#include <boost/any.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_int/serialize.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

//#include <boost/random.hpp>

#include <cryptopp/aes.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/hex.h>
#include <cryptopp/keccak.h>
#include <cryptopp/modes.h>
#include <cryptopp/oids.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pwdbased.h>
//#include <cryptopp/scrypt.h>
#include <cryptopp/secblock.h>
#include <cryptopp/sha.h>
#include <cryptopp/sha3.h>

// namespace asio = boost::asio;
namespace bs = boost::system;
namespace fs = boost::filesystem;
namespace mp = boost::multiprecision;
namespace io = boost::iostreams;
namespace pt = boost::property_tree;
namespace mi = boost::multi_index;
namespace po = boost::program_options;