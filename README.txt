1. cd to where your "hw5" file exists.
2. unzip everything to that directory.*
3. dos2unix -q run.sh run_test.sh run_error_test.sh; chmod +x run.sh run_test.sh run_error_test.sh
4. ./run.sh to run all tests.
5. ./run_test.sh <TEST_NUMBER> or ./run_error_test.sh <TEST_NUMBER> to run a single test.**

* make sure it looks like that:
current directory:
    hw5
    hw5-tests/
    run.sh
    run_test.sh
    run_error_test.sh

** if you are running tests directly with run_test.sh/run_error_test.sh:
    1. make sure to run before: dos2unix hw5-tests/*
    2. notice that run_error_test.sh is for tests that should result in compilation error,
       and run_test.sh for tests with no error/division by zero error.