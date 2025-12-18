#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "interlaced_core/filesystem.hpp"
#include "interlaced_core/json.hpp"
#include "interlaced_core/logging.hpp"
#include "interlaced_core/network.hpp"

TEST_CASE("sanity: arithmetic") {
    CHECK(1 + 1 == 2);
}

TEST_CASE("sanity: filesystem basics") {
    CHECK(interlaced::core::filesystem::FileSystem::exists("/nonexistent") == false);
    CHECK(interlaced::core::filesystem::FileSystem::exists("/tmp") == true); // Assuming /tmp exists on Unix-like systems
}

TEST_CASE("sanity: json basics") {
    std::string json_str = R"({"key": "value", "number": 42})";
    auto json = interlaced::core::json::JSON::parse_or_throw(json_str);
    CHECK(json["key"].as_string() == "value");
    CHECK(json["number"].as_number().to_int64() == 42);
}

TEST_CASE("sanity: logging basics") {
    // Temporarily redirect to avoid console output
    std::ostringstream dummy;
    interlaced::core::logging::Logger::set_output_streams(dummy, dummy);
    interlaced::core::logging::Logger::set_level(interlaced::core::logging::LOG_INFO);
    
    interlaced::core::logging::Logger::info("Sanity check log message");
    CHECK(dummy.str().find("Sanity check log message") != std::string::npos);
    
    // Reset
    interlaced::core::logging::Logger::set_output_streams(std::cout, std::cerr);
}

TEST_CASE("sanity: network basics") {
    // Test the simplified HTTP GET implementation
    std::string response = interlaced::core::network::Network::http_get("http://example.com");
    CHECK(response.find("HTTP response from http://example.com") != std::string::npos);
}
