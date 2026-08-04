#include "jobs.h"

static JobList s_job_list = {0};

i32 jobs_add(i32 pid, JobStatus status, cstr *command) {
    Job new_job = {
        .job_id = s_job_list.size > 0 ? s_job_list.data[s_job_list.size - 1].job_id + 1 : 1,
        .pid = pid,
        .status = status,
        .command = {0}
    };
    cstr_copy(&new_job.command, command);
    vec_append(&s_job_list, new_job);
    return new_job.job_id;
}

i32 jobs_remove(i32 job_id) {
    for (u32 i = 0; i < s_job_list.size; ++i) {
        if (s_job_list.data[i].job_id == job_id) {
            cstr_free(&s_job_list.data[i].command);
            vec_remove(&s_job_list, i);
            return 0; // Success
        }
    }
    return -1; // Job not found
}

JobList jobs_list() {
    return s_job_list;
}

void job_print(Job *job, i32 status) {
    // char marker = ' ';
    // for (u32 i = 0; i < s_job_list.size; ++i) {
    //   if(i == s_job_list.size - 1) {
    //       marker = '+';
    //   } else if(i == s_job_list.size - 2) {
    //       marker = '-';
    //   } else {
    //       marker = ' ';
    //   }
    //   Job *job = &s_job_list.data[i];
    //   if(status == JOB_DONE && job->status == JOB_DONE) {
    //       printf("[%d]%c %s \t%s\n",
    //           job->job_id,
    //           marker,
    //           "Done",
    //           job->command.data);
    //       continue;
    //   }
    //   if(status == JOB_RUNNING && job->status == JOB_RUNNING) {
    //       printf("[%d]%c %s \t%s %c\n",
    //           job->job_id,
    //           marker,
    //           "Running",
    //           job->command.data, '&');
    //   } else if(status == JOB_STOPPED && job->status == JOB_STOPPED) {
    //       printf("[%d]%c %s \t%s %c\n",
    //           job->job_id,
    //           marker,
    //           "Stopped",
    //           job->command.data);
    //   }
    // }
}