#include "meta.hpp"

#include <abyssparse.hpp>
#include <toml++/toml.hpp>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <charconv>
#include <cmath>

#include "VarList.hpp"

static thread_local CMeta* currentMeta = nullptr;

CMeta::CMeta(const std::string& rawdata_, bool abyssparse_ /* false for toml */, bool dataIsPath) : dataPath(dataIsPath), abyssparse(abyssparse_), rawdata(rawdata_) {
    if (!dataIsPath)
        return;

    rawdata = "";

    try {
        if (std::filesystem::exists(rawdata_ + ".hl")) {
            rawdata    = rawdata_ + ".hl";
            abyssparse = true;
            return;
        }

        if (std::filesystem::exists(rawdata_ + ".toml")) {
            rawdata    = rawdata_ + ".toml";
            abyssparse = false;
            return;
        }
    } catch (...) {}
}

std::optional<std::string> CMeta::parse() {
    if (rawdata.empty())
        return "Invalid meta (missing?)";

    std::optional<std::string> res;

    currentMeta = this;

    if (abyssparse)
        res = parseHL();
    else
        res = parseTOML();

    currentMeta = nullptr;

    if (!res && (!std::isfinite(parsedData.hotspotX) || !std::isfinite(parsedData.hotspotY) || !std::isfinite(parsedData.nominalSize)))
        return "Non-finite cursor geometry";
    if (!res && (parsedData.hotspotX < 0.F || parsedData.hotspotX > 1.F || parsedData.hotspotY < 0.F || parsedData.hotspotY > 1.F))
        return "Cursor hotspot must be between 0 and 1";
    return res;
}

static std::string removeBeginEndSpacesTabs(std::string str) {
    const auto first = str.find_first_not_of(" \t");
    return first == std::string::npos ? "" : str.substr(first, str.find_last_not_of(" \t") - first + 1);
}

static bool parseNonnegativeInt(const std::string& value, int& out) {
    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), out);
    return error == std::errc{} && end == value.data() + value.size() && out >= 0;
}

static Abyssparse::CParseResult parseDefineSize(const char* C, const char* V) {
    Abyssparse::CParseResult result;
    const std::string        VALUE = V;

    CVarList                 sizes(VALUE, 0, ';');

    for (const auto& sizeStr : sizes) {
        if (!sizeStr.contains(",")) {
            result.setError("Invalid define_size");
            return result;
        }

        auto                LHS   = removeBeginEndSpacesTabs(sizeStr.substr(0, sizeStr.find_first_of(",")));
        auto                RHS   = removeBeginEndSpacesTabs(sizeStr.substr(sizeStr.find_first_of(",") + 1));

        CMeta::SDefinedSize size;

        if (RHS.contains(",")) {
            const auto LL = removeBeginEndSpacesTabs(RHS.substr(0, RHS.find(',')));
            const auto RR = removeBeginEndSpacesTabs(RHS.substr(RHS.find(',') + 1));

            if (!parseNonnegativeInt(RR, size.delayMs)) {
                result.setError("Invalid frame delay");
                return result;
            }

            RHS = LL;
        }

        if (!std::regex_match(RHS, std::regex("^[A-Za-z0-9_\\-\\.]+$"))) {
            result.setError("Invalid cursor file name, characters must be within [A-Za-z0-9_\\-\\.] (if this seems like a mistake, check for invisible characters)");
            return result;
        }

        size.file = RHS;

        if (!size.file.ends_with(".svg")) {
            if (!parseNonnegativeInt(LHS, size.size) || size.size == 0) {
                result.setError("Invalid PNG size");
                return result;
            }
        } else
            size.size = 0;

        currentMeta->parsedData.definedSizes.push_back(size);
    }

    return result;
}

static Abyssparse::CParseResult parseOverride(const char* C, const char* V) {
    Abyssparse::CParseResult result;
    const std::string        VALUE = V;

    CVarList                 overrides(VALUE, 0, ';');

    for (const auto& o : overrides) {
        currentMeta->parsedData.overrides.push_back(o);
    }

    return result;
}

std::optional<std::string> CMeta::parseHL() {
    std::unique_ptr<Abyssparse::CConfig> meta;

    try {
        meta = std::make_unique<Abyssparse::CConfig>(rawdata.c_str(), Abyssparse::SConfigOptions{.pathIsStream = !dataPath});
        meta->addConfigValue("hotspot_x", Abyssparse::FLOAT{0.F});
        meta->addConfigValue("hotspot_y", Abyssparse::FLOAT{0.F});
        meta->addConfigValue("nominal_size", Abyssparse::FLOAT{1.F});
        meta->addConfigValue("resize_algorithm", Abyssparse::STRING{"nearest"});
        meta->registerHandler(::parseDefineSize, "define_size", {.allowFlags = false});
        meta->registerHandler(::parseOverride, "define_override", {.allowFlags = false});
        meta->commence();
        const auto RESULT = meta->parse();
        if (RESULT.error)
            return RESULT.getError();
    } catch (const char* err) { return "failed parsing meta: " + std::string{err}; }

    parsedData.hotspotX    = std::any_cast<Abyssparse::FLOAT>(meta->getConfigValue("hotspot_x"));
    parsedData.hotspotY    = std::any_cast<Abyssparse::FLOAT>(meta->getConfigValue("hotspot_y"));
    parsedData.nominalSize = std::clamp(std::any_cast<Abyssparse::FLOAT>(meta->getConfigValue("nominal_size")), 0.1F, 2.F);
    parsedData.resizeAlgo  = std::any_cast<Abyssparse::STRING>(meta->getConfigValue("resize_algorithm"));

    return {};
}

std::optional<std::string> CMeta::parseTOML() {
    try {
        auto MANIFEST = dataPath ? toml::parse_file(rawdata) : toml::parse(rawdata);

        parsedData.hotspotX    = MANIFEST["General"]["hotspot_x"].value_or(0.f);
        parsedData.hotspotY    = MANIFEST["General"]["hotspot_y"].value_or(0.f);
        parsedData.nominalSize = std::clamp(MANIFEST["General"]["nominal_size"].value_or(1.F), 0.1F, 2.F);

        const std::string OVERRIDES = MANIFEST["General"]["define_override"].value_or("");
        const std::string SIZES     = MANIFEST["General"]["define_size"].value_or("");

        //
        CVarList OVERRIDESLIST(OVERRIDES, 0, ';', true);
        for (auto& o : OVERRIDESLIST) {
            const auto RESULT = ::parseOverride("define_override", o.c_str());
            if (RESULT.error)
                return RESULT.getError();
        }

        CVarList SIZESLIST(SIZES, 0, ';', true);
        for (auto& s : SIZESLIST) {
            const auto RESULT = ::parseDefineSize("define_size", s.c_str());
            if (RESULT.error)
                return RESULT.getError();
        }

        parsedData.resizeAlgo = MANIFEST["General"]["resize_algorithm"].value_or("");
    } catch (std::exception& e) { return std::string{"Failed parsing toml: "} + e.what(); }

    return {};
}
