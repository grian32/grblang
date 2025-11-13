#!/bin/bash
./tests/build.sh

TEST_DIR="tests"

for TEST_FILE in "$TEST_DIR"/*.grb; do
    EXPECTED_OUTPUT=$(cat "${TEST_FILE}.out")
    ACTUAL_OUTPUT=$(./tests/grblang ${TEST_FILE})

    if [ "$ACTUAL_OUTPUT" == "$EXPECTED_OUTPUT" ]; then
        echo "Test $(basename "$TEST_FILE") passed!"
    else
        echo "Test $(basename "$TEST_FILE") failed!"
        echo "Expected: $EXPECTED_OUTPUT"
        echo "Got: $ACTUAL_OUTPUT"
    fi
done

./tests/cleanup.sh
