
#ifndef ABYSSCURSOR_H
#define ABYSSCURSOR_H

#ifdef __cplusplus

#define CAPI extern "C"

#else

#define CAPI

#endif

#include "shared.h"

struct abysscursor_manager_t;

/*!
    Simple struct for styles
*/
struct abysscursor_cursor_style_info {
    /*!
        Shape size.
        0 means "any" or "unspecified".
    */
    unsigned int size;
};

/*!
    Basic Abysscursor manager.

    Has to be created for either a specified theme, or
    nullptr if you want to use a default from the env.

    If no env is set, picks the first found.

    If none found, abysscursor_manager_valid will be false.

    If loading fails, abysscursor_manager_valid will be false.

    The caller gets the ownership, call abysscursor_manager_free to free this object.
*/
CAPI struct abysscursor_manager_t* abysscursor_manager_create(const char* theme_name);

/*!
    \since 0.1.6
    
    Same as abysscursor_manager_create, but with a logger.
*/
CAPI struct abysscursor_manager_t* abysscursor_manager_create_with_logger(const char* theme_name, PABYSSCURSORLOGFUNC fn);

/*!
    Free a abysscursor_manager_t*
*/
CAPI void abysscursor_manager_free(struct abysscursor_manager_t* manager);

/*!
    Returns true if the theme was successfully loaded,
    i.e. everything is A-OK and nothing should fail.
*/
CAPI int abysscursor_manager_valid(struct abysscursor_manager_t* manager);

/*!
    Loads a theme at a given style, synchronously.

    Returns whether it succeeded.
*/
CAPI int abysscursor_load_theme_style(struct abysscursor_manager_t* manager, struct abysscursor_cursor_style_info info);

/*!
    Returns a abysscursor_cursor_image_data*[] for a given cursor
    shape and size.

    The entire array needs to be freed instantly after using, see abysscursor_cursor_image_data_free()

    Surfaces stay valid.

    Once done with a size, call abysscursor_style_done()
*/
CAPI abysscursor_cursor_image_data** abysscursor_get_cursor_image_data(struct abysscursor_manager_t* manager, const char* shape, struct abysscursor_cursor_style_info info,
                                                                     int* out_size);

/*!
    Free a returned abysscursor_cursor_image_data.
*/
CAPI void abysscursor_cursor_image_data_free(abysscursor_cursor_image_data** data, int size);

/*!
    Marks a certain style as done, allowing it to be potentially freed
*/
CAPI void abysscursor_style_done(struct abysscursor_manager_t* manager, struct abysscursor_cursor_style_info info);

/*!
    \since 0.1.6

    Registers a logging function to a abysscursor_manager_t*

    PABYSSCURSORLOGFUNC's msg is owned by the caller and will be freed afterwards.

    fn can be null to remove a logger.
*/
CAPI void abysscursor_register_logging_function(struct abysscursor_manager_t* manager, PABYSSCURSORLOGFUNC fn);

/*!
    \since 0.1.6

    Returns the raw image data of a cursor shape, not rendered at all, alongside the metadata.

    The object needs to be freed instantly after using, see abysscursor_raw_shape_data_free()
*/
CAPI abysscursor_cursor_raw_shape_data* abysscursor_get_raw_shape_data(struct abysscursor_manager_t* manager, char* shape);

/*!
    \since 0.1.6

    See abysscursor_get_raw_shape_data.
    Frees the returned object.
*/
CAPI void abysscursor_raw_shape_data_free(abysscursor_cursor_raw_shape_data* data);

#endif