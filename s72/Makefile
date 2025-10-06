# S72 Interpreter Makefile
# Integrates with enhanced libid runtime

CC = gcc
CFLAGS = -std=c11 -O2 -g -Wall -Wextra -Wno-unused-parameter -DNO_GC=1
INCLUDES = -I../object/stage2/include -I../object/id -Isrc
LDFLAGS = -L../object/boot
LIBS = -lm

# Enhanced libid object file
LIBID_OBJ = ../object/stage2/libid.o

# Source files for M0
SOURCES = src/main.c \
          src/reader.c \
          src/ast.c \
          src/eval.c \
          src/object.c \
          src/types/number.c \
          src/types/string.c \
          src/types/symbol.c

OBJECTS = $(SOURCES:.c=.o)
TARGET = s72

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS) $(LIBID_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Ensure libid is built
$(LIBID_OBJ):
	@echo "Building enhanced libid..."
	@cd ../object && $(MAKE) stage2/libid.o

# Test target (M0 basic tests)
test: $(TARGET)
	@echo "Running M0 tests..."
	@echo "42" | ./$(TARGET)
	@echo '"hello world"' | ./$(TARGET)
	@echo "'symbol" | ./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Clean everything including libid
distclean: clean
	@cd ../object && $(MAKE) clean

# Development helpers
debug: CFLAGS += -DDEBUG_ALL=1 -O0
debug: $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all test clean distclean debug install

# Dependencies (will be auto-generated later)
src/main.o: src/main.c src/s72.h
src/reader.o: src/reader.c src/reader.h src/ast.h
src/ast.o: src/ast.c src/ast.h
src/eval.o: src/eval.c src/eval.h src/ast.h src/object.h
src/object.o: src/object.c src/object.h
src/types/number.o: src/types/number.c src/types/number.h src/object.h
src/types/string.o: src/types/string.c src/types/string.h src/object.h
src/types/symbol.o: src/types/symbol.c src/types/symbol.h src/object.h
