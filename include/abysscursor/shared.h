#include <cairo/cairo.h>

#ifndef ABYSSCURSOR_SHARED_H
#define ABYSSCURSOR_SHARED_H

/*!
    struct for a single cursor image
*/
struct SCursorImageData {
    cairo_surface_t* surface;
    int              size;
    int              delay;
    int              hotspotX;
    int              hotspotY;
};

typedef struct SCursorImageData abysscursor_cursor_image_data;

enum eAbysscursorLogLevel {
    HC_LOG_NONE = 0,
    HC_LOG_TRACE,
    HC_LOG_INFO,
    HC_LOG_WARN,
    HC_LOG_ERR,
    HC_LOG_CRITICAL,
};

enum eAbysscursorDataType {
    HC_DATA_PNG = 0,
    HC_DATA_SVG,
};

enum eAbysscursorResizeAlgo {
    HC_RESIZE_INVALID = 0,
    HC_RESIZE_NONE,
    HC_RESIZE_BILINEAR,
    HC_RESIZE_NEAREST,
};

struct SCursorRawShapeImageC {
    void*             data;
    unsigned long int len;
    int               size;
    int               delay;
};

typedef struct SCursorRawShapeImageC abysscursor_cursor_raw_shape_image;

struct SCursorRawShapeDataC {
    struct SCursorRawShapeImageC* images;
    unsigned long int             len;
    float                         hotspotX;
    float                         hotspotY;
    char*                         overridenBy;
    enum eAbysscursorResizeAlgo    resizeAlgo;
    enum eAbysscursorDataType      type;
    float                         nominalSize;
};

typedef struct SCursorRawShapeDataC abysscursor_cursor_raw_shape_data;

/*
    msg is owned by the caller and will be freed afterwards.
*/
typedef void (*PABYSSCURSORLOGFUNC)(enum eAbysscursorLogLevel level, char* msg);

#endif
