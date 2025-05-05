#!/bin/bash

# Test script for factory_manager program
# Displays TC# for each test case and checks requirements

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Compile the program first
echo "Compiling the program..."
make clean
make
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful!${NC}\n"

# Create test input files
echo "Creating test input files..."

# TC1: Valid input file with one belt
cat > test1.txt << EOF
1 101 3 5
EOF

# TC2: Valid input file with multiple belts
cat > test2.txt << EOF
3 101 3 5 102 2 3 103 5 2
EOF

# TC3: Invalid file - more belts than max
cat > test3.txt << EOF
2 101 3 5 102 2 3 103 5 2
EOF

# TC4: Invalid file - zero max belts
cat > test4.txt << EOF
0 101 3 5
EOF

# TC5: Invalid file - negative belt size
cat > test5.txt << EOF
1 101 -3 5
EOF

# TC6: Invalid file - negative elements
cat > test6.txt << EOF
1 101 3 -5
EOF

# TC7: Edge case - single element
cat > test7.txt << EOF
1 101 1 1
EOF

# TC8: Edge case - full belt utilization
cat > test8.txt << EOF
1 101 3 3
EOF
# TC9: No products (zero elements)
cat > test9.txt << EOF
1 101 3 0
EOF
# TC10: One more than min and one less than max
cat > test10.txt << EOF
1 101 2 1
EOF
# Function to run test and check output
run_test() {
    local test_num=$1
    local input_file=$2
    local expected_exit=$3
    local expected_pattern=$4
    local description=$5
    
    echo "========================================"
    echo -e "${GREEN}TC${test_num}: ${description}${NC}"
    echo "Input file: ${input_file}"
    echo "Expected exit code: ${expected_exit}"
    
    ./factory ${input_file} > output.txt 2> error.txt
    actual_exit=$?
    
    if [ ${actual_exit} -ne ${expected_exit} ]; then
        echo -e "${RED}FAIL: Wrong exit code (expected ${expected_exit}, got ${actual_exit})${NC}"
        return 1
    fi
    
    if [ -n "${expected_pattern}" ]; then
        if ! grep -q "${expected_pattern}" output.txt && ! grep -q "${expected_pattern}" error.txt; then
            echo -e "${RED}FAIL: Expected pattern '${expected_pattern}' not found${NC}"
            return 1
        fi
    fi
    
    echo -e "${GREEN}PASS${NC}"
    return 0
}

# Run all test cases
echo "Running test cases..."

# TC1: Valid input file with one belt
run_test 1 test1.txt 0 "[OK][factory_manager] Process_manager with id 101 has been created." "Single belt test"

# TC2: Valid input file with multiple belts
run_test 2 test2.txt 0 "[OK][factory_manager] Process_manager with id 103 has been created." "Multiple belts test"

# TC3: Invalid file - more belts than max
run_test 3 test3.txt 255 "[ERROR][factory_manager] Invalid file." "More belts than max test"

# TC4: Invalid file - zero max belts
run_test 4 test4.txt 255 "[ERROR][factory_manager] Invalid file." "Zero max belts test"

# TC5: Invalid file - negative belt size
run_test 5 test5.txt 255 "[ERROR][factory_manager] Invalid file." "Negative belt size test"

# TC6: Invalid file - negative elements
run_test 6 test6.txt 255 "[ERROR][factory_manager] Invalid file." "Negative elements test"

# TC7: Edge case - single element
run_test 7 test7.txt 0 "[OK][queue] Introduced element with id 0 in belt 101" "Single element test"

# TC8: Edge case - full belt utilization
run_test 8 test8.txt 0 "[OK][queue] Obtained element with id 0 in belt 101" "Full belt utilization test"

# TC9: No products (zero elements)
run_test 9 test9.txt 255 "[ERROR][process_manager] No process manager is going to be created as there are no elements to be produced" "Zero elements test"

# TC10: One more than min and one less than max
run_test 10 test10.txt 0 "[OK][process_manager] Process_manager with id 101 has been created." "Min-max test"
echo "========================================"
echo "Test completed"
echo "Cleaning up..."
rm -f test*.txt output.txt error.txt
make clean
