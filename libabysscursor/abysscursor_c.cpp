#include "abysscursor/abysscursor.h"
#include "abysscursor/abysscursor.hpp"

using namespace Abysscursor;

abysscursor_manager_t* abysscursor_manager_create(const char* theme_name) {
    return (abysscursor_manager_t*)new CAbysscursorManager(theme_name);
}

abysscursor_manager_t* abysscursor_manager_create_with_logger(const char* theme_name, PABYSSCURSORLOGFUNC fn) {
    return (abysscursor_manager_t*)new CAbysscursorManager(theme_name, fn);
}

void abysscursor_manager_free(abysscursor_manager_t* manager) {
    delete (CAbysscursorManager*)manager;
}

int abysscursor_manager_valid(abysscursor_manager_t* manager) {
    const auto MGR = (CAbysscursorManager*)manager;
    return MGR->valid();
}

int abysscursor_load_theme_style(abysscursor_manager_t* manager, abysscursor_cursor_style_info info_) {
    const auto       MGR = (CAbysscursorManager*)manager;
    SCursorStyleInfo info;
    info.size = info_.size;
    return MGR->loadThemeStyle(info);
}

struct SCursorImageData** abysscursor_get_cursor_image_data(struct abysscursor_manager_t* manager, const char* shape, struct abysscursor_cursor_style_info info_, int* out_size) {
    const auto       MGR = (CAbysscursorManager*)manager;
    SCursorStyleInfo info;
    info.size                      = info_.size;
    int                       size = 0;
    struct SCursorImageData** data = MGR->getShapesC(size, shape, info);
    *out_size                      = size;
    return data;
}

void abysscursor_cursor_image_data_free(abysscursor_cursor_image_data** data, int size) {
    for (int i = 0; i < size; ++i) {
        free(data[i]);
    }

    free(data);
}

void abysscursor_style_done(abysscursor_manager_t* manager, abysscursor_cursor_style_info info_) {
    const auto       MGR = (CAbysscursorManager*)manager;
    SCursorStyleInfo info;
    info.size = info_.size;
    MGR->cursorSurfaceStyleDone(info);
}

void abysscursor_register_logging_function(struct abysscursor_manager_t* manager, PABYSSCURSORLOGFUNC fn) {
    const auto MGR = (CAbysscursorManager*)manager;
    MGR->registerLoggingFunction(fn);
}

CAPI abysscursor_cursor_raw_shape_data* abysscursor_get_raw_shape_data(struct abysscursor_manager_t* manager, char* shape) {
    const auto MGR = (CAbysscursorManager*)manager;
    return MGR->getRawShapeDataC(shape);
}

CAPI void abysscursor_raw_shape_data_free(abysscursor_cursor_raw_shape_data* data) {
    if (data->overridenBy) {
        free(data->overridenBy);
        delete data;
        return;
    }

    delete[] data->images;
    delete data;
}
