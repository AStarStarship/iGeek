// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_JOB_QUEUE_H
#define IGEEK_JOB_QUEUE_H
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "Script.h"
namespace iGeek {

/* Max jobs in the queue. */
enum { JobQueueMax = 16 };

/* Job lifecycle states. */
enum {
  JobPending = 0,
  JobRunning,
  JobDone,
  JobFailed,
};

/* A fixed-size job spec: one entry in the queue.
   The Room dequeues a job, spawns envs, runs episodes,
   collects telemetry (into the Script), marks Done. */
struct JobSpec {
  IUC job_id;
  CHA job_name[32];
  CHA env_name[32];
  IUD seed;
  ISC episode_count;
  IUD max_frames;
  ISC agent_count;
  FPC reward_threshold;
  ISC status;
  FPD total_reward;
  IUD frames_run;
  ISC episodes_done;
};

/* The job queue: a fixed-size array of JobSpec. */
struct TJobQueue {
  JobSpec jobs[JobQueueMax];
  ISC count;
};

/* Initializes the queue to empty. */
void JobQueueInit(TJobQueue* queue);

/* Initializes a single job spec to safe defaults. */
void JobSpecInit(JobSpec* job);

/* Enqueues a job. @return True on success, false if queue is full. */
BOL JobEnqueue(TJobQueue* queue, const JobSpec* job);

/* Dequeues the next Pending job. @return True on success, false if empty. */
BOL JobDequeue(TJobQueue* queue, JobSpec* out_job);

/* Looks up a job by id. @return True if found. */
BOL JobLookup(const TJobQueue* queue, IUC job_id, JobSpec* out_job);

/* Sets a job's status. @return True if job found. */
BOL JobSetStatus(TJobQueue* queue, IUC job_id, ISC status);

/* Returns the number of jobs in the queue. */
ISC JobQueueCount(const TJobQueue* queue);

/* Returns the number of jobs in a given status. */
ISC JobQueueCountByStatus(const TJobQueue* queue, ISC status);

}  //< namespace iGeek
#endif
#endif
