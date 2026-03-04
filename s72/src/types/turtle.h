#ifndef S72_TURTLE_H
#define S72_TURTLE_H

#include "../s72.h"

// Pure libid turtle object structure
typedef struct t_Turtle {
    oop x;           // Number object for X coordinate
    oop y;           // Number object for Y coordinate
    oop heading;     // Number object for heading in degrees
    oop pen_down;    // Boolean object for pen state
    oop canvas;      // Canvas object
} *Turtle;

// Pure libid canvas object structure
typedef struct t_Canvas {
    oop width;       // Number object for canvas width
    oop height;      // Number object for canvas height
    oop data;        // String object containing flat canvas data
} *Canvas;

// Canvas constants
#define TURTLE_CANVAS_WIDTH 80
#define TURTLE_CANVAS_HEIGHT 24
#define TURTLE_CANVAS_CHAR '*'
#define TURTLE_EMPTY_CHAR ' '

// Global turtle vtables and singleton
extern oop s72_turtle_vtable;
extern oop s72_canvas_vtable;
extern oop s72_turtle_singleton;

// Pure libid turtle methods (variadic signatures like other S72 methods)
oop s72_turtle_forward_(oop closure, oop state, oop self, ...);
oop s72_turtle_turn_(oop closure, oop state, oop self, ...);
oop s72_turtle_penUp(oop closure, oop state, oop self);
oop s72_turtle_penDown(oop closure, oop state, oop self);
oop s72_turtle_clear(oop closure, oop state, oop self);
oop s72_turtle_show(oop closure, oop state, oop self);
oop s72_turtle_position(oop closure, oop state, oop self);
oop s72_turtle_heading_method(oop closure, oop state, oop self);

// Canvas methods
oop s72_canvas_clear(oop closure, oop state, oop self);
oop s72_canvas_drawLine_from_to_(oop closure, oop state, oop self, ...);
oop s72_canvas_show(oop closure, oop state, oop self);

// Object creation functions
oop s72_turtle_new(void);
oop s72_canvas_new(oop width_obj, oop height_obj);

// Initialization
void s72_turtle_init(void);

// Helper functions
double s72_turtle_deg_to_rad(double degrees);

#endif // S72_TURTLE_H
