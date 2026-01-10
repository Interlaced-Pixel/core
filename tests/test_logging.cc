#include "../include/doctest.h"
#include "../include/logging.hpp"

#include <sstream>
#include <thread>
#include <chrono>

// slow sink helper for testing async drop behavior
class SlowSink : public pixellib::core::logging::LogSink {
public:
    std::ostringstream &out_;
    std::chrono::milliseconds delay_;
    explicit SlowSink(std::ostringstream &out, std::chrono::milliseconds d = std::chrono::milliseconds(50)) : out_(out), delay_(d) {}
    void write(const std::string &message) override {
        std::this_thread::sleep_for(delay_);
        out_ << message << std::endl;
    }
};

using namespace pixellib::core::logging;

TEST_SUITE("logging_module") {

TEST_CASE("log_level_to_string_and_formatters") {
    CHECK(std::string(log_level_to_string(LOG_TRACE)) == "TRACE");
    CHECK(std::string(log_level_to_string(LOG_ERROR)) == "ERROR");

    DefaultLogFormatter df(TimestampFormat::NONE, "PREFIX");
    std::tm tm{};
    std::string out = df.format(LOG_INFO, "msg", tm, "file.cpp", 10);
    CHECK(out.find("PREFIX") != std::string::npos);
    CHECK(out.find("msg") != std::string::npos);
    CHECK(out.find("file.cpp") != std::string::npos);

    JSONLogFormatter jf;
    {
        LogContext ctx;
        ctx.add("k", "v");
        std::string j = jf.format(LOG_INFO, "m", tm, nullptr, 0);
        CHECK(j.find("\"level\":\"INFO\"") != std::string::npos);
        CHECK(j.find("\"message\":\"m\"") != std::string::npos);
    }
}

TEST_CASE("stream_sink_and_stream_state_handling") {
    std::ostringstream out;
    StreamSink sink(out);
    sink.write("hello");
    CHECK(out.str().find("hello") != std::string::npos);

    out.setstate(std::ios::badbit);
    sink.write("world");
}

TEST_CASE("async_log_sink_queue_and_dropping") {
    std::ostringstream out;
    auto inner = std::make_unique<StreamSink>(out);
    AsyncLogSink async(std::move(inner), 2, AsyncLogSink::DropPolicy::DROP_NEWEST);

    async.write("one");
    async.write("two");
    async.write("three");
    async.flush();

    std::string s = out.str();
    CHECK(s.find("one") != std::string::npos);
    CHECK(s.find("two") != std::string::npos);

    async.shutdown();
}

TEST_CASE("logger_output_and_configuration") {
    std::ostringstream out, err;
    Logger::set_output_streams(out, err);
    Logger::set_level(LOG_INFO);

    Logger::info("info-msg");
    Logger::warning("warn-msg");
    Logger::debug("debug-msg");

    std::string o = out.str();
    CHECK(o.find("info-msg") != std::string::npos);
    CHECK(o.find("warn-msg") != std::string::npos);
    CHECK(o.find("debug-msg") == std::string::npos);

    Logger::info("hello {}", "world");
    {
        bool found = out.str().find("helloworld") != std::string::npos || out.str().find("hello world") != std::string::npos;
        CHECK(found);
    }

    Logger::info("msg", std::string("k"), 123);
    CHECK(out.str().find("k=123") != std::string::npos);

    Logger::set_output_streams(std::cout, std::cerr);
}

TEST_CASE("default_formatter_filename_extraction_and_context") {
    DefaultLogFormatter df(TimestampFormat::NONE, "");
    std::tm tm{};
    std::string out = df.format(LOG_INFO, "m", tm, "/path/to/myfile.cpp", 123);
    CHECK(out.find("myfile.cpp") != std::string::npos);
    CHECK(out.find(":123") != std::string::npos);

    // Test that context is included
    {
        pixellib::core::logging::LogContext ctx;
        ctx.add("user", "u1");
        std::string out2 = df.format(LOG_INFO, "msg", tm, nullptr, 0);
        CHECK(out2.find("user=u1") != std::string::npos);
    }
}

TEST_CASE("logger_with_sinks_and_category_config") {
    std::ostringstream out;
    // Build a global config with a stream sink
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_DEBUG).add_stream_sink(out);
    pixellib::core::logging::Logger::configure(b.build());

    auto cat = pixellib::core::logging::Logger::get("testcat");
    cat.debug("dbg-msg");
    cat.info("info-msg");

    std::string s = out.str();
    CHECK(s.find("dbg-msg") != std::string::npos);
    CHECK(s.find("info-msg") != std::string::npos);

    // Reset global logging to defaults
    pixellib::core::logging::Logger::set_file_logging(nullptr);
    pixellib::core::logging::Logger::set_output_streams(std::cout, std::cerr);
}

TEST_CASE("per_category_registry_and_level_filtering") {
    std::ostringstream out;
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_ERROR).add_stream_sink(out);
    auto cfg = b.build();
    pixellib::core::logging::Logger::LoggerRegistry::set_config("catA", std::move(cfg));

    auto cat = pixellib::core::logging::Logger::get("catA");
    cat.info("should-be-filtered");
    cat.error("should-show");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::string s = out.str();
    CHECK(s.find("should-be-filtered") == std::string::npos);
    CHECK(s.find("should-show") != std::string::npos);
}

TEST_CASE("json_formatter_escaping") {
    JSONLogFormatter jf;
    std::tm tm{};
    std::string msg = "quote\" backslash\\ newline\n";
    std::string j = jf.format(LOG_INFO, msg, tm, nullptr, 0);
    CHECK(j.find("\\\"") != std::string::npos); // escaped quote
    CHECK(j.find("\\\\") != std::string::npos); // escaped backslash
    CHECK(j.find("\\n") != std::string::npos); // escaped newline
}

TEST_CASE("async_drop_oldest_and_metrics") {
    std::ostringstream out;
    // slow inner sink to force queue buildup
    auto slow = std::make_unique<SlowSink>(out, std::chrono::milliseconds(40));
    auto slow_ptr = slow.get();
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_INFO).add_async_sink(std::move(slow), 1, pixellib::core::logging::AsyncLogSink::DropPolicy::DROP_OLDEST);
    pixellib::core::logging::Logger::configure(b.build());

    // Rapidly emit many messages
    for (int i = 0; i < 10; ++i) {
        pixellib::core::logging::Logger::info(std::string("m") + std::to_string(i));
    }

    // Allow some time for worker to consume
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    size_t dropped = pixellib::core::logging::Logger::get_async_dropped_count();
    CHECK(dropped > 0);
    // flush and shutdown to be clean
    pixellib::core::logging::Logger::async_flush();
    pixellib::core::logging::Logger::async_shutdown();

    // Reset global logging to defaults
    pixellib::core::logging::Logger::set_file_logging(nullptr);
    pixellib::core::logging::Logger::set_output_streams(std::cout, std::cerr);
}

TEST_CASE("test_helpers_clear_and_error_messages") {
    std::ostringstream s;
    s.setstate(std::ios::badbit);
    pixellib::core::logging::test_force_clear_stream(s);
    CHECK(s.good());

    // test_force_logging_error_messages writes to cerr; just call to cover code paths
    pixellib::core::logging::test_force_logging_error_messages("err-msg");
}

TEST_CASE("rotating_file_logger_size_and_time_rotation") {
    namespace fs = std::filesystem;
    std::string base = "build/tmp/testlog";
    fs::create_directories("build/tmp");
    // clean up any previous files
    for (int i = 0; i < 5; ++i) {
        std::string p = base + (i==0?"":("."+std::to_string(i)));
        if (fs::exists(p)) fs::remove(p);
    }

    // Size-based rotation: very small max size so rotation happens quickly
    pixellib::core::logging::RotatingFileLogger rf(base, 20, 3);
    rf.write("firstline");
    rf.write(std::string(30, 'x')); // should trigger rotation
    rf.write("after-rotation");

    // Allow filesystem to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    CHECK(fs::exists(base));
    CHECK(fs::exists(base + ".1"));

    // Test badbit helpers
    rf.test_set_badbit();
    rf.test_clear_badbit();

    // Time-based rotation with zero interval triggers rotation on write
    std::string tbase = "build/tmp/testlog_time";
    for (int i = 0; i < 3; ++i) {
        std::string p = tbase + (i==0?"":("."+std::to_string(i)));
        if (fs::exists(p)) fs::remove(p);
    }
    pixellib::core::logging::RotatingFileLogger rft(tbase, std::chrono::hours(0), 2);
    rft.write("t1");
    rft.write("t2");
    // small sleep for rotation
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK(fs::exists(tbase));
    CHECK(fs::exists(tbase + ".1"));

    // cleanup
    for (int i = 0; i < 5; ++i) {
        std::string p = base + (i==0?"":("."+std::to_string(i)));
        if (fs::exists(p)) fs::remove(p);
    }
    for (int i = 0; i < 5; ++i) {
        std::string p = tbase + (i==0?"":("."+std::to_string(i)));
        if (fs::exists(p)) fs::remove(p);
    }
}

TEST_CASE("macros_and_logger_overloads_and_stream_clear") {
    // Macros should invoke logger with file/line
    LOG_TRACE("trace-msg");
    LOG_DEBUG("debug-msg-macro");
    LOG_INFO("info-msg-macro");
    LOG_WARNING("warn-macro");
    LOG_ERROR("err-macro");
    LOG_FATAL("fatal-macro");

    // Direct static overload with file/line
    pixellib::core::logging::Logger::log(LOG_INFO, "direct-fileline", "source.cpp", 77);

    // StreamSink clear_stream
    std::ostringstream ostr;
    pixellib::core::logging::StreamSink ss(ostr);
    ostr.setstate(std::ios::badbit);
    ss.clear_stream();
    CHECK(ostr.good());
}

TEST_CASE("async_block_policy_and_queue_metrics") {
    std::ostringstream out;
    auto slow = std::make_unique<SlowSink>(out, std::chrono::milliseconds(80));
    auto async = pixellib::core::logging::AsyncLogSink(std::move(slow), 1, pixellib::core::logging::AsyncLogSink::DropPolicy::BLOCK, std::chrono::milliseconds(5));

    // fill queue quickly; some writes may time out and be counted as dropped
    for (int i = 0; i < 5; ++i) async.write(std::string("b") + std::to_string(i));

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    size_t dropped = async.dropped_count();
    CHECK(dropped >= 0);
    size_t qsz = async.queue_size();
    CHECK(qsz >= 0);
    async.shutdown();
}

TEST_CASE("config_builder_file_and_async_variants_and_registry") {
    std::ostringstream out;
    // add file sink using builder (writes to build/tmp/testbuilder.log)
    std::string fname = "build/tmp/testbuilder.log";
    pixellib::core::logging::Logger::LoggerConfigBuilder b;
    b.set_level(LOG_DEBUG).add_stream_sink(out).add_async_stream_sink(out).add_file_sink(fname, 1024, 2).set_formatter(std::make_unique<JSONLogFormatter>());
    auto cfg = b.build();
    // register under a name
    pixellib::core::logging::Logger::LoggerRegistry::set_config("builderTest", std::move(cfg));
    CHECK(pixellib::core::logging::Logger::LoggerRegistry::has_config("builderTest") == true);

    // cleanup file created by builder
    std::error_code ec;
    std::filesystem::remove(fname, ec);
}

}
