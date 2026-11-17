
#include "job.h"
#include "scheduler.h"
#include "logger.h"
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    Job jobs[MAX_JOBS];
    size_t count=0;
    char input[128];
    char *end;
    long choice;
    int job_id;

    printf("===== CampusCompute =====\n");
    printf("FR1 - Job Management\n");
    printf("FR2 - CPU Scheduling\n");
    printf("FR3 - Activity Logging\n");
    printf("FR4 - Inter-Process Communication\n");

    if(!read_jobs(jobs,&count)) {
        printf("Unable to read job details.\n");
        return 1;
    }

    if(logger_start()) {
        char message[128];

        for(size_t i=0;i<count;i++) {
            snprintf(message,sizeof(message),
                     "Job %d created.",jobs[i].id);
            logger_log(message);
        }
    }

    while(1) {
        printf("\n===== CampusCompute Menu =====\n");
        printf("1. Display Jobs\n");
        printf("2. Start Job\n");
        printf("3. Pause Job\n");
        printf("4. Complete Job\n");
        printf("5. FCFS Scheduling\n");
        printf("6. Round Robin Scheduling\n");
        printf("7. Priority Scheduling\n");
        printf("8. IPC Demonstration\n");
        printf("9. Exit\n");
        printf("Enter choice: ");

        if(fgets(input,sizeof(input),stdin)==NULL)
            break;

        errno=0;
        choice=strtol(input,&end,10);

        if(end==input || errno==ERANGE) {
            printf("Invalid choice.\n");
            continue;
        }

        while(*end==' ' || *end=='\t' || *end=='\n')
            end++;

        if(*end!='\0' || choice<1 || choice>9) {
            printf("Invalid choice. Enter a number from 1 to 9.\n");
            continue;
        }

        if(choice==1) {
            display_jobs(jobs,count);
        }
        else if(choice==2 || choice==3 || choice==4) {
            printf("Enter Job ID: ");

            if(fgets(input,sizeof(input),stdin)==NULL)
                break;

            errno=0;
            long id=strtol(input,&end,10);

            while(*end==' ' || *end=='\t' || *end=='\n')
                end++;

            if(end==input || errno==ERANGE || *end!='\0' ||
               id<=0 || id>2147483647) {
                printf("Invalid Job ID.\n");
                continue;
            }

            job_id=(int)id;

            if(choice==2)
                start_job(jobs,count,job_id);
            else if(choice==3)
                pause_job(jobs,count,job_id);
            else
                complete_job(jobs,count,job_id);
        }
        else if(choice==5) {
            schedule_fcfs(jobs,count);
        }
        else if(choice==6) {
            printf("Enter time quantum: ");

            if(fgets(input,sizeof(input),stdin)==NULL)
                break;

            errno=0;
            long q=strtol(input,&end,10);

            while(*end==' ' || *end=='\t' || *end=='\n')
                end++;

            if(end==input || errno==ERANGE || *end!='\0' ||
               q<=0 || q>2147483647) {
                printf("Invalid time quantum.\n");
                continue;
            }

            schedule_round_robin(jobs,count,(int)q);
        }
        else if(choice==7) {
            schedule_priority(jobs,count);
        }
        else if(choice==8) {
            run_ipc_demo();
        }
        else {
            printf("Exiting CampusCompute.\n");
            break;
        }
    }

    logger_stop();

    return 0;
}
