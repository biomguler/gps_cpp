#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <PPEcdf.hpp>
#include <gps.hpp>
#include <iostream>
#include <rapidcsv.h>

using namespace rapidcsv;
using namespace gps;
using namespace Catch;
using namespace PPEcdf;

TEST_CASE( "idxSort sorts simple data", "[idxSort]" ) {
  std::vector<double> v({0.5, 0.4, 0.3, 0.2, 0.1});
  std::vector<size_t> idx({4, 3, 2, 1, 0});

  std::vector<size_t> sortedIdx = idxSort(v);

  REQUIRE_THAT(
               sortedIdx,
               Matchers::Equals(std::vector<size_t>{4, 3, 2, 1, 0}));
}

TEST_CASE( "idxSort sorts sorted simple data", "[idxSort]" ) {
  std::vector<double> v({0.1, 0.2, 0.3, 0.4, 0.5});
  std::vector<size_t> idx({0, 1, 2, 3, 4});

  std::vector<size_t> sortedIdx = idxSort(v);

  REQUIRE_THAT(
               sortedIdx,
               Matchers::Equals(idx));
}

TEST_CASE( "idxSort sorts simple duplicated data", "[idxSort]" ) {
  std::vector<double> v({0.4, 0.4, 0.4});
  std::vector<size_t> idx({0, 1, 2});

  std::vector<size_t> sortedIdx = idxSort(v);

  REQUIRE_THAT(
               sortedIdx,
               Matchers::Equals(idx));
}

TEST_CASE( "idxSort sorts triplet of doubles", "[idxSort]" ) {
  std::vector<double> v({0.2, 0.1, 0.3,});
  std::vector<size_t> idx({1, 0, 2});

  std::vector<size_t> sortedIdx = idxSort(v);

  REQUIRE_THAT(
               sortedIdx,
               Matchers::Equals(idx));
}

TEST_CASE( "idxSort sorts simple data with duplicates", "[idxSort]" ) {
  std::vector<double> v({0.4, 0.1, 0.3, 0.2});
  std::vector<size_t> idx({1, 3, 2, 0});

  std::vector<size_t> sortedIdx = idxSort(v);

  std::vector<double> ridx = reindex(v, sortedIdx);

  REQUIRE_THAT(
               idx,
               Matchers::Equals(sortedIdx));
}

TEST_CASE( "idxSort sorts simple data with all duplicates but one", "[idxSort]" ) {
  std::vector<double> v({.4, .4, .4, .4, .1});
  std::vector<size_t> idx({4, 0, 1, 2, 3});

  std::vector<size_t> sortedIdx = idxSort(v);

  REQUIRE_THAT(
               sortedIdx,
               Matchers::Equals(idx));
}

TEST_CASE( "reindex sorts simple data", "[reindex]" ) {
  std::vector<double> v({.5, .4, .3, .2, .1});
  std::vector<size_t> idx({4, 3, 2, 1, 0});
  std::vector<double> reindexed_v({.1, .2, .3, .4, .5});

  std::vector<double> reindexedV = reindex(v, idx);

  REQUIRE_THAT(
               reindexedV,
               Matchers::Approx(reindexed_v));
}

// NB: runif with set.seed(42)
TEST_CASE( "reindex sorts two vectors", "[reindex]" ) {
  std::vector u({0.9148060, 0.9370754, 0.2861395, 0.8304476, 0.6417455, 0.5190959, 0.7365883, 0.1346666, 0.6569923, 0.7050648});
  std::vector v({0.4577418, 0.7191123, 0.9346722, 0.2554288, 0.4622928, 0.9400145, 0.9782264, 0.1174874, 0.4749971, 0.5603327});

  std::vector sorted_u({0.1346666, 0.2861395, 0.5190959, 0.6417455, 0.6569923, 0.7050648, 0.7365883, 0.8304476, 0.9148060, 0.9370754});
  std::vector sorted_v({0.1174874, 0.9346722, 0.9400145, 0.4622928, 0.4749971, 0.5603327, 0.9782264, 0.2554288, 0.4577418, 0.7191123});

  std::vector<size_t> u_sorted_idx({7, 2, 5, 4, 8, 9, 6, 3, 0, 1});

  std::vector<size_t> idx = idxSort(u);

  REQUIRE_THAT(
               u_sorted_idx,
               Matchers::Equals(idx));

  std::vector<double> u_sorted = reindex(u, idx);
  std::vector<double> v_sorted = reindex(v, idx);

  REQUIRE_THAT(
               sorted_u,
               Matchers::Equals(u_sorted));

  REQUIRE_THAT(
               sorted_v,
               Matchers::Equals(v_sorted));
}


TEST_CASE( "ecdf estimates ecdf correctly on simple data set", "[ecdf]" ) {
  std::vector u({.1, .2, .3, .4, .5});

  std::vector<double> ecdfResult = ecdf(u);

  REQUIRE_THAT(
               ecdfResult,
               Matchers::Approx(std::vector<double>{.2, .4, .6, .8, 1.})
               );
}

TEST_CASE( "ecdf estimates ecdf correctly on unsorted simple data set", "[ecdf]" ) {
  std::vector u({.1, .2, .5, .4, .3});

  std::vector<double> ecdfResult = ecdf(u);

  REQUIRE_THAT(
               ecdfResult,
               Matchers::Approx(std::vector<double>{.2, .4, 1., .8, .6})
               );
}

TEST_CASE( "bivariatePPEcdf computes the ecdf for simple data", "[bivariatePPEcdf]" ) {
  std::vector<double> v({.5, .4, .3, .2, .1});
  std::vector<double> u({.5, .4, .3, .2, .1});

  std::vector<double> ecdf = bivariatePPEcdf(u, v);

  REQUIRE_THAT(
               ecdf,
               Matchers::Approx(std::vector<double>{1, .8, .6, .4, .2}));
}


TEST_CASE( "bivariatePPEcdf computes the ecdf for less simple data", "[bivariatePPEcdf]" ) {
  std::vector u({.1, .2, .3, .4, .5});
  std::vector v({.1, .2, .3, .5, .4});

  std::vector<double> ecdf = bivariatePPEcdf(u, v);

  REQUIRE_THAT(
               ecdf,
               Matchers::Approx(std::vector<double>{.2, .4, .6, .8, .8}));
}


TEST_CASE( "bivariatePPEcdf computes the ecdf for less simple data (II)", "[bivariatePPEcdf]" ) {
  std::vector u({.1, .2, .3, .4, .5});
  std::vector v({.5, .4, .3, .2, .1});

  std::vector<double> ecdf = bivariatePPEcdf(u, v);

  REQUIRE_THAT(
               ecdf,
               Matchers::Approx(std::vector<double>{.2, .2, .2, .2, .2}));
}


TEST_CASE( "bivariatePPEcdf computes the ecdf for less simple data (III)", "[bivariatePPEcdf]" ) {
  std::vector u({0.9148060, 0.9370754, 0.2861395, 0.8304476, 0.6417455, 0.5190959, 0.7365883, 0.1346666, 0.6569923, 0.7050648});
  std::vector v({0.4577418, 0.7191123, 0.9346722, 0.2554288, 0.4622928, 0.9400145, 0.9782264, 0.1174874, 0.4749971, 0.5603327});

  std::vector<double> ecdf = bivariatePPEcdf(u, v);
  std::vector<double> ecdf_res({0.3, 0.7, 0.2, 0.2, 0.2, 0.3, 0.7, 0.1, 0.3, 0.4});

  REQUIRE_THAT(
               ecdf,
               Matchers::Approx(ecdf_res));
}


TEST_CASE( "PP bivariate ecdf computes correct ecdf for uniformly distributed data", "[bivariatePPEcdf]" ) {

  Document data("test/data/1e3_unif.csv", LabelParams(), SeparatorParams(','));
  Document exemplar("test/data/1e3_unif_ecdf.csv", LabelParams(), SeparatorParams(','));

  std::vector<double> u = data.GetColumn<double>("u");
  std::vector<double> v = data.GetColumn<double>("v");
  std::vector<double> ecdfExemplar = exemplar.GetColumn<double>("ecdf");

  std::vector<double> ecdf = bivariatePPEcdf(u, v);

  REQUIRE_THAT(
              ecdf,
              Matchers::Approx(ecdfExemplar)
              );
}

TEST_CASE( "gps computes correct statistic on uniformly distributed data", "[gps]" ) {

  Document doc("test/data/1e3_unif.csv");

  std::vector<double> u = doc.GetColumn<double>("u");
  std::vector<double> v = doc.GetColumn<double>("v");

  double gpsResult = gpsStat(u,v, &bivariatePPEcdf, &gpsWeight);

  REQUIRE(gpsResult == Detail::Approx(0.87333));
}

TEST_CASE( "gpsStat runs in simple case", "[gpsStat]" ) {
  std::vector u({.1, .2, .3, .4, .5});
  std::vector v({.1, .2, .3, .5, .4});

  double gpsResult = gpsStat(u,v, &bivariatePPEcdf, &gpsWeight);

  REQUIRE(gpsResult == Detail::Approx(1.439137));
}


TEST_CASE( "Naive bivariate ecdf runs in simple case", "[ecdf]" ) {
  std::vector u({.1, .2, .3, .4, .5});
  std::vector v({.1, .2, .3, .5, .4});

  std::vector<double> ecdfResult = bivariateEcdfPar(u,v);

  for(auto x: ecdfResult) std::cerr << x << std::endl;

  REQUIRE_THAT(
               ecdfResult,
               Matchers::Approx(std::vector<double>{.2, .4, .6, .8, .8})
               );
}

TEST_CASE( "PP bivariate ecdf throws exception with differently-sized input vectors", "[ecdf]") {

  REQUIRE_THROWS_MATCHES(
                         bivariatePPEcdf(std::vector<double>{.1, .2}, std::vector<double>{.1, .3, .05}),
                         std::invalid_argument,
                         Message("Size of u and v differs.")
                         );
}

TEST_CASE( "Naive bivariate ecdf computes correct ecdf for uniformly distributed data", "[ecdf]" ) {

  Document data("test/data/1e3_unif.csv");
  Document exemplar("test/data/1e3_unif_ecdf.csv");

  std::vector<double> u = data.GetColumn<double>("u");
  std::vector<double> v = data.GetColumn<double>("v");
  std::vector<double> ecdfExemplar = exemplar.GetColumn<double>("ecdf");

  std::vector<double> ecdf = bivariateEcdfPar(u,v);

  REQUIRE_THAT(
               ecdf,
               Matchers::Approx(ecdfExemplar)
               );

}


TEST_CASE( "Naive bivariate ecdf throws exception with differently-sized input vectors", "[ecdf]") {
  REQUIRE_THROWS_MATCHES(
                         bivariateEcdfPar(std::vector<double>{.1, .2}, std::vector<double>{.1, .3, .05}),
                         std::invalid_argument,
                         Message("Size of u and v differs.")
                         );
}

TEST_CASE( "gps throws exception with differently-sized input vectors", "[ecdf]") {

  REQUIRE_THROWS_MATCHES(
                         gpsStat(std::vector<double>{.1, .2}, std::vector<double>{.1, .3, .05}, &bivariatePPEcdf, &gpsWeight),
                         std::invalid_argument,
                         Message("Size of u and v differs.")
                         );
}

TEST_CASE( "gps throws exception on undefined case", "[ecdf]") {

  REQUIRE_THROWS_MATCHES(
                         gpsStat(std::vector<double>{.1, .2, .3}, std::vector<double>{.1, .2, .3}, &bivariatePPEcdf, &gpsWeight),
                         std::invalid_argument,
                         Message("Indices of largest elements of u and v coincide. GPS statistic is undefined in this case.")
                 );
}

TEST_CASE( "perturbDuplicates distinguishes duplicates in simple case", "[perturbDuplicates]") {

  std::vector<double> u({.1,.1,.1,.2,.3,.4,.5});

  std::vector<double> noDup = perturbDuplicates_addEpsilon(u, 1.0);

  REQUIRE(noDup[0] == .1);
  REQUIRE(noDup[0] != noDup[1]);
  REQUIRE(noDup[0] != noDup[2]);
}
