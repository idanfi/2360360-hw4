#!/bin/bash
test_=$1

echo "TEST t$test_"
./hw5 < hw5-tests/t$test_.in 2>&1 > hw5-tests/t$test_.llvm
lli hw5-tests/t$test_.llvm > hw5-tests/t$test_.res
diff hw5-tests/t$test_.res hw5-tests/t$test_.out

