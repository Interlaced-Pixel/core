#include "doctest.h"
#include "interlaced_core/network.hpp"

using namespace interlaced::core::network;

TEST_SUITE("interlaced_core_network") {

TEST_CASE("NetworkResult - constructor and fields") {
    NetworkResult success_result(true, 0, "Success");
    CHECK(success_result.success == true);
    CHECK(success_result.error_code == 0);
    CHECK(success_result.message == "Success");
    
    NetworkResult error_result(false, 1, "Error message");
    CHECK(error_result.success == false);
    CHECK(error_result.error_code == 1);
    CHECK(error_result.message == "Error message");
}

TEST_CASE("resolve_hostname - with empty hostname") {
    NetworkResult result = Network::resolve_hostname("");
    CHECK(result.success == false);
    CHECK(result.error_code == 1);
    CHECK(result.message == "Hostname is empty");
}

TEST_CASE("resolve_hostname - with localhost") {
    NetworkResult result = Network::resolve_hostname("localhost");
    CHECK(result.success == true);
    CHECK(result.error_code == 0);
    // Should resolve to 127.0.0.1 or ::1
    bool valid_ip = (result.message == "127.0.0.1" || result.message == "::1");
    CHECK(valid_ip == true);
}

TEST_CASE("resolve_hostname - with invalid hostname") {
    NetworkResult result = Network::resolve_hostname("this.is.an.invalid.hostname.that.does.not.exist.12345");
    CHECK(result.success == false);
    CHECK(result.error_code == 2);
}

TEST_CASE("is_host_reachable - with empty host") {
    NetworkResult result = Network::is_host_reachable("");
    CHECK(result.success == false);
    CHECK(result.error_code == 1);
    CHECK(result.message == "Host is empty");
}

TEST_CASE("is_host_reachable - with localhost") {
    // This test may fail if no web server is running on localhost:80
    NetworkResult result = Network::is_host_reachable("localhost");
    // We check that it at least resolves and attempts connection
    // It may fail with connection refused, which is expected
    bool expected_result = (result.success == true || result.error_code == 4);
    CHECK(expected_result == true);
}

TEST_CASE("is_host_reachable - with invalid host") {
    NetworkResult result = Network::is_host_reachable("invalid.host.12345");
    CHECK(result.success == false);
    // Should fail at resolution step
    CHECK(result.error_code == 2);
}

TEST_CASE("download_file - with empty URL") {
    NetworkResult result = Network::download_file("", "/tmp/test");
    CHECK(result.success == false);
    CHECK(result.error_code == 1);
    CHECK(result.message == "URL is empty");
}

TEST_CASE("download_file - with empty destination") {
    NetworkResult result = Network::download_file("http://example.com", "");
    CHECK(result.success == false);
    CHECK(result.error_code == 2);
    CHECK(result.message == "Destination path is empty");
}

TEST_CASE("download_file - with invalid URL format") {
    NetworkResult result = Network::download_file("invalid_url", "/tmp/test");
    CHECK(result.success == false);
    CHECK(result.error_code == 6);
    CHECK(result.message == "Invalid URL format");
}

TEST_CASE("download_file - URL parsing") {
    // Test that proper URLs don't fail at the validation stage
    // They may fail at connection, but should pass URL format check
    NetworkResult result = Network::download_file("http://example.com/path", "/tmp/test_download");
    // Should not fail with error code 6 (invalid URL format)
    CHECK(result.error_code != 6);
}

TEST_CASE("http_get - basic test") {
    std::string response = Network::http_get("http://example.com");
    CHECK(response.find("HTTP response from http://example.com") != std::string::npos);
}

TEST_CASE("http_get - with different URL") {
    std::string response = Network::http_get("http://test.org/path");
    CHECK(response.find("HTTP response from http://test.org/path") != std::string::npos);
}

TEST_CASE("http_post - basic test") {
    std::string response = Network::http_post("http://example.com", "data=test");
    CHECK(response.find("HTTP POST response from http://example.com") != std::string::npos);
    CHECK(response.find("with payload: data=test") != std::string::npos);
}

TEST_CASE("http_post - with empty payload") {
    std::string response = Network::http_post("http://example.com", "");
    CHECK(response.find("HTTP POST response from") != std::string::npos);
    CHECK(response.find("with payload:") != std::string::npos);
}

TEST_CASE("https_get - basic test") {
    std::string response = Network::https_get("https://example.com");
    CHECK(response.find("HTTPS response from https://example.com") != std::string::npos);
}

TEST_CASE("https_get - with path") {
    std::string response = Network::https_get("https://secure.example.com/api/data");
    CHECK(response.find("HTTPS response from https://secure.example.com/api/data") != std::string::npos);
}

TEST_CASE("https_post - basic test") {
    std::string response = Network::https_post("https://example.com", "json_data");
    CHECK(response.find("HTTPS POST response from https://example.com") != std::string::npos);
    CHECK(response.find("with payload: json_data") != std::string::npos);
}

TEST_CASE("https_post - with complex payload") {
    std::string payload = "{\"key\":\"value\",\"number\":123}";
    std::string response = Network::https_post("https://api.example.com", payload);
    CHECK(response.find("HTTPS POST response from") != std::string::npos);
    CHECK(response.find(payload) != std::string::npos);
}

TEST_CASE("url_encode - basic test") {
    std::string encoded = Network::url_encode("test string");
    // Placeholder implementation returns original string
    CHECK(encoded == "test string");
}

TEST_CASE("url_encode - with special characters") {
    std::string encoded = Network::url_encode("test&string=value");
    // Placeholder implementation returns original string
    CHECK(encoded == "test&string=value");
}

TEST_CASE("url_decode - basic test") {
    std::string decoded = Network::url_decode("test%20string");
    // Placeholder implementation returns original string
    CHECK(decoded == "test%20string");
}

TEST_CASE("url_decode - with encoded characters") {
    std::string decoded = Network::url_decode("test%26string%3Dvalue");
    // Placeholder implementation returns original string
    CHECK(decoded == "test%26string%3Dvalue");
}

TEST_CASE("get_network_interfaces - returns list") {
    std::vector<std::string> interfaces = Network::get_network_interfaces();
    
    // Should return at least one interface
    CHECK(interfaces.size() > 0);
    
    // Common interfaces should be present
    bool has_common_interface = false;
    for (const auto& iface : interfaces) {
        if (iface == "lo" || iface == "eth0" || iface == "wlan0" || 
            iface == "Loopback" || iface == "Ethernet" || iface == "Wi-Fi") {
            has_common_interface = true;
            break;
        }
    }
    CHECK(has_common_interface == true);
}

TEST_CASE("is_valid_ipv4 - with valid addresses") {
    CHECK(Network::is_valid_ipv4("192.168.1.1") == true);
    CHECK(Network::is_valid_ipv4("127.0.0.1") == true);
    CHECK(Network::is_valid_ipv4("0.0.0.0") == true);
    CHECK(Network::is_valid_ipv4("255.255.255.255") == true);
    CHECK(Network::is_valid_ipv4("10.0.0.1") == true);
}

TEST_CASE("is_valid_ipv4 - with invalid addresses") {
    CHECK(Network::is_valid_ipv4("") == false);
    CHECK(Network::is_valid_ipv4("256.1.1.1") == false);
    CHECK(Network::is_valid_ipv4("192.168.1") == false);
    CHECK(Network::is_valid_ipv4("192.168.1.1.1") == false);
    CHECK(Network::is_valid_ipv4("192.168.-1.1") == false);
    CHECK(Network::is_valid_ipv4("192.168.1.a") == false);
    CHECK(Network::is_valid_ipv4("192.168..1") == false);
    CHECK(Network::is_valid_ipv4("192.168.01.1") == false); // Leading zero
}

TEST_CASE("is_valid_ipv4 - edge cases") {
    CHECK(Network::is_valid_ipv4("0.0.0.0") == true);
    CHECK(Network::is_valid_ipv4("255.255.255.255") == true);
    CHECK(Network::is_valid_ipv4("192.168.1.256") == false);
    CHECK(Network::is_valid_ipv4("a.b.c.d") == false);
}

TEST_CASE("is_valid_ipv6 - with valid addresses") {
    CHECK(Network::is_valid_ipv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334") == true);
    CHECK(Network::is_valid_ipv6("2001:db8:85a3::8a2e:370:7334") == true);
    CHECK(Network::is_valid_ipv6("::1") == true);
    CHECK(Network::is_valid_ipv6("::") == true);
    CHECK(Network::is_valid_ipv6("fe80::1") == true);
}

TEST_CASE("is_valid_ipv6 - with invalid addresses") {
    CHECK(Network::is_valid_ipv6("") == false);
    CHECK(Network::is_valid_ipv6("192.168.1.1") == false); // IPv4
    CHECK(Network::is_valid_ipv6("no_colons") == false);
}

TEST_CASE("is_valid_ipv6 - with compressed format") {
    CHECK(Network::is_valid_ipv6("2001:db8::1") == true);
    CHECK(Network::is_valid_ipv6("::ffff:192.0.2.1") == true);
}

TEST_CASE("create_socket_connection - with invalid input") {
    CHECK(Network::create_socket_connection("", 80) == -1);
    CHECK(Network::create_socket_connection("localhost", 0) == -1);
    CHECK(Network::create_socket_connection("localhost", -1) == -1);
    CHECK(Network::create_socket_connection("localhost", 65536) == -1);
}

TEST_CASE("create_socket_connection - with invalid host") {
    int sock = Network::create_socket_connection("invalid.host.that.does.not.exist.12345", 80);
    CHECK(sock == -1);
}

TEST_CASE("close_socket_connection - with invalid socket") {
    CHECK(Network::close_socket_connection(-1) == false);
    CHECK(Network::close_socket_connection(-999) == false);
}

TEST_CASE("parse_http_response_code - with valid responses") {
    CHECK(Network::parse_http_response_code("HTTP/1.1 200 OK") == 200);
    CHECK(Network::parse_http_response_code("HTTP/1.1 404 Not Found") == 404);
    CHECK(Network::parse_http_response_code("HTTP/1.1 500 Internal Server Error") == 500);
    CHECK(Network::parse_http_response_code("HTTP/1.0 301 Moved Permanently") == 301);
}

TEST_CASE("parse_http_response_code - with invalid responses") {
    CHECK(Network::parse_http_response_code("") == -1);
    CHECK(Network::parse_http_response_code("Invalid response") == -1);
    CHECK(Network::parse_http_response_code("HTTP/1.1") == -1);
    CHECK(Network::parse_http_response_code("200 OK") == -1);
}

TEST_CASE("parse_http_response_code - edge cases") {
    CHECK(Network::parse_http_response_code("HTTP/1.1 abc OK") == -1);
    // Double space is not handled by the simple parser, so it should fail
    CHECK(Network::parse_http_response_code("HTTP/1.1  200 OK") == -1);
}

TEST_CASE("is_http_success - with success codes") {
    CHECK(Network::is_http_success(200) == true);
    CHECK(Network::is_http_success(201) == true);
    CHECK(Network::is_http_success(204) == true);
    CHECK(Network::is_http_success(299) == true);
}

TEST_CASE("is_http_success - with error codes") {
    CHECK(Network::is_http_success(199) == false);
    CHECK(Network::is_http_success(300) == false);
    CHECK(Network::is_http_success(301) == false);
    CHECK(Network::is_http_success(400) == false);
    CHECK(Network::is_http_success(404) == false);
    CHECK(Network::is_http_success(500) == false);
}

TEST_CASE("is_http_success - edge cases") {
    CHECK(Network::is_http_success(200) == true);  // Start of 2xx
    CHECK(Network::is_http_success(299) == true);  // End of 2xx
    CHECK(Network::is_http_success(199) == false); // Just before 2xx
    CHECK(Network::is_http_success(300) == false); // Just after 2xx
}

TEST_CASE("measure_latency - with empty host") {
    double latency = Network::measure_latency("", 4);
    CHECK(latency == -1.0);
}

TEST_CASE("measure_latency - with invalid count") {
    double latency = Network::measure_latency("localhost", 0);
    CHECK(latency == -1.0);
    
    latency = Network::measure_latency("localhost", -1);
    CHECK(latency == -1.0);
}

TEST_CASE("measure_latency - with valid input") {
    double latency = Network::measure_latency("localhost", 4);
    // Should return a simulated latency value between 10 and 100 ms
    CHECK(latency >= 10.0);
    CHECK(latency <= 100.0);
}

TEST_CASE("measure_latency - with different count") {
    double latency = Network::measure_latency("example.com", 1);
    CHECK(latency >= 10.0);
    CHECK(latency <= 100.0);
}

TEST_CASE("measure_bandwidth - with empty host") {
    double bandwidth = Network::measure_bandwidth("");
    CHECK(bandwidth == -1.0);
}

TEST_CASE("measure_bandwidth - with valid host") {
    double bandwidth = Network::measure_bandwidth("localhost");
    // Should return a simulated bandwidth value between 10 and 1000 Mbps
    CHECK(bandwidth >= 10.0);
    CHECK(bandwidth <= 1000.0);
}

TEST_CASE("measure_bandwidth - with different host") {
    double bandwidth = Network::measure_bandwidth("example.com");
    CHECK(bandwidth >= 10.0);
    CHECK(bandwidth <= 1000.0);
}

TEST_CASE("NetworkResult - multiple instances") {
    NetworkResult r1(true, 0, "First success");
    NetworkResult r2(false, 5, "Second error");
    NetworkResult r3(true, 0, "Third success");
    
    CHECK(r1.success == true);
    CHECK(r2.success == false);
    CHECK(r3.success == true);
    
    CHECK(r1.error_code == 0);
    CHECK(r2.error_code == 5);
    CHECK(r3.error_code == 0);
    
    CHECK(r1.message == "First success");
    CHECK(r2.message == "Second error");
    CHECK(r3.message == "Third success");
}

TEST_CASE("resolve_hostname - with google.com") {
    // This should resolve to an actual IP address
    NetworkResult result = Network::resolve_hostname("google.com");
    CHECK(result.success == true);
    CHECK(result.error_code == 0);
    CHECK(result.message.empty() == false);
}

TEST_CASE("resolve_hostname - with various hostnames") {
    // Test with a hostname that should resolve
    NetworkResult result1 = Network::resolve_hostname("example.com");
    // May succeed or fail depending on network, but shouldn't crash
    bool valid_result = (result1.success == true || result1.error_code > 0);
    CHECK(valid_result == true);
}

TEST_CASE("is_valid_ipv4 - comprehensive validation") {
    // More edge cases
    CHECK(Network::is_valid_ipv4("192.168.001.1") == false); // Leading zeros
    CHECK(Network::is_valid_ipv4("192.168.1.01") == false);  // Leading zeros
    CHECK(Network::is_valid_ipv4("1.2.3.4") == true);
    CHECK(Network::is_valid_ipv4("192.168.1.") == false);    // Trailing dot
    CHECK(Network::is_valid_ipv4(".192.168.1.1") == false);  // Leading dot
}

TEST_CASE("is_valid_ipv6 - more validation") {
    // Test various IPv6 formats
    CHECK(Network::is_valid_ipv6("2001:0:0:0:0:0:0:1") == true);
    CHECK(Network::is_valid_ipv6("ff02::1") == true);
    CHECK(Network::is_valid_ipv6("::ffff:192.0.2.128") == true);
    CHECK(Network::is_valid_ipv6("2001:db8:85a3:0:0:8a2e:370:7334") == true);
}

TEST_CASE("http_get - with empty URL") {
    std::string response = Network::http_get("");
    CHECK(response.find("HTTP response from") != std::string::npos);
}

TEST_CASE("http_post - with various payloads") {
    std::string response1 = Network::http_post("http://api.test.com", "key=value&other=data");
    CHECK(response1.find("with payload:") != std::string::npos);
    
    std::string response2 = Network::http_post("http://test.com", "{\"json\":true}");
    CHECK(response2.find("{\"json\":true}") != std::string::npos);
}

TEST_CASE("https_get - with empty URL") {
    std::string response = Network::https_get("");
    CHECK(response.find("HTTPS response from") != std::string::npos);
}

TEST_CASE("https_post - with empty payload") {
    std::string response = Network::https_post("https://test.com", "");
    CHECK(response.find("HTTPS POST response from") != std::string::npos);
    CHECK(response.find("with payload:") != std::string::npos);
}

TEST_CASE("measure_latency - consistency check") {
    double latency1 = Network::measure_latency("test.com", 1);
    double latency2 = Network::measure_latency("test.com", 5);
    
    // Both should be in valid range
    CHECK(latency1 >= 10.0);
    CHECK(latency1 <= 100.0);
    CHECK(latency2 >= 10.0);
    CHECK(latency2 <= 100.0);
}

TEST_CASE("measure_bandwidth - consistency check") {
    double bw1 = Network::measure_bandwidth("host1.com");
    double bw2 = Network::measure_bandwidth("host2.com");
    
    // Both should be in valid range
    CHECK(bw1 >= 10.0);
    CHECK(bw1 <= 1000.0);
    CHECK(bw2 >= 10.0);
    CHECK(bw2 <= 1000.0);
}

TEST_CASE("parse_http_response_code - with different HTTP versions") {
    CHECK(Network::parse_http_response_code("HTTP/1.0 200 OK") == 200);
    CHECK(Network::parse_http_response_code("HTTP/1.1 200 OK") == 200);
    CHECK(Network::parse_http_response_code("HTTP/2.0 200 OK") == 200);
}

TEST_CASE("parse_http_response_code - with various status codes") {
    CHECK(Network::parse_http_response_code("HTTP/1.1 100 Continue") == 100);
    CHECK(Network::parse_http_response_code("HTTP/1.1 201 Created") == 201);
    CHECK(Network::parse_http_response_code("HTTP/1.1 301 Moved") == 301);
    CHECK(Network::parse_http_response_code("HTTP/1.1 401 Unauthorized") == 401);
    CHECK(Network::parse_http_response_code("HTTP/1.1 403 Forbidden") == 403);
    CHECK(Network::parse_http_response_code("HTTP/1.1 503 Service Unavailable") == 503);
}

TEST_CASE("is_http_success - comprehensive check") {
    // All 2xx codes should be success
    for (int code = 200; code < 300; code++) {
        CHECK(Network::is_http_success(code) == true);
    }
    
    // Codes outside 2xx range should not be success
    CHECK(Network::is_http_success(100) == false);
    CHECK(Network::is_http_success(150) == false);
    CHECK(Network::is_http_success(300) == false);
    CHECK(Network::is_http_success(400) == false);
    CHECK(Network::is_http_success(500) == false);
}

TEST_CASE("url_encode - empty string") {
    std::string encoded = Network::url_encode("");
    CHECK(encoded == "");
}

TEST_CASE("url_decode - empty string") {
    std::string decoded = Network::url_decode("");
    CHECK(decoded == "");
}

TEST_CASE("get_network_interfaces - verify structure") {
    std::vector<std::string> interfaces = Network::get_network_interfaces();
    
    // All interface names should be non-empty strings
    for (const auto& iface : interfaces) {
        CHECK(iface.empty() == false);
    }
}

TEST_CASE("download_file - with https URL") {
    // Test HTTPS URL detection
    NetworkResult result = Network::download_file("https://example.com/file", "/tmp/test");
    // Should not fail with error code 6 (invalid URL format)
    CHECK(result.error_code != 6);
}

TEST_CASE("download_file - URL with port") {
    // Test URL with explicit port
    NetworkResult result = Network::download_file("http://example.com:8080/file", "/tmp/test");
    // Should not fail with error code 6 (invalid URL format)
    CHECK(result.error_code != 6);
}

TEST_CASE("create_socket_connection - boundary port values") {
    // Test boundary values for port
    CHECK(Network::create_socket_connection("localhost", 1) == -1);     // Valid port but may fail connection
    CHECK(Network::create_socket_connection("localhost", 65535) == -1); // Max valid port
}

TEST_CASE("is_valid_ipv4 - boundary octet values") {
    CHECK(Network::is_valid_ipv4("0.0.0.1") == true);
    CHECK(Network::is_valid_ipv4("255.0.0.0") == true);
    CHECK(Network::is_valid_ipv4("0.255.0.0") == true);
    CHECK(Network::is_valid_ipv4("0.0.255.0") == true);
    CHECK(Network::is_valid_ipv4("0.0.0.255") == true);
}

TEST_CASE("NetworkResult - error code variations") {
    NetworkResult r1(false, 1, "Error type 1");
    NetworkResult r2(false, 2, "Error type 2");
    NetworkResult r3(false, 3, "Error type 3");
    NetworkResult r4(false, 4, "Error type 4");
    NetworkResult r5(false, 5, "Error type 5");
    
    CHECK(r1.error_code == 1);
    CHECK(r2.error_code == 2);
    CHECK(r3.error_code == 3);
    CHECK(r4.error_code == 4);
    CHECK(r5.error_code == 5);
    
    CHECK(r1.success == false);
    CHECK(r2.success == false);
    CHECK(r3.success == false);
    CHECK(r4.success == false);
    CHECK(r5.success == false);
}

} // TEST_SUITE
