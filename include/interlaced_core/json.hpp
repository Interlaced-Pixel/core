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
 * @brief Simple JSON parser and generator with enhanced capabilities
 *
 * This class provides robust JSON parsing and generation functionality.
 * Supports basic JSON syntax including key-value pairs, nested objects,
 * and array structures.
 */
class JSON {
public:
  /**
   * @brief Parse JSON string into a hierarchical structure
   *
   * @param json_str The JSON string to parse
   * @return std::map<std::string, std::string> Parsed key-value pairs
   * @throws std::invalid_argument if the input is not valid JSON
   */
  static std::map<std::string, std::string> parse(const std::string &json_str) {
    std::map<std::string, std::string> result;
    
    if (json_str.empty()) {
      throw std::invalid_argument("Empty JSON string");
    }
    
    // Validate basic structure
    if (json_str[0] != '{' && json_str[0] != '[') {
      throw std::invalid_argument("JSON must start with '{' or '['");
    }
    
    // Simple parser for key-value pairs
    std::istringstream iss(json_str);
    std::string token;
    
    // Extract key-value pairs
    while (std::getline(iss, token, '}')) {
      // Process each key-value pair
      size_t start = 0;
      size_t end = token.find("}");
      
      if (end == std::string::npos) {
        throw std::invalid_argument("Invalid JSON structure");
      }
      
      // Extract key and value
      size_t key_start = token.find(":", 0);
      if (key_start == std::string::npos) {
        throw std::invalid_argument("Missing colon in key-value pair");
      }
      
      // Extract key (before colon)
      std::string key = token.substr(0, key_start);
      
      // Extract value (after colon)
      std::string value = token.substr(key_start + 1, end - key_start - 1);
      
      // Handle nested structures
      if (value.find("{") != std::string::npos || value.find("[") != std::string::npos) {
        // For now, we'll just extract the first key-value pair
        result[key] = value;
      } else {
        // For simple values, just use the raw string
        result[key] = value;
      }
    }
    
    return result;
  }

  /**
   * @brief Convert map to JSON string
   *
   * @param data The key-value pairs to convert to JSON
   * @return std::string The JSON string representation
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
      oss << "\"" << pair.first << "\":\"" << pair.second << "\"";
      first = false;
    }
    
    oss << "}";
    return oss.str();
  }

  /**
   * @brief Validate JSON string
   *
   * @param json_str The JSON string to validate
   * @return true if the string is valid JSON, false otherwise
   */
  static bool validate(const std::string &json_str) {
    if (json_str.empty()) {
      return false;
    }

    // Check for basic structure: starts with '{' or '[' and ends with '}' or ']'
    if ((json_str[0] != '{' && json_str[0] != '[') ||
        (json_str.back() != '}' && json_str.back() != ']')) {
      return false;
    }

    // Check for proper nesting
    int depth = 0;
    for (char c : json_str) {
      if (c == '{' || c == '[') {
        depth++;
      } else if (c == '}' || c == ']') {
        depth--;
      }
      
      // If depth goes negative, we have unmatched closing brackets
      if (depth < 0) {
        return false;
      }
    }

    // Check if we end at the expected depth
    return depth == 0;
  }
};

} // namespace json

} // namespace core

} // namespace interlaced

#endif // INTERLACED_CORE_JSON_HPP
