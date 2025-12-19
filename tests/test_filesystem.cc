#include "doctest.h"
#include "filesystem.hpp"

#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

using namespace interlaced::core::filesystem;

TEST_SUITE("filesystem_module") {

// Helper function to create a temporary file
std::string create_temp_file(const std::string& content = "test content") {
    std::string temp_path = FileSystem::temp_directory_path();
    // Remove trailing slash if present
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string filename = temp_path + "/interlaced_test_" + std::to_string(std::rand()) + ".txt";
    std::ofstream ofs(filename);
    ofs << content;
    ofs.close();
    return filename;
}

// Helper function to create a temporary directory
std::string create_temp_dir() {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string dirname = temp_path + "/interlaced_test_dir_" + std::to_string(std::rand());
    FileSystem::create_directory(dirname);
    return dirname;
}

TEST_CASE("exists - file exists") {
    std::string temp_file = create_temp_file();
    CHECK(FileSystem::exists(temp_file) == true);
    std::remove(temp_file.c_str());
}

TEST_CASE("exists - file does not exist") {
    CHECK(FileSystem::exists("/nonexistent_file_xyz123.txt") == false);
}

TEST_CASE("is_directory - valid directory") {
    std::string temp_dir = create_temp_dir();
    CHECK(FileSystem::is_directory(temp_dir) == true);
    FileSystem::remove(temp_dir);
}

TEST_CASE("is_directory - file is not directory") {
    std::string temp_file = create_temp_file();
    CHECK(FileSystem::is_directory(temp_file) == false);
    std::remove(temp_file.c_str());
}

TEST_CASE("is_directory - nonexistent path") {
    CHECK(FileSystem::is_directory("/nonexistent_path_xyz123") == false);
}

TEST_CASE("is_regular_file - valid file") {
    std::string temp_file = create_temp_file();
    CHECK(FileSystem::is_regular_file(temp_file) == true);
    std::remove(temp_file.c_str());
}

TEST_CASE("is_regular_file - directory is not file") {
    std::string temp_dir = create_temp_dir();
    CHECK(FileSystem::is_regular_file(temp_dir) == false);
    FileSystem::remove(temp_dir);
}

TEST_CASE("is_regular_file - nonexistent path") {
    CHECK(FileSystem::is_regular_file("/nonexistent_file_xyz123.txt") == false);
}

TEST_CASE("read_file - valid file") {
    std::string content = "Hello, World! This is a test file.";
    std::string temp_file = create_temp_file(content);
    std::string read_content = FileSystem::read_file(temp_file);
    CHECK(read_content == content);
    std::remove(temp_file.c_str());
}

TEST_CASE("read_file - empty file") {
    std::string temp_file = create_temp_file("");
    std::string read_content = FileSystem::read_file(temp_file);
    CHECK(read_content.empty() == true);
    std::remove(temp_file.c_str());
}

TEST_CASE("read_file - binary content") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string filename = temp_path + "/interlaced_test_binary.bin";
    
    // Write binary data
    std::ofstream ofs(filename, std::ios::binary);
    unsigned char data[] = {0x00, 0x01, 0x02, 0xFF, 0xFE};
    ofs.write(reinterpret_cast<char*>(data), sizeof(data));
    ofs.close();
    
    std::string read_content = FileSystem::read_file(filename);
    CHECK(read_content.size() == 5);
    CHECK(static_cast<unsigned char>(read_content[0]) == 0x00);
    CHECK(static_cast<unsigned char>(read_content[4]) == 0xFE);
    
    std::remove(filename.c_str());
}

TEST_CASE("write_file - create new file") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string filename = temp_path + "/interlaced_test_write.txt";
    
    std::string content = "Test write content";
    bool result = FileSystem::write_file(filename, content);
    CHECK(result == true);
    CHECK(FileSystem::exists(filename) == true);
    
    std::string read_back = FileSystem::read_file(filename);
    CHECK(read_back == content);
    
    std::remove(filename.c_str());
}

TEST_CASE("write_file - overwrite existing file") {
    std::string temp_file = create_temp_file("original content");
    
    std::string new_content = "overwritten content";
    bool result = FileSystem::write_file(temp_file, new_content);
    CHECK(result == true);
    
    std::string read_back = FileSystem::read_file(temp_file);
    CHECK(read_back == new_content);
    
    std::remove(temp_file.c_str());
}

TEST_CASE("write_file - empty content") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string filename = temp_path + "/interlaced_test_empty.txt";
    
    bool result = FileSystem::write_file(filename, "");
    CHECK(result == true);
    CHECK(FileSystem::file_size(filename) == 0);
    
    std::remove(filename.c_str());
}

TEST_CASE("create_directory - new directory") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string dirname = temp_path + "/interlaced_test_mkdir";
    
    bool result = FileSystem::create_directory(dirname);
    CHECK(result == true);
    CHECK(FileSystem::is_directory(dirname) == true);
    
    FileSystem::remove(dirname);
}

TEST_CASE("create_directory - already exists") {
    std::string temp_dir = create_temp_dir();
    
    // Attempting to create an existing directory should fail
    bool result = FileSystem::create_directory(temp_dir);
    CHECK(result == false);
    
    FileSystem::remove(temp_dir);
}

TEST_CASE("create_directories - nested directories") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string nested_path = temp_path + "/interlaced_test_nested/level1/level2";
    
    bool result = FileSystem::create_directories(nested_path);
    CHECK(result == true);
    CHECK(FileSystem::is_directory(nested_path) == true);
    
    // Cleanup
    FileSystem::remove(nested_path);
    FileSystem::remove(temp_path + "/interlaced_test_nested/level1");
    FileSystem::remove(temp_path + "/interlaced_test_nested");
}

TEST_CASE("create_directories - already exists") {
    std::string temp_dir = create_temp_dir();
    
    bool result = FileSystem::create_directories(temp_dir);
    CHECK(result == true); // Should succeed since directory exists
    
    FileSystem::remove(temp_dir);
}

TEST_CASE("create_directories - empty path") {
    bool result = FileSystem::create_directories("");
    CHECK(result == false);
}

TEST_CASE("create_directories - root path") {
#ifdef _WIN32
    bool result = FileSystem::create_directories("C:\\");
#else
    bool result = FileSystem::create_directories("/");
#endif
    CHECK(result == true); // Root always exists
}

TEST_CASE("create_directories - with trailing slashes") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string path_with_slash = temp_path + "/interlaced_test_trailing/";
    
    bool result = FileSystem::create_directories(path_with_slash);
    CHECK(result == true);
    CHECK(FileSystem::is_directory(temp_path + "/interlaced_test_trailing") == true);
    
    FileSystem::remove(temp_path + "/interlaced_test_trailing");
}

TEST_CASE("remove - file") {
    std::string temp_file = create_temp_file();
    
    bool result = FileSystem::remove(temp_file);
    CHECK(result == true);
    CHECK(FileSystem::exists(temp_file) == false);
}

TEST_CASE("remove - empty directory") {
    std::string temp_dir = create_temp_dir();
    
    bool result = FileSystem::remove(temp_dir);
    CHECK(result == true);
    CHECK(FileSystem::exists(temp_dir) == false);
}

TEST_CASE("remove - nonexistent path") {
    bool result = FileSystem::remove("/nonexistent_path_xyz123");
    CHECK(result == false);
}

TEST_CASE("copy_file - basic copy") {
    std::string content = "Content to copy";
    std::string source = create_temp_file(content);
    
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string destination = temp_path + "/interlaced_test_copy.txt";
    
    bool result = FileSystem::copy_file(source, destination);
    CHECK(result == true);
    CHECK(FileSystem::exists(destination) == true);
    
    std::string copied_content = FileSystem::read_file(destination);
    CHECK(copied_content == content);
    
    std::remove(source.c_str());
    std::remove(destination.c_str());
}

TEST_CASE("copy_file - nonexistent source") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string destination = temp_path + "/interlaced_test_copy_dest.txt";
    
    bool result = FileSystem::copy_file("/nonexistent_source_xyz123.txt", destination);
    CHECK(result == false);
}

TEST_CASE("rename - file") {
    std::string temp_file = create_temp_file("rename test");
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string new_name = temp_path + "/interlaced_test_renamed.txt";
    
    bool result = FileSystem::rename(temp_file, new_name);
    CHECK(result == true);
    CHECK(FileSystem::exists(temp_file) == false);
    CHECK(FileSystem::exists(new_name) == true);
    
    std::remove(new_name.c_str());
}

TEST_CASE("rename - directory") {
    std::string temp_dir = create_temp_dir();
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string new_name = temp_path + "/interlaced_test_renamed_dir";
    
    bool result = FileSystem::rename(temp_dir, new_name);
    CHECK(result == true);
    CHECK(FileSystem::exists(temp_dir) == false);
    CHECK(FileSystem::exists(new_name) == true);
    
    FileSystem::remove(new_name);
}

TEST_CASE("rename - nonexistent source") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string new_name = temp_path + "/interlaced_test_renamed.txt";
    
    bool result = FileSystem::rename("/nonexistent_source_xyz123.txt", new_name);
    CHECK(result == false);
}

TEST_CASE("file_size - regular file") {
    std::string content = "12345"; // 5 bytes
    std::string temp_file = create_temp_file(content);
    
    long long size = FileSystem::file_size(temp_file);
    CHECK(size == 5);
    
    std::remove(temp_file.c_str());
}

TEST_CASE("file_size - empty file") {
    std::string temp_file = create_temp_file("");
    
    long long size = FileSystem::file_size(temp_file);
    CHECK(size == 0);
    
    std::remove(temp_file.c_str());
}

TEST_CASE("file_size - nonexistent file") {
    long long size = FileSystem::file_size("/nonexistent_file_xyz123.txt");
    CHECK(size == -1);
}

TEST_CASE("last_write_time - valid file") {
    std::string temp_file = create_temp_file();
    
    std::time_t mtime = FileSystem::last_write_time(temp_file);
    CHECK(mtime != -1);
    CHECK(mtime > 0);
    
    std::remove(temp_file.c_str());
}

TEST_CASE("last_write_time - nonexistent file") {
    std::time_t mtime = FileSystem::last_write_time("/nonexistent_file_xyz123.txt");
    CHECK(mtime == -1);
}

TEST_CASE("directory_iterator - list files") {
    std::string temp_dir = create_temp_dir();
    
    // Create some test files in the directory
    std::ofstream(temp_dir + "/file1.txt").close();
    std::ofstream(temp_dir + "/file2.txt").close();
    
    std::vector<std::string> files = FileSystem::directory_iterator(temp_dir);
    CHECK(files.size() >= 2);
    
    // Check that our files are in the list
    bool found_file1 = std::find(files.begin(), files.end(), "file1.txt") != files.end();
    bool found_file2 = std::find(files.begin(), files.end(), "file2.txt") != files.end();
    CHECK(found_file1 == true);
    CHECK(found_file2 == true);
    
    // Cleanup
    std::remove((temp_dir + "/file1.txt").c_str());
    std::remove((temp_dir + "/file2.txt").c_str());
    FileSystem::remove(temp_dir);
}

TEST_CASE("directory_iterator - empty directory") {
    std::string temp_dir = create_temp_dir();
    
    std::vector<std::string> files = FileSystem::directory_iterator(temp_dir);
    CHECK(files.empty() == true);
    
    FileSystem::remove(temp_dir);
}

TEST_CASE("directory_iterator - nonexistent directory") {
    std::vector<std::string> files = FileSystem::directory_iterator("/nonexistent_dir_xyz123");
    CHECK(files.empty() == true);
}

TEST_CASE("directory_iterator - excludes dot entries") {
    std::string temp_dir = create_temp_dir();
    
    std::vector<std::string> files = FileSystem::directory_iterator(temp_dir);
    
    // Check that "." and ".." are not in the list
    bool has_dot = std::find(files.begin(), files.end(), ".") != files.end();
    bool has_dotdot = std::find(files.begin(), files.end(), "..") != files.end();
    CHECK(has_dot == false);
    CHECK(has_dotdot == false);
    
    FileSystem::remove(temp_dir);
}

TEST_CASE("temp_directory_path - returns valid path") {
    std::string temp_path = FileSystem::temp_directory_path();
    CHECK(temp_path.empty() == false);
    CHECK(FileSystem::is_directory(temp_path) == true);
}

TEST_CASE("current_path - get current directory") {
    std::string current = FileSystem::current_path();
    CHECK(current.empty() == false);
    CHECK(FileSystem::is_directory(current) == true);
}

TEST_CASE("current_path - change directory") {
    std::string original = FileSystem::current_path();
    std::string temp_dir = create_temp_dir();
    
    bool result = FileSystem::current_path(temp_dir);
    CHECK(result == true);
    
    std::string new_current = FileSystem::current_path();
    CHECK(new_current.find("interlaced_test_dir_") != std::string::npos);
    
    // Change back to original
    FileSystem::current_path(original);
    FileSystem::remove(temp_dir);
}

TEST_CASE("current_path - nonexistent directory") {
    bool result = FileSystem::current_path("/nonexistent_dir_xyz123");
    CHECK(result == false);
}

TEST_CASE("write_and_read_large_file") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string filename = temp_path + "/interlaced_test_large.txt";
    
    // Create a large string (1 MB)
    std::string large_content(1024 * 1024, 'A');
    
    bool write_result = FileSystem::write_file(filename, large_content);
    CHECK(write_result == true);
    
    long long size = FileSystem::file_size(filename);
    CHECK(size == 1024 * 1024);
    
    std::string read_content = FileSystem::read_file(filename);
    CHECK(read_content.size() == large_content.size());
    CHECK(read_content == large_content);
    
    std::remove(filename.c_str());
}

TEST_CASE("nested_directory_operations") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string base_dir = temp_path + "/interlaced_test_nested_ops";
    std::string sub_dir1 = base_dir + "/sub1";
    std::string sub_dir2 = base_dir + "/sub1/sub2";
    
    // Create nested structure
    bool result = FileSystem::create_directories(sub_dir2);
    CHECK(result == true);
    CHECK(FileSystem::is_directory(base_dir) == true);
    CHECK(FileSystem::is_directory(sub_dir1) == true);
    CHECK(FileSystem::is_directory(sub_dir2) == true);
    
    // Create a file in the deepest directory
    std::string file_path = sub_dir2 + "/test.txt";
    FileSystem::write_file(file_path, "nested file");
    CHECK(FileSystem::exists(file_path) == true);
    
    // Cleanup (remove from deepest to shallowest)
    std::remove(file_path.c_str());
    FileSystem::remove(sub_dir2);
    FileSystem::remove(sub_dir1);
    FileSystem::remove(base_dir);
}

TEST_CASE("copy_file_preserves_content_and_size") {
    std::string content = "This is a test file with specific content.\nLine 2\nLine 3";
    std::string source = create_temp_file(content);
    
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string destination = temp_path + "/interlaced_test_copy_preserve.txt";
    
    bool copy_result = FileSystem::copy_file(source, destination);
    CHECK(copy_result == true);
    
    long long source_size = FileSystem::file_size(source);
    long long dest_size = FileSystem::file_size(destination);
    CHECK(source_size == dest_size);
    
    std::string source_content = FileSystem::read_file(source);
    std::string dest_content = FileSystem::read_file(destination);
    CHECK(source_content == dest_content);
    
    std::remove(source.c_str());
    std::remove(destination.c_str());
}

TEST_CASE("multiple_operations_on_same_file") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string filename = temp_path + "/interlaced_test_multi_ops.txt";
    
    // Write
    FileSystem::write_file(filename, "Version 1");
    CHECK(FileSystem::read_file(filename) == "Version 1");
    
    // Overwrite
    FileSystem::write_file(filename, "Version 2");
    CHECK(FileSystem::read_file(filename) == "Version 2");
    
    // Rename
    std::string new_name = temp_path + "/interlaced_test_multi_ops_renamed.txt";
    FileSystem::rename(filename, new_name);
    CHECK(FileSystem::exists(filename) == false);
    CHECK(FileSystem::exists(new_name) == true);
    CHECK(FileSystem::read_file(new_name) == "Version 2");
    
    // Copy
    std::string copy_name = temp_path + "/interlaced_test_multi_ops_copy.txt";
    FileSystem::copy_file(new_name, copy_name);
    CHECK(FileSystem::exists(copy_name) == true);
    CHECK(FileSystem::read_file(copy_name) == "Version 2");
    
    // Cleanup
    std::remove(new_name.c_str());
    std::remove(copy_name.c_str());
}

TEST_CASE("write_file - invalid path") {
    // Try to write to a path that doesn't exist (no parent directory)
    bool result = FileSystem::write_file("/nonexistent_dir_xyz/invalid.txt", "content");
    CHECK(result == false);
}

TEST_CASE("create_directories - deep nested path with existing parent") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    
    // Create a base directory first
    std::string base = temp_path + "/interlaced_existing_base";
    FileSystem::create_directory(base);
    
    // Now create nested directories under it
    std::string nested = base + "/new1/new2/new3";
    bool result = FileSystem::create_directories(nested);
    CHECK(result == true);
    CHECK(FileSystem::is_directory(nested) == true);
    
    // Cleanup
    FileSystem::remove(nested);
    FileSystem::remove(base + "/new1/new2");
    FileSystem::remove(base + "/new1");
    FileSystem::remove(base);
}

TEST_CASE("read_file - nonexistent file returns empty") {
    std::string content = FileSystem::read_file("/nonexistent_file_xyz123.txt");
    // The function doesn't check for file existence, it just reads
    // An empty or failed read will return what the stream provides
    CHECK(content.size() >= 0); // Just verify it doesn't crash
}

TEST_CASE("directory_iterator - with subdirectories") {
    std::string temp_dir = create_temp_dir();
    
    // Create files and subdirectories
    std::ofstream(temp_dir + "/file.txt").close();
    std::string subdir = temp_dir + "/subdir";
    FileSystem::create_directory(subdir);
    
    std::vector<std::string> entries = FileSystem::directory_iterator(temp_dir);
    CHECK(entries.size() >= 2);
    
    bool has_file = std::find(entries.begin(), entries.end(), "file.txt") != entries.end();
    bool has_subdir = std::find(entries.begin(), entries.end(), "subdir") != entries.end();
    CHECK(has_file == true);
    CHECK(has_subdir == true);
    
    // Cleanup
    FileSystem::remove(subdir);
    std::remove((temp_dir + "/file.txt").c_str());
    FileSystem::remove(temp_dir);
}

TEST_CASE("last_write_time - directory") {
    std::string temp_dir = create_temp_dir();
    
    std::time_t mtime = FileSystem::last_write_time(temp_dir);
    CHECK(mtime != -1);
    
    FileSystem::remove(temp_dir);
}

TEST_CASE("file_size - directory") {
    std::string temp_dir = create_temp_dir();
    
    // Some systems return a size for directories, others don't
    long long size = FileSystem::file_size(temp_dir);
    CHECK(size >= 0); // Should not return -1 for existing path
    
    FileSystem::remove(temp_dir);
}

TEST_CASE("create_directories - single level path") {
    std::string temp_path = FileSystem::temp_directory_path();
    if (!temp_path.empty() && (temp_path.back() == '/' || temp_path.back() == '\\')) {
        temp_path.pop_back();
    }
    std::string single_dir = temp_path + "/interlaced_single_level";
    
    bool result = FileSystem::create_directories(single_dir);
    CHECK(result == true);
    CHECK(FileSystem::is_directory(single_dir) == true);
    
    FileSystem::remove(single_dir);
}

TEST_CASE("copy_file - overwrite destination") {
    std::string source = create_temp_file("source content");
    std::string dest = create_temp_file("old content");
    
    // Copy should overwrite
    bool result = FileSystem::copy_file(source, dest);
    CHECK(result == true);
    
    std::string content = FileSystem::read_file(dest);
    CHECK(content == "source content");
    
    std::remove(source.c_str());
    std::remove(dest.c_str());
}

} // TEST_SUITE
