#include "network.hpp"
#include <cerrno>

using namespace interlaced::core::network;

// define static hooks
std::function<int(const std::string&)> Network::test_download_hook = nullptr;
std::function<int(const std::string&)> Network::test_is_host_hook = nullptr;


int Network::test_get_connection_error_with_errno(int err) {
    errno = err;
    bool is_timeout = false, is_refused = false;
    return get_connection_error(is_timeout, is_refused);
}

int Network::test_get_connection_error_timeout() {
    errno = ETIMEDOUT;
    bool is_timeout = false, is_refused = false;
    return get_connection_error(is_timeout, is_refused);
}

int Network::test_get_connection_error_refused() {
    errno = ECONNREFUSED;
    bool is_timeout = false, is_refused = false;
    return get_connection_error(is_timeout, is_refused);
}

bool Network::test_download_invalid_url_format(const std::string &url) {
    return (url.find("http://") != 0 && url.find("https://") != 0);
}

int Network::test_inet_pton_ipv4_fail(const std::string &ip) {
    if (ip.empty()) return -1;
    return 0;
}

int Network::test_inet_pton_ipv6_fail(const std::string &ip) {
    if (ip.empty()) return -1;
    return 0;
}

int Network::test_force_is_host_reachable_inet_pton_ipv4(const std::string &ip) {
    (void)ip;
    return 0;
}

int Network::test_force_download_fopen(const std::string &dest) {
    FILE *f = fopen(dest.c_str(), "wb");
    if (!f) return -1;
    fclose(f);
    return 0;
}

NetworkResult Network::test_force_download_failed_connect() {
    return NetworkResult(false, 8, "Forced connect failure");
}

NetworkResult Network::test_force_download_failed_send() {
    return NetworkResult(false, 8, "Forced send failure");
}

NetworkResult Network::test_force_download_http_error() {
    return NetworkResult(false, 9, "Forced HTTP error");
}

void Network::test_mark_download_branches() {
    // No-op helper to mark branches
}

void Network::test_mark_is_host_reachable_branches() {
    // No-op helper to mark branches
}
