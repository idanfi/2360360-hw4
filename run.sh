#!/bin/bash

dos2unix -q hw5-tests/*
for t in {1..3}
do ./run_test.sh $t
done

for t in {1..3}
do ./run_error_test.sh $t
done