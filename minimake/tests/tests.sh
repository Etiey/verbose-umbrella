#!/bin/bash

MINIMAKE="../minimake"
PASSED=0
FAILED=0

echo "===== Minimake Test Suite ====="
echo

# Check if minimake exists
if [ ! -f "$MINIMAKE" ]; then
    echo "ERROR: minimake executable not found at $MINIMAKE"
    echo "Please run 'make' first to build minimake"
    exit 1
fi

# Test 1: -h option
echo "Test 1: -h option..."
if $MINIMAKE -h > /dev/null 2>&1; then
    echo "  PASSED"
    ((PASSED++))
else
    echo "  FAILED"
    ((FAILED++))
fi

# Test 2: Pretty print
echo "Test 2: -p option..."
cat > test_makefile << 'EOF'
VAR=value
all: dep
	echo test
EOF

if $MINIMAKE -p -f test_makefile > /dev/null 2>&1; then
    echo "  PASSED"
    ((PASSED++))
else
    echo "  FAILED"
    ((FAILED++))
fi

rm -f test_makefile

# Test 3: Variable substitution
echo "Test 3: Variable substitution..."
cat > test_makefile << 'EOF'
VAR=hello
all:
	echo $(VAR)
EOF

OUTPUT=$($MINIMAKE -f test_makefile 2>&1)
if echo "$OUTPUT" | grep -q "hello"; then
    echo "  PASSED"
    ((PASSED++))
else
    echo "  FAILED"
    echo "  Expected: hello"
    echo "  Got: $OUTPUT"
    ((FAILED++))
fi

rm -f test_makefile

# Test 4: Simple rule execution
echo "Test 4: Simple rule execution..."
cat > test_makefile << 'EOF'
test:
	echo SUCCESS
EOF

OUTPUT=$($MINIMAKE -f test_makefile test 2>&1)
if echo "$OUTPUT" | grep -q "SUCCESS"; then
    echo "  PASSED"
    ((PASSED++))
else
    echo "  FAILED"
    ((FAILED++))
fi

rm -f test_makefile

# Test 5: Default target selection
echo "Test 5: Default target selection..."
cat > test_makefile << 'EOF'
first:
	echo FIRST

second:
	echo SECOND
EOF

OUTPUT=$($MINIMAKE -f test_makefile 2>&1)
if echo "$OUTPUT" | grep -q "FIRST"; then
    echo "  PASSED"
    ((PASSED++))
else
    echo "  FAILED"
    echo "  Expected: FIRST"
    echo "  Got: $OUTPUT"
    ((FAILED++))
fi

rm -f test_makefile

# Test 6: Error handling - missing target
echo "Test 6: Error handling - missing target..."
cat > test_makefile << 'EOF'
all:
	echo test
EOF

if $MINIMAKE -f test_makefile nonexistent 2>&1 | grep -q "No rule to make target"; then
    echo "  PASSED"
    ((PASSED++))
else
    echo "  FAILED"
    ((FAILED++))
fi

rm -f test_makefile

# Summary
echo "===== Test Summary ====="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "Total:  $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo "All tests passed!"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi
