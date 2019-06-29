#include <algorithm>
#include <array>
#include <bitset>
#include <vector>

#include "ecc_pub.h"
#include "public.h"
#include "publish.h"
#include "scheme_misc.h"

int main(int argc, char **argv)
{
  setlocale(LC_ALL, "");

  using scheme::Mode;
  using scheme::table::Type;

  Mode task_mode;
  std::string publish_file;
  std::string output_dir;
  Type table_type;
  std::vector<uint64_t> vrf_colnum_index;
  std::vector<bool> unique_key;
  uint64_t column_num;
  std::string data_dir;
#ifdef MULTICORE
  uint32_t omp_thread_num;
#endif

  try
  {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "-d data_dir -m table -f file -o output_dir -t table_type -k keys",
        "publish table file")(
        "-d data_dir -m plain -f file -o output_dir -c column_num",
        "publish plain file")(
        "data_dir,d", po::value<std::string>(&data_dir)->default_value("."),
        "Provide the configure file dir")(
        "mode,m", po::value<Mode>(&task_mode)->default_value(Mode::kPlain),
        "Provide pod mode (plain, table)")(
        "publish_file,f",
        po::value<std::string>(&publish_file)->default_value(""),
        "Provide the file which want to publish")(
        "output_dir,o",
        po::value<std::string>(&output_dir)->default_value(""),
        "Provide the publish path")(
        "table_type,t", po::value<Type>(&table_type)->default_value(Type::kCsv),
        "Provide the publish file type in table mode (csv)")(
        "column_num,c", po::value<uint64_t>(&column_num)->default_value(1023),
        "Provide the column number per block(line) in "
        "plain mode (default 1023)")(
        "vrf_colnum_index,k",
        po::value<std::vector<uint64_t>>(&vrf_colnum_index)->multitoken(),
        "Provide the publish file vrf key column index "
        "positions in table mode (for example: -k 0 1 3)")(
        "unique_key,u", po::value<std::vector<bool>>(&unique_key)->multitoken(),
        "Provide the flag if publish must unique the key"
        " in table mode (for example: -u 1 0 1)")
#ifdef MULTICORE
        ("omp_thread_num", po::value<uint32_t>(&omp_thread_num)->default_value(0),
         "Provide the number of the openmp thread, 1: disable openmp, 0: "
         "default.")
#endif
        ;

    boost::program_options::variables_map vmap;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, options), vmap);
    boost::program_options::notify(vmap);

    if (vmap.count("help"))
    {
      std::cout << options << std::endl;
      return -1;
    }

    if (output_dir.empty())
    {
      std::cout << "Want output_dir(-o)\n";
      std::cout << options << std::endl;
      return -1;
    }

    if (publish_file.empty() || !fs::is_regular(publish_file))
    {
      std::cout << "Open publish_file " << publish_file << " failed\n";
      std::cout << options << std::endl;
      return -1;
    }

    if (fs::file_size(publish_file) == 0)
    {
      std::cout << "The file size of " << publish_file << " is 0\n";
      std::cout << options << std::endl;
      return -1;
    }

    if (!fs::is_directory(output_dir) &&
        !fs::create_directories(output_dir))
    {
      std::cout << "Create " << output_dir << " failed\n";
      std::cout << options << std::endl;
      return -1;
    }

    if (task_mode == Mode::kPlain)
    {
      if (column_num == 0)
      {
        std::cout << "column_num can not be 0.\n";
        std::cout << options << std::endl;
        return -1;
      }
    }
    else
    {
      std::sort(vrf_colnum_index.begin(), vrf_colnum_index.end());
      vrf_colnum_index.erase(
          std::unique(vrf_colnum_index.begin(), vrf_colnum_index.end()),
          vrf_colnum_index.end());

      if (vrf_colnum_index.empty())
      {
        std::cout << "Want vrf_colnum_index in table mode.\n";
        std::cout << options << std::endl;
        return -1;
      }
      unique_key.resize(vrf_colnum_index.size());
    }
  }
  catch (std::exception &e)
  {
    std::cout << "Unknown parameters.\n"
              << e.what() << "\n"
              << "-h or --help to list all arguments.\n";
    return -1;
  }

#ifdef MULTICORE
  if (omp_thread_num)
  {
    std::cout << "set openmp threadnum: " << omp_thread_num << "\n";
    omp_set_num_threads(omp_thread_num);
  }
#endif

  InitEcc();

  std::string ecc_pub_file = data_dir + "/" + "ecc_pub.bin";
  if (!OpenOrCreateEccPub(ecc_pub_file))
  {
    std::cerr << "Open or create ecc pub file " << ecc_pub_file << " failed\n";
    return -1;
  }

  bool ret;
  switch (task_mode)
  {
  case Mode::kPlain:
  {
    ret = PublishPlain(std::move(publish_file), std::move(output_dir),
                       column_num);
    break;
  }
  case Mode::kTable:
  {
    ret = PublishTable(std::move(publish_file), std::move(output_dir),
                       std::move(table_type), std::move(vrf_colnum_index),
                       std::move(unique_key));
    break;
  }
  default:
    throw std::runtime_error("never reach");
  }

  if (ret)
  {
    std::cout << "publish success." << std::endl;
  }
  else
  {
    std::cout << "publish failed." << std::endl;
  }
  return ret ? 0 : -1;
}
