#!/bin/bash


echo "Testing enhanced client UI components..."


echo "Compiling client..."
make client


mkdir -p test_results


echo "Running UI component tests..."


echo "Test 1: Basic connection test"
./build/client | tee test_results/basic_test.log

echo "All tests completed. Check test_results directory for logs."
