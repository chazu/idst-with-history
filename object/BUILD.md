# Building IDST on macOS (Apple Silicon via Rosetta 2)

This guide covers building and running the IDST (Id Smalltalk) system on
modern macOS with Apple Silicon. All binaries are compiled for x86_64 and
run under Rosetta 2.

## Prerequisites

- macOS on Apple Silicon (M1/M2/M3/M4)
- Xcode Command Line Tools (`xcode-select --install`)
- Rosetta 2 (`softwareupdate --install-rosetta` if not already installed)

## Quick Start

From the `object/` directory:

    arch -x86_64 make

This builds the full two-stage self-hosting bootstrap:
- **boot** — pre-compiled boot compiler
- **stage1** — runtime + compiler built by boot
- **stage2** — runtime + compiler built by stage1 (self-hosting verification)

## Build Targets

### Full build (default)

    cd object
    arch -x86_64 make

Produces `stage2/idc`, `stage2/idc1`, `stage2/st80.so`, `stage2/idst.so`.

### Boot compiler only

    cd object/boot
    arch -x86_64 make

### Stage 1 only

    cd object
    arch -x86_64 make stage1/idc

### Stage 3 (triple bootstrap verification)

    cd object
    arch -x86_64 make stage3

### Reconfigure after changing .in templates

    cd object
    rm -f .config-stamp
    ./boot/configure --target=x86_64-apple-darwin23.6.0 \
        boot/Makefile id/Makefile idc/Makefile idc/idc st80/Makefile
    touch .config-stamp

## Cleaning

### Clean everything (stages + boot + GC)

    cd object
    arch -x86_64 make clean

### Remove stage directories only

    cd object
    rm -rf stage1 stage2 stage3

### Clean boot directory only

    cd object/boot
    rm -f *.o *.so idc1 gc.a

### Clean a single example

    cd object/examples/hw
    rm -f hw

## Compiling and Running Examples

Examples live in `object/examples/`. Each contains a `.st` source file.

### General pattern

From an example directory:

    # Compile
    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k EXAMPLE.st

    # Run
    arch -x86_64 env DYLD_LIBRARY_PATH=../../stage2 IDC_LIBDIR=../../stage2 ./EXAMPLE

The flags:
- `-B../../stage2/` — where idc finds its runtime (libid.o, .so files, gc_stubs.o)
- `-I../../st80` — where to find Smalltalk library sources
- `-g` — include debug info
- `-k` — keep intermediate C files (useful for debugging)
- `DYLD_LIBRARY_PATH` — tells the dynamic loader where to find .so bundles at runtime
- `IDC_LIBDIR` — tells libid where to search for import libraries

### Hello World

    cd object/examples/hw

    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k hw.st
    arch -x86_64 env DYLD_LIBRARY_PATH=../../stage2 IDC_LIBDIR=../../stage2 ./hw

Output:

    Hello, world

### Echo

    cd object/examples/echo

    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k echo.st
    arch -x86_64 env DYLD_LIBRARY_PATH=../../stage2 IDC_LIBDIR=../../stage2 ./echo hello world

Output:

    hello world

### AVL Tree (sorted collection)

    cd object/examples/avl

    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k avl.st
    arch -x86_64 env DYLD_LIBRARY_PATH=../../stage2 IDC_LIBDIR=../../stage2 ./avl

### Dispatch (selector-based virtual dispatch)

    cd object/examples/dispatch

    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k main.st
    arch -x86_64 env DYLD_LIBRARY_PATH=../../stage2 IDC_LIBDIR=../../stage2 ./main

### Typename (string to prototype lookup)

    cd object/examples/typename

    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k typename.st
    arch -x86_64 env DYLD_LIBRARY_PATH=../../stage2 IDC_LIBDIR=../../stage2 ./typename

### Compiling a shared library

    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k -s mylib.st -o mylib.so

### Compiling an object file (no linking)

    arch -x86_64 ../../stage2/idc -B../../stage2/ -I../../st80 -g -k -c myfile.st -o myfile.o

## Shell Helper

To avoid typing the full paths each time, you can set up a shell alias:

    # Add to ~/.bashrc or ~/.zshrc
    export IDST_ROOT="$HOME/dev/c/idst-with-history/object"
    alias idc="arch -x86_64 $IDST_ROOT/stage2/idc -B$IDST_ROOT/stage2/ -I$IDST_ROOT/st80"
    alias idrun="arch -x86_64 env DYLD_LIBRARY_PATH=$IDST_ROOT/stage2 IDC_LIBDIR=$IDST_ROOT/stage2"

Then:

    idc -g -k hw.st
    idrun ./hw

## idc Compiler Options

    -B prefix   Prepend prefix to compiler runtime files
    -c          Generate an object file (.o) instead of executable
    -g          Generate debugging information
    -I dir      Search dir for Pepsi import/include files
    -J dir      Search dir for C header files
    -k          Keep intermediate C files
    -n          Dry run (print commands without executing)
    -o file     Place output in file
    -O          Optimise
    -s          Generate a shared library (.so)
    -v          Verbose (print compilation commands)
    -Wi,opt     Pass opt to the Id compiler (idc1)
    -Wc,opt     Pass opt to the C compiler
    -Wl,opt     Pass opt to the linker

## Architecture Notes

### Why x86_64 / Rosetta 2?

The Id object model uses a variadic function pointer type for method dispatch:

    typedef oop (*_imp_t)(struct __send *_send, ...);

Actual methods have fixed-arity signatures. On x86_64, both variadic and
fixed-arity functions receive integer arguments in the same registers
(rdi, rsi, rdx, rcx, r8, r9), so the mismatch is invisible. On ARM64,
variadic arguments go on the stack while fixed-arity arguments go in
registers x0-x7, causing a fatal calling convention mismatch.

### Why no garbage collector?

Boehm GC 7.0 (vendored in `gc-7.0/`) segfaults under Rosetta 2 due to
memory layout incompatibilities. The GC has been replaced with stub
functions that use plain `malloc`/`calloc`. This means memory is never
reclaimed, which is fine for compilation and short-lived programs but
would be a problem for long-running applications.

The stubs live in `boot/src/gc_stubs.c` and libid.c is compiled with
`-DNO_GC` to use `calloc` instead of `GC_malloc` internally.
