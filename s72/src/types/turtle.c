#include "turtle.h"
#include "../object.h"
#include "number.h"
#include "transcript.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// External references
extern struct __libid *_libid;
extern oop s72_object_vtable;
extern S72Value S72_NIL;

// Global turtle instance and vtable
oop s72_turtle_singleton = 0;
extern oop s72_turtle_vtable;

// Math constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper functions
double s72_turtle_deg_to_rad(double degrees) {
    return degrees * M_PI / 180.0;
}

double s72_turtle_rad_to_deg(double radians) {
    return radians * 180.0 / M_PI;
}

// Canvas management
void s72_turtle_clear_canvas(S72Turtle *turtle) {
    for (int y = 0; y < turtle->canvas_height; y++) {
        for (int x = 0; x < turtle->canvas_width; x++) {
            turtle->canvas[y][x] = TURTLE_EMPTY_CHAR;
        }
        turtle->canvas[y][turtle->canvas_width] = '\0';
    }
}

void s72_turtle_draw_line(S72Turtle *turtle, double x1, double y1, double x2, double y2) {
    printf("DEBUG: draw_line called with (%.2f,%.2f) to (%.2f,%.2f)\n", x1, y1, x2, y2);
    printf("DEBUG: pen_down=%d\n", turtle->pen_down);

    if (!turtle->pen_down) return;

    // Simple line drawing using Bresenham-like algorithm
    int ix1 = (int)(x1 + turtle->canvas_width / 2);
    int iy1 = (int)(turtle->canvas_height / 2 - y1);
    int ix2 = (int)(x2 + turtle->canvas_width / 2);
    int iy2 = (int)(turtle->canvas_height / 2 - y2);

    printf("DEBUG: canvas size: %dx%d\n", turtle->canvas_width, turtle->canvas_height);
    printf("DEBUG: screen coords: (%d,%d) to (%d,%d)\n", ix1, iy1, ix2, iy2);
    
    // Clamp to canvas bounds
    if (ix1 < 0) ix1 = 0;
    if (ix1 >= turtle->canvas_width) ix1 = turtle->canvas_width - 1;
    if (iy1 < 0) iy1 = 0;
    if (iy1 >= turtle->canvas_height) iy1 = turtle->canvas_height - 1;
    if (ix2 < 0) ix2 = 0;
    if (ix2 >= turtle->canvas_width) ix2 = turtle->canvas_width - 1;
    if (iy2 < 0) iy2 = 0;
    if (iy2 >= turtle->canvas_height) iy2 = turtle->canvas_height - 1;
    
    // Draw line
    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = (ix1 < ix2) ? 1 : -1;
    int sy = (iy1 < iy2) ? 1 : -1;
    int err = dx - dy;
    
    int x = ix1, y = iy1;
    while (1) {
        if (x >= 0 && x < turtle->canvas_width && y >= 0 && y < turtle->canvas_height) {
            turtle->canvas[y][x] = TURTLE_CANVAS_CHAR;
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
}

void s72_turtle_print_canvas(S72Turtle *turtle) {
    for (int y = 0; y < turtle->canvas_height; y++) {
        printf("%s\n", turtle->canvas[y]);
    }
}

// Turtle object creation
oop s72_turtle_new(void) {
    if (!s72_turtle_vtable) {
        fprintf(stderr, "Error: Turtle vtable not initialized\n");
        return S72_NIL.obj;
    }

    oop turtle_obj = S72_ALLOC(s72_turtle_vtable, sizeof(S72Turtle));
    if (!turtle_obj) {
        fprintf(stderr, "Error: Failed to allocate turtle object\n");
        return S72_NIL.obj;
    }
    
    S72Turtle *turtle = (S72Turtle *)turtle_obj;
    
    // Initialize turtle state
    turtle->x = 0.0;
    turtle->y = 0.0;
    turtle->heading = 0.0;  // Start facing east
    turtle->pen_down = 1;   // Start with pen down
    turtle->canvas_width = TURTLE_CANVAS_WIDTH;
    turtle->canvas_height = TURTLE_CANVAS_HEIGHT;
    
    // Allocate canvas
    turtle->canvas = malloc(turtle->canvas_height * sizeof(char *));
    if (!turtle->canvas) {
        fprintf(stderr, "Error: Failed to allocate canvas rows\n");
        return S72_NIL.obj;
    }
    
    for (int y = 0; y < turtle->canvas_height; y++) {
        turtle->canvas[y] = malloc((turtle->canvas_width + 1) * sizeof(char));
        if (!turtle->canvas[y]) {
            fprintf(stderr, "Error: Failed to allocate canvas row %d\n", y);
            return S72_NIL.obj;
        }
    }
    
    s72_turtle_clear_canvas(turtle);
    
    return turtle_obj;
}

// Turtle method implementations
oop s72_turtle_forward_(oop closure, oop state, oop receiver, ...) {
    printf("DEBUG: turtle forward method called\n");
    va_list args;
    va_start(args, receiver);
    oop distance_obj = va_arg(args, oop);
    va_end(args);

    printf("DEBUG: turtle forward got distance_obj=%p\n", (void*)distance_obj);

    S72Value distance_val = { .obj = distance_obj };
    printf("DEBUG: turtle forward about to check if number\n");
    if (!s72_is_number(distance_val)) {
        fprintf(stderr, "Error: forward: expects a number\n");
        return S72_NIL.obj;
    }
    printf("DEBUG: turtle forward confirmed it's a number\n");

    S72Turtle *turtle = (S72Turtle *)receiver;
    double distance = s72_number_value(distance_val);

    printf("DEBUG: forward: distance=%.2f, heading=%.2f\n", distance, turtle->heading);

    // Calculate new position
    double old_x = turtle->x;
    double old_y = turtle->y;
    double rad = s72_turtle_deg_to_rad(turtle->heading);

    printf("DEBUG: forward: old_pos=(%.2f, %.2f), rad=%.4f\n", old_x, old_y, rad);

    turtle->x += distance * cos(rad);
    turtle->y += distance * sin(rad);

    printf("DEBUG: forward: new_pos=(%.2f, %.2f)\n", turtle->x, turtle->y);

    // Draw line if pen is down
    printf("DEBUG: forward: about to call draw_line\n");
    s72_turtle_draw_line(turtle, old_x, old_y, turtle->x, turtle->y);
    printf("DEBUG: forward: draw_line completed\n");

    return receiver;
}

oop s72_turtle_turn_(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop angle_obj = va_arg(args, oop);
    va_end(args);

    S72Value angle_val = { .obj = angle_obj };
    if (!s72_is_number(angle_val)) {
        fprintf(stderr, "Error: turn: expects a number\n");
        return S72_NIL.obj;
    }

    S72Turtle *turtle = (S72Turtle *)receiver;
    double angle = s72_number_value(angle_val);
    
    turtle->heading += angle;
    
    // Normalize heading to 0-360 range
    while (turtle->heading < 0) turtle->heading += 360;
    while (turtle->heading >= 360) turtle->heading -= 360;

    return receiver;
}

oop s72_turtle_penUp(oop closure, oop state, oop receiver) {
    (void)closure; (void)state; // Unused
    S72Turtle *turtle = (S72Turtle *)receiver;
    turtle->pen_down = 0;
    return receiver;
}

oop s72_turtle_penDown(oop closure, oop state, oop receiver) {
    (void)closure; (void)state; // Unused
    S72Turtle *turtle = (S72Turtle *)receiver;
    turtle->pen_down = 1;
    return receiver;
}

oop s72_turtle_goto_(oop closure, oop state, oop receiver, ...) {
    va_list args;
    va_start(args, receiver);
    oop x_obj = va_arg(args, oop);
    oop y_obj = va_arg(args, oop);
    va_end(args);

    S72Value x_val = { .obj = x_obj };
    S72Value y_val = { .obj = y_obj };
    if (!s72_is_number(x_val) || !s72_is_number(y_val)) {
        fprintf(stderr, "Error: goto: expects two numbers\n");
        return S72_NIL.obj;
    }

    S72Turtle *turtle = (S72Turtle *)receiver;
    double old_x = turtle->x;
    double old_y = turtle->y;

    turtle->x = s72_number_value(x_val);
    turtle->y = s72_number_value(y_val);
    
    // Draw line if pen is down
    s72_turtle_draw_line(turtle, old_x, old_y, turtle->x, turtle->y);

    return receiver;
}

oop s72_turtle_clear(oop closure, oop state, oop receiver) {
    (void)closure; (void)state; // Unused
    S72Turtle *turtle = (S72Turtle *)receiver;
    s72_turtle_clear_canvas(turtle);
    return receiver;
}

oop s72_turtle_show(oop closure, oop state, oop receiver) {
    (void)closure; (void)state; // Unused
    S72Turtle *turtle = (S72Turtle *)receiver;
    s72_turtle_print_canvas(turtle);
    return receiver;
}

oop s72_turtle_position(oop closure, oop state, oop receiver) {
    (void)closure; (void)state; // Unused
    S72Turtle *turtle = (S72Turtle *)receiver;
    printf("Position: (%.2f, %.2f)\n", turtle->x, turtle->y);
    return receiver;
}

oop s72_turtle_heading(oop closure, oop state, oop receiver) {
    (void)closure; (void)state; // Unused
    S72Turtle *turtle = (S72Turtle *)receiver;
    printf("Heading: %.2f degrees\n", turtle->heading);
    return receiver;
}

// Installation functions
void s72_turtle_install_methods(void) {
    if (!s72_turtle_vtable) {
        fprintf(stderr, "Error: Cannot install turtle methods - vtable not initialized\n");
        return;
    }

    // Install turtle methods using S72_METHOD macro
    oop sel_forward = _libid->intern("forward:");
    oop sel_turn = _libid->intern("turn:");
    oop sel_penUp = _libid->intern("penUp");
    oop sel_penDown = _libid->intern("penDown");
    oop sel_goto = _libid->intern("goto:");
    oop sel_clear = _libid->intern("clear");
    oop sel_show = _libid->intern("show");
    oop sel_position = _libid->intern("position");
    oop sel_heading = _libid->intern("heading");

    S72_METHOD(s72_turtle_vtable, sel_forward, s72_turtle_forward_);
    S72_METHOD(s72_turtle_vtable, sel_turn, s72_turtle_turn_);
    S72_METHOD(s72_turtle_vtable, sel_penUp, s72_turtle_penUp);
    S72_METHOD(s72_turtle_vtable, sel_penDown, s72_turtle_penDown);
    S72_METHOD(s72_turtle_vtable, sel_goto, s72_turtle_goto_);
    S72_METHOD(s72_turtle_vtable, sel_clear, s72_turtle_clear);
    S72_METHOD(s72_turtle_vtable, sel_show, s72_turtle_show);
    S72_METHOD(s72_turtle_vtable, sel_position, s72_turtle_position);
    S72_METHOD(s72_turtle_vtable, sel_heading, s72_turtle_heading);

    // Turtle methods installed successfully
}

void s72_turtle_init(void) {
    // Create turtle vtable (should already be created in main.c)
    if (!s72_turtle_vtable) {
        fprintf(stderr, "Error: Turtle vtable not initialized\n");
        return;
    }

    // Install methods
    s72_turtle_install_methods();

    // Create singleton turtle instance
    s72_turtle_singleton = s72_turtle_new();
    if (!s72_turtle_singleton) {
        fprintf(stderr, "Error: Failed to create turtle singleton\n");
        return;
    }

    // Turtle system initialized successfully
}
