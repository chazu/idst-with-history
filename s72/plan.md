# S72 Interpreter Implementation Plan

## Overview

This plan outlines the implementation of a Smalltalk-72-like interpreter/REPL using the enhanced libid runtime from `object/id/libid_enhanced.c`. The implementation follows the milestone-driven approach (M0-M5) specified in the requirements.

## Architecture Overview

### Core Components
- **Reader/Parser**: Tokenizes and parses S72-lite syntax into AST nodes
- **Evaluator**: Interprets AST nodes using message-passing semantics
- **Object System**: Prototype-based objects with method dictionaries
- **Runtime**: Built on enhanced libid with ARM64 compatibility
- **REPL**: Interactive read-eval-print loop with image persistence

### Integration with Enhanced libid
- Uses `libid_enhanced.c` (production version) instead of bootstrap libid
- Leverages enhanced dispatch macros (`_sendv0`, `_sendv1`, etc.)
- Utilizes ARM64-compatible memory management and GC integration
- Employs modern C11 compilation standards

## File Structure

```
s72/
├── src/
│   ├── main.c           # REPL, initialization, image loading
│   ├── reader.c/h       # Tokenizer and parser
│   ├── ast.h            # AST node definitions
│   ├── eval.c/h         # Evaluator and message dispatch
│   ├── object.c/h       # Object model and method dictionaries
│   ├── types/
│   │   ├── number.c/h   # Number type implementation
│   │   ├── boolean.c/h  # Boolean singletons (true/false/nil)
│   │   ├── string.c/h   # String type implementation
│   │   ├── symbol.c/h   # Interned symbol table
│   │   ├── list.c/h     # Linked list implementation
│   │   ├── array.c/h    # Fixed-size array implementation
│   │   ├── block.c/h    # Block/closure implementation
│   │   └── transcript.c/h # I/O singleton
│   ├── builtins.c/h     # Native method installation
│   ├── env.c/h          # Environment/scope management
│   └── snapshot.c/h     # Image serialization/deserialization
├── lang/
│   └── init.s72         # Standard library in S72
├── tests/
│   ├── numbers.s72      # Arithmetic tests
│   ├── booleans.s72     # Boolean logic tests
│   ├── blocks.s72       # Block/closure tests
│   ├── lists.s72        # List manipulation tests
│   ├── turtle.s72       # Turtle graphics demo
│   └── *.golden         # Expected output files
├── Makefile             # Build configuration
└── README.md            # Implementation specification
```

## Phase-by-Phase Implementation

### Phase M0: Skeleton (Foundation)
**Goal**: Basic REPL with number/string literals and symbol interning

**Components**:
1. **Build System Integration**
   - Create `s72/Makefile` that links with enhanced libid
   - Configure ARM64-compatible compilation flags
   - Set up dependency on `object/id/libid_enhanced.o`

2. **Core Infrastructure**
   - Initialize libid runtime in `main.c`
   - Set up symbol interning table using `_libid_intern()`
   - Create basic AST node types in `ast.h`
   - Implement minimal tokenizer in `reader.c`

3. **Basic Types**
   - Number boxing/unboxing using libid allocation
   - String creation and management
   - Symbol interning and lookup

4. **REPL Loop**
   - Read input line
   - Parse to AST (atoms, numbers, strings only)
   - Evaluate literals
   - Print results

**Deliverables**:
- `s72` executable that can evaluate: `42`, `"hello"`, `'symbol`
- Basic error handling and memory management

### Phase M1: Objects & Message Sending
**Goal**: Object model with method dictionaries and message dispatch

**Components**:
1. **Object Model** (`object.c/h`)
   - Object structure with parent pointer and method dictionary
   - Method structure (selector → C function or Block)
   - Prototype chain traversal for method lookup

2. **Message Dispatch** (`eval.c`)
   - `send(receiver, selector, argc, argv)` function
   - Integration with libid's enhanced dispatch macros
   - Method resolution through prototype chain
   - `doesNotUnderstand:` handling

3. **Number Arithmetic**
   - Install native methods for `+`, `-`, `*`, `/`, `=`
   - Type checking and error handling
   - Integration with libid's object allocation

4. **List Evaluation**
   - Parse `(receiver selector arg...)` syntax
   - Evaluate receiver and arguments
   - Perform message send

**Deliverables**:
- `(3 + 4)` → `7`
- `(10 * 2)` → `20`
- Basic arithmetic operations working

### Phase M2: Booleans & Control Flow
**Goal**: Boolean singletons and conditional execution with blocks

**Components**:
1. **Boolean Singletons** (`boolean.c/h`)
   - Create global `true`, `false`, `nil` objects
   - Install methods: `ifTrue:`, `ifFalse:`, `ifTrue:ifFalse:`
   - Boolean logic: `and:`, `or:`, `not`

2. **Block Implementation** (`block.c/h`)
   - Block structure: AST nodes + captured environment
   - Environment capture by reference
   - `value`, `value:`, `value:value:` methods

3. **Environment Management** (`env.c/h`)
   - Environment structure: symbol→value map + parent link
   - Variable lookup through environment chain
   - Block parameter binding

4. **Control Flow**
   - Conditional evaluation with blocks
   - Block execution on `value` messages

**Deliverables**:
- `(true ifTrue: [ (Transcript print: "yes") ] ifFalse: [ (Transcript print: "no") ])`
- Block creation and execution
- Conditional logic working

### Phase M3: Collections & I/O
**Goal**: List/Array collections and Transcript output

**Components**:
1. **List Implementation** (`list.c/h`)
   - Cons cell structure
   - Methods: `isEmpty`, `first`, `rest`, `cons:`, `length`, `each:`
   - List literal syntax: `'(a b c)`

2. **Array Implementation** (`array.c/h`)
   - Fixed-size array structure
   - Methods: `size`, `at:`, `at:put:`, `each:`
   - Array creation and access

3. **Transcript Singleton** (`transcript.c/h`)
   - Global I/O object
   - Methods: `print:`, `cr`, `show:`
   - Output formatting and buffering

4. **Collection Iteration**
   - `each:` methods taking blocks
   - Iterator protocol implementation

**Deliverables**:
- `(List cons: 1 cons: 2 cons: 3)`
- `(Transcript print: "Hello World") (Transcript cr)`
- Collection manipulation and output

### Phase M4: Image Persistence & Standard Library
**Goal**: Startup file loading and object graph serialization

**Components**:
1. **Startup File Loader** (`main.c`)
   - Parse and evaluate `lang/init.s72`
   - Define standard library functions in S72
   - Bootstrap essential definitions

2. **Image Serialization** (`snapshot.c/h`)
   - Serialize object graph to binary format
   - Handle object references and cycles
   - Restore object graph on startup

3. **Standard Library** (`lang/init.s72`)
   - Define convenience functions: `def`, `let`, `set`
   - Implement `times:`, `whileTrue:`, `map:`
   - Add syntactic sugar and utilities

4. **Enhanced REPL**
   - Command-line options for image loading/saving
   - Persistent session state
   - Error recovery and debugging

**Deliverables**:
- `lang/init.s72` loaded automatically
- Image save/restore functionality
- Rich standard library available

### Phase M5: Demo & Polish
**Goal**: Turtle graphics demo and production readiness

**Components**:
1. **Turtle Graphics** (`types/turtle.c/h`)
   - Turtle state: position, heading, pen state
   - Methods: `forward:`, `turn:`, `penUp`, `penDown`, `goto:`, `clear`
   - ASCII canvas rendering (initially)

2. **Demo Programs**
   - Spiral drawing example
   - Interactive turtle commands
   - Showcase language capabilities

3. **Testing Framework**
   - Golden file testing system
   - Automated test runner
   - CI integration (< 2s build+test)

4. **Documentation & Polish**
   - Usage examples and tutorials
   - Performance optimization
   - Error message improvements

**Deliverables**:
- Working turtle graphics demo
- Comprehensive test suite
- Production-ready interpreter

## Build System Integration

### Makefile Structure
```makefile
# s72/Makefile
CC = gcc
CFLAGS = -std=c11 -O2 -g -Wall -Wextra -I../object/id -I../object/stage2/include
LDFLAGS = -L../object/stage2 -lgc

LIBID_OBJ = ../object/stage2/libid.o
SOURCES = src/main.c src/reader.c src/eval.c src/object.c \
          src/types/number.c src/types/boolean.c src/types/string.c \
          src/types/symbol.c src/types/list.c src/types/block.c \
          src/types/transcript.c src/builtins.c src/env.c src/snapshot.c

s72: $(SOURCES) $(LIBID_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: s72
	./run_tests.sh

clean:
	rm -f s72 *.o

.PHONY: test clean
```

### Dependencies
- Enhanced libid runtime (`libid_enhanced.o`)
- Boehm GC library (via libid)
- Standard C11 library
- POSIX system calls for I/O

## Testing Strategy

### Golden File Testing
- Each test file (`tests/*.s72`) has corresponding `.golden` output
- Test runner executes S72 scripts and compares output
- Automated regression testing for all milestones

### Test Categories
1. **Unit Tests**: Individual type and operation testing
2. **Integration Tests**: Cross-component functionality
3. **REPL Tests**: Interactive session simulation
4. **Performance Tests**: Memory usage and execution speed
5. **Demo Tests**: End-to-end application scenarios

### CI Requirements
- Complete build + test cycle in < 2 seconds
- ARM64 and x86_64 compatibility testing
- Memory leak detection with Valgrind
- Static analysis with clang-analyzer

## Implementation Dependencies

### Critical Path
1. M0 depends on: Enhanced libid integration, basic AST/parser
2. M1 depends on: M0 + Object model + Message dispatch
3. M2 depends on: M1 + Block implementation + Environment management
4. M3 depends on: M2 + Collection types + I/O system
5. M4 depends on: M3 + Serialization + Standard library
6. M5 depends on: M4 + Demo implementation + Testing framework

### Risk Mitigation
- **libid Integration**: Thoroughly test enhanced libid interface early
- **ARM64 Compatibility**: Validate on target architecture throughout
- **Memory Management**: Integrate GC properly from the start
- **Performance**: Profile and optimize at each milestone
- **Testing**: Implement golden file testing from M0

## Success Criteria

### Functional Requirements
- ✅ Complete S72-lite syntax support
- ✅ Message-passing object model
- ✅ Block closures with proper scoping
- ✅ Collection types (List, Array)
- ✅ I/O through Transcript
- ✅ Image persistence
- ✅ Interactive REPL

### Non-Functional Requirements
- ✅ ARM64 compatibility
- ✅ < 2s build+test cycle
- ✅ Memory-safe operation
- ✅ Extensible architecture
- ✅ Clear error messages
- ✅ Comprehensive documentation

This implementation plan provides a structured approach to building the S72 interpreter while leveraging the enhanced libid runtime and maintaining compatibility with modern systems.

## Implementation Details

### Key libid Integration Points

#### Enhanced Dispatch Macros
The enhanced libid provides non-variadic dispatch macros that are ARM64-compatible:
```c
// Use these instead of variadic _sendv
_sendv0(selector, receiver)                    // 0 args
_sendv1(selector, receiver, arg1)              // 1 arg
_sendv2(selector, receiver, arg1, arg2)        // 2 args
// etc.
```

#### Memory Management
```c
// Object allocation through libid
oop obj = _libid_alloc(type_vtable, sizeof(MyObject));

// Garbage collection integration
_libid.gc_addRoots(&global_var_start, &global_var_end);
```

#### Symbol Interning
```c
// Intern selectors once at startup
static oop SEL_PLUS, SEL_PRINT, SEL_VALUE;

void intern_selectors(void) {
    SEL_PLUS = _libid_intern("+");
    SEL_PRINT = _libid_intern("print:");
    SEL_VALUE = _libid_intern("value");
}
```

### Critical Implementation Notes

1. **ARM64 Compatibility**: The enhanced libid eliminates variadic macro issues that caused problems on ARM64. Always use the numbered dispatch macros.

2. **Method Installation**: Use `_libid_method(type, selector, c_function)` to install native methods on prototype objects.

3. **Object Layout**: Objects must follow libid's vtable convention with the vtable pointer at offset -1.

4. **Error Handling**: Implement proper `doesNotUnderstand:` handling for missing methods.

5. **Environment Capture**: Blocks must capture environments by reference, not by value, to match Smalltalk semantics.

### Development Workflow

1. **Start with M0**: Get basic REPL working with literals only
2. **Incremental Testing**: Add golden file tests at each milestone
3. **Memory Validation**: Run with GC stress testing throughout
4. **Performance Monitoring**: Profile at M1, M3, and M5
5. **ARM64 Validation**: Test on target architecture at each phase

### Next Steps

1. Create the basic file structure in `s72/src/`
2. Implement M0 skeleton with enhanced libid integration
3. Set up build system and basic testing framework
4. Begin iterative development following the milestone plan
