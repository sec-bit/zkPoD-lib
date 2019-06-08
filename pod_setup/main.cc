#include "ecc_pub.h"
#include "public.h"

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");
  std::string output_file;
  uint64_t u1_size;
  uint64_t u2_size;
  uint32_t omp_thread_num;

  try {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "output,o", po::value<std::string>(&output_file)->default_value(""),
        "Provide the output ecc pub file")(
        "u1_size,a", po::value<uint64_t>(&u1_size)->default_value(1026),
        "Provide the max number of u1")(
        "u2_size,b", po::value<uint64_t>(&u2_size)->default_value(2),
        "Provide the max number of u2")(
        "omp_thread_num,t",
        po::value<uint32_t>(&omp_thread_num)->default_value(0),
        "Provide the number of the openmp thread, 1: disable openmp, 0: "
        "default.");

    boost::program_options::variables_map vmap;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, options), vmap);
    boost::program_options::notify(vmap);

    if (vmap.count("help")) {
      std::cout << options << std::endl;
      return -1;
    }

    if (output_file.empty()) {
      std::cout << "Want output_file(-o)\n";
      std::cout << options << std::endl;
      return -1;
    }

    if (u1_size <= 4) {
      std::cout << "u1_size must greater than 4." << std::endl;
      return -1;
    }

    if (u2_size <= 1) {
      std::cerr << "u2_size must greater than 1." << std::endl;
      return -1;
    }
  } catch (std::exception& e) {
    std::cout << "Unknown parameters.\n"
              << e.what() << "\n"
              << "-h or --help to list all arguments.\n";
    return -1;
  }

  if (omp_thread_num) {
#ifndef __clang__
    std::cout << "set openmp threadnum: " << omp_thread_num << "\n";
    omp_set_num_threads(omp_thread_num);
#endif
  }

  InitEcc();

  try {
    EccPub ecc_pub(u1_size, u2_size);
    if (!ecc_pub.Save(output_file)) {
      std::cerr << "Save data failed." << std::endl;
      return -1;
    }

    EccPub ecc_pub2(output_file);

    if (ecc_pub != ecc_pub2) {
      std::cerr << "Oops!" << std::endl;
      return -1;
    }
    std::cout << "Success." << std::endl;
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return -1;
  }
}
