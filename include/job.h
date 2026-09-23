#ifndef JOB_H
#define JOB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_JOBS 32

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE,
} JobStatus;

typedef struct {
    int job_id;
    pid_t pid;
    char *command;
    JobStatus status;
} Job;

typedef struct {
    Job jobs[MAX_JOBS];
    int num_jobs;
    int next_job_id;
} JobTable;

extern JobTable job_table;

// Function declarations
void job_table_init(void);
int add_job(pid_t pid, const char *cmd);
void remove_job(int job_id);
Job* find_job_by_pid(pid_t pid);
Job* find_job_by_id(int job_id);
void print_jobs(void);
void check_background_jobs(void);
void reap_zombie(pid_t pid);

#endif
