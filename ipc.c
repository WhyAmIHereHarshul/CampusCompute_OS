
#define _POSIX_C_SOURCE 200809L

#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#define MESSAGE_SIZE 256

typedef struct {
    char message[MESSAGE_SIZE];
} SharedData;

int run_ipc_demo(void) {
    char shm_name[64];
    char sem_name[64];

    int shm_fd=-1;
    SharedData *shared=NULL;
    sem_t *semaphore=SEM_FAILED;
    pid_t pid;
    int status;
    int success=0;

    snprintf(shm_name,sizeof(shm_name),
             "/campus_shm_%ld",(long)getpid());

    snprintf(sem_name,sizeof(sem_name),
             "/campus_sem_%ld",(long)getpid());

    shm_fd=shm_open(shm_name,O_CREAT|O_EXCL|O_RDWR,0600);

    if(shm_fd==-1) {
        perror("Shared memory creation failed");
        goto cleanup;
    }

    if(ftruncate(shm_fd,sizeof(SharedData))==-1) {
        perror("Shared memory sizing failed");
        goto cleanup;
    }

    shared=mmap(NULL,sizeof(SharedData),
                PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);

    if(shared==MAP_FAILED) {
        shared=NULL;
        perror("Shared memory mapping failed");
        goto cleanup;
    }

    semaphore=sem_open(sem_name,O_CREAT|O_EXCL,0600,0);

    if(semaphore==SEM_FAILED) {
        perror("Semaphore creation failed");
        goto cleanup;
    }

    fflush(stdout);

    pid=fork();

    if(pid==-1) {
        perror("Fork failed");
        goto cleanup;
    }

    if(pid==0) {
        printf("[CHILD] Waiting for parent message...\n");

        while(sem_wait(semaphore)==-1) {
            if(errno!=EINTR) {
                perror("[CHILD] Semaphore wait failed");
                munmap(shared,sizeof(SharedData));
                close(shm_fd);
                sem_close(semaphore);
                _exit(EXIT_FAILURE);
            }
        }

        printf("[CHILD] Message received: %s\n",shared->message);

        munmap(shared,sizeof(SharedData));
        close(shm_fd);
        sem_close(semaphore);

        _exit(EXIT_SUCCESS);
    }

    printf("[PARENT] Shared memory created.\n");
    printf("[PARENT] Writing message to shared memory.\n");

    snprintf(shared->message,MESSAGE_SIZE,
             "Hello from CampusCompute parent process!");

    if(sem_post(semaphore)==-1) {
        perror("Semaphore signal failed");
        kill(pid,SIGTERM);
        while(waitpid(pid,&status,0)==-1 && errno==EINTR);
        goto cleanup;
    }

    printf("[PARENT] Message sent to child.\n");

    while(waitpid(pid,&status,0)==-1) {
        if(errno!=EINTR) {
            perror("waitpid failed");
            goto cleanup;
        }
    }

    if(WIFEXITED(status) && WEXITSTATUS(status)==EXIT_SUCCESS) {
        printf("[PARENT] Child process completed successfully.\n");
        success=1;
    }
    else {
        printf("[PARENT] Child process did not complete successfully.\n");
    }

cleanup:
    if(shared!=NULL)
        munmap(shared,sizeof(SharedData));

    if(shm_fd!=-1)
        close(shm_fd);

    if(semaphore!=SEM_FAILED)
        sem_close(semaphore);

    if(shm_fd!=-1 && shm_unlink(shm_name)==-1)
        perror("Shared memory cleanup failed");

    if(semaphore!=SEM_FAILED && sem_unlink(sem_name)==-1)
        perror("Semaphore cleanup failed");

    printf("[IPC] Shared memory and semaphore cleanup completed.\n");

    return success;
}
