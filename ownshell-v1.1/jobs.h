#pragma once

#include "typedef.h"
#include "cstr.h"
#include "vec.h"

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} JobStatus;

typedef struct {
    i32 job_id;
    i32 pid;
    JobStatus status;
    cstr command;
} Job;

typedef struct {
    Job *data;
    u32 size;
    u32 capacity;
} JobList;

i32 jobs_add(i32 pid, JobStatus status, cstr *command);
i32 jobs_remove(i32 job_id);
JobList jobs_list();
// void job_print(Job *job, i32 status);