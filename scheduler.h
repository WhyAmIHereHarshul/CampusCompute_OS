
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "job.h"

void schedule_fcfs(const Job jobs[],size_t count);
void schedule_round_robin(const Job jobs[],size_t count,int quantum);
void schedule_priority(const Job jobs[],size_t count);

#endif

