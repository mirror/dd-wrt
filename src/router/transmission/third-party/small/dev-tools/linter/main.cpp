//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

/** \file
 * \brief A very simple amalgamator to generate the single header version of
 * futures
 */

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <vector>
#include <string_view>

namespace fs = std::filesystem;

struct config
{
    // Input
    std::vector<fs::path> include_paths;
    // Log
    bool show_progress{ false };
    bool verbose{ false };
    // Linting options
    bool fix_include_guards{ true };
};

constexpr bool
is_key(std::string_view arg) {
    return !arg.empty() && arg.front() == '-';
};

constexpr bool
is_false(std::string_view value) {
    return is_key(value) || value.empty() || value == "false"
           || value == "FALSE" || value == "0";
}

bool
parse_config(config &c, const std::vector<std::string_view> &args) {
    auto find_key = [&](std::string_view key) {
        return std::find_if(args.begin(), args.end(), [&](std::string_view arg) {
            return is_key(arg) && arg.substr(arg.find_first_not_of('-')) == key;
        });
    };

    auto key_it = find_key("show_progress");
    if (key_it != args.end()) {
        c.show_progress = is_key(*key_it) || !is_false(*key_it);
    }

    key_it = find_key("verbose");
    if (key_it != args.end()) {
        c.verbose = is_key(*key_it) || !is_false(*key_it);
    }

    key_it = find_key("fix_include_guards");
    if (key_it != args.end()) {
        c.fix_include_guards = is_key(*key_it) || !is_false(*key_it);
    }

    auto get_values = [&](std::string_view key) {
        auto arg_begin = find_key(key);
        if (arg_begin == args.end()) {
            return std::make_pair(arg_begin, arg_begin);
        }
        ++arg_begin;
        return std::
            make_pair(arg_begin, std::find_if(arg_begin, args.end(), is_key));
    };

    auto [include_paths_begin, include_paths_end] = get_values("include_paths");
    c.include_paths = { include_paths_begin, include_paths_end };
    if (c.include_paths.empty()) {
        std::cerr << "No include paths provided\n";
        return false;
    }

    auto exist_as_directory = [](const fs::path &p) {
        if (!fs::exists(p)) {
            std::cerr << "Path " << p << " does not exist\n";
            return false;
        }
        if (!fs::is_directory(p)) {
            std::cerr << "Path " << p << " is not a directory\n";
            return false;
        }
        return true;
    };
    if (!std::all_of(
            c.include_paths.begin(),
            c.include_paths.end(),
            exist_as_directory))
    {
        return false;
    }

    return true;
}

bool
parse_config(config &c, int argc, char **argv) {
    std::vector<std::string_view> args(argv, argv + argc);
    return parse_config(c, args);
}

std::pair<fs::path, bool>
find_file(
    const std::vector<fs::path> &include_paths,
    const std::ssub_match &filename) {
    for (const auto &path: include_paths) {
        auto p = path / filename.str();
        if (fs::exists(p)) {
            return std::make_pair(p, true);
        }
    }
    return std::make_pair(fs::path(filename.first, filename.second), false);
}

bool
is_parent(const fs::path &dir, const fs::path &p) {
    for (auto b = dir.begin(), s = p.begin(); b != dir.end(); ++b, ++s) {
        if (s == p.end() || *s != *b) {
            return false;
        }
    }
    return true;
}

std::vector<fs::path>::const_iterator
find_parent_path(
    const std::vector<fs::path> &include_paths,
    const fs::path &filename) {
    return std::find_if(
        include_paths.begin(),
        include_paths.end(),
        [&filename](const fs::path &dir) { return is_parent(dir, filename); });
}

bool
is_cpp_file(const fs::path &p) {
    constexpr std::string_view extensions[] = {".h", ".hpp", ".cpp"};
    for (const auto &extension: extensions) {
        if (p.extension() == extension) {
            return true;
        }
    }
    return false;
}

int
main(int argc, char **argv) {
    config c;
    if (!parse_config(c, argc, argv)) {
        return 1;
    }

    // Find files in directory
    std::string content;
    std::vector<fs::path> file_paths;
    for (const auto &include_path: c.include_paths) {
        fs::recursive_directory_iterator dir_paths{ include_path };
        for (auto &p: dir_paths) {
            if (is_cpp_file(p.path())) {
                file_paths.emplace_back(p);
            }
        }
    }

    // Fix include guards
    if (c.fix_include_guards) {
        if (c.show_progress) {
            std::cout << "Fixing include guards in patched files\n";
        }
        for (auto &p: file_paths) {
            // Validate the file
            if (c.verbose) {
                std::cout << p << '\n';
            }
            if (!fs::exists(p)) {
                if (c.verbose) {
                    std::cout << "File is not in include paths\n";
                }
                continue;
            }

            std::ifstream t(p);
            if (!t) {
                if (c.show_progress) {
                    std::cout << "Failed to open file\n";
                }
                continue;
            }

            auto parent_it = find_parent_path(c.include_paths, p);
            if (parent_it == c.include_paths.end()) {
                if (c.show_progress) {
                    std::cout << "Cannot find include paths for " << p << "\n";
                }
                continue;
            }

            // Look for current guard
            std::string file_content{
                std::istreambuf_iterator<char>(t),
                std::istreambuf_iterator<char>()
            };
            std::regex include_guard_expression(
                "(^|\n) *# *ifndef *([a-zA-Z0-9_/\\. ]+)");
            std::smatch include_guard_match;
            if (std::regex_search(
                    file_content,
                    include_guard_match,
                    include_guard_expression))
            {
                if (c.verbose) {
                    std::cout
                        << "Found guard " << include_guard_match[2] << '\n';
                }
            } else {
                if (c.show_progress) {
                    std::cout << "Cannot find include guard for " << p << "\n";
                }
                continue;
            }

            // Calculate expected guard
            std::string prev_guard = include_guard_match[2].str();
            fs::path relative_p = fs::relative(p, *parent_it);
            std::string expected_guard = relative_p.string();
            std::transform(
                expected_guard.begin(),
                expected_guard.end(),
                expected_guard.begin(),
                [](char x) {
                    if (x == '/' || x == '.' || x == '-') {
                        return '_';
                    }
                    return static_cast<char>(std::toupper(x));
                });

            if (prev_guard == expected_guard) {
                if (c.verbose) {
                    std::cout << "Guard " << prev_guard << " is correct\n";
                }
                continue;
            } else {
                if (c.show_progress) {
                    std::cout << "Convert guard from " << prev_guard << " to "
                              << expected_guard << '\n';
                }
            }

            // Check if the expected guard is OK
            bool new_guard_ok = std::all_of(
                expected_guard.begin(),
                expected_guard.end(),
                [](char x) { return std::isalnum(x) || x == '_'; });
            if (!new_guard_ok) {
                if (c.show_progress) {
                    std::cout << "Inferred guard " << expected_guard
                              << " is not OK\n";
                }
                continue;
            }

            // Replace all guards in the file
            std::size_t guard_search_begin = 0;
            std::size_t guard_match_pos;
            while ((guard_match_pos = file_content
                                          .find(prev_guard, guard_search_begin))
                   != std::string::npos)
            {
                file_content
                    .replace(guard_match_pos, prev_guard.size(), expected_guard);
                guard_search_begin = guard_match_pos + prev_guard.size();
            }

            t.close();
            std::ofstream fout(p);
            fout << file_content;
        }
    }

    return 0;
}
