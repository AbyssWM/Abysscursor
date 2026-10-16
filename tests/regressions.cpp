#include <abysscursor/abysscursor.hpp>
#include <abysscursor/abysscursor.h>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

using namespace Abysscursor;
static_assert(!std::is_copy_constructible_v<CAbysscursorManager>);

static void require(bool condition, const char* message) {
    if (!condition)
        throw std::runtime_error(message);
}

static CAbysscursorManager manager(const char* name) {
    SManagerOptions options;
    options.allowDefaultFallback = false;
    return CAbysscursorManager(name, options);
}

int main() {
    try {
        for (const auto* theme : {"review-badtoml", "review-badsize", "review-baddimension", "review-badpng", "review-badzip", "review-badgeometry", "review-badhotspot", "review-badwhitespace", "review-baddirectory"}) {
            auto invalid = manager(theme);
            require(!invalid.valid(), theme);
            require(!invalid.loadThemeStyle({48}), "invalid manager must reject style");
            require(invalid.getShape("left_ptr", {48}).images.empty(), "invalid manager shape");
            require(invalid.getRawShapeData("left_ptr").images.empty(), "invalid manager raw shape");
            invalid.cursorSurfaceStyleDone({48});
        }
        auto missing = manager("missing");
        require(!missing.valid(), "missing theme");
        require(missing.getRawShapeData(nullptr).images.empty(), "null raw shape");
        auto toml = manager("review-toml");
        require(toml.valid(), "TOML without overrides must work");
        auto mgr = manager("review-fixture");
        require(mgr.valid(), "fixture manager");
        auto raw = mgr.getRawShapeData("left_ptr");
        require(raw.images.size() == 1 && raw.resizeAlgo == HC_RESIZE_NEAREST, "raw PNG metadata");
        const auto bytes = raw.images[0].data;
        require(mgr.loadThemeStyle({48}) && mgr.loadThemeStyle({48}), "repeated style loading");
        auto hand = mgr.getShape("hand", {48});
        require(hand.images.size() == 1, "SVG style must not duplicate frames");
        auto arrow = mgr.getShape("arrow", {48});
        require(arrow.images.size() == 1 && arrow.images[0].size == 96 && arrow.images[0].hotspotX == 24 && arrow.images[0].hotspotY == 48 && arrow.images[0].delay == 125, "resampling metadata");
        require(cairo_surface_status(arrow.images[0].surface) == CAIRO_STATUS_SUCCESS && cairo_image_surface_get_width(arrow.images[0].surface) == 96, "rendered surface");
        raw = mgr.getRawShapeData("left_ptr");
        require(raw.images.size() == 1 && raw.images[0].data == bytes, "raw images must exclude generated surfaces");
        require(mgr.loadThemeStyle({32}) && mgr.getShape("left_ptr", {32}).images.size() == 1, "resampling must use original images when other styles exist");
        require(mgr.getRawShapeData("arrow").overridenBy == "left_ptr", "raw alias");
        require(mgr.getRawShapeData(nullptr).images.empty(), "valid manager null raw shape");
        require(!mgr.loadThemeStyle({0}) && !mgr.loadThemeStyle({std::numeric_limits<unsigned int>::max()}), "invalid style size");
        mgr.cursorSurfaceStyleDone({48});
        require(mgr.getShape("hand", {48}).images.empty(), "released SVG style");
        require(mgr.loadThemeStyle({48}) && mgr.getShape("hand", {48}).images.size() == 1, "style reload");
        auto badsvg = manager("review-badsvg");
        require(badsvg.valid() && !badsvg.loadThemeStyle({48}) && !badsvg.loadThemeStyle({48}), "invalid SVG must keep failing");
        auto partialsvg = manager("review-partialsvg");
        require(partialsvg.valid() && !partialsvg.loadThemeStyle({48}) && !partialsvg.loadThemeStyle({48}), "partially rendered SVG must keep failing");
        int count = 123;
        require(!abysscursor_manager_valid(nullptr) && !abysscursor_load_theme_style(nullptr, {48}), "null C manager");
        require(!abysscursor_get_cursor_image_data(nullptr, "left_ptr", {48}, &count) && count == 0, "null C output");
        require(!abysscursor_get_raw_shape_data(nullptr, nullptr), "null C raw shape");
        abysscursor_cursor_image_data_free(nullptr, 1);
        abysscursor_raw_shape_data_free(nullptr);
        abysscursor_style_done(nullptr, {48});
        abysscursor_register_logging_function(nullptr, nullptr);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
