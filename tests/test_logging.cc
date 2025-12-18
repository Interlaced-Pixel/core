#include "doctest.h"
#include "interlaced_core/logging.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using namespace interlaced::core::logging;

TEST_SUITE("logging module") {

TEST_CASE("log_level_to_string") {
    CHECK(log_level_to_string(LOG_DEBUG) == "DEBUG");
    CHECK(log_level_to_string(LOG_INFO) == "INFO");
    CHECK(log_level_to_string(LOG_WARNING) == "WARNING");
    CHECK(log_level_to_string(LOG_ERROR) == "ERROR");
}

TEST_CASE("DefaultLogFormatter - standard timestamp") {
    DefaultLogFormatter formatter(TimestampFormat::STANDARD);
    std::tm time_info = {};
    time_info.tm_year = 123; // 2023
    time_info.tm_mon = 0;    // January
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_INFO, "Test message", time_info);
    CHECK(result.find("[2023-01-15 14:30:45] [INFO] Test message") != std::string::npos);
}

TEST_CASE("DefaultLogFormatter - ISO8601 timestamp") {
    DefaultLogFormatter formatter(TimestampFormat::ISO8601);
    std::tm time_info = {};
    time_info.tm_year = 123;
    time_info.tm_mon = 0;
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_WARNING, "Warning message", time_info);
    CHECK(result.find("[2023-01-15T14:30:45Z] [WARNING] Warning message") != std::string::npos);
}

TEST_CASE("DefaultLogFormatter - no timestamp") {
    DefaultLogFormatter formatter(TimestampFormat::NONE);
    std::tm time_info = {};
    std::string result = formatter.format(LOG_ERROR, "Error message", time_info);
    CHECK(result == "[ERROR] Error message");
}

TEST_CASE("DefaultLogFormatter - with prefix") {
    DefaultLogFormatter formatter(TimestampFormat::STANDARD, "PREFIX");
    std::tm time_info = {};
    time_info.tm_year = 123;
    time_info.tm_mon = 0;
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_DEBUG, "Debug message", time_info);
    CHECK(result.find("PREFIX [2023-01-15 14:30:45] [DEBUG] Debug message") != std::string::npos);
}

TEST_CASE("DefaultLogFormatter - with file and line") {
    DefaultLogFormatter formatter(TimestampFormat::STANDARD);
    std::tm time_info = {};
    time_info.tm_year = 123;
    time_info.tm_mon = 0;
    time_info.tm_mday = 15;
    time_info.tm_hour = 14;
    time_info.tm_min = 30;
    time_info.tm_sec = 45;

    std::string result = formatter.format(LOG_INFO, "Test message", time_info, "/path/to/file.cpp", 42);
    CHECK(result.find("Test message (file.cpp:42)") != std::string::npos);
}

TEST_CASE("Logger - set and get level") {
    // Reset to default
    Logger::set_level(LOG_INFO);

    // Test setting different levels
    Logger::set_level(LOG_DEBUG);
    // Note: We can't directly test the current level since it's private,
    // but we can test behavior by logging and capturing output

    Logger::set_level(LOG_ERROR);
    Logger::set_level(LOG_INFO); // Reset for other tests
}

TEST_CASE("Logger - basic logging to streams") {
    // Capture output
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    Logger::debug("Debug message");
    Logger::info("Info message");
    Logger::warning("Warning message");
    Logger::error("Error message");

    std::string output = output_buffer.str();
    std::string error = error_buffer.str();

    CHECK(output.find("Debug message") != std::string::npos);
    CHECK(output.find("Info message") != std::string::npos);
    CHECK(output.find("Warning message") != std::string::npos);
    CHECK(error.find("Error message") != std::string::npos);

    // Reset streams
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - level filtering") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_WARNING); // Only warnings and errors should appear

    Logger::debug("Debug message"); // Should be filtered out
    Logger::info("Info message");   // Should be filtered out
    Logger::warning("Warning message");
    Logger::error("Error message");

    std::string output = output_buffer.str();
    std::string error = error_buffer.str();

    CHECK(output.find("Debug message") == std::string::npos);
    CHECK(output.find("Info message") == std::string::npos);
    CHECK(output.find("Warning message") != std::string::npos);
    CHECK(error.find("Error message") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - structured logging") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    Logger::info("User logged in", "user_id", 12345, "ip_address", "192.168.1.1");

    std::string output = output_buffer.str();
    CHECK(output.find("User logged in user_id=12345 ip_address=192.168.1.1") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - type-safe formatting") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    Logger::info("User {} logged in from {}", "alice", "192.168.1.1");

    std::string output = output_buffer.str();
    CHECK(output.find("User alice logged in from 192.168.1.1") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - custom formatter") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Set custom formatter
    auto custom_formatter = std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE, "CUSTOM");
    Logger::set_formatter(std::move(custom_formatter));

    Logger::info("Test message");

    std::string output = output_buffer.str();
    CHECK(output.find("CUSTOM [INFO] Test message") != std::string::npos);

    // Reset formatter
    Logger::set_formatter(nullptr);
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("RotatingFileLogger - basic file creation") {
    // Create a temporary directory for testing
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_logs";
    fs::create_directories(temp_dir);

    std::string base_filename = (temp_dir / "test.log").string();

    {
        RotatingFileLogger logger(base_filename, 1024, 3); // 1KB max, 3 files

        logger.write("Test message 1");
        logger.write("Test message 2");
    }

    // Check that file was created and contains messages
    CHECK(fs::exists(base_filename));

    std::ifstream file(base_filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("Test message 1") != std::string::npos);
    CHECK(content.find("Test message 2") != std::string::npos);

    // Cleanup
    fs::remove_all(temp_dir);
}

TEST_CASE("RotatingFileLogger - size-based rotation") {
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_logs_rotation";
    fs::create_directories(temp_dir);

    std::string base_filename = (temp_dir / "rotate.log").string();

    {
        RotatingFileLogger logger(base_filename, 50, 3); // Very small max size

        // Write messages that will exceed the size limit
        logger.write("This is a long message that will cause rotation");
        logger.write("Another long message to trigger rotation again");
        logger.write("Final message");
    }

    // Check that rotation occurred
    CHECK(fs::exists(base_filename + ".1"));
    CHECK(fs::exists(base_filename + ".2"));

    // Cleanup
    fs::remove_all(temp_dir);
}

TEST_CASE("Logger - file logging") {
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_file_logging";
    fs::create_directories(temp_dir);

    std::string log_filename = (temp_dir / "app.log").string();

    Logger::set_file_logging(log_filename, 1024, 2);
    Logger::set_level(LOG_DEBUG);

    Logger::info("Application started");
    Logger::warning("This is a warning");
    Logger::error("This is an error");

    // Disable file logging to flush
    Logger::set_file_logging(nullptr);

    // Check log file
    CHECK(fs::exists(log_filename));

    std::ifstream file(log_filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    CHECK(content.find("Application started") != std::string::npos);
    CHECK(content.find("This is a warning") != std::string::npos);
    CHECK(content.find("This is an error") != std::string::npos);

    // Cleanup
    fs::remove_all(temp_dir);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - time-based file rotation") {
    fs::path temp_dir = fs::temp_directory_path() / "interlaced_test_time_rotation";
    fs::create_directories(temp_dir);

    std::string log_filename = (temp_dir / "time_rotate.log").string();

    // Set up time-based rotation with very short interval for testing
    Logger::set_file_logging(log_filename, std::chrono::hours(1), 3);
    Logger::set_level(LOG_DEBUG);

    Logger::info("First message");

    // Simulate time passing (in a real scenario, this would happen over time)
    // For testing, we'll just write multiple messages

    Logger::info("Second message");
    Logger::info("Third message");

    Logger::set_file_logging(nullptr);

    CHECK(fs::exists(log_filename));

    // Cleanup
    fs::remove_all(temp_dir);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logger - thread safety") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Run logging from multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i]() {
            Logger::info("Message from thread {}", i);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::string output = output_buffer.str();

    // Check that all messages were logged (thread safety)
    for (int i = 0; i < 10; ++i) {
        CHECK(output.find("Message from thread " + std::to_string(i)) != std::string::npos);
    }

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

TEST_CASE("Logging macros") {
    std::ostringstream output_buffer;
    std::ostringstream error_buffer;

    Logger::set_output_streams(output_buffer, error_buffer);
    Logger::set_level(LOG_DEBUG);

    // Test macros
    LOG_DEBUG("Debug macro test");
    LOG_INFO("Info macro test");
    LOG_WARNING("Warning macro test");
    LOG_ERROR("Error macro test");

    std::string output = output_buffer.str();
    std::string error = error_buffer.str();

    CHECK(output.find("Debug macro test") != std::string::npos);
    CHECK(output.find("Info macro test") != std::string::npos);
    CHECK(output.find("Warning macro test") != std::string::npos);
    CHECK(error.find("Error macro test") != std::string::npos);

    // Check that file and line info is included
    CHECK(output.find("test_logging.cc") != std::string::npos);

    // Reset
    Logger::set_output_streams(std::cout, std::cerr);
    Logger::set_level(LOG_INFO);
}

} // TEST_SUITE