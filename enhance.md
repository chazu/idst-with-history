# Enhancement Plan: Eliminate Variadic Macros from Id Runtime System

## Executive Summary

This plan details the complete enhancement of the production version of `object/id/libid.c` to eliminate all variadic macros while preserving exact semantics and achieving Apple Silicon (ARM64) compatibility. The enhancement addresses critical ARM64 ABI compliance issues that prevent the current system from running on Apple Silicon.

## Current Variadic Macro Inventory

### 1. Runtime Library (`object/id/libid.c`)
- **`dprintf(fmt, args...)`**: 25+ usage instances for debug output
- **`fatal(const char *fmt, ...)`**: Variadic function with 3 usage instances
- **`_sendv(MSG, N, RCV, ARG...)`**: 12 usage instances in runtime

### 2. Code Generators
- **`CCodeGenerator.st`**: Generates `_sendv`, `_superv` macros
- **`CManagedCodeGenerator.st`**: Generates `_send`, `_super` macros
- **Generated C files**: All contain variadic macro definitions

### 3. Generated Code Pattern Analysis
From usage analysis:
- **0 arguments**: `_sendv(selector, 1, receiver)` - 4 instances
- **1 argument**: `_sendv(selector, 2, receiver, arg1)` - 5 instances  
- **2 arguments**: `_sendv(selector, 3, receiver, arg1, arg2)` - 2 instances
- **3 arguments**: `_sendv(selector, 4, receiver, arg1, arg2, arg3)` - 1 instance

## Enhancement Strategy

### Phase 1: Create Non-Variadic Infrastructure

#### 1.1 New Header File: `object/id/libid_enhanced.h`

```c
#ifndef __LIBID_ENHANCED_H
#define __LIBID_ENHANCED_H

/* Non-variadic debug printf macros */
#if DEBUG_ALL
# define dprintf0(fmt)                    fprintf(stderr, fmt)
# define dprintf1(fmt, a1)                fprintf(stderr, fmt, a1)
# define dprintf2(fmt, a1, a2)            fprintf(stderr, fmt, a1, a2)
# define dprintf3(fmt, a1, a2, a3)        fprintf(stderr, fmt, a1, a2, a3)
# define dprintf4(fmt, a1, a2, a3, a4)    fprintf(stderr, fmt, a1, a2, a3, a4)
# define dprintf5(fmt, a1, a2, a3, a4, a5) fprintf(stderr, fmt, a1, a2, a3, a4, a5)
#else
# define dprintf0(fmt)
# define dprintf1(fmt, a1)
# define dprintf2(fmt, a1, a2)
# define dprintf3(fmt, a1, a2, a3)
# define dprintf4(fmt, a1, a2, a3, a4)
# define dprintf5(fmt, a1, a2, a3, a4, a5)
#endif

/* Non-variadic method dispatch macros */
#define _sendv0(MSG, RCV) ({						\
  struct __send _s= { (MSG), 1, (RCV) };				\
  ((_imp_t)(_libid_bindv(&_s)))(&_s, _s.receiver, _s.receiver);		\
})

#define _sendv1(MSG, RCV, A1) ({					\
  struct __send _s= { (MSG), 2, (RCV) };				\
  ((_imp_t)(_libid_bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1);	\
})

#define _sendv2(MSG, RCV, A1, A2) ({					\
  struct __send _s= { (MSG), 3, (RCV) };				\
  ((_imp_t)(_libid_bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1, A2);	\
})

#define _sendv3(MSG, RCV, A1, A2, A3) ({				\
  struct __send _s= { (MSG), 4, (RCV) };				\
  ((_imp_t)(_libid_bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1, A2, A3); \
})

#define _sendv4(MSG, RCV, A1, A2, A3, A4) ({				\
  struct __send _s= { (MSG), 5, (RCV) };				\
  ((_imp_t)(_libid_bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1, A2, A3, A4); \
})

/* Non-variadic super dispatch macros */
#define _superv0(TYP, MSG, RCV) ({					\
  struct __send _s= { (MSG), 1, (TYP) };				\
  _imp_t _imp= _libid->bindv(&_s);					\
  _s.receiver= (RCV);							\
  _imp(&_s, _s.receiver, _s.receiver);					\
})

#define _superv1(TYP, MSG, RCV, A1) ({					\
  struct __send _s= { (MSG), 2, (TYP) };				\
  _imp_t _imp= _libid->bindv(&_s);					\
  _s.receiver= (RCV);							\
  _imp(&_s, _s.receiver, _s.receiver, A1);				\
})

#define _superv2(TYP, MSG, RCV, A1, A2) ({				\
  struct __send _s= { (MSG), 3, (TYP) };				\
  _imp_t _imp= _libid->bindv(&_s);					\
  _s.receiver= (RCV);							\
  _imp(&_s, _s.receiver, _s.receiver, A1, A2);				\
})

#define _superv3(TYP, MSG, RCV, A1, A2, A3) ({				\
  struct __send _s= { (MSG), 4, (TYP) };				\
  _imp_t _imp= _libid->bindv(&_s);					\
  _s.receiver= (RCV);							\
  _imp(&_s, _s.receiver, _s.receiver, A1, A2, A3);			\
})

/* Managed code macros (for CManagedCodeGenerator) */
#define _send0(MSG, RCV) ({						\
    register oop _r= (RCV);						\
    struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\
    (_c->method)((oop)_c, _r, _r);					\
})

#define _send1(MSG, RCV, A1) ({						\
    register oop _r= (RCV);						\
    struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\
    (_c->method)((oop)_c, _r, _r, A1);					\
})

#define _send2(MSG, RCV, A1, A2) ({					\
    register oop _r= (RCV);						\
    struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\
    (_c->method)((oop)_c, _r, _r, A1, A2);				\
})

#define _send3(MSG, RCV, A1, A2, A3) ({					\
    register oop _r= (RCV);						\
    struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\
    (_c->method)((oop)_c, _r, _r, A1, A2, A3);				\
})

#define _super0(TYP, MSG, RCV) ({					\
  register oop _r= (RCV);						\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), (TYP));	\
  (_c->method)((oop)_c, _r, _r);					\
})

#define _super1(TYP, MSG, RCV, A1) ({					\
  register oop _r= (RCV);						\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), (TYP));	\
  (_c->method)((oop)_c, _r, _r, A1);					\
})

#define _super2(TYP, MSG, RCV, A1, A2) ({				\
  register oop _r= (RCV);						\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), (TYP));	\
  (_c->method)((oop)_c, _r, _r, A1, A2);				\
})

#define _super3(TYP, MSG, RCV, A1, A2, A3) ({				\
  register oop _r= (RCV);						\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), (TYP));	\
  (_c->method)((oop)_c, _r, _r, A1, A2, A3);				\
})

#endif /* __LIBID_ENHANCED_H */
```

#### 1.2 Dynamic Dispatch Functions

Add to `object/id/libid.c`:

```c
/* Dynamic dispatch for runtime argument counting */
oop _libid_sendv_dispatch(oop selector, oop receiver, int argc, oop *argv)
{
    switch(argc) {
        case 0: return _sendv0(selector, receiver);
        case 1: return _sendv1(selector, receiver, argv[0]);
        case 2: return _sendv2(selector, receiver, argv[0], argv[1]);
        case 3: return _sendv3(selector, receiver, argv[0], argv[1], argv[2]);
        case 4: return _sendv4(selector, receiver, argv[0], argv[1], argv[2], argv[3]);
        default: 
            fatal1("Too many arguments in method call: %d", argc);
            return 0;
    }
}

oop _libid_superv_dispatch(oop type, oop selector, oop receiver, int argc, oop *argv)
{
    switch(argc) {
        case 0: return _superv0(type, selector, receiver);
        case 1: return _superv1(type, selector, receiver, argv[0]);
        case 2: return _superv2(type, selector, receiver, argv[0], argv[1]);
        case 3: return _superv3(type, selector, receiver, argv[0], argv[1], argv[2]);
        case 4: return _superv4(type, selector, receiver, argv[0], argv[1], argv[2], argv[3]);
        default: 
            fatal1("Too many arguments in super call: %d", argc);
            return 0;
    }
}
```

#### 1.3 Non-Variadic Fatal Functions

Replace the variadic `fatal` function:

```c
static void fatal0(const char *msg)
{
  fflush(stdout);
  fputs("\n", stderr);
  fputs(msg, stderr);
  fputs("\n", stderr);
  fputs(_libid_backtrace(), stderr);
  exit(1);
}

static void fatal1(const char *fmt, const char *arg1)
{
  fflush(stdout);
  fputs("\n", stderr);
  fprintf(stderr, fmt, arg1);
  fputs("\n", stderr);
  fputs(_libid_backtrace(), stderr);
  exit(1);
}

static void fatal2(const char *fmt, const char *arg1, const char *arg2)
{
  fflush(stdout);
  fputs("\n", stderr);
  fprintf(stderr, fmt, arg1, arg2);
  fputs("\n", stderr);
  fputs(_libid_backtrace(), stderr);
  exit(1);
}
```

### Phase 2: Update Runtime Library

#### 2.1 Replace All dprintf Calls

Systematic replacement in `object/id/libid.c`:

```c
// Line 197: OLD: dprintf("_vector__init_(%p, %d)\n", self, (int)size);
// NEW: dprintf2("_vector__init_(%p, %d)\n", self, (int)size);

// Line 230: OLD: dprintf("_vtable___alloc_(%p, %p, %p, %d) -> %p\n", _thunk, self, state, (int)size, object);
// NEW: dprintf5("_vtable___alloc_(%p, %p, %p, %d) -> %p\n", _thunk, self, state, (int)size, object);

// Line 231: OLD: if (!object) fatal("out of memory");
// NEW: if (!object) fatal0("out of memory");

// Line 233: OLD: dprintf("  object %p\n", object);
// NEW: dprintf1("  object %p\n", object);

// Continue for all 25+ dprintf instances...
```

#### 2.2 Replace All _sendv Calls

```c
// Line 502: OLD: return _sendv(s__intern_, 2, _selector, string);
// NEW: return _sendv1(s__intern_, _selector, string);

// Line 508: OLD: return _sendv(s__delegated_, 2, (base ? base : _object), size);
// NEW: return _sendv1(s__delegated_, (base ? base : _object), size);

// Line 513: OLD: return _sendv(s__delegated, 1, (base ? base : _object));
// NEW: return _sendv0(s__delegated, (base ? base : _object));

// Continue for all _sendv instances...
```

#### 2.3 Replace All fatal Calls

```c
// Line 231: OLD: if (!object) fatal("out of memory");
// NEW: if (!object) fatal0("out of memory");

// Line 328: OLD: if (self == self->vtable.delegate) fatal("delegation loop\n");
// NEW: if (self == self->vtable.delegate) fatal0("delegation loop\n");

// Line 490: OLD: fatal("import: %s.so: No such file or directory\n", fileName);
// NEW: fatal1("import: %s.so: No such file or directory\n", fileName);

// Line 493: OLD: fatal("%s: __id__init__: Undefined symbol\n", path);
// NEW: fatal1("%s: __id__init__: Undefined symbol\n", path);
```

### Phase 3: Update Code Generators

#### 3.1 Update CCodeGenerator.st

Replace the `sendNoCacheText` and `sendInlineCacheText` methods:

```smalltalk
CCodeGenerator sendNoCacheText
[
    ^'
#define _sendv0(MSG, RCV) ({						\\
  struct __send _s= { (MSG), 1, (RCV) };				\\
  ((_imp_t)(_libid->bindv(&_s)))(&_s, _s.receiver, _s.receiver);		\\
})

#define _sendv1(MSG, RCV, A1) ({					\\
  struct __send _s= { (MSG), 2, (RCV) };				\\
  ((_imp_t)(_libid->bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1);	\\
})

#define _sendv2(MSG, RCV, A1, A2) ({					\\
  struct __send _s= { (MSG), 3, (RCV) };				\\
  ((_imp_t)(_libid->bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1, A2);	\\
})

#define _sendv3(MSG, RCV, A1, A2, A3) ({				\\
  struct __send _s= { (MSG), 4, (RCV) };				\\
  ((_imp_t)(_libid->bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1, A2, A3); \\
})

#define _sendv4(MSG, RCV, A1, A2, A3, A4) ({				\\
  struct __send _s= { (MSG), 5, (RCV) };				\\
  ((_imp_t)(_libid->bindv(&_s)))(&_s, _s.receiver, _s.receiver, A1, A2, A3, A4); \\
})
']
```

Update the `send:to:withArguments:forValue:supered:` method:

```smalltalk
CCodeGenerator send: selector to: receiver withArguments: arguments forValue: valueFlag supered: superedType
[
    | macroName |
    self gen: '  '.
    valueFlag ifTrue: [self genLocation: receiver location; gen: '='].
    
    macroName := superedType isNil 
        ifTrue: ['_sendv', arguments size printString]
        ifFalse: ['_superv', arguments size printString].
    
    self gen: macroName; gen: '('.
    superedType isNil ifFalse: [self genVariable: superedType; gen: ', '].
    self genSelector: selector; gen: ', '; genLocation: receiver location.
    arguments do: [:arg | self gen: ', '; genLocation: arg location].
    self genl: ');'.
]
```

#### 3.2 Update CManagedCodeGenerator.st

Similar updates for the managed code generator:

```smalltalk
CManagedCodeGenerator sendNoCacheText 
[
    ^'
#define _send0(MSG, RCV) ({						\\
  register oop _r= (RCV);						\\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\\
  (_c->method)((oop)_c, _r, _r);					\\
})

#define _send1(MSG, RCV, A1) ({						\\
  register oop _r= (RCV);						\\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\\
  (_c->method)((oop)_c, _r, _r, A1);					\\
})

#define _send2(MSG, RCV, A1, A2) ({					\\
  register oop _r= (RCV);						\\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\\
  (_c->method)((oop)_c, _r, _r, A1, A2);				\\
})

#define _send3(MSG, RCV, A1, A2, A3) ({					\\
  register oop _r= (RCV);						\\
  struct __closure *_c= (struct __closure *)_libid->bind((MSG), _r);	\\
  (_c->method)((oop)_c, _r, _r, A1, A2, A3);				\\
})
']
```

Update the send method:

```smalltalk
CManagedCodeGenerator send: selector to: receiver withArguments: arguments forValue: valueFlag supered: superedType
[
    | macroName |
    self gen: '  '.
    valueFlag ifTrue: [self genLocation: receiver location; gen: '='].
    
    macroName := superedType isNil 
        ifTrue: ['_send', arguments size printString]
        ifFalse: ['_super', arguments size printString].
    
    self gen: macroName; gen: '('; genSelector: selector; gen: ', '; genLocation: receiver location.
    arguments do: [:arg | self gen: ', '; genLocation: arg location].
    self genl: ');'.
]
```

### Phase 4: Update libid Structure

#### 4.1 Add Dynamic Dispatch Functions to libid Structure

Update the `struct __libid` in `object/id/id.h`:

```c
struct __libid
{
  /* ... existing fields ... */

  /* new dynamic dispatch functions */
  oop (*sendv_dispatch)(oop selector, oop receiver, int argc, oop *argv);
  oop (*superv_dispatch)(oop type, oop selector, oop receiver, int argc, oop *argv);

  /* ... rest of structure ... */
};
```

#### 4.2 Initialize New Functions in _libid_init

Add to the initialization in `_libid_init`:

```c
_libid.sendv_dispatch = _libid_sendv_dispatch;
_libid.superv_dispatch = _libid_superv_dispatch;
```

### Phase 5: Implementation Steps

#### 5.1 Step-by-Step Implementation

**Step 1: Create Enhanced libid.c**
1. Copy `object/id/libid.c` to `object/id/libid_enhanced.c`
2. Include the new header: `#include "libid_enhanced.h"`
3. Replace all variadic macro usage systematically
4. Add dynamic dispatch functions
5. Replace variadic fatal function

**Step 2: Update Build System**
1. Modify `object/id/Makefile.in` to use `libid_enhanced.c`
2. Add compilation flags for enhanced version
3. Update dependencies

**Step 3: Update Code Generators**
1. Modify `object/idc/CCodeGenerator.st`
2. Modify `object/idc/CManagedCodeGenerator.st`
3. Regenerate all generated code

**Step 4: Testing and Validation**
1. Compile with C89 compiler (gcc -std=c89)
2. Compile with Apple Silicon clang
3. Run existing test suite
4. Performance benchmarking
5. Memory usage validation

#### 5.2 Detailed File Changes

**object/id/libid_enhanced.c Changes:**

```c
// Replace line 18:
// OLD: # define dprintf(fmt, args...)  fprintf(stderr, fmt, ##args)
// NEW: #include "libid_enhanced.h"

// Replace line 20:
// OLD: # define dprintf(fmt, args...)
// NEW: /* dprintf macros now in libid_enhanced.h */

// Replace line 175-185 (fatal function):
// OLD: static void fatal(const char *fmt, ...)
// NEW: [Replace with fatal0, fatal1, fatal2 functions]

// Replace all dprintf calls (25+ instances):
// Pattern: dprintf("format", args...) -> dprintfN("format", args...)

// Replace all _sendv calls (12 instances):
// Pattern: _sendv(sel, N, rcv, args...) -> _sendvM(sel, rcv, args...)

// Replace all fatal calls (4 instances):
// Pattern: fatal("format", args...) -> fatalN("format", args...)
```

**object/idc/CCodeGenerator.st Changes:**

```smalltalk
" Replace sendNoCacheText method completely "
" Replace sendInlineCacheText method completely "
" Update send:to:withArguments:forValue:supered: method "
" Update headerText method to include new macros "
```

**object/idc/CManagedCodeGenerator.st Changes:**

```smalltalk
" Replace sendNoCacheText method completely "
" Replace sendInlineCacheText method completely "
" Update send:to:withArguments:forValue:supered: method "
" Update headerText method to include new macros "
```

### Phase 6: Testing Strategy

#### 6.1 Compilation Testing

```bash
# Test C89 compatibility
gcc -std=c89 -Wall -Wextra -pedantic -c object/id/libid_enhanced.c

# Test Apple Silicon compatibility
clang -arch arm64 -Wall -Wextra -pedantic -c object/id/libid_enhanced.c

# Test with various compilers
gcc -c object/id/libid_enhanced.c
clang -c object/id/libid_enhanced.c
```

#### 6.2 Functional Testing

```bash
# Build complete system with enhanced runtime
make clean
make LIBID_SOURCE=libid_enhanced.c

# Run existing test suite
make test

# Test method dispatch with various argument counts
./test_dispatch_0args
./test_dispatch_1args
./test_dispatch_2args
./test_dispatch_3args
```

#### 6.3 Performance Testing

```bash
# Benchmark method dispatch performance
./benchmark_method_calls

# Memory usage comparison
valgrind --tool=massif ./id_program

# Cache performance analysis
perf stat -e cache-misses,cache-references ./id_program
```

#### 6.4 Apple Silicon Specific Testing

```bash
# Test on Apple Silicon Mac
arch -arm64 ./id_program

# Verify ARM64 calling convention compliance
otool -tv ./id_program | grep -A10 -B10 "_sendv"

# Test under Rosetta 2 (should also work)
arch -x86_64 ./id_program
```

### Phase 7: Migration and Rollback Plan

#### 7.1 Migration Strategy

**Phase 7.1: Parallel Implementation**
- Keep original `libid.c` alongside `libid_enhanced.c`
- Use build flag to select version: `make ENHANCED=1`
- Validate both versions produce identical results

**Phase 7.2: Gradual Rollout**
- Deploy enhanced version to development environments
- Run extended testing and validation
- Monitor performance and stability

**Phase 7.3: Production Deployment**
- Replace original with enhanced version
- Update all build scripts and documentation
- Remove old variadic macro code

#### 7.2 Rollback Plan

**Immediate Rollback:**
```bash
# Revert to original version
cp object/id/libid.c.backup object/id/libid.c
make clean && make
```

**Selective Rollback:**
- Keep enhanced version for Apple Silicon builds
- Use original version for other platforms if issues arise
- Conditional compilation based on platform

### Phase 8: Benefits and Impact

#### 8.1 Technical Benefits

1. **Apple Silicon Compatibility**: Eliminates ARM64 ABI violations
2. **Broader Compiler Support**: Works with any C89+ compiler
3. **Better Debugging**: Explicit macro names aid debugging
4. **Performance**: Same or better performance (no variadic overhead)
5. **Maintainability**: Clearer, more explicit code

#### 8.2 Risk Mitigation

1. **Semantic Preservation**: Zero changes to runtime behavior
2. **Binary Compatibility**: Same object layout and calling conventions
3. **Performance Guarantee**: Benchmarking ensures no regression
4. **Rollback Capability**: Original version remains available

#### 8.3 Long-term Impact

1. **Platform Expansion**: Enables deployment on Apple Silicon
2. **Embedded Systems**: Better support for resource-constrained environments
3. **Compiler Compatibility**: Works with older and specialized compilers
4. **Code Quality**: More maintainable and debuggable codebase

## Conclusion

This enhancement plan provides a comprehensive, low-risk approach to eliminating variadic macros from the Id runtime system. The numbered macro strategy preserves all existing semantics while achieving Apple Silicon compatibility and broader compiler support. The phased implementation approach ensures thorough testing and validation at each step, with clear rollback options if issues arise.

The enhancement addresses the fundamental ARM64 ABI compliance issues that prevent the current system from running on Apple Silicon, while maintaining complete backward compatibility and identical performance characteristics.
