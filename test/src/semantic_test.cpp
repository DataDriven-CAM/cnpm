#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#define DOCTEST_CONFIG_USE_STD_HEADERS // Add this line
#include <doctest/doctest.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <meta>
#include <future>
#include <execution>
#include <thread>
#include <utility>

#include "npm/utils/SemanticVersioning.h"

TEST_SUITE("semantics"){

TEST_CASE("test a set"){
    sylvanmats::npm::utils::SemanticVersioning sv;
    CHECK_EQ(sv.operator()("1.0.0", [](std::string_view base, std::string_view branch, std::string_view wildcard){
        std::cout<<"base: "<<base<<" wildcard: "<<wildcard<<std::endl;
    }), true);
    CHECK(sylvanmats::npm::utils::parse_version("1.2.3") == sylvanmats::npm::utils::parse_version("v1.2.3"));
    CHECK(sylvanmats::npm::utils::parse_version("2.0.0") > sylvanmats::npm::utils::parse_version("1.2.3"));
    
    // Section 11: Pre-release validation priorities
    CHECK(sylvanmats::npm::utils::parse_version("1.0.0-alpha") < sylvanmats::npm::utils::parse_version("1.0.0"));
    CHECK(sylvanmats::npm::utils::parse_version("1.0.0-alpha.1") < sylvanmats::npm::utils::parse_version("1.0.0-alpha.beta"));
    CHECK(sylvanmats::npm::utils::parse_version("1.0.0-beta.2") < sylvanmats::npm::utils::parse_version("1.0.0-beta.11")); // Numeric check!
    CHECK(sylvanmats::npm::utils::parse_version("1.0.0-rc.1") > sylvanmats::npm::utils::parse_version("1.0.0-beta.11"));
    
    // Section 10: Build metadata ignorance rule
    CHECK(sylvanmats::npm::utils::parse_version("1.0.0+sha.1") == sylvanmats::npm::utils::parse_version("1.0.0+sha.2"));

    // Tilde Rules (~): Lock minor, float patches
    CHECK(sylvanmats::npm::utils::evaluate_range_bounds("~", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("1.2.3")));
    CHECK(sylvanmats::npm::utils::evaluate_range_bounds("~", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("1.2.4")));
    CHECK(!sylvanmats::npm::utils::evaluate_range_bounds("~", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("1.3.0")));
    CHECK(!sylvanmats::npm::utils::evaluate_range_bounds("^", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("2.0.0")));
    CHECK(sylvanmats::npm::utils::evaluate_range_bounds("^", sylvanmats::npm::utils::parse_version("0.2.3")).satisfies(sylvanmats::npm::utils::parse_version("0.2.5")));
    CHECK(!sylvanmats::npm::utils::evaluate_range_bounds("^", sylvanmats::npm::utils::parse_version("0.3.0")).satisfies(sylvanmats::npm::utils::parse_version("0.2.3")));
    CHECK(!sylvanmats::npm::utils::evaluate_range_bounds("^", sylvanmats::npm::utils::parse_version("0.0.4")).satisfies(sylvanmats::npm::utils::parse_version("0.0.3")));

    CHECK(sylvanmats::npm::utils::evaluate_range_bounds(">", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("1.2.4")));
    CHECK(!sylvanmats::npm::utils::evaluate_range_bounds(">", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("1.2.3")));
    CHECK(sylvanmats::npm::utils::evaluate_range_bounds(">=", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("1.2.3")));
    CHECK(sylvanmats::npm::utils::evaluate_range_bounds("<=", sylvanmats::npm::utils::parse_version("1.2.3")).satisfies(sylvanmats::npm::utils::parse_version("1.2.2")));
    CHECK_EQ(sv.operator()("DataDriven-CAM/json-thresher", [](std::string_view base, std::string_view branch, std::string_view wildcard){
        std::cout<<"base: "<<base<<" wildcard: "<<wildcard<<std::endl;
    }), false);
}
}
