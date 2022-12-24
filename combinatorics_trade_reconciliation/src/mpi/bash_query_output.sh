#!/bin/bash


echo "Count of unique combinations found by each job"
grep -cH "Unique Combo" ./output/*_output.txt
