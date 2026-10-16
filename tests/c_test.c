/*
    abyssparse-test in C.
    Renders a cursor shape to /tmp at 48px

    For better explanations, see the cpp tests.
*/

#include <abysscursor/abysscursor.h>
#include <stdio.h>
#include <stdlib.h>

void logFunction(enum eAbysscursorLogLevel level, char* message) {
    printf("[hc] %s\n", message);
}

int main(int argc, char** argv) {
    struct abysscursor_manager_t* mgr = abysscursor_manager_create_with_logger(NULL, logFunction);

    if (!mgr) {
        printf("mgr null\n");
        return 1;
    }

    if (!abysscursor_manager_valid(mgr)) {
        printf("mgr is invalid\n");
        return 1;
    }

    abysscursor_cursor_raw_shape_data* shapeData = abysscursor_get_raw_shape_data(mgr, "left_ptr");
    if (!shapeData) {
        printf("failed querying left_ptr\n");
        return 1;
    }

    if (shapeData->overridenBy) {
        abysscursor_cursor_raw_shape_data* ov = abysscursor_get_raw_shape_data(mgr, shapeData->overridenBy);
        abysscursor_raw_shape_data_free(shapeData);
        shapeData = ov;
    }

    if (!shapeData || shapeData->len <= 0) {
        printf("left_ptr has no images\n");
        return 1;
    }

    printf("left_ptr images: %ld\n", shapeData->len);

    for (size_t i = 0; i < shapeData->len; ++i) {
        printf("left_ptr image size: %ld\n", shapeData->images[i].len);
    }

    abysscursor_raw_shape_data_free(shapeData);
    shapeData = NULL;

    struct abysscursor_cursor_style_info info = {.size = 48};
    if (!abysscursor_load_theme_style(mgr, info)) {
        printf("load failed\n");
        return 1;
    }

    int                            dataSize = 0;
    abysscursor_cursor_image_data** data     = abysscursor_get_cursor_image_data(mgr, "left_ptr", info, &dataSize);
    if (data == NULL || dataSize <= 0) {
        printf("data failed\n");
        return 1;
    }

    int ret = cairo_surface_write_to_png(data[0]->surface, "arrowC.png");

    abysscursor_cursor_image_data_free(data, dataSize);
    abysscursor_style_done(mgr, info);

    if (ret) {
        printf("cairo failed\n");
        return 1;
    }

    abysscursor_manager_free(mgr);
    return 0;
}
