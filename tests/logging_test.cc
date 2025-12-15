#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <regex>
#include <sstream>
#include <thread>

#include "interlaced_core/logging.hpp"

// Helper functions to reset logger state and clean test files.
namespace {
void CleanupTestFiles() {
  std::filesystem::remove("test_log.txt");
  std::filesystem::remove("test_log.txt.1");
  std::filesystem::remove("test_log.txt.2");
}

void ResetLoggerState() {
  CleanupTestFiles();
  interlaced::core::logging::Logger::set_level(interlaced::core::logging::LOG_DEBUG);
  interlaced::core::logging::Logger::set_output_streams(std::cout, std::cerr);
  interlaced::core::logging::Logger::set_formatter(nullptr);
  interlaced::core::logging::Logger::set_file_logging(nullptr);
}
} // namespace

// Test log_level_to_string function
TEST(Logging, LogLevelToString) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  EXPECT_STREQ(log_level_to_string(LOG_DEBUG), "DEBUG");
  EXPECT_STREQ(log_level_to_string(LOG_INFO), "INFO");
  EXPECT_STREQ(log_level_to_string(LOG_WARNING), "WARNING");
  EXPECT_STREQ(log_level_to_string(LOG_ERROR), "ERROR");
  EXPECT_STREQ(log_level_to_string(static_cast<LogLevel>(999)), "UNKNOWN");
}

// Test Logger::set_level and level filtering
TEST(Logging, SetLevel) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  // Capture output
  std::stringstream output;
  std::stringstream error_output;
  Logger::set_output_streams(output, error_output);

  // Set level to WARNING
  Logger::set_level(LOG_WARNING);

  // Debug and info should not appear
  Logger::debug("Debug message");
  Logger::info("Info message");
  EXPECT_TRUE(output.str().empty());

  // Warning and error should appear
  Logger::warning("Warning message");
  Logger::error("Error message");

  std::string output_str = output.str();
  std::string error_str = error_output.str();

  EXPECT_TRUE(output_str.find("Warning message") != std::string::npos);
  EXPECT_TRUE(error_str.find("Error message") != std::string::npos);
}

// Test basic logging functionality
TEST(Logging, BasicLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  std::stringstream error_output;
  Logger::set_output_streams(output, error_output);

  Logger::debug("Debug message");
  Logger::info("Info message");
  Logger::warning("Warning message");
  Logger::error("Error message");

  std::string output_str = output.str();
  std::string error_str = error_output.str();

  EXPECT_TRUE(output_str.find("[DEBUG] Debug message") != std::string::npos);
  EXPECT_TRUE(output_str.find("[INFO] Info message") != std::string::npos);
  EXPECT_TRUE(output_str.find("[WARNING] Warning message") !=
              std::string::npos);
  EXPECT_TRUE(error_str.find("[ERROR] Error message") != std::string::npos);
}

// Test logging with file and line information
TEST(Logging, LoggingWithFileLine) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Test message", "test.cpp", 42);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Test message") != std::string::npos);
  EXPECT_TRUE(output_str.find("test.cpp:42") != std::string::npos);
}

// Test structured logging with key-value pairs
TEST(Logging, StructuredLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("User login", "user_id", 12345, "ip", "192.168.1.1");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("User login") != std::string::npos);
  EXPECT_TRUE(output_str.find("user_id=12345") != std::string::npos);
  EXPECT_TRUE(output_str.find("ip=192.168.1.1") != std::string::npos);
}

// Test custom formatter
TEST(Logging, CustomFormatter) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  class TestFormatter : public LogFormatter {
  public:
    std::string format(LogLevel level, const std::string &message,
                       const std::tm &time_info, const char *file = nullptr,
                       int line = 0) override {
      return "CUSTOM: " + message;
    }
  };

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);
  Logger::set_formatter(std::make_unique<TestFormatter>());

  Logger::info("Test message");

  EXPECT_TRUE(output.str().find("CUSTOM: Test message") != std::string::npos);
}

// Test DefaultLogFormatter
TEST(Logging, DefaultLogFormatter) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  DefaultLogFormatter formatter(TimestampFormat::STANDARD, "PREFIX");

  std::tm time_info = {};
  time_info.tm_year = 125; // 2025
  time_info.tm_mon = 11;   // December
  time_info.tm_mday = 15;
  time_info.tm_hour = 12;
  time_info.tm_min = 30;
  time_info.tm_sec = 45;

  std::string result =
      formatter.format(LOG_INFO, "Test message", time_info, "file.cpp", 100);

  EXPECT_TRUE(result.find("PREFIX") != std::string::npos);
  EXPECT_TRUE(result.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(result.find("Test message") != std::string::npos);
  EXPECT_TRUE(result.find("file.cpp:100") != std::string::npos);
}

// Test DefaultLogFormatter with different timestamp formats
TEST(Logging, DefaultLogFormatterTimestampFormats) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::tm time_info = {};
  time_info.tm_year = 125;
  time_info.tm_mon = 11;
  time_info.tm_mday = 15;
  time_info.tm_hour = 12;
  time_info.tm_min = 30;
  time_info.tm_sec = 45;

  // Test ISO8601 format
  DefaultLogFormatter iso_formatter(TimestampFormat::ISO8601);
  std::string iso_result = iso_formatter.format(LOG_DEBUG, "Test", time_info);
  EXPECT_TRUE(iso_result.find("2025-12-15T12:30:45Z") != std::string::npos);

  // Test NONE format (no timestamp, but still include level)
  DefaultLogFormatter none_formatter(TimestampFormat::NONE);
  std::string none_result = none_formatter.format(LOG_DEBUG, "Test", time_info);
  // Ensure there is no timestamp-like prefix (e.g., "2025-")
  EXPECT_TRUE(none_result.find("2025-") == std::string::npos);
  EXPECT_TRUE(none_result.find("[DEBUG] Test") != std::string::npos);
}

// Test RotatingFileLogger size-based rotation
TEST(Logging, RotatingFileLoggerSizeBased) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  const size_t max_size = 100; // Small size for testing

  RotatingFileLogger logger(filename, max_size, 2);

  // Write messages until rotation occurs (small number to make rotation behavior deterministic)
  std::string long_message = "This is a long message that should trigger "
                             "rotation when written multiple times.";
  for (int i = 0; i < 2; ++i) {
    logger.write(long_message);
  }

  // Check that files exist
  EXPECT_TRUE(std::filesystem::exists(filename));
  EXPECT_TRUE(std::filesystem::exists(filename + ".1"));
  EXPECT_FALSE(std::filesystem::exists(filename + ".2")); // Should not exist yet

  // Write more to trigger another rotation
  for (int i = 0; i < 2; ++i) {
    logger.write(long_message);
  }

  EXPECT_TRUE(std::filesystem::exists(filename + ".2"));
}

// Test RotatingFileLogger time-based rotation (simulated with size)
TEST(Logging, RotatingFileLoggerTimeBased) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";

  // Use size-based rotation for testing (time-based would require waiting)
  RotatingFileLogger logger(filename, 50, 2);

  logger.write("First message - this should trigger rotation soon");

  logger.write("Second message - should be in new file");

  // Check that files exist
  EXPECT_TRUE(std::filesystem::exists(filename));
  EXPECT_TRUE(std::filesystem::exists(filename + ".1"));
}

// Test file logging integration
TEST(Logging, FileLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  Logger::set_file_logging(filename, 1024, 3);

  Logger::info("File log message");
  Logger::error("File error message");

  // Force flush by setting different logging
  Logger::set_file_logging(nullptr);

  // Check file contents
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open());

  std::string line;
  std::getline(file, line);
  EXPECT_TRUE(line.find("[INFO] File log message") != std::string::npos);

  std::getline(file, line);
  EXPECT_TRUE(line.find("[ERROR] File error message") != std::string::npos);
}

// Test thread safety
TEST(Logging, ThreadSafety) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  const int num_threads = 10;
  const int messages_per_thread = 100;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([i, messages_per_thread]() {
      for (int j = 0; j < messages_per_thread; ++j) {
        Logger::info("Thread " + std::to_string(i) + " message " +
                     std::to_string(j));
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  std::string output_str = output.str();

  // Count total messages (should be num_threads * messages_per_thread)
  std::regex message_regex(R"(\[INFO\])");
  auto matches_begin =
      std::sregex_iterator(output_str.begin(), output_str.end(), message_regex);
  auto matches_end = std::sregex_iterator();
  int message_count = std::distance(matches_begin, matches_end);

  EXPECT_EQ(message_count, num_threads * messages_per_thread);
}

// Test macros
TEST(Logging, Macros) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  std::stringstream error_output;
  Logger::set_output_streams(output, error_output);

  LOG_INFO("Macro info message");
  LOG_WARNING("Macro warning message");
  LOG_ERROR("Macro error message");

  std::string output_str = output.str();
  std::string error_str = error_output.str();

  EXPECT_TRUE(output_str.find("Macro info message") != std::string::npos);
  EXPECT_TRUE(output_str.find("Macro warning message") != std::string::npos);
  EXPECT_TRUE(error_str.find("Macro error message") != std::string::npos);
}

// Test variadic template logging
TEST(Logging, VariadicTemplateLogging) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("User {} logged in from {}", "john_doe", "192.168.1.1");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("User john_doe logged in from 192.168.1.1") !=
              std::string::npos);
}

// Test error handling in RotatingFileLogger
TEST(Logging, RotatingFileLoggerErrorHandling) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  // Try to create logger with invalid path
  const std::string invalid_path = "/invalid/path/test.log";
  RotatingFileLogger logger(invalid_path, 1024, 2);

  // This should not crash, should fall back to cerr
  testing::internal::CaptureStderr();
  logger.write("Test message");
  std::string captured = testing::internal::GetCapturedStderr();

  EXPECT_TRUE(captured.find("Test message") != std::string::npos);
}

// Test logger with custom streams
TEST(Logging, CustomStreams) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream custom_output;
  std::stringstream custom_error;

  Logger::set_output_streams(custom_output, custom_error);

  Logger::info("Info to custom stream");
  Logger::error("Error to custom stream");

  EXPECT_TRUE(custom_output.str().find("Info to custom stream") !=
              std::string::npos);
  EXPECT_TRUE(custom_error.str().find("Error to custom stream") !=
              std::string::npos);
}

// Test log level filtering edge cases
TEST(Logging, LogLevelFiltering) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  // Capture both normal and error output in the same buffer for this test
  Logger::set_output_streams(output, output);

  // Set to ERROR level - only errors should pass
  Logger::set_level(LOG_ERROR);

  Logger::debug("Debug - should not appear");
  Logger::info("Info - should not appear");
  Logger::warning("Warning - should not appear");
  Logger::error("Error - should appear");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Debug") == std::string::npos);
  EXPECT_TRUE(output_str.find("Info") == std::string::npos);
  EXPECT_TRUE(output_str.find("Warning") == std::string::npos);
  EXPECT_TRUE(output_str.find("Error") != std::string::npos);
}

// Test timestamp formatting in logs
TEST(Logging, TimestampFormatting) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Test message");

  std::string output_str = output.str();

  // Check for timestamp format YYYY-MM-DD HH:MM:SS
  std::regex timestamp_regex(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\])");
  EXPECT_TRUE(std::regex_search(output_str, timestamp_regex));
}

// Test file path extraction in logging
TEST(Logging, FilePathExtraction) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Test", "/full/path/to/file.cpp", 123);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("file.cpp:123") != std::string::npos);
  EXPECT_TRUE(output_str.find("/full/path/to/") == std::string::npos);
}

// Test multiple rotations in RotatingFileLogger
TEST(Logging, MultipleRotations) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  const std::string filename = "test_log.txt";
  const size_t max_size = 50; // Very small for testing

  RotatingFileLogger logger(filename, max_size, 3);

  // Write enough to cause multiple rotations
  for (int i = 0; i < 20; ++i) {
    logger.write("Message " + std::to_string(i));
  }

  EXPECT_TRUE(std::filesystem::exists(filename));
  EXPECT_TRUE(std::filesystem::exists(filename + ".1"));
  EXPECT_TRUE(std::filesystem::exists(filename + ".2"));
  EXPECT_TRUE(std::filesystem::exists(filename + ".3"));
}

// Test empty message logging
TEST(Logging, EmptyMessage) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("[INFO]") != std::string::npos);
}

// Test logging with special characters
TEST(Logging, SpecialCharacters) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info(
      "Message with special chars: \n\t\"quotes\" 'apostrophes' @#$%^&*()");

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("Message with special chars") !=
              std::string::npos);
}

// Test formatter with null file parameter
TEST(Logging, FormatterNullFile) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  DefaultLogFormatter formatter;

  std::tm time_info = {};
  std::string result =
      formatter.format(LOG_INFO, "Test", time_info, nullptr, 0);

  EXPECT_TRUE(result.find("Test") != std::string::npos);
  // Ensure no file/line suffix (i.e., no parentheses containing filename:line)
  EXPECT_TRUE(result.find("(") == std::string::npos); // No file/line information
}

// Test structured logging with various data types
TEST(Logging, StructuredLoggingTypes) {
  ResetLoggerState();
  using namespace interlaced::core::logging;

  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::info("Test", "int", 42, "float", 3.14f, "string",
               std::string("hello"), "bool", true);

  std::string output_str = output.str();
  EXPECT_TRUE(output_str.find("int=42") != std::string::npos);
  EXPECT_TRUE(output_str.find("float=3.14") != std::string::npos);
  EXPECT_TRUE(output_str.find("string=hello") != std::string::npos);
  EXPECT_TRUE(output_str.find("bool=1") != std::string::npos);
}

// Test logger reset functionality
TEST(Logging, LoggerReset) {
  using namespace interlaced::core::logging;

  // Set custom settings
  Logger::set_level(LOG_ERROR);
  Logger::set_formatter(
      std::make_unique<DefaultLogFormatter>(TimestampFormat::NONE));

  // Reset via helper
  ResetLoggerState();

  // Verify reset
  std::stringstream output;
  Logger::set_output_streams(output, std::cerr);

  Logger::debug("Should appear now");
  EXPECT_TRUE(output.str().find("Should appear now") != std::string::npos);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}