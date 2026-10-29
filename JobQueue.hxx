// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_JOB_QUEUE_HXX
#define IGEEK_JOB_QUEUE_HXX 1
#include "JobQueue.h"
namespace iGeek {

void JobSpecInit(JobSpec* job) {
  if (!job) return;
  job->job_id = IUC(0);
  for (ISN i = 0; i < 32; ++i) job->job_name[i] = 0;
  for (ISN i = 0; i < 32; ++i) job->env_name[i] = 0;
  job->seed = IUD(0);
  job->episode_count = ISC(0);
  job->max_frames = IUD(0);
  job->agent_count = ISC(0);
  job->reward_threshold = FPC(0.0f);
  job->status = JobPending;
  job->total_reward = FPD(0.0);
  job->frames_run = IUD(0);
  job->episodes_done = ISC(0);
}

void JobQueueInit(TJobQueue* queue) {
  if (!queue) return;
  queue->count = ISC(0);
  for (ISC i = 0; i < JobQueueMax; ++i) JobSpecInit(&queue->jobs[i]);
}

BOL JobEnqueue(TJobQueue* queue, const JobSpec* job) {
  if (!queue || !job || queue->count >= JobQueueMax) return false;
  queue->jobs[queue->count] = *job;
  queue->jobs[queue->count].status = JobPending;
  ++queue->count;
  return true;
}

BOL JobDequeue(TJobQueue* queue, JobSpec* out_job) {
  if (!queue || !out_job) return false;
  for (ISC i = 0; i < queue->count; ++i) {
    if (queue->jobs[i].status == JobPending) {
      *out_job = queue->jobs[i];
      queue->jobs[i].status = JobRunning;
      return true;
    }
  }
  return false;
}

BOL JobLookup(const TJobQueue* queue, IUC job_id, JobSpec* out_job) {
  if (!queue || !out_job) return false;
  for (ISC i = 0; i < queue->count; ++i) {
    if (queue->jobs[i].job_id == job_id) {
      *out_job = queue->jobs[i];
      return true;
    }
  }
  return false;
}

BOL JobSetStatus(TJobQueue* queue, IUC job_id, ISC status) {
  if (!queue) return false;
  for (ISC i = 0; i < queue->count; ++i) {
    if (queue->jobs[i].job_id == job_id) {
      queue->jobs[i].status = status;
      return true;
    }
  }
  return false;
}

ISC JobQueueCount(const TJobQueue* queue) {
  return queue ? queue->count : ISC(0);
}

ISC JobQueueCountByStatus(const TJobQueue* queue, ISC status) {
  if (!queue) return ISC(0);
  ISC n = 0;
  for (ISC i = 0; i < queue->count; ++i) {
    if (queue->jobs[i].status == status) ++n;
  }
  return n;
}

}  //< namespace iGeek
#endif
