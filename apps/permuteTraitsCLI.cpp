#include <iostream>
#include <gps.hpp>
#include <boost/program_options.hpp>
#include <rapidcsv.h>
#include <omp.h>
#include <PPEcdf.hpp>

namespace po = boost::program_options;
using namespace po;
using namespace gps;
using namespace rapidcsv;

int main(int argc, const char* argv[]) {
  po::options_description desc("Allowed options");

  std::string inputFile;
  std::string outputFile;
  std::string columnA;
  std::string columnB;
  int perturbN = 0;
  double epsilonMultiple = 2.0;
  int cores;
  int draws;

  desc.add_options()
    ("help", "Print help message")
    ("inputFile,i", po::value<std::string>(&inputFile), "Path to input file")
    ("outputFile,o", po::value<std::string>(&outputFile), "Path to output file")
    ("columnA,a", po::value<std::string>(&columnA), "Label of column A")
    ("columnB,b", po::value<std::string>(&columnB), "Label of column B")
    ("perturbN,p", po::value<int>(&perturbN), "No. of perturbation iterations")
    ("epsilonMultiple,e", po::value<double>(&epsilonMultiple), "Multiple of epsilon to use in perturbation procedure")
    ("cores,c", po::value<int>(&cores), "No. of cores")
    ("draws,n", po::value<int>(&draws), "No. of draws")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if(vm.count("inputFile")) {
    Document input(inputFile, LabelParams(), SeparatorParams('\t'));

    std::vector<double> u = input.GetColumn<double>(columnA);
    std::vector<double> v = input.GetColumn<double>(columnB);

    for(size_t i = 0; i < perturbN; ++i) {
      u = perturbDuplicates_addEpsilon(u, epsilonMultiple);
      v = perturbDuplicates_addEpsilon(v, epsilonMultiple);
    }

    std::vector<std::vector<double>> gpsPermutations;

    int drawsPerCore = draws / cores;

    omp_set_num_threads(cores);

    #pragma omp parallel for
    for(int k = 0; k < cores; ++k) {
      gpsPermutations.push_back(permuteAndSampleGps(u, v, drawsPerCore, &PPEcdf::bivariatePPEcdf));
    }

    std::stringstream stringOutput;

    stringOutput << "GPS" << std::endl;

    for(int i = 0; i < cores; ++i) {
      for(int j = 0; j < drawsPerCore; ++j) {
        stringOutput << gpsPermutations[i][j] << std::endl;
      }
    }

    Document output(stringOutput, LabelParams(), SeparatorParams('\t'));
    output.Save(outputFile);
  } else {
      std::cout << desc << std::endl;
  }

  return 0;
}
