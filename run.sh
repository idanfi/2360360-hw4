#!/bin/bash

dos2unix -q hw5-tests/*
for t in {1..21}
do ./run_test.sh $t
done

for t in {1..45}
do ./run_error_test.sh $t
done