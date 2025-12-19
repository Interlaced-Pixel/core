#include "doctest.h"
#include "interlaced_core/filesystem.hpp"
#include <string>

using namespace interlaced::core::filesystem;

TEST_SUITE("interlaced_core_filesystem") {

TEST_CASE("exists - basic functionality") {
    CHECK(FileSystem::exists("/nonexistent") == false);
    CHECK(FileSystem::exists("/tmp") == true);
}

TEST_CASE("is_directory - test directory detection") {
    // Test with actual directory
    CHECK(FileSystem::is_directory("/tmp") == true);
    
    // Test with non-existent path
    CHECK(FileSystem::is_directory("/nonexistent_directory_12345") == false);
    
    // Create a temporary file and test
    std::string temp_file = "/tmp/test_file_" + std::to_string(std::time(nullptr));
    FileSystem::write_file(temp_file, "test");
    CHECK(FileSystem::is_directory(temp_file) == false);
    FileSystem::remove(temp_file);
}

TEST_CASE("is_regular_file - test file detection") {
    // Create a temporary file
    std::string temp_file = "/tmp/test_regular_file_" + std::to_string(std::time(nullptr));
    FileSystem::write_file(temp_file, "test content");
    CHECK(FileSystem::is_regular_file(temp_file) == true);
    
    // Test with directory
    CHECK(FileSystem::is_regular_file("/tmp") == false);
    
    // Test with non-existent path
    CHECK(FileSystem::is_regular_file("/nonexistent_file_12345") == false);
    
    // Clean up
    FileSystem::remove(temp_file);
}

TEST_CASE("write_file and read_file") {
    std::string temp_file = "/tmp/test_readwrite_" + std::to_string(std::time(nullptr));
    std::string content = "Hello, World!\nThis is a test file.";
    
    // Write file
    CHECK(FileSystem::write_file(temp_file, content) == true);
    
    // Read file
    std::string read_content = FileSystem::read_file(temp_file);
    CHECK(read_content == content);
    
    // Clean up
    FileSystem::remove(temp_file);
}

TEST_CASE("write_file - failure case") {
    // Try to write to an invalid path (should fail)
    CHECK(FileSystem::write_file("/invalid/path/that/does/not/exist/file.txt", "test") == false);
}

TEST_CASE("create_directory") {
    std::string temp_dir = "/tmp/test_dir_" + std::to_string(std::time(nullptr));
    
    // Create directory
    CHECK(FileSystem::create_directory(temp_dir) == true);
    CHECK(FileSystem::is_directory(temp_dir) == true);
    
    // Try to create existing directory (should fail)
    CHECK(FileSystem::create_directory(temp_dir) == false);
    
    // Clean up
    FileSystem::remove(temp_dir);
}

TEST_CASE("create_directories - recursive creation") {
    std::string base_dir = "/tmp/test_nested_" + std::to_string(std::time(nullptr));
    std::string nested_dir = base_dir + "/level1/level2/level3";
    
    // Create nested directories
    CHECK(FileSystem::create_directories(nested_dir) == true);
    CHECK(FileSystem::is_directory(nested_dir) == true);
    
    // Test with existing directory
    CHECK(FileSystem::create_directories(nested_dir) == true);
    
    // Clean up (from deepest to shallowest)
    FileSystem::remove(nested_dir);
    FileSystem::remove(base_dir + "/level1/level2");
    FileSystem::remove(base_dir + "/level1");
    FileSystem::remove(base_dir);
}

TEST_CASE("create_directories - edge cases") {
    // Test with empty path
    CHECK(FileSystem::create_directories("") == false);
    
    // Test with root directory
    CHECK(FileSystem::create_directories("/") == true);
    
    // Test with trailing slashes
    std::string temp_dir = "/tmp/test_trailing_" + std::to_string(std::time(nullptr));
    CHECK(FileSystem::create_directories(temp_dir + "/") == true);
    CHECK(FileSystem::is_directory(temp_dir) == true);
    FileSystem::remove(temp_dir);
}

TEST_CASE("remove - file and directory") {
    // Test removing file
    std::string temp_file = "/tmp/test_remove_file_" + std::to_string(std::time(nullptr));
    FileSystem::write_file(temp_file, "test");
    CHECK(FileSystem::exists(temp_file) == true);
    CHECK(FileSystem::remove(temp_file) == true);
    CHECK(FileSystem::exists(temp_file) == false);
    
    // Test removing empty directory
    std::string temp_dir = "/tmp/test_remove_dir_" + std::to_string(std::time(nullptr));
    FileSystem::create_directory(temp_dir);
    CHECK(FileSystem::exists(temp_dir) == true);
    CHECK(FileSystem::remove(temp_dir) == true);
    CHECK(FileSystem::exists(temp_dir) == false);
}

TEST_CASE("copy_file") {
    std::string source = "/tmp/test_copy_src_" + std::to_string(std::time(nullptr));
    std::string dest = "/tmp/test_copy_dst_" + std::to_string(std::time(nullptr));
    std::string content = "Test content for copying";
    
    // Write source file
    FileSystem::write_file(source, content);
    
    // Copy file
    CHECK(FileSystem::copy_file(source, dest) == true);
    
    // Verify destination exists and has same content
    CHECK(FileSystem::exists(dest) == true);
    CHECK(FileSystem::read_file(dest) == content);
    
    // Clean up
    FileSystem::remove(source);
    FileSystem::remove(dest);
}

TEST_CASE("copy_file - failure cases") {
    // Try to copy non-existent file
    CHECK(FileSystem::copy_file("/nonexistent_source_12345", "/tmp/dest") == false);
    
    // Try to copy to invalid destination
    std::string source = "/tmp/test_copy_fail_src_" + std::to_string(std::time(nullptr));
    FileSystem::write_file(source, "test");
    CHECK(FileSystem::copy_file(source, "/invalid/path/dest") == false);
    FileSystem::remove(source);
}

TEST_CASE("rename - file and directory") {
    // Test renaming file
    std::string old_file = "/tmp/test_rename_old_" + std::to_string(std::time(nullptr));
    std::string new_file = "/tmp/test_rename_new_" + std::to_string(std::time(nullptr));
    std::string content = "Test content";
    
    FileSystem::write_file(old_file, content);
    CHECK(FileSystem::rename(old_file, new_file) == true);
    CHECK(FileSystem::exists(old_file) == false);
    CHECK(FileSystem::exists(new_file) == true);
    CHECK(FileSystem::read_file(new_file) == content);
    
    // Clean up
    FileSystem::remove(new_file);
}

TEST_CASE("file_size") {
    std::string temp_file = "/tmp/test_size_" + std::to_string(std::time(nullptr));
    std::string content = "12345";
    
    // Write file with known content
    FileSystem::write_file(temp_file, content);
    
    // Check size
    CHECK(FileSystem::file_size(temp_file) == static_cast<long long>(content.size()));
    
    // Test with non-existent file
    CHECK(FileSystem::file_size("/nonexistent_file_12345") == -1);
    
    // Clean up
    FileSystem::remove(temp_file);
}

TEST_CASE("last_write_time") {
    std::string temp_file = "/tmp/test_time_" + std::to_string(std::time(nullptr));
    
    // Get time before creating file
    std::time_t before = std::time(nullptr);
    
    // Create file
    FileSystem::write_file(temp_file, "test");
    
    // Get modification time
    std::time_t mod_time = FileSystem::last_write_time(temp_file);
    
    // Should be valid and recent
    CHECK(mod_time >= before);
    CHECK(mod_time <= std::time(nullptr) + 1);
    
    // Test with non-existent file
    CHECK(FileSystem::last_write_time("/nonexistent_file_12345") == -1);
    
    // Clean up
    FileSystem::remove(temp_file);
}

TEST_CASE("directory_iterator") {
    std::string temp_dir = "/tmp/test_iterator_" + std::to_string(std::time(nullptr));
    
    // Create directory with some files
    FileSystem::create_directory(temp_dir);
    FileSystem::write_file(temp_dir + "/file1.txt", "content1");
    FileSystem::write_file(temp_dir + "/file2.txt", "content2");
    FileSystem::create_directory(temp_dir + "/subdir");
    
    // List directory contents
    std::vector<std::string> contents = FileSystem::directory_iterator(temp_dir);
    
    // Should have 3 entries
    CHECK(contents.size() == 3);
    
    // Clean up
    FileSystem::remove(temp_dir + "/file1.txt");
    FileSystem::remove(temp_dir + "/file2.txt");
    FileSystem::remove(temp_dir + "/subdir");
    FileSystem::remove(temp_dir);
}

TEST_CASE("directory_iterator - non-existent directory") {
    std::vector<std::string> contents = FileSystem::directory_iterator("/nonexistent_dir_12345");
    CHECK(contents.empty() == true);
}

TEST_CASE("temp_directory_path") {
    std::string temp_path = FileSystem::temp_directory_path();
    
    // Should not be empty
    CHECK(temp_path.empty() == false);
    
    // Should exist
    CHECK(FileSystem::exists(temp_path) == true);
    
    // Should be a directory
    CHECK(FileSystem::is_directory(temp_path) == true);
}

TEST_CASE("current_path - get and set") {
    // Get current directory
    std::string original = FileSystem::current_path();
    CHECK(original.empty() == false);
    
    // Change to /tmp
    CHECK(FileSystem::current_path("/tmp") == true);
    std::string current = FileSystem::current_path();
    // macOS may use /private/tmp
    bool is_tmp = (current == "/tmp" || current == "/private/tmp");
    CHECK(is_tmp == true);
    
    // Change back to original
    CHECK(FileSystem::current_path(original) == true);
}

TEST_CASE("current_path - failure case") {
    // Try to change to non-existent directory
    CHECK(FileSystem::current_path("/nonexistent_directory_12345") == false);
}

TEST_CASE("create_directories - nested path where parent creation would theoretically fail") {
    // This tests the recursive creation more thoroughly
    // Create a deep nested structure
    std::string deep_path = "/tmp/test_deep_" + std::to_string(std::time(nullptr));
    for (int i = 0; i < 5; i++) {
        deep_path += "/level" + std::to_string(i);
    }
    
    CHECK(FileSystem::create_directories(deep_path) == true);
    CHECK(FileSystem::is_directory(deep_path) == true);
    
    // Clean up from deepest to shallowest
    std::string cleanup = deep_path;
    for (int i = 0; i < 6; i++) {
        FileSystem::remove(cleanup);
        size_t pos = cleanup.find_last_of('/');
        if (pos != std::string::npos) {
            cleanup = cleanup.substr(0, pos);
        }
    }
}

TEST_CASE("read_file - with binary content") {
    std::string temp_file = "/tmp/test_binary_" + std::to_string(std::time(nullptr));
    
    // Write binary content (including null bytes)
    std::string binary_content;
    for (int i = 0; i < 256; i++) {
        binary_content += static_cast<char>(i);
    }
    
    FileSystem::write_file(temp_file, binary_content);
    std::string read_back = FileSystem::read_file(temp_file);
    
    CHECK(read_back.size() == binary_content.size());
    CHECK(read_back == binary_content);
    
    FileSystem::remove(temp_file);
}

TEST_CASE("copy_file - verify stream states") {
    std::string source = "/tmp/test_copy_verify_src_" + std::to_string(std::time(nullptr));
    std::string dest = "/tmp/test_copy_verify_dst_" + std::to_string(std::time(nullptr));
    
    // Create source with known content
    std::string content = "Content for copy verification test";
    FileSystem::write_file(source, content);
    
    // Copy and verify
    CHECK(FileSystem::copy_file(source, dest) == true);
    CHECK(FileSystem::read_file(dest) == content);
    
    // Clean up
    FileSystem::remove(source);
    FileSystem::remove(dest);
}

} // TEST_SUITE
