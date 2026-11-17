
#include "scheduler.h"
#include <stdio.h>

void schedule_fcfs(const Job jobs[],size_t count) {
    int order[MAX_JOBS];
    long long completion[MAX_JOBS];
    long long total_tat=0,total_wt=0;
    long long current_time=0;

    if(jobs==NULL || count==0 || count>MAX_JOBS) {
        printf("No valid jobs available.\n");
        return;
    }

    for(size_t i=0;i<count;i++) {
        if(jobs[i].arrival_time<0 || jobs[i].burst_time<=0) {
            printf("Invalid job data.\n");
            return;
        }
        order[i]=(int)i;
    }

    for(size_t i=0;i<count-1;i++) {
        for(size_t j=0;j<count-i-1;j++) {
            int a=order[j];
            int b=order[j+1];

            if(jobs[a].arrival_time>jobs[b].arrival_time ||
              (jobs[a].arrival_time==jobs[b].arrival_time &&
               jobs[a].id>jobs[b].id)) {
                int temp=order[j];
                order[j]=order[j+1];
                order[j+1]=temp;
            }
        }
    }

    printf("\n===== FCFS Scheduling =====\n");
    printf("Tie-breaking: Earlier arrival, then smaller Job ID.\n");
    printf("\nExecution Timeline:\n");

    for(size_t i=0;i<count;i++) {
        int index=order[i];

        if(current_time<jobs[index].arrival_time) {
            printf("| IDLE (%lld-%d) ",current_time,
                   jobs[index].arrival_time);
            current_time=jobs[index].arrival_time;
        }

        long long start=current_time;
        current_time+=jobs[index].burst_time;
        completion[index]=current_time;

        printf("| J%d (%lld-%lld) ",jobs[index].id,
               start,current_time);
    }

    printf("|\n");

    printf("\n%-10s %-10s %-10s %-10s %-10s %-10s\n",
           "Job ID","Arrival","Burst","CT","TAT","WT");

    for(size_t i=0;i<count;i++) {
        int index=order[i];
        long long tat=completion[index]-jobs[index].arrival_time;
        long long wt=tat-jobs[index].burst_time;

        total_tat+=tat;
        total_wt+=wt;

        printf("%-10d %-10d %-10d %-10lld %-10lld %-10lld\n",
               jobs[index].id,jobs[index].arrival_time,
               jobs[index].burst_time,completion[index],tat,wt);
    }

    printf("\nAverage Turnaround Time: %.2f\n",
           (double)total_tat/count);
    printf("Average Waiting Time: %.2f\n",
           (double)total_wt/count);
}

void schedule_round_robin(const Job jobs[],size_t count,int quantum) {
    int order[MAX_JOBS];
    int queue[MAX_JOBS];
    int remaining[MAX_JOBS];
    long long completion[MAX_JOBS];
    long long total_tat=0,total_wt=0;
    int head=0,tail=0,qcount=0,front=0;
    long long current_time=0;

    if(jobs==NULL || count==0 || count>MAX_JOBS || quantum<=0) {
        printf("Invalid jobs or time quantum.\n");
        return;
    }

    for(size_t i=0;i<count;i++) {
        if(jobs[i].arrival_time<0 || jobs[i].burst_time<=0) {
            printf("Invalid job data.\n");
            return;
        }

        order[i]=(int)i;
        remaining[i]=jobs[i].burst_time;
    }

    for(size_t i=0;i<count-1;i++) {
        for(size_t j=0;j<count-i-1;j++) {
            int a=order[j];
            int b=order[j+1];

            if(jobs[a].arrival_time>jobs[b].arrival_time ||
              (jobs[a].arrival_time==jobs[b].arrival_time &&
               jobs[a].id>jobs[b].id)) {
                int temp=order[j];
                order[j]=order[j+1];
                order[j+1]=temp;
            }
        }
    }

    printf("\n===== Round Robin Scheduling =====\n");
    printf("Time Quantum: %d\n",quantum);
    printf("Tie-breaking: Earlier arrival, then smaller Job ID.\n");
    printf("\nExecution Timeline:\n");

    while(front<(int)count || qcount>0) {
        if(qcount==0) {
            int next=order[front];

            if(current_time<jobs[next].arrival_time) {
                printf("| IDLE (%lld-%d) ",current_time,
                       jobs[next].arrival_time);
                current_time=jobs[next].arrival_time;
            }

            while(front<(int)count &&
                  jobs[order[front]].arrival_time<=current_time) {
                queue[tail]=order[front];
                tail=(tail+1)%MAX_JOBS;
                qcount++;
                front++;
            }
        }

        int index=queue[head];
        head=(head+1)%MAX_JOBS;
        qcount--;

        int run_time=remaining[index]<quantum ?
                     remaining[index] : quantum;

        long long start=current_time;
        current_time+=run_time;
        remaining[index]-=run_time;

        printf("| J%d (%lld-%lld) ",jobs[index].id,
               start,current_time);

        while(front<(int)count &&
              jobs[order[front]].arrival_time<=current_time) {
            queue[tail]=order[front];
            tail=(tail+1)%MAX_JOBS;
            qcount++;
            front++;
        }

        if(remaining[index]>0) {
            queue[tail]=index;
            tail=(tail+1)%MAX_JOBS;
            qcount++;
        }
        else {
            completion[index]=current_time;
        }
    }

    printf("|\n");

    printf("\n%-10s %-10s %-10s %-10s %-10s %-10s\n",
           "Job ID","Arrival","Burst","CT","TAT","WT");

    for(size_t i=0;i<count;i++) {
        long long tat=completion[i]-jobs[i].arrival_time;
        long long wt=tat-jobs[i].burst_time;

        total_tat+=tat;
        total_wt+=wt;

        printf("%-10d %-10d %-10d %-10lld %-10lld %-10lld\n",
               jobs[i].id,jobs[i].arrival_time,
               jobs[i].burst_time,completion[i],tat,wt);
    }

    printf("\nAverage Turnaround Time: %.2f\n",
           (double)total_tat/count);
    printf("Average Waiting Time: %.2f\n",
           (double)total_wt/count);
}

void schedule_priority(const Job jobs[],size_t count) {
    int completed[MAX_JOBS]={0};
    long long completion[MAX_JOBS];
    long long current_time=0;
    long long total_tat=0,total_wt=0;
    size_t finished=0;

    if(jobs==NULL || count==0 || count>MAX_JOBS) {
        printf("No valid jobs available.\n");
        return;
    }

    for(size_t i=0;i<count;i++) {
        if(jobs[i].arrival_time<0 ||
           jobs[i].burst_time<=0 ||
           jobs[i].priority<=0) {
            printf("Invalid job data.\n");
            return;
        }
    }

    printf("\n===== Non-preemptive Priority Scheduling =====\n");
    printf("Smaller priority number means higher priority.\n");
    printf("Tie-breaking: Higher priority, then smaller Job ID.\n");
    printf("\nExecution Timeline:\n");

    while(finished<count) {
        int selected=-1;

        for(size_t i=0;i<count;i++) {
            if(completed[i] || jobs[i].arrival_time>current_time)
                continue;

            if(selected==-1 ||
               jobs[i].priority<jobs[selected].priority ||
               (jobs[i].priority==jobs[selected].priority &&
                jobs[i].id<jobs[selected].id)) {
                selected=(int)i;
            }
        }

        if(selected==-1) {
            int next=-1;

            for(size_t i=0;i<count;i++) {
                if(!completed[i] &&
                   (next==-1 ||
                    jobs[i].arrival_time<jobs[next].arrival_time ||
                    (jobs[i].arrival_time==jobs[next].arrival_time &&
                     jobs[i].id<jobs[next].id))) {
                    next=(int)i;
                }
            }

            printf("| IDLE (%lld-%d) ",current_time,
                   jobs[next].arrival_time);
            current_time=jobs[next].arrival_time;
            continue;
        }

        long long start=current_time;
        current_time+=jobs[selected].burst_time;
        completion[selected]=current_time;
        completed[selected]=1;
        finished++;

        printf("| J%d (%lld-%lld) ",
               jobs[selected].id,start,current_time);
    }

    printf("|\n");

    printf("\n%-10s %-10s %-10s %-10s %-10s %-10s\n",
           "Job ID","Arrival","Burst","Priority","CT","TAT");

    for(size_t i=0;i<count;i++) {
        long long tat=completion[i]-jobs[i].arrival_time;
        long long wt=tat-jobs[i].burst_time;

        total_tat+=tat;
        total_wt+=wt;

        printf("%-10d %-10d %-10d %-10d %-10lld %-10lld\n",
               jobs[i].id,jobs[i].arrival_time,
               jobs[i].burst_time,jobs[i].priority,
               completion[i],tat);
    }

    printf("\nWaiting Times:\n");

    for(size_t i=0;i<count;i++) {
        long long tat=completion[i]-jobs[i].arrival_time;
        long long wt=tat-jobs[i].burst_time;
        printf("Job %d: %lld\n",jobs[i].id,wt);
    }

    printf("\nAverage Turnaround Time: %.2f\n",
           (double)total_tat/count);
    printf("Average Waiting Time: %.2f\n",
           (double)total_wt/count);
}

