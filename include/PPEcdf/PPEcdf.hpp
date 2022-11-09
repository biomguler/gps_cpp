#pragma once

#include <vector>

namespace PPEcdf {
  std::vector<double> bivariatePPEcdf(std::vector<double> u, std::vector<double> v);
  std::vector<size_t> idxSort(const std::vector<double>& v);
  std::vector<double> reindex(const std::vector<double>& v, const std::vector<size_t>& idx);
}
