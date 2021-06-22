#!/bin/bash
test_=$1

echo "COMPILATION ERROR TEST $test_"
./hw5 < hw5-tests/te$test_.in 2>&1 > hw5-tests/te$test_.res
diff hw5-tests/te$test_.res hw5-tests/te$test_.out

