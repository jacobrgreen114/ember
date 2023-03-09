// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#include <filesystem>
#include <fstream>
#include <iostream>
#include <rapidxml/rapidxml.hpp>
#include <span>

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

template <typename StringType>
NORETURN auto panic(StringType msg) {
  std::cerr << msg << std::endl;
  exit(-1);
}

class FileConfiguration {
  std::string symbol_name_;
  std::string namespace_;
  fs::path file_path_;
  fs::path source_dest_;
  fs::path header_dest_;

 public:
  FileConfiguration(std::string symbol_name, fs::path file_path)
      : symbol_name_{std::move(symbol_name)}, file_path_{std::move(file_path)} {
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

auto create_header_file(const FileConfiguration& config,
                        const std::vector<data_t>& data) {
  auto file = std::ofstream{config.header_dest(), std::ios::trunc};

  file << "#pragma once\n"
       << "#include <array>\n";

  if (!config.ns().empty()) {
    file << "namespace " << config.ns() << "{\n";
  }

  file << "extern const std::array<" << data_t_name << ", " << data.size()
       << "> " << config.symbol() << ";\n";

  if (!config.ns().empty()) {
    file << "}";
  }

  file << std::endl;
}

auto create_source_file(const FileConfiguration& config,
                        const std::vector<data_t>& data) {
  auto file = std::ofstream{config.source_dest()};

  file << "#include " << config.header_dest() << '\n';

  if (!config.ns().empty()) {
    file << "namespace " << config.ns() << "{\n";
  }

  file << "constexpr std::array<" << data_t_name << ", " << data.size() << "> "
       << config.symbol() << " = {\n    ";

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

  file << "\n};\n";

  if (!config.ns().empty()) {
    file << "}";
  }

  file << std::endl;
}

auto generate_files(const FileConfiguration& config) {
  const auto data = read_whole_binary_file(config.path());
  create_header_file(config, data);
  create_source_file(config, data);
}

auto main(int argc, const char* const argv[]) -> int {
  const auto args = std::span{argv, static_cast<size_t>(argc)};

  const auto symbol_name = std::string_view{args[1]};
  const auto binary_file_path = fs::proximate(args[2]);

  const auto config =
      FileConfiguration{std::string{symbol_name}, binary_file_path};

  generate_files(config);

  return 0;
}