#include "manifest.hpp"

#include <toml++/toml.hpp>
#include <abyssparse.hpp>

#include <filesystem>

CManifest::CManifest(const std::string& path_) {
    try {
        if (std::filesystem::exists(path_ + ".hl")) {
            path           = path_ + ".hl";
            selectedParser = PARSER_ABYSSPARSE;
            return;
        }

        if (std::filesystem::exists(path_ + ".toml")) {
            path           = path_ + ".toml";
            selectedParser = PARSER_TOML;
            return;
        }
    } catch (...) { ; }
}

std::optional<std::string> CManifest::parse() {
    if (path.empty())
        return "Failed to find an appropriate manifest.";

    if (selectedParser == PARSER_ABYSSPARSE)
        return parseHL();
    if (selectedParser == PARSER_TOML)
        return parseTOML();

    return "No parser available for " + path;
}

std::optional<std::string> CManifest::parseHL() {
    std::unique_ptr<Abyssparse::CConfig> manifest;
    try {
        // TODO: unify this between util and lib
        manifest = std::make_unique<Abyssparse::CConfig>(path.c_str(), Abyssparse::SConfigOptions{.throwAllErrors = true});
        manifest->addConfigValue("cursors_directory", Abyssparse::STRING{""});
        manifest->addConfigValue("name", Abyssparse::STRING{""});
        manifest->addConfigValue("description", Abyssparse::STRING{""});
        manifest->addConfigValue("version", Abyssparse::STRING{""});
        manifest->addConfigValue("author", Abyssparse::STRING{""});
        manifest->commence();
        manifest->parse();
    } catch (const char* err) { return std::string{"failed: "} + err; }

    parsedData.cursorsDirectory = std::any_cast<Abyssparse::STRING>(manifest->getConfigValue("cursors_directory"));
    parsedData.name             = std::any_cast<Abyssparse::STRING>(manifest->getConfigValue("name"));
    parsedData.description      = std::any_cast<Abyssparse::STRING>(manifest->getConfigValue("description"));
    parsedData.version          = std::any_cast<Abyssparse::STRING>(manifest->getConfigValue("version"));
    parsedData.author           = std::any_cast<Abyssparse::STRING>(manifest->getConfigValue("author"));

    return {};
}

std::optional<std::string> CManifest::parseTOML() {
    try {
        auto MANIFEST = toml::parse_file(path);

        parsedData.cursorsDirectory = MANIFEST["General"]["cursors_directory"].value_or("");
        parsedData.name             = MANIFEST["General"]["name"].value_or("");
        parsedData.description      = MANIFEST["General"]["description"].value_or("");
        parsedData.version          = MANIFEST["General"]["version"].value_or("");
        parsedData.author           = MANIFEST["General"]["author"].value_or("");
    } catch (...) { return "Failed parsing toml"; }

    return {};
}

std::string CManifest::getPath() {
    return path;
}