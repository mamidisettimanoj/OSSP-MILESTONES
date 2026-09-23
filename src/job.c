#include "../include/job.h"

JobTable job_table = {0};

void job_table_init(void) {
    job_table.num_jobs = 0;
    job_table.next_job_id = 1;
    memset(job_table.jobs, 0, sizeof(job_table.jobs));
}

int add_job(pid_t pid, const char *cmd) {
    if (job_table.num_jobs >= MAX_JOBS) {
        fprintf(stderr, "Job table full\n");
        return -1;
    }
    
    int job_id = job_table.next_job_id++;
    Job *job = &job_table.jobs[job_table.num_jobs];
    
    job->job_id = job_id;
    job->pid = pid;
    job->status = JOB_RUNNING;
    job->command = (char *)malloc(strlen(cmd) + 1);
    if (job->command == NULL) {
        perror("malloc");
        return -1;
    }
    strcpy(job->command, cmd);
    
    job_table.num_jobs++;
    printf("[%d] %d\n", job_id, pid);
    
    return job_id;
}

void remove_job(int job_id) {
    for (int i = 0; i < job_table.num_jobs; i++) {
        if (job_table.jobs[i].job_id == job_id) {
            free(job_table.jobs[i].command);
            
            for (int j = i; j < job_table.num_jobs - 1; j++) {
                job_table.jobs[j] = job_table.jobs[j + 1];
            }
            
            job_table.num_jobs--;
            return;
        }
    }
}

Job* find_job_by_pid(pid_t pid) {
    for (int i = 0; i < job_table.num_jobs; i++) {
        if (job_table.jobs[i].pid == pid) {
            return &job_table.jobs[i];
        }
    }
    return NULL;
}

Job* find_job_by_id(int job_id) {
    for (int i = 0; i < job_table.num_jobs; i++) {
        if (job_table.jobs[i].job_id == job_id) {
            return &job_table.jobs[i];
        }
    }
    return NULL;
}

void print_jobs(void) {
    if (job_table.num_jobs == 0) {
        printf("No jobs\n");
        return;
    }
    
    for (int i = 0; i < job_table.num_jobs; i++) {
        Job *job = &job_table.jobs[i];
        const char *status_str = "Unknown";
        
        switch (job->status) {
            case JOB_RUNNING:
                status_str = "Running";
                break;
            case JOB_STOPPED:
                status_str = "Stopped";
                break;
            case JOB_DONE:
                status_str = "Done";
                break;
        }
        
        printf("[%d]  %s    %d    %s\n", job->job_id, status_str, job->pid, job->command);
    }
}

void check_background_jobs(void) {
    for (int i = 0; i < job_table.num_jobs; i++) {
        Job *job = &job_table.jobs[i];
        
        if (job->status == JOB_RUNNING) {
            int status;
            pid_t result = waitpid(job->pid, &status, WNOHANG);
            
            if (result == job->pid) {
                if (WIFEXITED(status)) {
                    job->status = JOB_DONE;
                    printf("[%d]+  Done                    %s\n", job->job_id, job->command);
                } else if (WIFSIGNALED(status)) {
                    job->status = JOB_DONE;
                    int sig = WTERMSIG(status);
                    printf("[%d]+  Terminated by signal %d %s\n", job->job_id, sig, job->command);
                }
            }
        }
    }
    
    // Remove completed jobs
    for (int i = 0; i < job_table.num_jobs; i++) {
        if (job_table.jobs[i].status == JOB_DONE) {
            remove_job(job_table.jobs[i].job_id);
            i--;
        }
    }
}

void reap_zombie(pid_t pid) {
    int status;
    waitpid(pid, &status, 0);
}
