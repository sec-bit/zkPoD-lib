#include "capi/scheme_plain_atomic_swap_test_capi.h"
#include "capi/scheme_plain_atomic_swap_vc_test_capi.h"
#include "capi/scheme_plain_complaint_test_capi.h"
#include "capi/scheme_plain_ot_complaint_test_capi.h"
#include "capi/scheme_table_atomic_swap_test_capi.h"
#include "capi/scheme_table_atomic_swap_vc_test_capi.h"
#include "capi/scheme_table_complaint_test_capi.h"
#include "capi/scheme_table_ot_complaint_test_capi.h"
#include "capi/scheme_table_ot_vrfq_test_capi.h"
#include "capi/scheme_table_vrfq_test_capi.h"
#include "ecc_pub.h"
#include "public.h"
#include "scheme_atomic_swap_test.h"
#include "scheme_atomic_swap_vc_test.h"
#include "scheme_complaint_test.h"
#include "scheme_misc.h"
#include "scheme_ot_complaint_test.h"
#include "scheme_ot_vrfq_test.h"
#include "scheme_vrfq_test.h"
#include "zkp.h"
#include "zkp_key.h"

namespace
{
void DumpEccPub()
{
  auto const &ecc_pub = GetEccPub();
  std::cout << "g1: " << G1ToStr(G1One()) << "\n";
  std::cout << "g2: " << G1ToStr(G1One()) << "\n";
  std::cout << "u1 size: " << ecc_pub.u1().size() << "\n";
  std::cout << "u2 size: " << ecc_pub.u2().size() << "\n";
  std::cout << "\nu1 list(z,x,y):\n";
  for (auto const &i : ecc_pub.u1())
  {
    std::cout << G1ToStr(i) << "\n";
  }
  std::cout << "\nu2 list(z,x,y):\n";
  for (auto const &i : ecc_pub.u2())
  {
    std::cout << G2ToStr(i) << "\n";
  }
  std::cout << "\n";
}
} // namespace

bool InitAll(std::string const &data_dir)
{
  InitEcc();

#ifdef _DEBUG
  InitZkp(false);
#else
  InitZkp(true);
#endif

  auto ecc_pub_file = data_dir + "/" + "ecc_pub.bin";

  if (!OpenOrCreateEccPub(ecc_pub_file))
  {
    std::cerr << "Open or create ecc pub file " << ecc_pub_file << " failed\n";
    return false;
  }

  std::string zkp_key_dir = data_dir + "/" + "zksnark_key";
  if (zkp_key_dir.empty() || !fs::is_directory(zkp_key_dir))
  {
    std::cerr << "Open zkp_key_dir " << zkp_key_dir << " failed\n";
    return false;
  }

  ZkpKey::instance(zkp_key_dir);

  if (ZkpKey::instance().IsEmpty())
  {
    std::cerr << "Warning: zk key file not exist.\n";
  }

  return true;
}

int main(int argc, char **argv)
{
  setlocale(LC_ALL, "");

  using scheme::Action;
  using scheme::Mode;
  Mode mode;
  Action action;
  std::string publish_dir;
  std::string output_dir;
  std::string data_dir;
  std::string query_key;
  std::vector<std::string> query_values;
  std::vector<std::string> phantom_values;
  std::vector<Range> demand_ranges;
  std::vector<Range> phantom_ranges;
  bool use_capi = false;
  bool test_evil = false;
  bool dump_ecc_pub = false;
#ifdef MULTICORE
  uint32_t omp_thread_num = 0;
#endif

  try
  {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "data_dir,d", po::value<std::string>(&data_dir)->default_value("."),
        "Provide the configure file dir")("mode,m", po::value<Mode>(&mode),
                                          "Provide pod mode (plain, table)")(
        "action,a", po::value<Action>(&action),
        "Provide action (range_pod, ot_range_pod, vrf_query, ot_vrf_query...)")(
        "publish_dir,p", po::value<std::string>(&publish_dir),
        "Provide the publish dir")("output_dir,o",
                                   po::value<std::string>(&output_dir),
                                   "Provide the output dir")(
        "demand_ranges",
        po::value<std::vector<Range>>(&demand_ranges)->multitoken(),
        "Provide the demand ranges")(
        "phantom_ranges",
        po::value<std::vector<Range>>(&phantom_ranges)->multitoken(),
        "Provide the phantom range(plain mode)")(
        "query_key,k", po::value<std::string>(&query_key),
        "Provide the query key name(table mode)")(
        "key_value,v",
        po::value<std::vector<std::string>>(&query_values)->multitoken(),
        "Provide the query key values(table mode, for example "
        "-v value_a value_b value_c)")(
        "phantom_key,n",
        po::value<std::vector<std::string>>(&phantom_values)->multitoken(),
        "Provide the query key phantoms(table mode, for example -n "
        "phantoms_a phantoms_b phantoms_c)")
#ifdef MULTICORE
        (
            "omp_thread_num", po::value<uint32_t>(&omp_thread_num),
            "Provide the number of the openmp thread, 1: disable "
            "openmp, 0: default.")
#endif
            ("use_c_api,c", "")("test_evil", "")(
                "dump_ecc_pub", "");

    boost::program_options::variables_map vmap;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, options), vmap);
    boost::program_options::notify(vmap);

    if (vmap.count("help"))
    {
      std::cerr << options << std::endl;
      return -1;
    }

    if (vmap.count("use_c_api"))
    {
      use_capi = true;
    }

    if (vmap.count("test_evil"))
    {
      test_evil = true;
    }

    if (vmap.count("dump_ecc_pub"))
    {
      dump_ecc_pub = true;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "Unknown parameters.\n"
              << e.what() << "\n"
              << "-h or --help to list all arguments.\n";
    return -1;
  }

#ifdef MULTICORE
  if (omp_thread_num)
  {
    std::cout << "omp_set_num_threads: " << omp_thread_num << "\n";
    omp_set_num_threads(omp_thread_num);
  }
  std::cout << "omp_get_max_threads: " << omp_get_max_threads() << "\n";
#endif

  if (!InitAll(data_dir))
  {
    std::cerr << "Init failed\n";
    return -1;
  }

  if (dump_ecc_pub)
  {
    DumpEccPub();
    return 0;
  }

  if (output_dir.empty())
  {
    std::cerr << "Want output_dir(-o)" << std::endl;
    return -1;
  }

  if (publish_dir.empty() || !fs::is_directory(publish_dir))
  {
    std::cerr << "Open publish_dir " << publish_dir << " failed\n";
    return -1;
  }

  if (!fs::is_directory(output_dir) && !fs::create_directories(output_dir))
  {
    std::cerr << "Create " << output_dir << " failed\n";
    return -1;
  }

  // clean the output_dir
  for (auto &entry :
       boost::make_iterator_range(fs::directory_iterator(output_dir), {}))
  {
    fs::remove_all(entry);
  }

  if (mode == Mode::kPlain)
  {
    if (action == Action::kVrfQuery || action == Action::kOtVrfQuery ||
        action == Action::kVrfPod || action == Action::kOtVrfPod)
    {
      std::cerr << "Plain mode does not support vrf query action\n";
      return -1;
    }
  }

  if (action == Action::kVrfQuery)
  {
    auto func =
        use_capi ? scheme::table::vrfq::capi::Test : scheme::table::vrfq::Test;
    return func(publish_dir, output_dir, query_key, query_values) ? 0 : -1;
  }

  if (action == Action::kOtVrfQuery)
  {
    auto func = use_capi ? scheme::table::ot_vrfq::capi::Test
                         : scheme::table::ot_vrfq::Test;
    return func(publish_dir, output_dir, query_key, query_values,
                phantom_values)
               ? 0
               : -1;
  }

  if (action == Action::kComplaintPod)
  {
    decltype(scheme::table::complaint::Test) *func;
    if (mode == Mode::kPlain)
    {
      func = use_capi ? scheme::plain::complaint::capi::Test
                      : scheme::plain::complaint::Test;
    }
    else
    {
      func = use_capi ? scheme::table::complaint::capi::Test
                      : scheme::table::complaint::Test;
    }
    return func(publish_dir, output_dir, demand_ranges, test_evil) ? 0 : -1;
  }

  if (action == Action::kOtComplaintPod)
  {
    decltype(scheme::table::ot_complaint::Test) *func;
    if (mode == Mode::kPlain)
    {
      func = use_capi ? scheme::plain::ot_complaint::capi::Test
                      : scheme::plain::ot_complaint::Test;
    }
    else
    {
      func = use_capi ? scheme::table::ot_complaint::capi::Test
                      : scheme::table::ot_complaint::Test;
    }
    return func(publish_dir, output_dir, demand_ranges, phantom_ranges,
                test_evil)
               ? 0
               : -1;
  }

  if (action == Action::kAtomicSwapPod)
  {
    decltype(scheme::table::atomic_swap::Test) *func;
    if (mode == Mode::kPlain)
    {
      func = use_capi ? scheme::plain::atomic_swap::capi::Test
                      : scheme::plain::atomic_swap::Test;
    }
    else
    {
      func = use_capi ? scheme::table::atomic_swap::capi::Test
                      : scheme::table::atomic_swap::Test;
    }
    return func(publish_dir, output_dir, demand_ranges, test_evil) ? 0 : -1;
  }

  if (action == Action::kAtomicSwapPodVc)
  {
    decltype(scheme::table::atomic_swap_vc::Test) *func;
    if (mode == Mode::kPlain)
    {
      func = use_capi ? scheme::plain::atomic_swap_vc::capi::Test
                      : scheme::plain::atomic_swap_vc::Test;
    }
    else
    {
      func = use_capi ? scheme::table::atomic_swap_vc::capi::Test
                      : scheme::table::atomic_swap_vc::Test;
    }
    return func(publish_dir, output_dir, demand_ranges, test_evil) ? 0 : -1;
  }

  std::cerr << "Not implement yet.\n";
  return -1;
}
