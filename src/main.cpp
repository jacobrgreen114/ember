// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <algorithm>

#include <rapidxml/rapidxml.hpp>
#include <CLI/CLI.hpp>

#ifndef NODISCARD
#define NODISCARD [[nodiscard]]
#endif

#ifndef NORETURN
#define NORETURN [[noreturn]]
#endif

namespace fs = std::filesystem;

constexpr auto formatting_bytes_per_line = 8;

constexpr auto hdr_ext = std::string_view{".hpp"};
constexpr auto src_ext = std::string_view{".cpp"};

constexpr auto ember_node_name = std::string_view{"Ember"};

constexpr auto ember_file_name = std::string_view{".emberfile"};

using data_t = uint8_t;

constexpr auto data_t_name = std::string_view{"uint8_t"};
constexpr auto data_t_signed_name = std::string_view{"int8_t"};

class exit_exception : public std::runtime_error {
  int exit_code_;

 public:
  explicit exit_exception(int exit_code, const char* message)
      : std::runtime_error(message), exit_code_{exit_code} {}

  explicit exit_exception(int exit_code, const std::string& message)
      : std::runtime_error(message), exit_code_{exit_code} {}

  NODISCARD auto code() const { return exit_code_; }
};

template <typename StringType>
NORETURN auto panic(StringType msg) {
  throw exit_exception{-1, msg};
}

class FileConfiguration {
  std::string symbol_name_;
  std::string namespace_;
  fs::path file_path_;
  fs::path source_dest_;
  fs::path header_dest_;
  bool sign_ = false;

 public:
  FileConfiguration(std::string symbol_name, fs::path file_path)
      : symbol_name_{std::move(symbol_name)}, file_path_{std::move(file_path)} {
    if (symbol_name_.empty()) {
      symbol_name_ = file_path_.filename().string();
      std::replace_if(
          symbol_name_.begin(), symbol_name_.end(),
          [](auto& c) { return c == '.'; }, '_');
    }

    source_dest_ = file_path_;
    header_dest_ = file_path_;
    source_dest_ += src_ext;
    header_dest_ += hdr_ext;
  }

  NODISCARD auto& ns() const { return namespace_; }
  NODISCARD auto& symbol() const { return symbol_name_; }
  NODISCARD auto& path() const { return file_path_; }
  NODISCARD auto& source_dest() const { return source_dest_; }
  NODISCARD auto& header_dest() const { return header_dest_; }
  NODISCARD auto sign() const { return sign_; }
};

#if EMBER_FILE_SUPPORT

class Configuration {
  std::vector<FileConfiguration> files;
};

auto parse_ember_file(const fs::path& path) -> Configuration {
  auto ember = read_whole_text_file(path);

  auto doc = rapidxml::xml_document{};
  doc.parse<0>(ember.data());

  auto element = doc.first_node(ember_node_name.data(), ember_node_name.size());
  if (element == nullptr) {
    panic(".emberfile does not contain root Ember node");
  }
}

#endif

auto read_whole_binary_file(const fs::path& path) -> std::vector<data_t> {
  auto file = std::basic_ifstream<data_t, std::char_traits<data_t>>{
      path, std::ios::binary | std::ios::ate};

  auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  auto buffer = std::vector<data_t>(size);
  if (!file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()))) {
    throw std::runtime_error{"Failed to read file."};
  }

  return buffer;
}

auto read_whole_text_file(const fs::path& path) -> std::string {
  auto file = std::ifstream{path};
  auto text = std::stringstream{};
  text << file.rdbuf();
  return text.str();
}

// header_name.hpp
auto create_header_file(const FileConfiguration& config,
                        const std::vector<data_t>& data) {
  auto file = std::ofstream{config.header_dest(), std::ios::trunc};

  // #pragma once
  // #include <cstdint>
  // #include <array>
  file << "#pragma once\n"
       << "#include <cstdint>\n"
       << "#include <array>\n";

  // namespace <namespace> {
  if (!config.ns().empty()) {
    file << "namespace " << config.ns() << "{\n";
  }

  // extern const std::array<uint8, 16>;
  file << "extern const std::array<";
  file << (!config.sign() ? data_t_name : data_t_signed_name);
  file << ", " << data.size() << "> " << config.symbol() << ";\n";

  // } // <namespace>
  if (!config.ns().empty()) {
    file << "}";
  }

  file << std::endl;
}

// header_name.cpp
auto create_source_file(const FileConfiguration& config,
                        const std::vector<data_t>& data) {
  auto file = std::ofstream{config.source_dest()};

  // #include "<header_file>"
  file << "#include " << config.header_dest() << '\n';

  // namespace <namespace> {
  if (!config.ns().empty()) {
    file << "namespace " << config.ns() << "{\n";
  }

  // constexpr std::array<uint8, 16> = {
  file << "constexpr std::array<";
  file << (!config.sign() ? data_t_name : data_t_signed_name);
  file << ", " << data.size() << "> " << config.symbol() << " = {\n    ";

  // 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  file << std::hex;
  const auto n_1 = data.size() - 1;
  for (auto i = 0; i < data.size(); ++i) {
    file << "0x" << (uint32_t)data[i];
    if (i != n_1) {
      file << ", ";
      if ((i + 1) % formatting_bytes_per_line == 0) {
        file << "\n    ";
      }
    }
  }

  // }
  file << "\n};\n";

  // } // <namespace>
  if (!config.ns().empty()) {
    file << "} // " << config.ns();
  }

  file << std::endl;
}

auto generate_files(const FileConfiguration& config) {
  const auto data = read_whole_binary_file(config.path());
  create_header_file(config, data);
  create_source_file(config, data);
}

auto ember_main(const std::span<std::string_view> args) {
  const auto symbol_name = args[1];
  const auto binary_file_path = fs::proximate(args[2]);

  const auto config =
      FileConfiguration{std::string{symbol_name}, binary_file_path};

  generate_files(config);
}

auto main(int argc, const char* const argv[]) -> int {
  // const auto cstr_args = std::span{argv, static_cast<size_t>(argc)};
  // auto args = std::vector<std::string_view>{};
  // args.reserve(cstr_args.size());
  // std::transform(cstr_args.begin(), cstr_args.end(),
  // std::back_inserter(args),
  //                [](auto& arg) { return arg; });

  auto app = CLI::App{"Ember"};

  auto binary_file = std::string{};
  app.add_option("-i", binary_file, "binary file to bake")->required(true);

  auto symbol_name = std::string{};
  app.add_option("-s,--symbol", symbol_name, "symbol");

  CLI11_PARSE(app, argc, argv);

  const auto binary_file_path = fs::proximate(binary_file);

  const auto config = FileConfiguration{std::string{symbol_name}, binary_file_path};

  generate_files(config);

  // try {
  //
  // } catch (const exit_exception& exit) {
  //   std::cerr << exit.what() << std::endl;
  //   return exit.code();
  // } catch (...) {
  //   throw;
  // }

  return 0;
}