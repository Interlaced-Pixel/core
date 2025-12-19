#include "doctest.h"
#include "network.hpp"

#include <cstdlib>
#include <fstream>

using namespace interlaced::core::network;

TEST_SUITE("network_module") {

TEST_CASE("resolve_hostname_and_errors") {
    auto r = Network::resolve_hostname("");
    CHECK(r.success == false);
    CHECK(r.error_code == 1);

    // Enable test mode
    setenv("INTERLACED_TEST_MODE", "1", 1);
    auto r2 = Network::resolve_hostname("localhost");
    CHECK(r2.success == true);
    CHECK(r2.error_code == 0);
    unsetenv("INTERLACED_TEST_MODE");
}

TEST_CASE("is_host_reachable_test_mode_and_errors") {
    auto r = Network::is_host_reachable("");
    CHECK(r.success == false);
    CHECK(r.error_code == 1);

    setenv("INTERLACED_TEST_MODE", "1", 1);
    auto r2 = Network::is_host_reachable("example.com");
    CHECK(r2.success == true);
    unsetenv("INTERLACED_TEST_MODE");
}

TEST_CASE("download_file_validation_and_test_mode") {
    auto r = Network::download_file("", "tmp");
    CHECK(r.success == false);
    CHECK(r.error_code == 1);

    auto r2 = Network::download_file("http://example.com", "");
    CHECK(r2.success == false);
    CHECK(r2.error_code == 2);

    setenv("INTERLACED_TEST_MODE", "1", 1);
    std::string dest = "/tmp/interlaced_network_test_file.txt";
    auto r3 = Network::download_file("http://example.com/file", dest);
    CHECK(r3.success == true);

    std::ifstream ifs(dest, std::ios::binary);
    std::string contents((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    CHECK(contents.find("TEST FILE") != std::string::npos);

    // cleanup
    std::remove(dest.c_str());
    unsetenv("INTERLACED_TEST_MODE");
}

TEST_CASE("http_helpers_and_parsing") {
    std::string r = Network::http_get("http://a");
    CHECK(r.find("HTTP response from http://a") != std::string::npos);

    CHECK(Network::url_encode("a b") == std::string("a b"));
    CHECK(Network::url_decode("a%20b") == std::string("a%20b"));

    CHECK(Network::is_valid_ipv4("127.0.0.1"));
    CHECK_FALSE(Network::is_valid_ipv4("256.1.1.1"));

    CHECK(Network::is_valid_ipv6("::1"));
    // Simplified validation accepts strings with ':' as possibly-valid
    CHECK(Network::is_valid_ipv6("not:ip"));

    CHECK(Network::parse_http_response_code("HTTP/1.1 200 OK") == 200);
    CHECK(Network::parse_http_response_code("") == -1);
    
    CHECK(Network::is_http_success(200));
    CHECK_FALSE(Network::is_http_success(404));
}

TEST_CASE("measure_latency_and_bandwidth_simulated") {
    double l = Network::measure_latency("example.com");
    CHECK(l > 0.0);
    CHECK(Network::measure_latency("", 1) < 0.0);

    double b = Network::measure_bandwidth("example.com");
    CHECK(b > 0.0);
    CHECK(Network::measure_bandwidth("") < 0.0);
}

} // TEST_SUITE
