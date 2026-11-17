
#ifndef JOB_H
#define JOB_H

#include <stddef.h>

#define MAX_JOBS 50

typedef enum {
    JOB_READY,
    JOB_RUNNING,
    JOB_COMPLETED
} JobStatus;

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int priority;
    JobStatus status;
} Job;

int read_jobs(Job jobs[], size_t *count);
void display_jobs(const Job jobs[], size_t count);
int start_job(Job jobs[], size_t count, int job_id);
int pause_job(Job jobs[], size_t count, int job_id);
int complete_job(Job jobs[], size_t count, int job_id);
const char *job_status_name(JobStatus status);

#endif
