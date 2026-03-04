#include "turtle.h"
#include "number.h"
#include "boolean.h"
#include "string.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

// External references
extern struct __libid *_libid;
extern S72Value S72_TRUE, S72_FALSE, S72_NIL;

// Global vtables and singleton (declared in main.c)
extern oop s72_turtle_vtable;
extern oop s72_canvas_vtable;
oop s72_turtle_singleton = 0;

// Math constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper functions
double s72_turtle_deg_to_rad(double degrees) {
    return degrees * M_PI / 180.0;
}

// ============================================================================
// Canvas Implementation (Pure libid object)
// ============================================================================

oop s72_canvas_new(oop width_obj, oop height_obj) {
    printf("DEBUG: s72_canvas_new called\n");

    if (!s72_canvas_vtable) {
        fprintf(stderr, "Error: Canvas vtable not initialized\n");
        return 0;
    }

    printf("DEBUG: Canvas vtable OK, allocating canvas object\n");

    // Allocate canvas object using libid
    oop canvas_obj = S72_ALLOC(s72_canvas_vtable, sizeof(struct t_Canvas));
    if (!canvas_obj) {
        fprintf(stderr, "Error: Failed to allocate canvas object\n");
        return 0;
    }

    printf("DEBUG: Canvas object allocated at %p\n", canvas_obj);
    
    Canvas canvas = (Canvas)canvas_obj;
    canvas->width = width_obj;
    canvas->height = height_obj;
    
    // Get numeric values for canvas size
    S72Value width_val = {width_obj};
    S72Value height_val = {height_obj};
    
    if (!s72_is_number(width_val) || !s72_is_number(height_val)) {
        fprintf(stderr, "Error: Canvas dimensions must be numbers\n");
        return 0;
    }
    
    int w = (int)s72_number_value(width_val);
    int h = (int)s72_number_value(height_val);
    
    printf("DEBUG: Creating canvas data, w=%d, h=%d\n", w, h);

    // Create canvas data as a single string (flat array)
    size_t data_size = w * h + h; // +h for newlines
    char *canvas_data = _libid->balloc(data_size + 1);
    if (!canvas_data) {
        fprintf(stderr, "Error: Failed to allocate canvas data\n");
        return 0;
    }

    printf("DEBUG: Canvas data allocated, initializing\n");

    // Initialize canvas with empty characters and newlines
    int pos = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            canvas_data[pos++] = TURTLE_EMPTY_CHAR;
        }
        canvas_data[pos++] = '\n';
    }
    canvas_data[pos] = '\0';

    printf("DEBUG: Canvas data initialized, creating string object\n");

    // Create string object for canvas data
    canvas->data = s72_string_new(canvas_data).obj;

    printf("DEBUG: Canvas string object created\n");
    
    return canvas_obj;
}

oop s72_canvas_clear(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Canvas canvas = (Canvas)self;
    
    // Get canvas dimensions
    S72Value width_val = {canvas->width};
    S72Value height_val = {canvas->height};
    
    int w = (int)s72_number_value(width_val);
    int h = (int)s72_number_value(height_val);
    
    // Get mutable access to canvas data
    S72Value data_val = {canvas->data};
    char *data = (char*)s72_string_data(data_val);
    
    // Clear canvas
    int pos = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            data[pos++] = TURTLE_EMPTY_CHAR;
        }
        data[pos++] = '\n';
    }
    
    return self;
}

oop s72_canvas_drawLine_from_to_(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;

    va_list args;
    va_start(args, self);
    oop from_x = va_arg(args, oop);
    oop from_y = va_arg(args, oop);
    oop to_x = va_arg(args, oop);
    oop to_y = va_arg(args, oop);
    va_end(args);

    Canvas canvas = (Canvas)self;

    // Extract coordinates
    S72Value fx_val = {from_x}, fy_val = {from_y};
    S72Value tx_val = {to_x}, ty_val = {to_y};
    
    if (!s72_is_number(fx_val) || !s72_is_number(fy_val) || 
        !s72_is_number(tx_val) || !s72_is_number(ty_val)) {
        fprintf(stderr, "Error: drawLine requires numeric coordinates\n");
        return self;
    }
    
    double x1 = s72_number_value(fx_val);
    double y1 = s72_number_value(fy_val);
    double x2 = s72_number_value(tx_val);
    double y2 = s72_number_value(ty_val);
    
    // Get canvas dimensions
    S72Value width_val = {canvas->width};
    S72Value height_val = {canvas->height};
    
    int w = (int)s72_number_value(width_val);
    int h = (int)s72_number_value(height_val);
    
    // Convert world coordinates to screen coordinates
    int ix1 = (int)(x1 + w/2);
    int iy1 = (int)(y1 + h/2);
    int ix2 = (int)(x2 + w/2);
    int iy2 = (int)(y2 + h/2);
    
    // Bresenham's line algorithm
    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = ix1 < ix2 ? 1 : -1;
    int sy = iy1 < iy2 ? 1 : -1;
    int err = dx - dy;
    
    // Get mutable access to canvas data
    S72Value data_val = {canvas->data};
    char *data = (char*)s72_string_data(data_val);
    
    int x = ix1, y = iy1;
    while (1) {
        // Draw pixel if within bounds
        if (x >= 0 && x < w && y >= 0 && y < h) {
            int pos = y * (w + 1) + x; // +1 for newline
            data[pos] = TURTLE_CANVAS_CHAR;
        }
        
        if (x == ix2 && y == iy2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    
    return self;
}

oop s72_canvas_show(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Canvas canvas = (Canvas)self;
    S72Value data_val = {canvas->data};
    
    printf("%s", s72_string_data(data_val));
    return self;
}

// ============================================================================
// Turtle Implementation (Pure libid object)
// ============================================================================

oop s72_turtle_new(void) {
    printf("DEBUG: s72_turtle_new called\n");

    if (!s72_turtle_vtable) {
        fprintf(stderr, "Error: Turtle vtable not initialized\n");
        return 0;
    }

    printf("DEBUG: Turtle vtable OK, allocating turtle object\n");

    // Allocate turtle object using libid
    oop turtle_obj = S72_ALLOC(s72_turtle_vtable, sizeof(struct t_Turtle));
    if (!turtle_obj) {
        fprintf(stderr, "Error: Failed to allocate turtle object\n");
        return 0;
    }

    printf("DEBUG: Turtle object allocated at %p\n", turtle_obj);
    
    Turtle turtle = (Turtle)turtle_obj;
    
    // Initialize turtle state with proper libid objects
    turtle->x = s72_number_new(0.0).obj;
    turtle->y = s72_number_new(0.0).obj;
    turtle->heading = s72_number_new(0.0).obj;
    turtle->pen_down = S72_TRUE.obj;
    
    // Create canvas
    oop width = s72_number_new(TURTLE_CANVAS_WIDTH).obj;
    oop height = s72_number_new(TURTLE_CANVAS_HEIGHT).obj;
    turtle->canvas = s72_canvas_new(width, height);
    
    return turtle_obj;
}

oop s72_turtle_forward_(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;

    va_list args;
    va_start(args, self);
    oop distance_obj = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: s72_turtle_forward_ called with self=%p, distance_obj=%p\n", self, distance_obj);

    Turtle turtle = (Turtle)self;

    printf("DEBUG: Turtle cast successful\n");
    
    // Get current state
    S72Value x_val = {turtle->x};
    S72Value y_val = {turtle->y};
    S72Value heading_val = {turtle->heading};
    S72Value distance_val = {distance_obj};
    
    if (!s72_is_number(distance_val)) {
        fprintf(stderr, "Error: forward: expects a number\n");
        return self;
    }
    
    double old_x = s72_number_value(x_val);
    double old_y = s72_number_value(y_val);
    double heading = s72_number_value(heading_val);
    double distance = s72_number_value(distance_val);
    
    // Calculate new position
    double rad = s72_turtle_deg_to_rad(heading);
    double new_x = old_x + distance * cos(rad);
    double new_y = old_y + distance * sin(rad);
    
    // Update turtle position with new number objects
    turtle->x = s72_number_new(new_x).obj;
    turtle->y = s72_number_new(new_y).obj;
    
    // Draw line if pen is down
    S72Value pen_val = {turtle->pen_down};
    if (pen_val.obj == S72_TRUE.obj) {
        oop old_x_obj = s72_number_new(old_x).obj;
        oop old_y_obj = s72_number_new(old_y).obj;
        oop new_x_obj = s72_number_new(new_x).obj;
        oop new_y_obj = s72_number_new(new_y).obj;
        
        s72_canvas_drawLine_from_to_(0, 0, turtle->canvas, old_x_obj, old_y_obj, new_x_obj, new_y_obj);
    }
    
    return self;
}

oop s72_turtle_turn_(oop closure, oop state, oop self, ...) {
    (void)closure; (void)state;

    va_list args;
    va_start(args, self);
    oop angle_obj = va_arg(args, oop);
    va_end(args);

    Turtle turtle = (Turtle)self;
    
    S72Value angle_val = {angle_obj};
    if (!s72_is_number(angle_val)) {
        fprintf(stderr, "Error: turn: expects a number\n");
        return self;
    }
    
    S72Value heading_val = {turtle->heading};
    double current_heading = s72_number_value(heading_val);
    double angle = s72_number_value(angle_val);
    
    double new_heading = current_heading + angle;
    
    // Normalize to 0-360 range
    while (new_heading < 0) new_heading += 360;
    while (new_heading >= 360) new_heading -= 360;
    
    // Update heading with new number object
    turtle->heading = s72_number_new(new_heading).obj;

    return self;
}

oop s72_turtle_penUp(oop closure, oop state, oop self) {
    (void)closure; (void)state;

    Turtle turtle = (Turtle)self;
    turtle->pen_down = S72_FALSE.obj;
    return self;
}

oop s72_turtle_penDown(oop closure, oop state, oop self) {
    (void)closure; (void)state;

    Turtle turtle = (Turtle)self;
    turtle->pen_down = S72_TRUE.obj;
    return self;
}

oop s72_turtle_clear(oop closure, oop state, oop self) {
    (void)closure; (void)state;

    Turtle turtle = (Turtle)self;

    // Reset turtle state
    turtle->x = s72_number_new(0.0).obj;
    turtle->y = s72_number_new(0.0).obj;
    turtle->heading = s72_number_new(0.0).obj;
    turtle->pen_down = S72_TRUE.obj;

    // Clear canvas
    s72_canvas_clear(0, 0, turtle->canvas);

    return self;
}

oop s72_turtle_show(oop closure, oop state, oop self) {
    (void)closure; (void)state;

    Turtle turtle = (Turtle)self;
    s72_canvas_show(0, 0, turtle->canvas);
    return self;
}

oop s72_turtle_position(oop closure, oop state, oop self) {
    (void)closure; (void)state;

    Turtle turtle = (Turtle)self;
    S72Value x_val = {turtle->x};
    S72Value y_val = {turtle->y};

    printf("Position: (%.2f, %.2f)\n",
           s72_number_value(x_val),
           s72_number_value(y_val));

    return self;  // Return self like other turtle methods
}

oop s72_turtle_heading_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;

    Turtle turtle = (Turtle)self;
    S72Value heading_val = {turtle->heading};

    printf("Heading: %.2f degrees\n", s72_number_value(heading_val));

    return self;  // Return self like other turtle methods
}

// ============================================================================
// Initialization
// ============================================================================

void s72_turtle_init(void) {
    // Vtables should already be created in main.c
    if (!s72_turtle_vtable || !s72_canvas_vtable) {
        fprintf(stderr, "Error: Turtle vtables not initialized\n");
        return;
    }

    // Install turtle methods
    oop sel_forward = S72_INTERN("forward:");
    oop sel_turn = S72_INTERN("turn:");
    oop sel_penUp = S72_INTERN("penUp");
    oop sel_penDown = S72_INTERN("penDown");
    oop sel_clear = S72_INTERN("clear");
    oop sel_show = S72_INTERN("show");
    oop sel_position = S72_INTERN("position");
    oop sel_heading = S72_INTERN("heading");

    S72_METHOD(s72_turtle_vtable, sel_forward, s72_turtle_forward_);
    S72_METHOD(s72_turtle_vtable, sel_turn, s72_turtle_turn_);
    S72_METHOD(s72_turtle_vtable, sel_penUp, s72_turtle_penUp);
    S72_METHOD(s72_turtle_vtable, sel_penDown, s72_turtle_penDown);
    S72_METHOD(s72_turtle_vtable, sel_clear, s72_turtle_clear);
    S72_METHOD(s72_turtle_vtable, sel_show, s72_turtle_show);
    S72_METHOD(s72_turtle_vtable, sel_position, s72_turtle_position);
    S72_METHOD(s72_turtle_vtable, sel_heading, s72_turtle_heading_method);

    // Install canvas methods
    oop sel_canvas_clear = S72_INTERN("clear");
    oop sel_canvas_show = S72_INTERN("show");
    oop sel_drawLine = S72_INTERN("drawLine:from:to:");

    S72_METHOD(s72_canvas_vtable, sel_canvas_clear, s72_canvas_clear);
    S72_METHOD(s72_canvas_vtable, sel_canvas_show, s72_canvas_show);
    S72_METHOD(s72_canvas_vtable, sel_drawLine, s72_canvas_drawLine_from_to_);

    // Create singleton turtle
    s72_turtle_singleton = s72_turtle_new();

    // Pure libid turtle system initialized successfully
}
