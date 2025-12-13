#!/bin/bash

# Check if the correct number of arguments is passed
if [ $# -lt 3 ]; then
  echo "Usage: $0 <filelist.txt> <output_file> <output_entries>"
  exit 1
fi

# Assign arguments to variables
filelist=$1
output_file=$2
output_entries=$3

# Ensure the input filelist exists
if [ ! -f "$filelist" ]; then
  echo "Error: $filelist not found!"
  exit 1
fi

file_number=0

# Read through each line of the filelist.txt and submit a condor job
while IFS= read -r input_dir; do

  ((file_number++))  
  # Submit the job with condor_submit
#  condor_submit condorjob_stau_multi.jdl \
#    -append input_dir="$input_dir" \
#    -append output="$output" \
#    -append output_entries="$output_entries" \
#    file_number="$file_number"

  condor_submit condorjob_stau_multi.jdl \
  -append "input_dir='${input_dir}'" \
  -append "output_file='${output_file}'" \
  -append "output_entries='${output_entries}'" \
  -append "file_number='${file_number}'"
  
  echo "Submitted job for input_dir: $input_dir with output: $output and output_entries: $output_entries, and file_number: $file_number" 

done < "$filelist"

