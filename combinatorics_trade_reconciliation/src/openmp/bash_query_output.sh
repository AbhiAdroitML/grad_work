#!/bin/bash


echo "Count of unique combinations found by each job"
grep -cH "Combo Found" ./output/*_output.txt
