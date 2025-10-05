# Enhanced libid Test Suite

This test suite verifies the Apple Silicon ARM64 compatibility and numbered macro functionality of the enhanced libid runtime system.

## Overview

The test suite consists of 6 comprehensive tests that verify different aspects of the enhanced libid:

1. **Test 1: Selector Interning** - Basic functionality and object identity
2. **Test 2: Object Creation** - Prototype creation and method dispatch compilation
3. **Test 3: Memory Allocation** - GC integration and ARM64 memory management
4. **Test 4: Method Installation** - Custom method installation and function pointers
5. **Test 5: Import/Export** - Global namespace and module system
6. **Test 6: ARM64 Stress Test** - Comprehensive numbered macro and ABI compliance testing

## Quick Start

### Running Tests

For a **concise test output** that shows only pass/fail status:
```bash
make test-quiet
```

For **full verbose output** (may be very long):
```bash
make test-full
```

For **compilation verification only** (recommended):
```bash
make test
```

### Prerequisites

1. **Enhanced libid built**:
   ```bash
   cd ../object/id
   BIN="../stage1/" make ../stage1/libid.o
   ```

2. **Homebrew bdw-gc installed**:
   ```bash
   brew install bdw-gc
   ```

### Running Tests

```bash
# Build and run all tests
make test

# Just build (compilation test)
make compile-test

# Run individual tests
make test1  # Selector interning
make test2  # Object creation
make test3  # Memory allocation
make test4  # Method installation
make test5  # Import/export
make test6  # ARM64 stress test

# Check dependencies
make check-deps

# Clean up
make clean
```

## Output Modes

### Quiet Mode (`-q` or `--quiet`)

The test runner supports a quiet mode that significantly reduces output verbosity:

- **Suppresses**: Detailed test output, system info, verbose method traces
- **Shows**: Test names, pass/fail status, timing, and failure details
- **Usage**: `./run_all_tests -q` or `make test-quiet`

**Example quiet output:**
```
🧪 Enhanced libid Test Suite (Quiet Mode)
Testing Apple Silicon ARM64 compatibility and numbered macro functionality

Running Test 1... PASSED (0.0s)
Running Test 2... PASSED (0.0s)
Running Test 7... CRASHED (0.0s)
Running Test 8... FAILED (exit code 1, 0.0s)

🏁 Test Suite Summary
======================================================================
Total Tests: 10
✅ Passed: 8
❌ Failed: 2
```

This mode is especially useful when some tests produce thousands of lines of output (like performance tests), making it impossible to see which tests actually failed.

## Test Details

### Test 1: Selector Interning (Basic)
- **Purpose**: Verify basic libid initialization and selector interning
- **Tests**: 
  - libid initialization
  - Selector creation and identity
  - String interning correctness
- **Expected**: All selectors with same string return same object

### Test 2: Object Creation (Simple)
- **Purpose**: Test prototype creation and numbered macro compilation
- **Tests**:
  - Object prototype creation
  - Object identity and delegation
  - Numbered macro compilation (_sendv0, _sendv1, etc.)
- **Expected**: Objects created successfully, macros compile

### Test 3: Memory Allocation (GC Integration)
- **Purpose**: Verify GC integration and ARM64 memory management
- **Tests**:
  - Pointer allocation (_libid_palloc)
  - Atomic allocation (_libid_balloc)
  - Various allocation sizes
  - Memory corruption detection
  - Large-scale object creation
- **Expected**: All allocations succeed, no corruption

### Test 4: Method Installation (Advanced)
- **Purpose**: Test custom method installation and function pointers
- **Tests**:
  - Custom method installation
  - Methods with different argument counts
  - Function pointer compatibility
  - Multiple methods per object
- **Expected**: Methods install correctly, function pointers work

### Test 5: Import/Export (Module System)
- **Purpose**: Verify global namespace functionality
- **Tests**:
  - Object export/import
  - Identity preservation
  - Export overwriting
  - Complex export names
- **Expected**: Objects maintain identity across import/export

### Test 6: ARM64 Stress Test (Comprehensive)
- **Purpose**: Comprehensive ARM64 ABI compliance and macro testing
- **Tests**:
  - All numbered macros (_sendv0 through _sendv5)
  - ARM64 register usage patterns
  - Function pointer arrays
  - Memory alignment verification
  - Dynamic dispatch function compilation
- **Expected**: All macros compile, no ABI violations

## Expected Output

### Successful Test Run
```
🧪 Enhanced libid Test Suite
Testing Apple Silicon ARM64 compatibility and numbered macro functionality

🖥️  System Information:
   System: Darwin MacBook-Pro.local 23.6.0 Darwin Kernel Version 23.6.0: arm64
   Architecture: arm64
   Compiler: Apple clang version 15.0.0 (clang-1500.3.9.4)

================================================================================
Running Test 1: Selector Interning (Basic functionality)
Executable: ./test1_selector_interning
================================================================================
=== Test 1: Selector Interning Test ===
✓ libid initialized successfully
✓ First selector 'hello' interned: 0x600000004040
✓ Second selector 'hello' interned: 0x600000004040
✓ Third selector 'world' interned: 0x600000004080
✓ PASS: Same selector interned twice returns same object
✓ PASS: Different selectors return different objects
✓ PASS: Complex selector interning works correctly

=== Test 1 Results ===
✓ All selector interning tests PASSED
✓ Basic libid functionality verified
✓ Memory management working

✅ Test 1 PASSED (0.1 seconds)

[... similar output for Tests 2-6 ...]

🏁 Test Suite Summary
================================================================================
Total Tests: 6
✅ Passed: 6
❌ Failed: 0
⏭️  Skipped: 0
⏱️  Total Time: 2.3 seconds

🎉 ALL TESTS PASSED! Enhanced libid is working correctly on this platform.
```

### Failed Test Example
```
❌ Test 3 FAILED with exit code 1 (0.5 seconds)

🏁 Test Suite Summary
================================================================================
Total Tests: 6
✅ Passed: 2
❌ Failed: 1
⏭️  Skipped: 3
⏱️  Total Time: 1.8 seconds

💥 1 TESTS FAILED. Please check the output above for details.
```

## Troubleshooting

### Common Issues

1. **libid.o not found**:
   ```bash
   cd ../object/id
   BIN="../stage1/" make ../stage1/libid.o
   ```

2. **GC library missing**:
   ```bash
   brew install bdw-gc
   ```

3. **Compilation errors**:
   - Check that you're on ARM64 architecture
   - Verify enhanced libid was built correctly
   - Check compiler version compatibility

4. **Runtime failures**:
   - May indicate ARM64 ABI violations in original code
   - Verify numbered macros are being used
   - Check GC initialization

### Debug Mode

For detailed debugging, compile with debug flags:
```bash
make CFLAGS="-Wall -Wextra -g -DDEBUG_ALL=1" test
```

## Architecture Verification

The tests specifically verify:

- ✅ **ARM64 ABI Compliance**: No variadic macro violations
- ✅ **Register Usage**: Proper x0-x7 register allocation
- ✅ **Function Calls**: Standard calling conventions
- ✅ **Memory Alignment**: ARM64-compatible alignment
- ✅ **GC Integration**: Modern bdw-gc 8.x compatibility
- ✅ **Numbered Macros**: All _sendv0-_sendv5 variants work

## Success Criteria

All tests should pass on Apple Silicon ARM64. This indicates:

1. **Enhanced libid works correctly** on ARM64
2. **Variadic macro issues resolved** - no ABI violations
3. **GC integration successful** - modern bdw-gc works
4. **Numbered macros functional** - all variants compile and work
5. **ARM64 compatibility achieved** - native performance

If any tests fail, it indicates potential issues with the ARM64 port that need investigation.
