#include "doctest.h"
#include "json.hpp"

#include <string>
#include <sstream>
#include <limits>

using namespace interlaced::core::json;

TEST_SUITE("json_module") {

// ============================================================================
// Basic Type Parsing Tests
// ============================================================================

TEST_CASE("parse_null") {
    JSON result;
    bool success = JSON::parse("null", result);
    CHECK(success == true);
    CHECK(result.is_null() == true);
    CHECK(result.type() == JSON::Type::Null);
}

TEST_CASE("parse_bool_true") {
    JSON result;
    bool success = JSON::parse("true", result);
    CHECK(success == true);
    CHECK(result.is_bool() == true);
    CHECK(result.as_bool() == true);
}

TEST_CASE("parse_bool_false") {
    JSON result;
    bool success = JSON::parse("false", result);
    CHECK(success == true);
    CHECK(result.is_bool() == true);
    CHECK(result.as_bool() == false);
}

TEST_CASE("parse_integer") {
    JSON result;
    bool success = JSON::parse("42", result);
    CHECK(success == true);
    CHECK(result.is_number() == true);
    CHECK(result.as_number().to_int64() == 42);
    CHECK(result.as_number().is_integral() == true);
}

TEST_CASE("parse_negative_integer") {
    JSON result;
    bool success = JSON::parse("-123", result);
    CHECK(success == true);
    CHECK(result.is_number() == true);
    CHECK(result.as_number().to_int64() == -123);
}

TEST_CASE("parse_zero") {
    JSON result;
    bool success = JSON::parse("0", result);
    CHECK(success == true);
    CHECK(result.is_number() == true);
    CHECK(result.as_number().to_int64() == 0);
}

TEST_CASE("parse_floating_point") {
    JSON result;
    bool success = JSON::parse("3.14", result);
    CHECK(success == true);
    CHECK(result.is_number() == true);
    CHECK(result.as_number().to_double() - 3.14 < 0.001);
    CHECK(result.as_number().is_integral() == false);
}

TEST_CASE("parse_scientific_notation") {
    JSON result;
    bool success = JSON::parse("1.5e10", result);
    CHECK(success == true);
    CHECK(result.is_number() == true);
    CHECK(result.as_number().to_double() - 1.5e10 < 1e5);
}

TEST_CASE("parse_scientific_notation_negative_exponent") {
    JSON result;
    bool success = JSON::parse("2.5e-3", result);
    CHECK(success == true);
    CHECK(result.is_number() == true);
    CHECK(result.as_number().to_double() - 0.0025 < 0.0001);
}

TEST_CASE("parse_simple_string") {
    JSON result;
    bool success = JSON::parse("\"hello\"", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    CHECK(result.as_string() == "hello");
}

TEST_CASE("parse_empty_string") {
    JSON result;
    bool success = JSON::parse("\"\"", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    CHECK(result.as_string() == "");
}

TEST_CASE("parse_string_with_escapes") {
    JSON result;
    bool success = JSON::parse(R"("Hello\nWorld\tTest")", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    CHECK(result.as_string() == "Hello\nWorld\tTest");
}

TEST_CASE("parse_string_with_quotes") {
    JSON result;
    bool success = JSON::parse(R"("Say \"Hi\"")", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    CHECK(result.as_string() == "Say \"Hi\"");
}

TEST_CASE("parse_string_with_backslash") {
    JSON result;
    bool success = JSON::parse(R"("C:\\path\\to\\file")", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    CHECK(result.as_string() == "C:\\path\\to\\file");
}

TEST_CASE("parse_string_with_unicode_escape") {
    JSON result;
    bool success = JSON::parse(R"("\u0041")", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    CHECK(result.as_string() == "A");
}

TEST_CASE("parse_string_with_unicode_multibyte") {
    JSON result;
    bool success = JSON::parse(R"("\u00E9")", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    // Unicode character é (U+00E9) encodes to 2 bytes in UTF-8:
    // 0xC3 0xA9, as it falls in the range U+0080 to U+07FF
    CHECK(result.as_string().size() == 2);
}

TEST_CASE("parse_empty_array") {
    JSON result;
    bool success = JSON::parse("[]", result);
    CHECK(success == true);
    CHECK(result.is_array() == true);
    CHECK(result.as_array().size() == 0);
}

TEST_CASE("parse_array_with_elements") {
    JSON result;
    bool success = JSON::parse("[1, 2, 3]", result);
    CHECK(success == true);
    CHECK(result.is_array() == true);
    CHECK(result.as_array().size() == 3);
    CHECK(result.as_array()[0].as_number().to_int64() == 1);
    CHECK(result.as_array()[1].as_number().to_int64() == 2);
    CHECK(result.as_array()[2].as_number().to_int64() == 3);
}

TEST_CASE("parse_array_mixed_types") {
    JSON result;
    bool success = JSON::parse(R"([1, "hello", true, null])", result);
    CHECK(success == true);
    CHECK(result.is_array() == true);
    CHECK(result.as_array().size() == 4);
    CHECK(result.as_array()[0].is_number() == true);
    CHECK(result.as_array()[1].is_string() == true);
    CHECK(result.as_array()[2].is_bool() == true);
    CHECK(result.as_array()[3].is_null() == true);
}

TEST_CASE("parse_nested_array") {
    JSON result;
    bool success = JSON::parse("[[1, 2], [3, 4]]", result);
    CHECK(success == true);
    CHECK(result.is_array() == true);
    CHECK(result.as_array().size() == 2);
    CHECK(result.as_array()[0].is_array() == true);
    CHECK(result.as_array()[0].as_array().size() == 2);
}

TEST_CASE("parse_empty_object") {
    JSON result;
    bool success = JSON::parse("{}", result);
    CHECK(success == true);
    CHECK(result.is_object() == true);
    CHECK(result.as_object().size() == 0);
}

TEST_CASE("parse_simple_object") {
    JSON result;
    bool success = JSON::parse(R"({"name": "John", "age": 30})", result);
    CHECK(success == true);
    CHECK(result.is_object() == true);
    CHECK(result.as_object().size() == 2);
    CHECK(result["name"].as_string() == "John");
    CHECK(result["age"].as_number().to_int64() == 30);
}

TEST_CASE("parse_object_with_nested_object") {
    JSON result;
    bool success = JSON::parse(R"({"person": {"name": "Alice", "age": 25}})", result);
    CHECK(success == true);
    CHECK(result["person"].is_object() == true);
    CHECK(result["person"]["name"].as_string() == "Alice");
    CHECK(result["person"]["age"].as_number().to_int64() == 25);
}

TEST_CASE("parse_object_with_array") {
    JSON result;
    bool success = JSON::parse(R"({"numbers": [1, 2, 3]})", result);
    CHECK(success == true);
    CHECK(result["numbers"].is_array() == true);
    CHECK(result["numbers"].as_array().size() == 3);
}

TEST_CASE("parse_with_whitespace") {
    JSON result;
    bool success = JSON::parse("  \n\t{\n  \"key\"  : \"value\"\n}  ", result);
    CHECK(success == true);
    CHECK(result["key"].as_string() == "value");
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_CASE("parse_invalid_json_empty") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("", result, &error);
    CHECK(success == false);
    CHECK(error.position == 0);
}

TEST_CASE("parse_invalid_json_trailing_chars") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("123 extra", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_invalid_literal") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("nul", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_invalid_number_format") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("01", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_invalid_array_missing_comma") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("[1 2]", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_unterminated_string") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("\"hello", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_invalid_escape_sequence") {
    JSON result;
    JsonError error;
    bool success = JSON::parse(R"("\x")", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_control_character_in_string") {
    JSON result;
    JsonError error;
    std::string json_with_control = "\"hello\x01world\"";
    bool success = JSON::parse(json_with_control, result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_incomplete_unicode_escape") {
    JSON result;
    JsonError error;
    bool success = JSON::parse(R"("\u00")", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_invalid_unicode_hex") {
    JSON result;
    JsonError error;
    bool success = JSON::parse(R"("\u00XY")", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_object_missing_colon") {
    JSON result;
    JsonError error;
    bool success = JSON::parse(R"({"key" "value"})", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_object_missing_comma") {
    JSON result;
    JsonError error;
    bool success = JSON::parse(R"({"key1": "value1" "key2": "value2"})", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_or_throw_success") {
    JSON result = JSON::parse_or_throw("42");
    CHECK(result.is_number() == true);
    CHECK(result.as_number().to_int64() == 42);
}

TEST_CASE("parse_or_throw_failure") {
    bool caught = false;
    try {
        JSON::parse_or_throw("invalid");
    } catch (const std::invalid_argument& e) {
        caught = true;
        std::string msg = e.what();
        CHECK(msg.find("JSON parse error") != std::string::npos);
    }
    CHECK(caught == true);
}

TEST_CASE("validate_valid_json") {
    CHECK(JSON::validate("null") == true);
    CHECK(JSON::validate("42") == true);
    CHECK(JSON::validate("\"hello\"") == true);
    CHECK(JSON::validate("[]") == true);
    CHECK(JSON::validate("{}") == true);
}

TEST_CASE("validate_invalid_json") {
    CHECK(JSON::validate("") == false);
    CHECK(JSON::validate("invalid") == false);
    CHECK(JSON::validate("[1, 2") == false);
}

// ============================================================================
// JSON Construction Tests
// ============================================================================

TEST_CASE("construct_null") {
    JSON j(nullptr);
    CHECK(j.is_null() == true);
}

TEST_CASE("construct_bool") {
    JSON j_true(true);
    JSON j_false(false);
    CHECK(j_true.is_bool() == true);
    CHECK(j_true.as_bool() == true);
    CHECK(j_false.as_bool() == false);
}

TEST_CASE("construct_number_from_double") {
    JSON j(3.14);
    CHECK(j.is_number() == true);
    CHECK(j.as_number().to_double() - 3.14 < 0.001);
}

TEST_CASE("construct_string") {
    JSON j(std::string("hello"));
    CHECK(j.is_string() == true);
    CHECK(j.as_string() == "hello");
}

TEST_CASE("construct_array") {
    JSON::array_t arr;
    arr.push_back(JSON(1.0));
    arr.push_back(JSON(2.0));
    JSON j = JSON::array(std::move(arr));
    CHECK(j.is_array() == true);
    CHECK(j.as_array().size() == 2);
}

TEST_CASE("construct_object") {
    JSON::object_t obj;
    obj.emplace_back("key1", JSON(std::string("value1")));
    obj.emplace_back("key2", JSON(42.0));
    JSON j = JSON::object(std::move(obj));
    CHECK(j.is_object() == true);
    CHECK(j.as_object().size() == 2);
}

TEST_CASE("construct_number_from_string") {
    JSON j = JSON::number("123");
    CHECK(j.is_number() == true);
    CHECK(j.as_number().to_int64() == 123);
}

// ============================================================================
// JSON Modification Tests
// ============================================================================

TEST_CASE("push_back_to_array") {
    JSON j = JSON::array({});
    j.push_back(JSON(1.0));
    j.push_back(JSON(2.0));
    CHECK(j.as_array().size() == 2);
    CHECK(j.as_array()[0].as_number().to_int64() == 1);
    CHECK(j.as_array()[1].as_number().to_int64() == 2);
}

TEST_CASE("array_subscript_operator") {
    JSON j = JSON::array({});
    JSON& ref = j.push_back(JSON(42.0));
    CHECK(ref.as_number().to_int64() == 42);
}

TEST_CASE("object_subscript_operator_existing_key") {
    JSON::object_t obj;
    obj.emplace_back("key", JSON(std::string("value")));
    JSON j = JSON::object(std::move(obj));
    JSON& val = j["key"];
    CHECK(val.as_string() == "value");
}

TEST_CASE("object_subscript_operator_new_key") {
    JSON j = JSON::object({});
    j["new_key"] = JSON(std::string("new_value"));
    CHECK(j["new_key"].as_string() == "new_value");
}

TEST_CASE("object_find_existing_key") {
    JSON::object_t obj;
    obj.emplace_back("key", JSON(42.0));
    JSON j = JSON::object(std::move(obj));
    const JSON* found = j.find("key");
    CHECK(found != nullptr);
    CHECK(found->as_number().to_int64() == 42);
}

TEST_CASE("object_find_missing_key") {
    JSON j = JSON::object({});
    const JSON* found = j.find("missing");
    CHECK(found == nullptr);
}

TEST_CASE("object_find_on_non_object") {
    JSON j = JSON::array({});
    const JSON* found = j.find("key");
    CHECK(found == nullptr);
}

// ============================================================================
// Stringify Tests
// ============================================================================

TEST_CASE("stringify_null") {
    JSON j(nullptr);
    CHECK(j.stringify() == "null");
}

TEST_CASE("stringify_bool") {
    JSON j_true(true);
    JSON j_false(false);
    CHECK(j_true.stringify() == "true");
    CHECK(j_false.stringify() == "false");
}

TEST_CASE("stringify_number") {
    JSON j(42.0);
    CHECK(j.stringify() == "42");
}

TEST_CASE("stringify_string") {
    JSON j(std::string("hello"));
    CHECK(j.stringify() == "\"hello\"");
}

TEST_CASE("stringify_string_with_escapes") {
    JSON j(std::string("Hello\nWorld"));
    std::string result = j.stringify();
    CHECK(result.find("\\n") != std::string::npos);
}

TEST_CASE("stringify_empty_array") {
    JSON j = JSON::array({});
    CHECK(j.stringify() == "[]");
}

TEST_CASE("stringify_array") {
    JSON::array_t arr;
    arr.push_back(JSON(1.0));
    arr.push_back(JSON(2.0));
    arr.push_back(JSON(3.0));
    JSON j = JSON::array(std::move(arr));
    CHECK(j.stringify() == "[1,2,3]");
}

TEST_CASE("stringify_empty_object") {
    JSON j = JSON::object({});
    CHECK(j.stringify() == "{}");
}

TEST_CASE("stringify_object") {
    JSON::object_t obj;
    obj.emplace_back("name", JSON(std::string("John")));
    obj.emplace_back("age", JSON(30.0));
    JSON j = JSON::object(std::move(obj));
    std::string result = j.stringify();
    CHECK(result.find("\"name\"") != std::string::npos);
    CHECK(result.find("\"John\"") != std::string::npos);
    CHECK(result.find("\"age\"") != std::string::npos);
    CHECK(result.find("30") != std::string::npos);
}

TEST_CASE("stringify_pretty") {
    JSON::object_t obj;
    obj.emplace_back("key", JSON(std::string("value")));
    JSON j = JSON::object(std::move(obj));
    
    StringifyOptions opts;
    opts.pretty = true;
    opts.indent = 2;
    
    std::string result = j.stringify(opts);
    CHECK(result.find("\n") != std::string::npos);
    CHECK(result.find("  ") != std::string::npos);
}

TEST_CASE("stringify_escape_solidus") {
    JSON j(std::string("path/to/file"));
    
    StringifyOptions opts;
    opts.escape_solidus = true;
    
    std::string result = j.stringify(opts);
    CHECK(result.find("\\/") != std::string::npos);
}

TEST_CASE("stringify_nested_structure") {
    JSON::array_t inner_arr;
    inner_arr.push_back(JSON(1.0));
    inner_arr.push_back(JSON(2.0));
    
    JSON::object_t obj;
    obj.emplace_back("numbers", JSON::array(std::move(inner_arr)));
    obj.emplace_back("name", JSON(std::string("test")));
    
    JSON j = JSON::object(std::move(obj));
    std::string result = j.stringify();
    
    CHECK(result.find("\"numbers\"") != std::string::npos);
    CHECK(result.find("[1,2]") != std::string::npos);
}

// ============================================================================
// Type Checking and Accessors Tests
// ============================================================================

TEST_CASE("as_bool_with_fallback") {
    JSON j_true(true);
    JSON j_num(42.0);
    CHECK(j_true.as_bool(false) == true);
    CHECK(j_num.as_bool(true) == true); // fallback because not bool
}

TEST_CASE("as_number_throws_on_wrong_type") {
    JSON j(std::string("not a number"));
    bool caught = false;
    try {
        j.as_number();
    } catch (const std::logic_error&) {
        caught = true;
    }
    CHECK(caught == true);
}

TEST_CASE("as_string_throws_on_wrong_type") {
    JSON j(42.0);
    bool caught = false;
    try {
        j.as_string();
    } catch (const std::logic_error&) {
        caught = true;
    }
    CHECK(caught == true);
}

TEST_CASE("as_array_throws_on_wrong_type") {
    JSON j(std::string("not an array"));
    bool caught = false;
    try {
        j.as_array();
    } catch (const std::logic_error&) {
        caught = true;
    }
    CHECK(caught == true);
}

TEST_CASE("as_object_throws_on_wrong_type") {
    JSON j(std::string("not an object"));
    bool caught = false;
    try {
        j.as_object();
    } catch (const std::logic_error&) {
        caught = true;
    }
    CHECK(caught == true);
}

TEST_CASE("mutable_array_access") {
    JSON j = JSON::array({});
    j.push_back(JSON(1.0));
    j.as_array()[0] = JSON(2.0);
    CHECK(j.as_array()[0].as_number().to_int64() == 2);
}

TEST_CASE("mutable_object_access") {
    JSON j = JSON::object({});
    j["key"] = JSON(std::string("value"));
    j.as_object()[0].second = JSON(std::string("new_value"));
    CHECK(j["key"].as_string() == "new_value");
}

// ============================================================================
// Number Conversion Tests
// ============================================================================

TEST_CASE("number_to_int64_valid") {
    JSON j = JSON::number("12345");
    CHECK(j.as_number().to_int64() == 12345);
}

TEST_CASE("number_to_int64_fallback") {
    JSON j = JSON::number("not_a_number");
    CHECK(j.as_number().to_int64(999) == 999);
}

TEST_CASE("number_to_double_valid") {
    JSON j = JSON::number("3.14159");
    CHECK(j.as_number().to_double() - 3.14159 < 0.00001);
}

TEST_CASE("number_to_double_fallback") {
    JSON j = JSON::number("invalid");
    CHECK(j.as_number().to_double(0.0) == 0.0);
}

TEST_CASE("number_is_integral") {
    JSON j_int = JSON::number("42");
    JSON j_float = JSON::number("3.14");
    CHECK(j_int.as_number().is_integral() == true);
    CHECK(j_float.as_number().is_integral() == false);
}

TEST_CASE("number_large_integer") {
    std::string large_num = "9223372036854775807"; // INT64_MAX
    JSON j = JSON::number(large_num);
    CHECK(j.as_number().to_int64() == std::numeric_limits<int64_t>::max());
}

// ============================================================================
// Complex Parsing Scenarios
// ============================================================================

TEST_CASE("parse_complex_nested_structure") {
    std::string json = R"({
        "name": "Test",
        "data": {
            "values": [1, 2, 3],
            "metadata": {
                "created": "2024-01-01",
                "modified": "2024-01-02"
            }
        },
        "flags": [true, false, true]
    })";
    
    JSON result;
    bool success = JSON::parse(json, result);
    CHECK(success == true);
    CHECK(result["name"].as_string() == "Test");
    CHECK(result["data"]["values"].as_array().size() == 3);
    CHECK(result["data"]["metadata"]["created"].as_string() == "2024-01-01");
    CHECK(result["flags"].as_array()[0].as_bool() == true);
}

TEST_CASE("roundtrip_parse_stringify") {
    std::string original = R"({"key":"value","number":42,"array":[1,2,3]})";
    JSON parsed = JSON::parse_or_throw(original);
    std::string stringified = parsed.stringify();
    JSON reparsed = JSON::parse_or_throw(stringified);
    
    CHECK(reparsed["key"].as_string() == "value");
    CHECK(reparsed["number"].as_number().to_int64() == 42);
    CHECK(reparsed["array"].as_array().size() == 3);
}

TEST_CASE("parse_array_with_trailing_comma_fails") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("[1, 2,]", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_object_with_trailing_comma_fails") {
    JSON result;
    JsonError error;
    bool success = JSON::parse(R"({"key": "value",})", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_number_with_leading_plus_fails") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("+123", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_all_escape_sequences") {
    JSON result;
    bool success = JSON::parse(R"("\"\\/\b\f\n\r\t")", result);
    CHECK(success == true);
    std::string s = result.as_string();
    CHECK(s.find('"') != std::string::npos);
    CHECK(s.find('\\') != std::string::npos);
    CHECK(s.find('/') != std::string::npos);
    CHECK(s.find('\b') != std::string::npos);
    CHECK(s.find('\f') != std::string::npos);
    CHECK(s.find('\n') != std::string::npos);
    CHECK(s.find('\r') != std::string::npos);
    CHECK(s.find('\t') != std::string::npos);
}

TEST_CASE("stringify_special_characters") {
    JSON j(std::string("\"\\/\b\f\n\r\t"));
    std::string result = j.stringify();
    CHECK(result.find("\\\"") != std::string::npos);
    CHECK(result.find("\\\\") != std::string::npos);
    CHECK(result.find("\\b") != std::string::npos);
    CHECK(result.find("\\f") != std::string::npos);
    CHECK(result.find("\\n") != std::string::npos);
    CHECK(result.find("\\r") != std::string::npos);
    CHECK(result.find("\\t") != std::string::npos);
}

TEST_CASE("stringify_control_characters") {
    JSON j(std::string("\x01\x02\x1F"));
    std::string result = j.stringify();
    // Control characters should be escaped as \u00XX
    CHECK(result.find("\\u00") != std::string::npos);
}

TEST_CASE("parse_unicode_surrogate_pair") {
    // U+1F600 (😀) is encoded as surrogate pair D83D DE00
    JSON result;
    bool success = JSON::parse(R"("\uD83D\uDE00")", result);
    CHECK(success == true);
    CHECK(result.is_string() == true);
    // UTF-8 encoding of emoji
    CHECK(result.as_string().size() == 4);
}

TEST_CASE("parse_unicode_high_surrogate_missing_low") {
    JSON result;
    JsonError error;
    // High surrogate without low surrogate
    bool success = JSON::parse(R"("\uD83D")", result, &error);
    CHECK(success == false);
    CHECK(error.message.find("Missing low surrogate") != std::string::npos);
}

TEST_CASE("parse_unicode_invalid_low_surrogate") {
    JSON result;
    JsonError error;
    // High surrogate with invalid low surrogate (not in DC00-DFFF range)
    bool success = JSON::parse(R"("\uD83D\u1234")", result, &error);
    CHECK(success == false);
    CHECK(error.message.find("Invalid low surrogate") != std::string::npos);
}

TEST_CASE("parse_unicode_incomplete_low_surrogate") {
    JSON result;
    JsonError error;
    // High surrogate with incomplete low surrogate
    bool success = JSON::parse(R"("\uD83D\uDE")", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_unicode_codepoint_boundary_cases") {
    // Test various unicode codepoint boundaries
    JSON result1, result2, result3;
    
    // U+007F (last 1-byte UTF-8)
    CHECK(JSON::parse(R"("\u007F")", result1) == true);
    
    // U+07FF (last 2-byte UTF-8)
    CHECK(JSON::parse(R"("\u07FF")", result2) == true);
    
    // U+FFFF (last 3-byte UTF-8 in BMP)
    CHECK(JSON::parse(R"("\uFFFF")", result3) == true);
}

TEST_CASE("parse_unicode_max_codepoint") {
    // U+10FFFF is the maximum valid Unicode codepoint
    // Encoded as surrogate pair: D800-DBFF for high, DC00-DFFF for low
    // U+10FFFF = D83F DFFF
    JSON result;
    bool success = JSON::parse(R"("\uDBFF\uDFFF")", result);
    CHECK(success == true);
}

TEST_CASE("parse_number_with_exponent_capital_e") {
    JSON result;
    bool success = JSON::parse("1.5E10", result);
    CHECK(success == true);
    CHECK(result.is_number() == true);
}

TEST_CASE("parse_number_negative_with_fraction_and_exponent") {
    JSON result;
    bool success = JSON::parse("-2.5e-3", result);
    CHECK(success == true);
    CHECK(result.as_number().to_double() - (-0.0025) < 0.0001);
}

TEST_CASE("parse_number_invalid_fraction") {
    JSON result;
    JsonError error;
    // Decimal point without digits after it
    bool success = JSON::parse("3.", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_number_invalid_exponent") {
    JSON result;
    JsonError error;
    // Exponent without digits
    bool success = JSON::parse("3e", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_object_empty_key") {
    JSON result;
    bool success = JSON::parse(R"({"": "value"})", result);
    CHECK(success == true);
    CHECK(result[""].as_string() == "value");
}

TEST_CASE("parse_deeply_nested_arrays") {
    JSON result;
    std::string json = "[[[[[[[[[[1]]]]]]]]]]";
    bool success = JSON::parse(json, result);
    CHECK(success == true);
    // Navigate to the innermost value
    const JSON* current = &result;
    for (int i = 0; i < 10; ++i) {
        CHECK(current->is_array() == true);
        current = &current->as_array()[0];
    }
    CHECK(current->as_number().to_int64() == 1);
}

TEST_CASE("parse_deeply_nested_objects") {
    JSON result;
    std::string json = R"({"a":{"b":{"c":{"d":{"e":"value"}}}}})";
    bool success = JSON::parse(json, result);
    CHECK(success == true);
    CHECK(result["a"]["b"]["c"]["d"]["e"].as_string() == "value");
}

TEST_CASE("stringify_pretty_nested_arrays") {
    JSON::array_t inner;
    inner.push_back(JSON(1.0));
    inner.push_back(JSON(2.0));
    
    JSON::array_t outer;
    outer.push_back(JSON::array(std::move(inner)));
    outer.push_back(JSON(3.0));
    
    JSON j = JSON::array(std::move(outer));
    
    StringifyOptions opts;
    opts.pretty = true;
    opts.indent = 2;
    
    std::string result = j.stringify(opts);
    CHECK(result.find("\n") != std::string::npos);
    CHECK(result.find("  ") != std::string::npos);
}

TEST_CASE("stringify_pretty_nested_objects") {
    JSON::object_t inner;
    inner.emplace_back("inner_key", JSON(std::string("inner_value")));
    
    JSON::object_t outer;
    outer.emplace_back("outer_key", JSON::object(std::move(inner)));
    
    JSON j = JSON::object(std::move(outer));
    
    StringifyOptions opts;
    opts.pretty = true;
    
    std::string result = j.stringify(opts);
    CHECK(result.find("\n") != std::string::npos);
    CHECK(result.find("outer_key") != std::string::npos);
    CHECK(result.find("inner_key") != std::string::npos);
}

TEST_CASE("stringify_without_escape_solidus") {
    JSON j(std::string("path/to/file"));
    
    StringifyOptions opts;
    opts.escape_solidus = false;
    
    std::string result = j.stringify(opts);
    CHECK(result.find("/") != std::string::npos);
    CHECK(result.find("\\/") == std::string::npos);
}

TEST_CASE("number_format_invalid_fallback") {
    // Test number parsing with invalid formats using fallback
    JSON j = JSON::number("not_a_number");
    // When parsing fails, fallback should be used
    CHECK(j.as_number().to_int64(999) == 999);
    CHECK(j.as_number().to_double(1.5) - 1.5 < 0.001);
}

TEST_CASE("number_large_value") {
    // Test a very large number
    std::string huge = "99999999999999999999";
    JSON j = JSON::number(huge);
    // strtoll should parse this (it may overflow to INT64_MAX)
    int64_t val = j.as_number().to_int64(-1);
    // Should be a large positive value or fallback
    CHECK(val != 0);
}

TEST_CASE("parse_unterminated_array") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("[1, 2, 3", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_unterminated_object") {
    JSON result;
    JsonError error;
    bool success = JSON::parse(R"({"key": "value")", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_object_non_string_key") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("{123: \"value\"}", result, &error);
    CHECK(success == false);
}

TEST_CASE("parse_unexpected_character") {
    JSON result;
    JsonError error;
    bool success = JSON::parse("@", result, &error);
    CHECK(success == false);
    CHECK(error.message.find("Unexpected character") != std::string::npos);
}

TEST_CASE("stringify_backslash_in_string") {
    JSON j(std::string("C:\\Users\\test"));
    std::string result = j.stringify();
    // Should have escaped backslashes
    CHECK(result.find("\\\\") != std::string::npos);
}

TEST_CASE("empty_json_default_constructor") {
    JSON j;
    CHECK(j.is_null() == true);
    CHECK(j.type() == JSON::Type::Null);
}

TEST_CASE("number_representation_preserved") {
    JSON j1 = JSON::number("123");
    JSON j2 = JSON::number("123.0");
    JSON j3 = JSON::number("1.23e2");
    
    CHECK(j1.as_number().repr == "123");
    CHECK(j2.as_number().repr == "123.0");
    CHECK(j3.as_number().repr == "1.23e2");
}

} // TEST_SUITE
