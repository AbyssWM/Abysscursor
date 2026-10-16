#include <cairo/cairo.h>
#include <zip.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

static void archive(const fs::path& path, const std::string& meta, const std::vector<unsigned char>& image, bool toml = false, bool svg = false, bool brokenFrame = false) {
    int error = 0;
    zip_t* zip = zip_open(path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip)
        throw std::runtime_error("cannot create fixture archive");
    auto add = [zip](const char* name, const void* data, size_t size) {
        auto* source = zip_source_buffer(zip, data, size, 0);
        if (!source)
            throw std::runtime_error("cannot create fixture source");
        if (zip_file_add(zip, name, source, ZIP_FL_ENC_UTF_8) < 0) {
            zip_source_free(source);
            throw std::runtime_error("cannot add fixture entry");
        }
    };
    try {
        add(toml ? "meta.toml" : "meta.hl", meta.data(), meta.size());
        add(svg ? "image.svg" : "image.png", image.data(), image.size());
        if (brokenFrame)
            add("broken.svg", "bad", 3);
        if (zip_close(zip) < 0)
            throw std::runtime_error("cannot write fixture archive");
    } catch (...) {
        zip_discard(zip);
        throw;
    }
}

int main(int argc, char** argv) {
    try {
        if (argc != 2)
            throw std::runtime_error("fixture home argument missing");
        const fs::path home = argv[1];
        fs::create_directories(home / "system");
        auto theme = [&](const char* name) {
            const auto path = home / ".local/share/icons" / name;
            fs::create_directories(path / "cursors");
            std::ofstream manifest(path / "manifest.hl");
            manifest << "name = " << name << "\ncursors_directory = cursors\n";
            if (!manifest)
                throw std::runtime_error("cannot write fixture manifest");
            return path / "cursors";
        };
        std::vector<unsigned char> png;
        auto* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 8, 8);
        auto* cr = cairo_create(surface);
        cairo_set_source_rgba(cr, 1, 0, 0, 1);
        cairo_paint(cr);
        cairo_destroy(cr);
        const auto status = cairo_surface_write_to_png_stream(surface, [](void* out, const unsigned char* bytes, unsigned int size) {
            auto& data = *static_cast<std::vector<unsigned char>*>(out);
            data.insert(data.end(), bytes, bytes + size);
            return CAIRO_STATUS_SUCCESS;
        }, &png);
        cairo_surface_destroy(surface);
        if (status != CAIRO_STATUS_SUCCESS)
            throw std::runtime_error("cannot encode fixture PNG");
        const std::string meta = "define_size = 8, image.png, 125\nresize_algorithm = nearest\nhotspot_x = 0.25\nhotspot_y = 0.5\nnominal_size = 0.5\ndefine_override = arrow\n";
        const std::string svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"8\" height=\"8\"><rect width=\"8\" height=\"8\" fill=\"red\"/></svg>";
        const std::vector<unsigned char> svgBytes(svg.begin(), svg.end());
        auto valid = theme("review-fixture");
        archive(valid / "left_ptr.hlc", meta, png);
        archive(valid / "hand.hlc", "define_size = 0, image.svg, 75\n", svgBytes, false, true);
        archive(theme("review-toml") / "left_ptr.hlc", "[General]\ndefine_size = '8,image.png,125'\nresize_algorithm = 'nearest'\n", png, true);
        archive(theme("review-badtoml") / "left_ptr.hlc", "[General]\ndefine_size = 'broken'\n", png, true);
        archive(theme("review-badsize") / "left_ptr.hlc", "define_size = 4294967304, image.png\n", png);
        archive(theme("review-baddimension") / "left_ptr.hlc", "define_size = 16, image.png\n", png);
        archive(theme("review-badpng") / "left_ptr.hlc", meta, {png.begin(), png.begin() + png.size() / 2});
        archive(theme("review-badsvg") / "left_ptr.hlc", "define_size = 0, image.svg\n", {'b', 'a', 'd'}, false, true);
        archive(theme("review-partialsvg") / "left_ptr.hlc", "define_size = 0, image.svg; 0, broken.svg\n", svgBytes, false, true, true);
        archive(theme("review-badgeometry") / "left_ptr.hlc", "[General]\ndefine_size = '8,image.png'\nnominal_size = nan\n", png, true);
        archive(theme("review-badhotspot") / "left_ptr.hlc", "[General]\ndefine_size = '8,image.png'\nhotspot_x = 1e30\n", png, true);
        archive(theme("review-badwhitespace") / "left_ptr.hlc", "[General]\ndefine_size = '    ,     '\n", png, true);
        const auto badDirectory = theme("review-baddirectory").parent_path();
        std::ofstream(badDirectory / "not-a-directory") << "file";
        std::ofstream(badDirectory / "manifest.hl") << "name = review-baddirectory\ncursors_directory = not-a-directory\n";
        auto badzip = theme("review-badzip");
        std::ofstream(badzip / "left_ptr.hlc") << "not a zip";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
