#ifndef S72_TURTLE_PROPER_H
#define S72_TURTLE_PROPER_H

#include "../s72.h"

// Canvas constants
#define TURTLE_CANVAS_WIDTH 80
#define TURTLE_CANVAS_HEIGHT 24
#define TURTLE_CANVAS_CHAR '*'
#define TURTLE_EMPTY_CHAR ' '

// Forward declarations for libid object types
typedef struct t_Turtle *Turtle;
typedef struct t_Canvas *Canvas;

// Turtle object structure (libid-native)
struct t_Turtle {
    // libid object header (vtable pointer at offset -1)
    oop x;           // Number object for X coordinate
    oop y;           // Number object for Y coordinate  
    oop heading;     // Number object for heading in degrees
    oop pen_down;    // Boolean object for pen state
    oop canvas;      // Canvas object
};

// Canvas object structure (libid-native)
struct t_Canvas {
    // libid object header (vtable pointer at offset -1)
    oop width;       // Number object for canvas width
    oop height;      // Number object for canvas height
    oop data;        // String object containing flat canvas data
};

// Global turtle vtables
extern oop Turtle_vtable;
extern oop Canvas_vtable;

// Global turtle singleton
extern oop turtle_singleton;

// Turtle creation and access
oop Turtle_new(void);
oop Canvas_new(oop width, oop height);

// Turtle methods (proper libid methods)
oop Turtle_forward_(oop closure, oop state, oop self, oop distance);
oop Turtle_turn_(oop closure, oop state, oop self, oop angle);
oop Turtle_penUp(oop closure, oop state, oop self);
oop Turtle_penDown(oop closure, oop state, oop self);
oop Turtle_clear(oop closure, oop state, oop self);
oop Turtle_show(oop closure, oop state, oop self);
oop Turtle_position(oop closure, oop state, oop self);
oop Turtle_heading_method(oop closure, oop state, oop self);

// Canvas methods (proper libid methods)
oop Canvas_clear(oop closure, oop state, oop self);
oop Canvas_drawLine_from_to_(oop closure, oop state, oop self, oop from_point, oop to_point);
oop Canvas_show(oop closure, oop state, oop self);

// Point helper (could be a proper object too)
typedef struct {
    double x, y;
} Point;

Point point_make(double x, double y);
oop point_to_oop(Point p);
Point point_from_oop(oop point_obj);

// Initialization
void turtle_init(void);

#endif // S72_TURTLE_PROPER_H
