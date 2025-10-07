#ifndef S72_TURTLE_H
#define S72_TURTLE_H

#include "../s72.h"

// Turtle state structure
typedef struct {
    double x, y;        // Current position
    double heading;     // Current heading in degrees (0 = east, 90 = north)
    int pen_down;       // Pen state: 1 = down (drawing), 0 = up (not drawing)
    int canvas_width;   // Canvas dimensions
    int canvas_height;
    char **canvas;      // 2D array for ASCII canvas
} S72Turtle;

// Canvas constants
#define TURTLE_CANVAS_WIDTH 80
#define TURTLE_CANVAS_HEIGHT 24
#define TURTLE_CANVAS_CHAR '*'
#define TURTLE_EMPTY_CHAR ' '

// Global turtle instance
extern oop s72_turtle_singleton;
extern oop s72_turtle_vtable;

// Turtle methods
oop s72_turtle_forward_(oop closure, oop state, oop receiver, ...);
oop s72_turtle_turn_(oop closure, oop state, oop receiver, ...);
oop s72_turtle_penUp(oop closure, oop state, oop receiver);
oop s72_turtle_penDown(oop closure, oop state, oop receiver);
oop s72_turtle_goto_(oop closure, oop state, oop receiver, ...);
oop s72_turtle_clear(oop closure, oop state, oop receiver);
oop s72_turtle_show(oop closure, oop state, oop receiver);
oop s72_turtle_position(oop closure, oop state, oop receiver);
oop s72_turtle_heading(oop closure, oop state, oop receiver);

// Turtle initialization and utilities
void s72_turtle_init(void);
void s72_turtle_install_methods(void);
oop s72_turtle_new(void);
void s72_turtle_clear_canvas(S72Turtle *turtle);
void s72_turtle_draw_line(S72Turtle *turtle, double x1, double y1, double x2, double y2);
void s72_turtle_print_canvas(S72Turtle *turtle);

// Helper functions
double s72_turtle_deg_to_rad(double degrees);
double s72_turtle_rad_to_deg(double radians);

#endif // S72_TURTLE_H
