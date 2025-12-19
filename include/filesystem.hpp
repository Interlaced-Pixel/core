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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INTERLACED_CORE_FILESYSTEM_HPP
#define INTERLACED_CORE_FILESYSTEM_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <ctime>

// Platform-specific includes and definitions
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
// Only define S_ISDIR and S_ISREG if they're not already defined
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif
#define MKDIR(path, mode) _mkdir(path)
#else
#include <unistd.h>
#include <dirent.h>
#define MKDIR(path, mode) mkdir(path, mode)
#endif

namespace interlaced {

    namespace core {

        namespace filesystem {

            /**
             * @brief Filesystem utility functions
             *
             * This class provides static methods for common filesystem operations
             * such as checking file existence, reading/writing files, and creating directories.
             */
            class FileSystem {
            public:
                /**
                 * @brief Check if a file or directory exists
                 *
                 * @param path The path to check
                 * @return true if the path exists, false otherwise
                 */
                static bool exists(const std::string& path) {
                    struct stat buffer;
                    return (stat(path.c_str(), &buffer) == 0);
                }

                /**
                 * @brief Check if a path is a directory
                 *
                 * @param path The path to check
                 * @return true if the path is a directory, false otherwise
                 */
                static bool is_directory(const std::string& path) {
                    struct stat buffer;
                    if (stat(path.c_str(), &buffer) == 0) {
                        return S_ISDIR(buffer.st_mode);
                    }
                    return false;
                }

                /**
                 * @brief Check if a path is a regular file
                 *
                 * @param path The path to check
                 * @return true if the path is a regular file, false otherwise
                 */
                static bool is_regular_file(const std::string& path) {
                    struct stat buffer;
                    if (stat(path.c_str(), &buffer) == 0) {
                        return S_ISREG(buffer.st_mode);
                    }
                    return false;
                }

                /**
                 * @brief Read the entire file into a string
                 *
                 * @param path The path of the file to read
                 * @return std::string The contents of the file
                 */
                static std::string read_file(const std::string& path) {
                    std::ifstream file(path, std::ios::binary);
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    return buffer.str();
                }

                /**
                 * @brief Write string content to a file
                 *
                 * @param path The path of the file to write
                 * @param content The content to write to the file
                 * @return true if successful, false otherwise
                 */
                static bool write_file(const std::string& path, const std::string& content) {
                    std::ofstream file(path, std::ios::binary);
                    if (file.is_open()) {
                        file << content;
                        file.close();
                        return true;
                    }
                    return false;
                }

                /**
                 * @brief Create a directory
                 *
                 * @param path The path of the directory to create
                 * @return true if successful, false otherwise
                 */
                static bool create_directory(const std::string& path) {
                    return MKDIR(path.c_str(), 0777) == 0;
                }

                /**
                 * @brief Create directories recursively
                 *
                 * @param path The path of the directories to create
                 * @return true if successful, false otherwise
                 */
                static bool create_directories(const std::string& path) {
                    // Handle empty path
                    if (path.empty()) return false;

                    // Handle root directory
                    if (path == "/" || path == "\\" ||
                        (path.length() == 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/'))) {
                        return true;
                    }

                    // If directory already exists, return true
                    if (is_directory(path)) {
                        return true;
                    }

                    // Create parent directories first
                    std::string parent = path;

                    // Remove trailing slashes
                    while (!parent.empty() && (parent.back() == '/' || parent.back() == '\\')) {
                        parent.pop_back();
                    }

                    // Find the last separator
                    size_t pos = parent.find_last_of("/\\\\");
                    if (pos != std::string::npos) {
                        std::string parent_dir = parent.substr(0, pos);
                        if (!parent_dir.empty() && !is_directory(parent_dir)) {
                            if (!create_directories(parent_dir)) {
                                return false;
                            }
                        }
                    }

                    // Create this directory
                    return create_directory(path);
                }

                /**
                 * @brief Remove a file or empty directory
                 *
                 * @param path The path to remove
                 * @return true if successful, false otherwise
                 */
                static bool remove(const std::string& path) {
                    if (is_directory(path)) {
                        // For directories, use rmdir on Unix or _rmdir on Windows
        #ifdef _WIN32
                        return _rmdir(path.c_str()) == 0;
        #else
                        return ::rmdir(path.c_str()) == 0;
        #endif
                    } else {
                        // For files, use unlink
                        return ::unlink(path.c_str()) == 0;
                    }
                }

                /**
                 * @brief Copy a file
                 *
                 * @param source The source file path
                 * @param destination The destination file path
                 * @return true if successful, false otherwise
                 */
                static bool copy_file(const std::string& source, const std::string& destination) {
                    std::ifstream src(source, std::ios::binary);
                    std::ofstream dst(destination, std::ios::binary);

                    if (!src.is_open() || !dst.is_open()) {
                        return false;
                    }

                    dst << src.rdbuf();
                    return src.good() && dst.good();
                }

                /**
                 * @brief Move/rename a file or directory
                 *
                 * @param source The source path
                 * @param destination The destination path
                 * @return true if successful, false otherwise
                 */
                static bool rename(const std::string& source, const std::string& destination) {
                    return ::rename(source.c_str(), destination.c_str()) == 0;
                }

                /**
                 * @brief Get the size of a file
                 *
                 * @param path The path of the file
                 * @return The size of the file in bytes, or -1 if the file doesn't exist
                 */
                static long long file_size(const std::string& path) {
                    struct stat buffer;
                    if (stat(path.c_str(), &buffer) == 0) {
                        return static_cast<long long>(buffer.st_size);
                    }
                    return -1;
                }

                /**
                 * @brief Get the last modification time of a file or directory
                 *
                 * @param path The path to check
                 * @return The modification time as time_t, or -1 if the path doesn't exist
                 */
                static std::time_t last_write_time(const std::string& path) {
                    struct stat buffer;
                    if (stat(path.c_str(), &buffer) == 0) {
                        return buffer.st_mtime;
                    }
                    return -1;
                }

                /**
                 * @brief List the contents of a directory
                 *
                 * @param path The directory path
                 * @return A vector of file/directory names in the directory
                 */
                static std::vector<std::string> directory_iterator(const std::string& path) {
                    std::vector<std::string> result;

        #ifdef _WIN32
                    WIN32_FIND_DATAA findData;
                    HANDLE hFind = FindFirstFileA((path + "\\\\*").c_str(), &findData);

                    if (hFind != INVALID_HANDLE_VALUE) {
                        do {
                            std::string filename(findData.cFileName);
                            if (filename != "." && filename != "..") {
                                result.push_back(filename);
                            }
                        } while (FindNextFileA(hFind, &findData));
                        FindClose(hFind);
                    }
        #else
                    DIR* dir = opendir(path.c_str());
                    if (dir) {
                        struct dirent* entry;
                        while ((entry = readdir(dir)) != nullptr) {
                            std::string filename(entry->d_name);
                            if (filename != "." && filename != "..") {
                                result.push_back(filename);
                            }
                        }
                        closedir(dir);
                    }
        #endif

                    return result;
                }

                /**
                 * @brief Get the system's temporary directory path
                 *
                 * @return The temporary directory path
                 */
                static std::string temp_directory_path() {
        #ifdef _WIN32
                    char buffer[MAX_PATH];
                    GetTempPathA(MAX_PATH, buffer);
                    return std::string(buffer);
        #else
                    const char* tmpdir = getenv("TMPDIR");
                    if (tmpdir) {
                        return std::string(tmpdir);
                    }
                    return "/tmp";
        #endif
                }

                /**
                 * @brief Get the current working directory
                 *
                 * @return The current working directory path
                 */
                static std::string current_path() {
        #ifdef _WIN32
                    char buffer[MAX_PATH];
                    if (_getcwd(buffer, MAX_PATH)) {
                        return std::string(buffer);
                    }
        #else
                    char buffer[PATH_MAX];
                    if (getcwd(buffer, PATH_MAX)) {
                        return std::string(buffer);
                    }
        #endif
                    return "";
                }

                /**
                 * @brief Change the current working directory
                 *
                 * @param path The path to change to
                 * @return true if successful, false otherwise
                 */
                static bool current_path(const std::string& path) {
        #ifdef _WIN32
                    return _chdir(path.c_str()) == 0;
        #else
                    return chdir(path.c_str()) == 0;
        #endif
                }
            };

        }
    }

}

#endif // INTERLACED_CORE_FILESYSTEM_HPP
