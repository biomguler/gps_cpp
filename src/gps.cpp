#include <gps.hpp>
#include <Eigen/Dense>
#include <fastCDFOnSample.h>
#include <boost/math/distributions/empirical_cumulative_distribution_function.hpp>
#include <boost/random.hpp>
#include <boost/range/algorithm/random_shuffle.hpp>
#include <math.h>
#include <map>
#include <omp.h>
#include <numeric>

using namespace Eigen;
using boost::math::empirical_cumulative_distribution_function;
using namespace std;

namespace gps {

  vector<double> ecdf(vector<double> reference) {
    vector<double> refCopy = reference;

    size_t n = reference.size();

    sort(refCopy.begin(), refCopy.end());

    vector<double> estEcdf;

    for(int i = 0; i < n; ++i) {
      estEcdf.push_back((double) (upper_bound(refCopy.begin(), refCopy.end(), reference[i])-refCopy.begin())/n);
    }

    return estEcdf;
  }

  double gpsStat(vector<double> u, vector<double> v, function<vector<double>(const vector<double>&, const vector<double>&)> bivariateEcdf, function<double (const double&, const double&, const double&)> weightFunction) {
    if(u.size() != v.size()) {
      throw invalid_argument("Size of u and v differs.");
    }

    size_t n = u.size();

    if(distance(u.begin(), max_element(u.begin(), u.end())) == distance(
    v.begin(), max_element(v.begin(), v.end()))) {
        throw invalid_argument("Indices of largest elements of u and v coincide. GPS statistic is undefined in this case.");
      }

    vector<double> uEcdf = ecdf(u);
    vector<double> vEcdf = ecdf(v);

    vector<double> bivariateEcdfVec = bivariateEcdf(u, v);

    double max = -numeric_limits<double>::max();

    for(int i = 0; i < n; ++i) {
      double cdf_uv = bivariateEcdfVec[i];
      double cdf_u = uEcdf[i];
      double cdf_v = vEcdf[i];
      double denom = sqrt(cdf_u*cdf_v - pow(cdf_u, 2.)*pow(cdf_v, 2.));

      double numerator = abs(cdf_uv - cdf_u*cdf_v);

      double maximand = sqrt((double) n / log((double) n))*numerator/denom;

      if(maximand > max) max = maximand;
    }

  return max;
}

  double meanStat(vector<double> u, vector<double> v, function<vector<double>(const vector<double>&, const vector<double>&)> bivariateEcdf, function<double (const double&, const double&, const double&)> weightFunction) {
    if(u.size() != v.size()) {
      throw invalid_argument("Size of u and v differs.");
    }

    size_t n = u.size();

    vector<double> uEcdf = ecdf(u);
    vector<double> vEcdf = ecdf(v);

    vector<double> bivariateEcdfVec = bivariateEcdf(u, v);

    double numerator_sum = 0;
    double denominator_sum = 0;

    for(int i = 0; i < n; ++i) {
      double cdf_uv = bivariateEcdfVec[i];
      double cdf_u = uEcdf[i];
      double cdf_v = vEcdf[i];
      double weight = weightFunction(cdf_u, cdf_v, cdf_uv);

      double numerator = weight*pow(cdf_uv - cdf_u*cdf_v, 2.);

      numerator_sum += numerator;
      denominator_sum += weight;
    }

  return numerator_sum/denominator_sum;
}

  vector<double> bivariateEcdfLW(const vector<double>& u, const vector<double>& v) {
    if(u.size() != v.size()) {
      throw invalid_argument("Size of u and v differs.");
    }

    size_t n = u.size();

    ArrayXd toAdd = ArrayXd::Constant(n, 1.);

    // NB: XXd for two-dimensional and double entries
    ArrayXXd ptr(2, n);

    for(int i = 0; i < n; i++) {
      ptr(0, i) = u[i];
      ptr(1, i) = v[i];
    }

    ArrayXd ecdf_arr = StOpt::fastCDFOnSample(ptr, toAdd);

    return vector<double>(ecdf_arr.data(), ecdf_arr.data() + ecdf_arr.size());
}

  vector<double> bivariateEcdfPar(const vector<double>& u, const vector<double>& v) {
    if(u.size() != v.size()) {
      throw invalid_argument("Size of u and v differs.");
    }

    size_t n = u.size();

    vector<double> ecdf;

    ecdf.reserve(n);

    #pragma omp parallel for
    for(size_t i = 0; i < n; ++i) {
        int count = 0;

        for(size_t j = 0; j < n; ++j) {
          if(u[j] <= u[i] && v[j] <= v[i]) {
            ++count;
          }
        }

        ecdf[i] = (double) count/n;
      }

    return ecdf;
  }

  vector<double> permuteAndSampleStat(
                                          vector<double> u,
                                          vector<double> v,
                                          size_t n,
                                          function<double (vector<double>, vector<double>, function<vector<double>(const vector<double>&, const vector<double>&)>, function<double (const double&, const double&, const double&)>)> stat,
                                          function<vector<double>(const vector<double>&, const vector<double>&)> bivariateEcdf,
                                          function<double (const double&, const double&, const double&)> weightFunction) {
    vector<double> sample;

    size_t i = 0;

    while(i < n) {
      boost::range::random_shuffle(v);

      if(distance(u.begin(), max_element(u.begin(), u.end())) != distance(v.begin(), max_element(v.begin(), v.end()))) {
        sample.push_back(stat(u, v, bivariateEcdf, weightFunction));
        ++i;
      }

    }

    return sample;
  }

  vector<double> perturbDuplicates_addEpsilon(vector<double> values, double multiple) {
    map<double, int> freqMap;

    for(size_t i = 0; i < values.size(); ++i) {
      freqMap[values[i]]++;

      if(freqMap[values[i]] > 1) {
        double candidate_replacement = values[i] + (double) (freqMap[values[i]])* multiple*numeric_limits<double>::epsilon();
        //double candidate_replacement = nextafter(values[i], 1.0);

        while((0.5 * (candidate_replacement + values[i])) == values[i]) {
          cout << "Replacement is equal, incrementing" << endl;
          //candidate_replacement = nextafter(values[i], 1.0);
          candidate_replacement += (freqMap[values[i]])*multiple*numeric_limits<double>::epsilon();
        }

        values[i] = candidate_replacement;
      }
    }

    return values;
  }

  map<double, int> returnFreqMap(vector<double> values) {
    map<double, int> freqMap;

    for(size_t i = 0; i < values.size(); ++i){
      freqMap[values[i]]++;
    }

    return freqMap;
  }

  double gpsWeight(double cdf_u, double cdf_v, double cdf_uv) {
    return 1./sqrt(cdf_u*cdf_v - pow(cdf_u, 2.)*pow(cdf_v, 2.));
  }

  double pseudoADWeight(double cdf_u, double cdf_v, double cdf_uv) {
    return 1./cdf_uv*(1-cdf_uv);
  }

  double squareNumerator(double cdf_u, double cdf_v, double cdf_uv) {
    return pow(cdf_uv - cdf_u*cdf_v, 2.);
  }

  double normNumerator(double cdf_u, double cdf_v, double cdf_uv) {
    return abs(cdf_uv - cdf_u*cdf_v);
  }
}
