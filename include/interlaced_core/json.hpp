/*
 * Interlaced Core Library
 * Copyright (c) 2025 Interlaced Pixel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INTERLACED_CORE_JSON_HPP
#define INTERLACED_CORE_JSON_HPP

#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cctype>

namespace interlaced {

namespace core {

namespace json {

/**
 * @brief Simple JSON parser and generator with enhanced capabilities.
 *
 * @details Provides lightweight parsing and stringification helpers for
 * key-value JSON objects. Nested objects and arrays are preserved as raw
 * substrings rather than fully materialized structures to keep the
 * implementation header-only and dependency free.
 */
class JSON {
private:
  /**
   * @brief Skip whitespace characters in-place.
   *
   * @param str Input JSON string being parsed.
   * @param pos Current parsing offset; advanced past any whitespace.
   */
  static void skip_whitespace(const std::string &str, size_t &pos) {
    while (pos < str.length() && std::isspace(static_cast<unsigned char>(str[pos]))) {
      pos++;
    }
  }

  /**
   * @brief Convert JSON escape sequences back to their literal characters.
   *
   * @param str Raw JSON string segment containing escape sequences.
   * @return std::string Unescaped string content.
   */
  static std::string unescape_string(const std::string &str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
      if (str[i] == '\\' && i + 1 < str.length()) {
        switch (str[i + 1]) {
          case '"':  result += '"'; break;
          case '\\': result += '\\'; break;
          case '/':  result += '/'; break;
          case 'n':  result += '\n'; break;
          case 'r':  result += '\r'; break;
          case 't':  result += '\t'; break;
          case 'b':  result += '\b'; break;
          case 'f':  result += '\f'; break;
          default:   result += str[i]; result += str[i + 1]; break;
        }
        ++i; // Skip the escaped character
      } else {
        result += str[i];
      }
    }
    return result;
  }

  /**
   * @brief Escape special characters so a string is JSON-safe.
   *
   * @param str Plain string to encode for JSON output.
   * @return std::string Escaped string suitable for inclusion in JSON.
   */
  static std::string escape_string(const std::string &str) {
    std::string result;
    result.reserve(str.length() * 2);
    
    for (char c : str) {
      switch (c) {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        default:   result += c; break;
      }
    }
    return result;
  }

  /**
   * @brief Parse a JSON string value.
   *
   * @param str JSON text being parsed; the opening quote is already consumed.
   * @param pos Mutable offset; advanced past the closing quote.
   * @return std::string Extracted string with escapes resolved.
   * @throws std::invalid_argument When the string literal is unterminated.
   */
  static std::string parse_string(const std::string &str, size_t &pos) {
    std::string result;
    
    while (pos < str.length()) {
      if (str[pos] == '"') {
        pos++; // Consume closing quote
        return unescape_string(result);
      } else if (str[pos] == '\\' && pos + 1 < str.length()) {
        result += str[pos];
        result += str[pos + 1];
        pos += 2;
      } else {
        result += str[pos];
        pos++;
      }
    }
    
    throw std::invalid_argument("Unterminated string");
  }

  /**
   * @brief Check if a string token is a valid JSON number.
   *
   * @param str Candidate numeric token.
   * @return true if the token matches JSON number grammar, false otherwise.
   */
  static bool is_number(const std::string &str) {
    if (str.empty()) return false;
    
    size_t i = 0;
    if (str[i] == '-') i++;
    
    if (i >= str.length() || !std::isdigit(static_cast<unsigned char>(str[i]))) {
      return false;
    }
    
    while (i < str.length() && std::isdigit(static_cast<unsigned char>(str[i]))) {
      i++;
    }
    
    if (i < str.length() && str[i] == '.') {
      i++;
      if (i >= str.length() || !std::isdigit(static_cast<unsigned char>(str[i]))) {
        return false;
      }
      while (i < str.length() && std::isdigit(static_cast<unsigned char>(str[i]))) {
        i++;
      }
    }
    
    if (i < str.length() && (str[i] == 'e' || str[i] == 'E')) {
      i++;
      if (i < str.length() && (str[i] == '+' || str[i] == '-')) i++;
      if (i >= str.length() || !std::isdigit(static_cast<unsigned char>(str[i]))) {
        return false;
      }
      while (i < str.length() && std::isdigit(static_cast<unsigned char>(str[i]))) {
        i++;
      }
    }
    
    return i == str.length();
  }

public:
  /**
   * @brief Parse a flat JSON object into key-value pairs.
   *
   * @param json_str Input JSON text; must represent an object.
   * @return std::map<std::string, std::string> Map of keys to raw values
   * (strings are unescaped, primitives kept as-is, nested structures stored
   * as their original substrings).
   * @throws std::invalid_argument When the input is empty or structurally invalid.
   */
  static std::map<std::string, std::string> parse(const std::string &json_str) {
    std::map<std::string, std::string> result;
    
    if (json_str.empty()) {
      throw std::invalid_argument("Empty JSON string");
    }
    
    size_t pos = 0;
    skip_whitespace(json_str, pos);
    
    if (pos >= json_str.length() || json_str[pos] != '{') {
      throw std::invalid_argument("JSON object must start with '{'");
    }
    pos++;
    
    skip_whitespace(json_str, pos);
    
    if (pos < json_str.length() && json_str[pos] == '}') {
      return result;
    }
    
    while (pos < json_str.length()) {
      skip_whitespace(json_str, pos);
      
      if (pos >= json_str.length() || json_str[pos] != '"') {
        throw std::invalid_argument("Expected string key");
      }
      pos++;
      
      std::string key = parse_string(json_str, pos);
      
      skip_whitespace(json_str, pos);
      
      if (pos >= json_str.length() || json_str[pos] != ':') {
        throw std::invalid_argument("Expected ':' after key");
      }
      pos++;
      
      skip_whitespace(json_str, pos);
      
      std::string value;
      if (pos >= json_str.length()) {
        throw std::invalid_argument("Unexpected end of JSON");
      }
      
      if (json_str[pos] == '"') {
        pos++;
        value = parse_string(json_str, pos);
      } else if (json_str[pos] == 't' && pos + 4 <= json_str.length() && json_str.substr(pos, 4) == "true") {
        value = "true";
        pos += 4;
      } else if (json_str[pos] == 'f' && pos + 5 <= json_str.length() && json_str.substr(pos, 5) == "false") {
        value = "false";
        pos += 5;
      } else if (json_str[pos] == 'n' && pos + 4 <= json_str.length() && json_str.substr(pos, 4) == "null") {
        value = "null";
        pos += 4;
      } else if (json_str[pos] == '-' || std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
        size_t start = pos;
        if (json_str[pos] == '-') pos++;
        
        if (pos >= json_str.length() || !std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
          throw std::invalid_argument("Invalid number format");
        }
        
        while (pos < json_str.length() && std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
          pos++;
        }
        
        if (pos < json_str.length() && json_str[pos] == '.') {
          pos++;
          if (pos >= json_str.length() || !std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            throw std::invalid_argument("Invalid number format");
          }
          while (pos < json_str.length() && std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            pos++;
          }
        }
        
        if (pos < json_str.length() && (json_str[pos] == 'e' || json_str[pos] == 'E')) {
          pos++;
          if (pos < json_str.length() && (json_str[pos] == '+' || json_str[pos] == '-')) {
            pos++;
          }
          if (pos >= json_str.length() || !std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            throw std::invalid_argument("Invalid number format");
          }
          while (pos < json_str.length() && std::isdigit(static_cast<unsigned char>(json_str[pos]))) {
            pos++;
          }
        }
        
        value = json_str.substr(start, pos - start);
      } else if (json_str[pos] == '{' || json_str[pos] == '[') {
        int depth = 0;
        size_t start = pos;
        
        do {
          if (json_str[pos] == '{' || json_str[pos] == '[') {
            depth++;
          } else if (json_str[pos] == '}' || json_str[pos] == ']') {
            depth--;
          }
          pos++;
        } while (pos < json_str.length() && depth > 0);
        
        value = json_str.substr(start, pos - start);
      } else {
        throw std::invalid_argument("Unexpected character in value");
      }
      
      result[key] = value;
      
      skip_whitespace(json_str, pos);
      
      if (pos >= json_str.length()) {
        throw std::invalid_argument("Unexpected end of JSON");
      }
      
      if (json_str[pos] == '}') {
        pos++;
        break;
      } else if (json_str[pos] == ',') {
        pos++;
      } else {
        throw std::invalid_argument("Expected ',' or '}'");
      }
    }
    
    return result;
  }

  /**
   * @brief Convert key-value pairs to a JSON object string.
   *
   * @param data Map of keys to values; primitives and nested JSON must already
   * be encoded appropriately.
   * @return std::string JSON object text.
   */
  static std::string stringify(const std::map<std::string, std::string> &data) {
    if (data.empty()) {
      return "{}";
    }
    
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    
    for (const auto &pair : data) {
      if (!first) {
        oss << ",";
      }
      
      oss << "\"" << escape_string(pair.first) << "\":";
      
      const std::string &value = pair.second;
      if (value == "true" || value == "false" || value == "null" || is_number(value)) {
        oss << value;
      } else if (!value.empty() && (value[0] == '{' || value[0] == '[')) {
        oss << value;
      } else {
        oss << "\"" << escape_string(value) << "\"";
      }
      
      first = false;
    }
    
    oss << "}";
    return oss.str();
  }

  /**
   * @brief Validate basic JSON structure.
   *
   * @param json_str Candidate JSON text.
   * @return true if quotes are balanced and braces/brackets are properly nested, false otherwise.
   */
  static bool validate(const std::string &json_str) {
    if (json_str.empty()) {
      return false;
    }

    size_t pos = 0;
    skip_whitespace(json_str, pos);
    
    if (pos >= json_str.length() || (json_str[pos] != '{' && json_str[pos] != '[')) {
      return false;
    }

    int depth = 0;
    bool in_string = false;
    bool escape_next = false;
    
    for (size_t i = pos; i < json_str.length(); ++i) {
      char c = json_str[i];
      
      if (escape_next) {
        escape_next = false;
        continue;
      }
      
      if (c == '\\' && in_string) {
        escape_next = true;
        continue;
      }
      
      if (c == '"') {
        in_string = !in_string;
        continue;
      }
      
      if (in_string) {
        continue;
      }
      
      if (c == '{' || c == '[') {
        depth++;
      } else if (c == '}' || c == ']') {
        depth--;
        if (depth < 0) {
          return false;
        }
      }
    }
    
    return !in_string && depth == 0;
  }
};

} // namespace json

} // namespace core

} // namespace interlaced

#endif // INTERLACED_CORE_JSON_HPP
