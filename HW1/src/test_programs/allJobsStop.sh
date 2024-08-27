#!/bin/bash

# Get a list of all running jobs
running_jobs=$(jobCommander poll running)

# Check if no running jobs are found
if [ -z "$running_jobs" ]; then
    echo "No running jobs found."
else
    # Stop every running process
    echo "$running_jobs" | while read -r running_job_id; do
        # Stop the running job using jobCommander stop command
        jobCommander stop "$running_job_id"
        # Print a message indicating the stopped running job
        echo "Stopped running job: $running_job_id"
    done
fi

# Get a list of all queued jobs
queued_jobs=$(jobCommander poll queued)

# Check if no queued jobs are found
if [ -z "$queued_jobs" ]; then
    echo "No queued jobs found."
else
    # Stop every waiting process
    echo "$queued_jobs" | while read -r queued_job_id; do
        # Stop the queued job using jobCommander stop command
        jobCommander stop "$queued_job_id"
        # Print a message indicating the stopped queued job
        echo "Stopped queued job: $queued_job_id"
    done
fi

# Print a message indicating that all jobs have been stopped
echo "All jobs have been stopped."
