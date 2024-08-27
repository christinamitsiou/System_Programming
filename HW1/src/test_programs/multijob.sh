#!/bin/bash

# Store the argument count
argc=$#

# Check if enough arguments were provided (need to be 1 or more)
if [ "$argc" -lt 1 ]; then
    # Print an error message and exit
    echo "Not enough arguments: Input must be 1 or more files"
    exit 1
fi

# Count the jobs that failed to execute
count_failure=0

# Go through all the arguments (files) one by one
for file in "$@"; do
    # If the file exists
    if [ -f "$file" ]; then
        # Read its content
        while IFS= read -r line || [[ -n "$line" ]]; do
            # Issue the job to execute using jobCommander
            jobCommander issueJob "$line"
        done < "$file"
    else
        # If the file doesn't exist, print an error message and go on to the next
        echo "File $file does not exist."
        count_failure=$((count_failure + 1))
    fi
done

# The number of jobs that were issued is the number of arguments given minus the number of failures
count_success=$((argc - count_failure))

# Print the number of successful and failed jobs
echo "$count_success jobs have been issued successfully to jobCommander."
echo "$count_failure jobs failed to be issued to jobCommander."
