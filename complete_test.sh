#!/bin/bash

# ========================================
# CS 5600 Project 2 - Complete Test Suite with Concurrency
# ========================================

echo "CS 5600 Project 2 - Comprehensive Test Framework"
echo "================================================="
echo ""


make stairs
echo "✓ Concurrent mock program compiled as 'stairs'"
echo ""

# Step 2:
echo "Step 2: Creating enhanced test input files..."

# Test 1:
cat > test1_continuous.txt << EOF
10 5
0 1 0
1 1 50
2 1 100
3 1 150
4 1 200
5 -1 1000
6 -1 1050
7 -1 1100
8 -1 1150
9 -1 1200
EOF

# Test 2:
cat > test2_alternating.txt << EOF
10 5
0 1 0
1 -1 100
2 1 200
3 -1 300
4 1 400
5 -1 500
6 1 600
7 -1 700
8 1 800
9 -1 900
EOF

# Test 3:
cat > test3_deadlock_risk.txt << EOF
2 5
0 1 0
1 -1 0
EOF

# Test 4:
cat > test4_dynamic_join.txt << EOF
15 10
0 1 0
1 1 100
2 1 200
3 1 250
4 1 300
5 -1 1500
6 -1 1600
7 1 1650
8 1 1700
9 -1 2000
10 -1 2050
11 1 2100
12 1 2150
13 -1 2200
14 -1 2250
EOF


echo "✓ Enhanced test input files created"
echo ""

# Step 3:
echo "Step 3: Running comprehensive tests..."
echo "========================================="

mkdir -p test_outputs

run_test() {
    local test_name=$1
    local input_file=$2
    local description=$3
    local output_file="test_outputs/${test_name}_output.txt"
    
    echo ""
    echo "Running $test_name"
    echo "Description: $description"
    echo "-----------------------------------------"
    
    timeout 30 ./stairs < $input_file > $output_file 2>&1
    local exit_code=$?
    
    if [ $exit_code -eq 124 ]; then
        echo "❌ TIMEOUT: Possible deadlock or infinite wait"
        echo "TIMEOUT" > test_outputs/${test_name}_result.txt
        return
    fi
    
    local total=$(head -n 1 $input_file | awk '{print $1}')
    local finished=$(grep -c "finished stairs" $output_file)
    local direction_changes=$(grep -c "direction can change" $output_file)
    local waiting=$(grep -c "waiting for direction" $output_file)
    local avg_time=$(grep "Average Turnaround" $output_file | awk '{print $(NF-1)}')
    
    echo "  Total customers: $total"
    echo "  Finished: $finished"
    echo "  Direction changes: $direction_changes"
    echo "  Wait events: $waiting"
    echo "  Avg turnaround: $avg_time ms"
    
    if [ "$finished" -eq "$total" ]; then
        echo "✅ PASSED"
        echo "PASS" > test_outputs/${test_name}_result.txt
    else
        echo "❌ FAILED: Only $finished/$total completed"
        echo "FAIL" > test_outputs/${test_name}_result.txt
    fi
}

run_test "Test1_Continuous" "test1_continuous.txt" \
    "Same direction customers arrive in bursts"

run_test "Test2_Alternating" "test2_alternating.txt" \
    "Alternating arrivals testing frequent direction switches"

run_test "Test3_Deadlock" "test3_deadlock_risk.txt" \
    "Two opposite directions arrive simultaneously"

run_test "Test4_Dynamic" "test4_dynamic_join.txt" \
    "Customers join while stairs are in use"

# Step 4:Report
echo ""
echo "========================================="
echo "         COMPREHENSIVE TEST REPORT"
echo "========================================="

pass_count=0
fail_count=0
timeout_count=0

for result_file in test_outputs/*_result.txt; do
    result=$(cat $result_file)
    if [ "$result" = "PASS" ]; then
        ((pass_count++))
    elif [ "$result" = "TIMEOUT" ]; then
        ((timeout_count++))
        ((fail_count++))
    else
        ((fail_count++))
    fi
done

total_tests=4
echo "Total Tests: $total_tests"
echo "Passed: $pass_count"
echo "Failed: $fail_count"
echo "  - Timeouts/Deadlocks: $timeout_count"
echo "Success Rate: $((pass_count * 100 / total_tests))%"
echo ""

cat > TEST_REPORT.md << EOF
# CS 5600 Project 2 - Comprehensive Concurrency Test Report

## Test Environment
- Date: $(date)
- Concurrency: Multi-threaded with pthread
- Framework Version: 3.0 (Full Concurrency)

---

## Test Results Summary

| Test | Description | Status | Details |
|------|-------------|--------|---------|
| Test 1 | Continuous same-direction arrivals | $(cat test_outputs/Test1_Continuous_result.txt) | Burst arrivals |
| Test 2 | Alternating direction arrivals | $(cat test_outputs/Test2_Alternating_result.txt) | Frequent switches |
| Test 3 | Deadlock risk scenario | $(cat test_outputs/Test3_Deadlock_result.txt) | Simultaneous opposite |
| Test 4 | Dynamic joining | $(cat test_outputs/Test4_Dynamic_result.txt) | Join during use |
---

## Detailed Test Analysis

### Test 1: Continuous Same-Direction Arrivals
**Scenario**: Customers going the same direction arrive in quick succession
- First 5 customers (up) arrive 0-200ms
- Next 5 customers (down) arrive 1000-1200ms

**Key Metrics**:
- Completed: $(grep -c "finished stairs" test_outputs/Test1_Continuous_output.txt)/10
- Direction changes: $(grep -c "direction can change" test_outputs/Test1_Continuous_output.txt)
- Waiting events: $(grep -c "waiting for direction" test_outputs/Test1_Continuous_output.txt)

**Tests**: Can multiple same-direction customers enter continuously

---

### Test 2: Alternating Direction Arrivals
**Scenario**: Customers alternate direction every 100ms

**Key Metrics**:
- Completed: $(grep -c "finished stairs" test_outputs/Test2_Alternating_output.txt)/10
- Direction changes: $(grep -c "direction can change" test_outputs/Test2_Alternating_output.txt)
- Average wait: $(grep "Average" test_outputs/Test2_Alternating_output.txt | awk '{print $NF}')

**Tests**: Frequent direction switching efficiency

---

### Test 3: Deadlock Risk Scenario ⚠️ CRITICAL
**Scenario**: Two customers from opposite directions arrive within 10ms

**Key Metrics**:
- Completed: $(grep -c "finished stairs" test_outputs/Test3_Deadlock_output.txt)/2
- Timeout: $(grep -c "TIMEOUT" test_outputs/Test3_Deadlock_result.txt)

**Tests**: The most critical deadlock scenario

**Expected Behavior**:
- One customer should get priority
- The other should wait
- NO DEADLOCK

---

### Test 4: Dynamic Joining During Usage
**Scenario**: New customers arrive while stairs are actively in use

**Timeline**:
- 0-300ms: 5 customers going up
- 1500-1700ms: Mixed arrivals while stairs busy
- 2000-2250ms: More mixed arrivals

**Key Metrics**:
- Completed: $(grep -c "finished stairs" test_outputs/Test4_Dynamic_output.txt)/15
- On-stairs events: $(grep -c "on_stairs:" test_outputs/Test4_Dynamic_output.txt)

**Tests**: Can customers join mid-crossing

### Test 5: Extreme Starvation Prevention ⚠️ CRITICAL
**Scenario**: 29 customers going down, 1 customer going up - all arrive simultaneously

**Key Metrics**:
- Completed: $(grep -c "Finished Stairs" test_outputs/Test5_Extreme_Starvation_output.txt)/30
- Customer 29 (up) wait time: [Check if minority completes]
- Direction changes: $(grep -c "Crossing direction reset" test_outputs/Test5_Extreme_Starvation_output.txt)

**Tests**: Single minority customer must not starve despite 29:1 ratio

**Expected Behavior**:
- Customer 29 (going up) should eventually complete
- System must give minority a chance even under extreme imbalance
- NO infinite waiting

## Critical Concurrency Issues to Check

### ✅ Passed Tests Should Show:
1. All customers complete successfully
2. Proper mutex/condition variable usage
3. Fair scheduling between directions
4. No race conditions

### ❌ Failed Tests May Indicate:
1. **Deadlock**: Timeout in Test 3
2. **Starvation**: Minority never completes in Test 6
3. **Race condition**: Random failures, wrong counts
4. **Incorrect synchronization**: Customers on stairs simultaneously from different directions



echo "✓ Comprehensive test report generated: TEST_REPORT.md"
echo ""
echo "========================================="
echo "   CONCURRENCY TEST FRAMEWORK COMPLETE"
echo "========================================="
