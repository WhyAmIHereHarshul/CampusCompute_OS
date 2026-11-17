
#define _POSIX_C_SOURCE 200809L

#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define QUEUE_SIZE 10
#define MESSAGE_SIZE 256

typedef struct {
    char message[MESSAGE_SIZE];
} LogEvent;

static LogEvent queue[QUEUE_SIZE];
static int front=0,rear=0,count=0;
static int shutdown_requested=0;
static int logger_running=0;

static FILE *log_file=NULL;

static pthread_mutex_t queue_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_empty=PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_full=PTHREAD_COND_INITIALIZER;

static pthread_t consumer_thread;

static void *consumer(void *arg) {
    (void)arg;

    while(1) {
        LogEvent event;

        pthread_mutex_lock(&queue_mutex);

        while(count==0 && !shutdown_requested)
            pthread_cond_wait(&not_empty,&queue_mutex);

        if(count==0 && shutdown_requested) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        event=queue[front];
        front=(front+1)%QUEUE_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&queue_mutex);

        printf("[CONSUMER] Processing event: %s\n",event.message);

        time_t now=time(NULL);
        struct tm time_info;
        char timestamp[32];

        localtime_r(&now,&time_info);
        strftime(timestamp,sizeof(timestamp),
                 "%Y-%m-%d %H:%M:%S",&time_info);

        fprintf(log_file,"[%s] %s\n",timestamp,event.message);
        fflush(log_file);
    }

    printf("[CONSUMER] All pending events processed. Thread exiting.\n");

    return NULL;
}

int logger_start(void) {
    if(logger_running) {
        printf("Logger is already running.\n");
        return 0;
    }

    log_file=fopen("activity.log","a");

    if(log_file==NULL) {
        perror("Unable to open activity.log");
        return 0;
    }

    front=0;
    rear=0;
    count=0;
    shutdown_requested=0;

    int result=pthread_create(&consumer_thread,NULL,consumer,NULL);

    if(result!=0) {
        fprintf(stderr,"Unable to create consumer thread: %s\n",
                strerror(result));
        fclose(log_file);
        log_file=NULL;
        return 0;
    }

    logger_running=1;
    printf("Activity logger started.\n");
    return 1;
}

int logger_log(const char *message) {
    if(message==NULL || !logger_running)
        return 0;

    int result=pthread_mutex_lock(&queue_mutex);

    if(result!=0) {
        fprintf(stderr,"Mutex lock failed: %s\n",strerror(result));
        return 0;
    }

    while(count==QUEUE_SIZE && !shutdown_requested)
        pthread_cond_wait(&not_full,&queue_mutex);

    if(shutdown_requested) {
        pthread_mutex_unlock(&queue_mutex);
        return 0;
    }

    snprintf(queue[rear].message,MESSAGE_SIZE,"%s",message);

    printf("[PRODUCER] Event added to queue: %s\n",message);

    rear=(rear+1)%QUEUE_SIZE;
    count++;

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&queue_mutex);

    return 1;
}

void logger_stop(void) {
    if(!logger_running)
        return;

    pthread_mutex_lock(&queue_mutex);

    shutdown_requested=1;

    pthread_cond_broadcast(&not_empty);
    pthread_cond_broadcast(&not_full);

    pthread_mutex_unlock(&queue_mutex);

    int result=pthread_join(consumer_thread,NULL);

    if(result!=0)
        fprintf(stderr,"Unable to join consumer thread: %s\n",
                strerror(result));

    if(log_file!=NULL) {
        fclose(log_file);
        log_file=NULL;
    }

    logger_running=0;
    printf("Activity logger stopped.\n");
}
