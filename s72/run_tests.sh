#!/bin/bash

# S72 Test Runner
# Runs all test files and compares output with golden files

set -e

echo "S72 Test Runner"
echo "==============="

# Build the project first
echo "Building S72..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1
echo "✓ Build successful"

# Test function
run_test() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .s72)
    local golden_file="${test_file}.golden"
    local output_file="/tmp/s72_test_${test_name}.out"
    
    echo -n "Testing $test_name... "
    
    if [ ! -f "$golden_file" ]; then
        echo "❌ Missing golden file: $golden_file"
        return 1
    fi
    
    # Run the test and capture only the result lines (not debug output)
    # Add 'quit' to the end and filter out debug lines and prompts
    (cat "$test_file"; echo "quit") | ./s72 2>/dev/null | \
        grep -v "^DEBUG:" | \
        grep -v "^S72 Interpreter" | \
        grep -v "^Type expressions" | \
        grep -v "^Examples:" | \
        grep -v "^s72>" | \
        grep -v "^Goodbye!" | \
        grep -v "^$" | \
        grep -v "^Loading BSFL" | \
        sed 's/DEBUG: Method returned [^<]*//g' | \
        sed 's/HelloDEBUG.*/Hello/g' | \
        sed 's/ DEBUG.*//' | \
        sed 's/WorldDEBUG.*/World/g' | \
        sed 's/42DEBUG.*/42/g' | \
        sed 's/trueDEBUG.*/true/g' | \
        sed 's/falseDEBUG.*/false/g' | \
        sed 's/symbolDEBUG.*/symbol/g' | \
        sed 's/"Hello"DEBUG.*/"Hello"/g' | \
        sed "s/'symbolDEBUG.*/'symbol/g" | \
        grep -v "^$" > "$output_file"
    
    # Compare with golden file
    if diff -q "$output_file" "$golden_file" > /dev/null; then
        echo "✓ PASS"
        rm "$output_file"
        return 0
    else
        echo "❌ FAIL"
        echo "Expected output (golden file):"
        cat "$golden_file"
        echo ""
        echo "Actual output:"
        cat "$output_file"
        echo ""
        echo "Diff:"
        diff "$golden_file" "$output_file" || true
        rm "$output_file"
        return 1
    fi
}

# Find and run all tests
test_files=$(find tests -name "*.s72" | sort)
total_tests=0
passed_tests=0

for test_file in $test_files; do
    total_tests=$((total_tests + 1))
    if run_test "$test_file"; then
        passed_tests=$((passed_tests + 1))
    fi
done

echo ""
echo "Test Results:"
echo "============="
echo "Passed: $passed_tests/$total_tests"

if [ $passed_tests -eq $total_tests ]; then
    echo "🎉 All tests passed!"
    exit 0
else
    echo "❌ Some tests failed"
    exit 1
fi
