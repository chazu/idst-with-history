#include "turtle_proper.h"
#include "number.h"
#include "boolean.h"
#include "string.h"
#include <math.h>
#include <stdio.h>

// External references
extern struct __libid *_libid;

// Global vtables and singleton
oop Turtle_vtable = 0;
oop Canvas_vtable = 0;
oop turtle_singleton = 0;

// Math helpers
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double deg_to_rad(double degrees) {
    return degrees * M_PI / 180.0;
}

// ============================================================================
// Canvas Implementation (Pure libid object)
// ============================================================================

oop Canvas_new(oop width_obj, oop height_obj) {
    if (!Canvas_vtable) {
        fprintf(stderr, "Error: Canvas vtable not initialized\n");
        return 0;
    }
    
    // Allocate canvas object using libid
    oop canvas_obj = _libid->alloc(Canvas_vtable, sizeof(struct t_Canvas));
    if (!canvas_obj) {
        fprintf(stderr, "Error: Failed to allocate canvas object\n");
        return 0;
    }
    
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
    
    // Create canvas data as a single string (flat array)
    size_t data_size = w * h + h; // +h for newlines
    char *canvas_data = _libid->balloc(data_size + 1);
    if (!canvas_data) {
        fprintf(stderr, "Error: Failed to allocate canvas data\n");
        return 0;
    }
    
    // Initialize canvas with empty characters and newlines
    int pos = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            canvas_data[pos++] = TURTLE_EMPTY_CHAR;
        }
        canvas_data[pos++] = '\n';
    }
    canvas_data[pos] = '\0';
    
    // Create string object for canvas data
    canvas->data = s72_string_new(canvas_data).obj;
    
    return canvas_obj;
}

oop Canvas_clear(oop closure, oop state, oop self) {
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

oop Canvas_drawLine_from_to_(oop closure, oop state, oop self, oop from_obj, oop to_obj) {
    (void)closure; (void)state;
    
    Canvas canvas = (Canvas)self;
    
    // Extract points (simplified - in real implementation, points would be proper objects)
    Point from = point_from_oop(from_obj);
    Point to = point_from_oop(to_obj);
    
    // Get canvas dimensions
    S72Value width_val = {canvas->width};
    S72Value height_val = {canvas->height};
    
    int w = (int)s72_number_value(width_val);
    int h = (int)s72_number_value(height_val);
    
    // Convert world coordinates to screen coordinates
    int x1 = (int)(from.x + w/2);
    int y1 = (int)(from.y + h/2);
    int x2 = (int)(to.x + w/2);
    int y2 = (int)(to.y + h/2);
    
    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    // Get mutable access to canvas data
    S72Value data_val = {canvas->data};
    char *data = (char*)s72_string_data(data_val);
    
    int x = x1, y = y1;
    while (1) {
        // Draw pixel if within bounds
        if (x >= 0 && x < w && y >= 0 && y < h) {
            int pos = y * (w + 1) + x; // +1 for newline
            data[pos] = TURTLE_CANVAS_CHAR;
        }
        
        if (x == x2 && y == y2) break;
        
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

oop Canvas_show(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Canvas canvas = (Canvas)self;
    S72Value data_val = {canvas->data};
    
    printf("%s", s72_string_data(data_val));
    return self;
}

// ============================================================================
// Turtle Implementation (Pure libid object)
// ============================================================================

oop Turtle_new(void) {
    if (!Turtle_vtable) {
        fprintf(stderr, "Error: Turtle vtable not initialized\n");
        return 0;
    }
    
    // Allocate turtle object using libid
    oop turtle_obj = _libid->alloc(Turtle_vtable, sizeof(struct t_Turtle));
    if (!turtle_obj) {
        fprintf(stderr, "Error: Failed to allocate turtle object\n");
        return 0;
    }
    
    Turtle turtle = (Turtle)turtle_obj;
    
    // Initialize turtle state with proper libid objects
    turtle->x = s72_number_new(0.0).obj;
    turtle->y = s72_number_new(0.0).obj;
    turtle->heading = s72_number_new(0.0).obj;
    turtle->pen_down = S72_TRUE.obj;
    
    // Create canvas
    oop width = s72_number_new(TURTLE_CANVAS_WIDTH).obj;
    oop height = s72_number_new(TURTLE_CANVAS_HEIGHT).obj;
    turtle->canvas = Canvas_new(width, height);
    
    return turtle_obj;
}

oop Turtle_forward_(oop closure, oop state, oop self, oop distance_obj) {
    (void)closure; (void)state;
    
    Turtle turtle = (Turtle)self;
    
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
    double rad = deg_to_rad(heading);
    double new_x = old_x + distance * cos(rad);
    double new_y = old_y + distance * sin(rad);
    
    // Update turtle position with new number objects
    turtle->x = s72_number_new(new_x).obj;
    turtle->y = s72_number_new(new_y).obj;
    
    // Draw line if pen is down
    S72Value pen_val = {turtle->pen_down};
    if (pen_val.obj == S72_TRUE.obj) {
        Point from = point_make(old_x, old_y);
        Point to = point_make(new_x, new_y);
        
        oop from_obj = point_to_oop(from);
        oop to_obj = point_to_oop(to);
        
        Canvas_drawLine_from_to_(0, 0, turtle->canvas, from_obj, to_obj);
    }
    
    return self;
}

oop Turtle_turn_(oop closure, oop state, oop self, oop angle_obj) {
    (void)closure; (void)state;
    
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

oop Turtle_penUp(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Turtle turtle = (Turtle)self;
    turtle->pen_down = S72_FALSE.obj;
    return self;
}

oop Turtle_penDown(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Turtle turtle = (Turtle)self;
    turtle->pen_down = S72_TRUE.obj;
    return self;
}

oop Turtle_clear(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Turtle turtle = (Turtle)self;
    
    // Reset turtle state
    turtle->x = s72_number_new(0.0).obj;
    turtle->y = s72_number_new(0.0).obj;
    turtle->heading = s72_number_new(0.0).obj;
    turtle->pen_down = S72_TRUE.obj;
    
    // Clear canvas
    Canvas_clear(0, 0, turtle->canvas);
    
    return self;
}

oop Turtle_show(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Turtle turtle = (Turtle)self;
    Canvas_show(0, 0, turtle->canvas);
    return self;
}

oop Turtle_position(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Turtle turtle = (Turtle)self;
    S72Value x_val = {turtle->x};
    S72Value y_val = {turtle->y};
    
    printf("Position: (%.2f, %.2f)\n", 
           s72_number_value(x_val), 
           s72_number_value(y_val));
    
    return self;
}

oop Turtle_heading_method(oop closure, oop state, oop self) {
    (void)closure; (void)state;
    
    Turtle turtle = (Turtle)self;
    S72Value heading_val = {turtle->heading};
    
    printf("Heading: %.2f degrees\n", s72_number_value(heading_val));
    
    return self;
}

// ============================================================================
// Point Helpers (could be proper objects too)
// ============================================================================

Point point_make(double x, double y) {
    Point p = {x, y};
    return p;
}

oop point_to_oop(Point p) {
    // Simplified: return a 2-element array or create a proper Point object
    // For now, just encode as two numbers (this is a hack)
    return s72_number_new(p.x).obj; // Would need proper Point object
}

Point point_from_oop(oop point_obj) {
    // Simplified: decode from number (this is a hack)
    S72Value val = {point_obj};
    if (s72_is_number(val)) {
        return point_make(s72_number_value(val), 0.0);
    }
    return point_make(0.0, 0.0);
}

// ============================================================================
// Initialization
// ============================================================================

void turtle_init(void) {
    // Create vtables
    Turtle_vtable = _libid->proto(_libid->_object);
    Canvas_vtable = _libid->proto(_libid->_object);
    
    if (!Turtle_vtable || !Canvas_vtable) {
        fprintf(stderr, "Error: Failed to create turtle vtables\n");
        return;
    }
    
    // Install turtle methods
    oop sel_forward = _libid->intern("forward:");
    oop sel_turn = _libid->intern("turn:");
    oop sel_penUp = _libid->intern("penUp");
    oop sel_penDown = _libid->intern("penDown");
    oop sel_clear = _libid->intern("clear");
    oop sel_show = _libid->intern("show");
    oop sel_position = _libid->intern("position");
    oop sel_heading = _libid->intern("heading");
    
    _libid->method(Turtle_vtable, sel_forward, (_imp_t)Turtle_forward_);
    _libid->method(Turtle_vtable, sel_turn, (_imp_t)Turtle_turn_);
    _libid->method(Turtle_vtable, sel_penUp, (_imp_t)Turtle_penUp);
    _libid->method(Turtle_vtable, sel_penDown, (_imp_t)Turtle_penDown);
    _libid->method(Turtle_vtable, sel_clear, (_imp_t)Turtle_clear);
    _libid->method(Turtle_vtable, sel_show, (_imp_t)Turtle_show);
    _libid->method(Turtle_vtable, sel_position, (_imp_t)Turtle_position);
    _libid->method(Turtle_vtable, sel_heading, (_imp_t)Turtle_heading_method);
    
    // Install canvas methods
    oop sel_canvas_clear = _libid->intern("clear");
    oop sel_canvas_show = _libid->intern("show");
    oop sel_drawLine = _libid->intern("drawLine:from:to:");
    
    _libid->method(Canvas_vtable, sel_canvas_clear, (_imp_t)Canvas_clear);
    _libid->method(Canvas_vtable, sel_canvas_show, (_imp_t)Canvas_show);
    _libid->method(Canvas_vtable, sel_drawLine, (_imp_t)Canvas_drawLine_from_to_);
    
    // Create singleton turtle
    turtle_singleton = Turtle_new();
    
    printf("Turtle system initialized with pure libid objects\n");
}
