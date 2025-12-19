#include "doctest.h"
#include "filesystem.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace interlaced::core::filesystem;

static std::string make_temp_dir() {
    std::string tmpl = "/tmp/interlaced_test_XXXXXX";
    char *buf = &tmpl[0];
    char *res = mkdtemp(buf);
    if (!res) return std::string();
    return std::string(res);
}

static void remove_dir_tree(const std::string &path) {
    // Simple recursive removal for test purposes (non-robust)
    auto entries = FileSystem::directory_iterator(path);
    for (const auto &e : entries) {
        std::string p = path + "/" + e;
        if (FileSystem::is_directory(p)) {
            remove_dir_tree(p);
            FileSystem::remove(p);
        } else {
            FileSystem::remove(p);
        }
    }
    FileSystem::remove(path);
}

TEST_SUITE("filesystem_module") {

TEST_CASE("read_write_and_exists") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string file = dir + "/test.txt";
    CHECK(FileSystem::write_file(file, "hello world"));
    CHECK(FileSystem::exists(file));
    CHECK(FileSystem::is_regular_file(file));

    std::string contents = FileSystem::read_file(file);
    CHECK(contents == "hello world");

    CHECK(FileSystem::file_size(file) == static_cast<long long>(contents.size()));

    // cleanup
    CHECK(FileSystem::remove(file));
    CHECK(FileSystem::remove(dir));
}

TEST_CASE("create_directories_and_directory_iterator") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string nested = dir + "/a/b/c";
    CHECK(FileSystem::create_directories(nested));
    CHECK(FileSystem::is_directory(nested));

    // Create files
    std::string f1 = dir + "/a/f1.txt";
    std::string f2 = dir + "/a/b/f2.txt";
    CHECK(FileSystem::write_file(f1, "x"));
    CHECK(FileSystem::write_file(f2, "y"));

    auto entries = FileSystem::directory_iterator(dir + "/a");
    CHECK(std::find(entries.begin(), entries.end(), "f1.txt") != entries.end());
    CHECK(std::find(entries.begin(), entries.end(), "b") != entries.end());

    remove_dir_tree(dir);
}

TEST_CASE("copy_and_rename") {
    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());

    std::string src = dir + "/src.txt";
    std::string dst = dir + "/dst.txt";
    CHECK(FileSystem::write_file(src, "content"));
    CHECK(FileSystem::copy_file(src, dst));
    CHECK(FileSystem::exists(dst));

    std::string renamed = dir + "/renamed.txt";
    CHECK(FileSystem::rename(dst, renamed));
    CHECK(FileSystem::exists(renamed));

    remove_dir_tree(dir);
}

TEST_CASE("temp_and_current_path") {
    std::string tmp = FileSystem::temp_directory_path();
    CHECK(!tmp.empty());

    std::string dir = make_temp_dir();
    REQUIRE(!dir.empty());
    std::string cwd = FileSystem::current_path();

    CHECK(FileSystem::current_path(dir));
    auto canonical = [](const std::string &p) {
        char buf[PATH_MAX];
        if (realpath(p.c_str(), buf)) return std::string(buf);
        return p;
    };
    CHECK(canonical(FileSystem::current_path()) == canonical(dir));

    // restore
    CHECK(FileSystem::current_path(cwd));
    remove_dir_tree(dir);
}

} // TEST_SUITE
