
#include "job.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

static int read_integer(const char *prompt,int *value) {
    char input[128];
    char *end;
    long number;

    while(1) {
        printf("%s",prompt);
        fflush(stdout);

        if(fgets(input,sizeof(input),stdin)==NULL)
            return 0;

        if(strchr(input,'\n')==NULL && !feof(stdin)) {
            int ch;
            while((ch=getchar())!='\n' && ch!=EOF);
            printf("Input is too long. Try again.\n");
            continue;
        }

        errno=0;
        number=strtol(input,&end,10);

        if(end==input || errno==ERANGE ||
           number<INT_MIN || number>INT_MAX) {
            printf("Invalid input. Enter a valid integer.\n");
            continue;
        }

        while(isspace((unsigned char)*end))
            end++;

        if(*end!='\0') {
            printf("Invalid input. Enter only an integer.\n");
            continue;
        }

        *value=(int)number;
        return 1;
    }
}

static int job_exists(const Job jobs[],size_t count,int id) {
    for(size_t i=0;i<count;i++) {
        if(jobs[i].id==id)
            return 1;
    }

    return 0;
}

int read_jobs(Job jobs[],size_t *count) {
    int n;

    if(jobs==NULL || count==NULL)
        return 0;

    *count=0;

    while(1) {
        if(!read_integer("Enter number of jobs (1-50): ",&n))
            return 0;

        if(n>=1 && n<=MAX_JOBS)
            break;

        printf("Number of jobs must be between 1 and %d.\n",MAX_JOBS);
    }

    for(int i=0;i<n;i++) {
        printf("\nEnter details for Job %d\n",i+1);

        while(1) {
            if(!read_integer("Job ID: ",&jobs[i].id))
                return 0;

            if(jobs[i].id<=0) {
                printf("Job ID must be positive.\n");
                continue;
            }

            if(job_exists(jobs,i,jobs[i].id)) {
                printf("Job ID already exists. Enter a unique ID.\n");
                continue;
            }

            break;
        }

        while(1) {
            if(!read_integer("Arrival Time: ",&jobs[i].arrival_time))
                return 0;

            if(jobs[i].arrival_time>=0)
                break;

            printf("Arrival time cannot be negative.\n");
        }

        while(1) {
            if(!read_integer("CPU Burst Time: ",&jobs[i].burst_time))
                return 0;

            if(jobs[i].burst_time>0)
                break;

            printf("CPU burst time must be greater than zero.\n");
        }

        while(1) {
            if(!read_integer("Priority: ",&jobs[i].priority))
                return 0;

            if(jobs[i].priority>0)
                break;

            printf("Priority must be greater than zero.\n");
        }

        jobs[i].status=JOB_READY;
        *count=i+1;
    }

    return 1;
}

const char *job_status_name(JobStatus status) {
    switch(status) {
        case JOB_READY:
            return "READY";
        case JOB_RUNNING:
            return "RUNNING";
        case JOB_COMPLETED:
            return "COMPLETED";
        default:
            return "INVALID";
    }
}

void display_jobs(const Job jobs[],size_t count) {
    if(jobs==NULL) {
        printf("Invalid job list.\n");
        return;
    }

    if(count==0) {
        printf("No jobs available.\n");
        return;
    }

    printf("\n%-10s %-15s %-15s %-12s %-12s\n",
           "Job ID","Arrival Time","Burst Time","Priority","Status");

    for(size_t i=0;i<count;i++) {
        printf("%-10d %-15d %-15d %-12d %-12s\n",
               jobs[i].id,
               jobs[i].arrival_time,
               jobs[i].burst_time,
               jobs[i].priority,
               job_status_name(jobs[i].status));
    }
}

static int find_job(const Job jobs[],size_t count,int job_id) {
    for(size_t i=0;i<count;i++) {
        if(jobs[i].id==job_id)
            return (int)i;
    }

    return -1;
}

int start_job(Job jobs[],size_t count,int job_id) {
    int index;
    char message[128];

    if(jobs==NULL || (index=find_job(jobs,count,job_id))==-1) {
        printf("Job not found.\n");
        return 0;
    }

    if(jobs[index].status!=JOB_READY) {
        printf("Job cannot be started in its current state.\n");
        return 0;
    }

    jobs[index].status=JOB_RUNNING;

    snprintf(message,sizeof(message),
             "Job %d started and is now RUNNING.",job_id);
    logger_log(message);

    printf("Job %d is now RUNNING.\n",job_id);
    return 1;
}

int pause_job(Job jobs[],size_t count,int job_id) {
    int index;
    char message[128];

    if(jobs==NULL || (index=find_job(jobs,count,job_id))==-1) {
        printf("Job not found.\n");
        return 0;
    }

    if(jobs[index].status!=JOB_RUNNING) {
        printf("Only a running job can be paused.\n");
        return 0;
    }

    jobs[index].status=JOB_READY;

    snprintf(message,sizeof(message),
             "Job %d paused and returned to READY.",job_id);
    logger_log(message);

    printf("Job %d is now READY.\n",job_id);
    return 1;
}

int complete_job(Job jobs[],size_t count,int job_id) {
    int index;
    char message[128];

    if(jobs==NULL || (index=find_job(jobs,count,job_id))==-1) {
        printf("Job not found.\n");
        return 0;
    }

    if(jobs[index].status!=JOB_RUNNING) {
        printf("Only a running job can be completed.\n");
        return 0;
    }

    jobs[index].status=JOB_COMPLETED;

    snprintf(message,sizeof(message),
             "Job %d has COMPLETED.",job_id);
    logger_log(message);

    printf("Job %d has COMPLETED.\n",job_id);
    return 1;
}
