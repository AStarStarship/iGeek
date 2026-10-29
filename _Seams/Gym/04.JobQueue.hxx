// Copyright AStarship <https://astarship.net>.
//
// Gym TJobQueue seam unit: enqueue/dequeue/lookup/status round-trip.
// Gated on IGEEK_GYM_MANIFEST.
#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "../../JobQueue.hxx"
namespace iGeekGymTest {
using namespace iGeek;

ISC j_pass = 0;
ISC j_fail = 0;

inline void JCheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    j_pass++;
    ::_::StdOut() << "PASS " << what << '\n';
  } else {
    j_fail++;
    ::_::StdOut() << "FAIL " << what << '\n';
  }
}

inline const CHA* JobQueue(const CHA* args) {
  (void)args;
  A_TEST_BEGIN;
  ::_::StdOut() << Headingf("Testing Gym TJobQueue (enqueue/dequeue/lookup)...");

  TJobQueue queue;
  JobQueueInit(&queue);

  /* Initially empty. */
  JCheck(JobQueueCount(&queue) == ISC(0), "initially empty");

  /* Enqueue 3 jobs. */
  JobSpec j1;
  JobSpecInit(&j1);
  j1.job_id = IUC(1);
  CopyStr(j1.job_name, 32, "ppo_run");
  CopyStr(j1.env_name, 32, "car_world");
  j1.seed = IUD(42);
  j1.episode_count = ISC(10);
  j1.max_frames = IUD(5000);
  j1.agent_count = ISC(8);
  j1.reward_threshold = FPC(0.8f);
  JCheck(JobEnqueue(&queue, &j1), "enqueue job 1");

  JobSpec j2;
  JobSpecInit(&j2);
  j2.job_id = IUC(2);
  CopyStr(j2.job_name, 32, "qlearn_run");
  CopyStr(j2.env_name, 32, "maze_world");
  j2.seed = IUD(7);
  j2.episode_count = ISC(5);
  j2.max_frames = IUD(2000);
  j2.agent_count = ISC(4);
  j2.reward_threshold = FPC(0.5f);
  JCheck(JobEnqueue(&queue, &j2), "enqueue job 2");

  JobSpec j3;
  JobSpecInit(&j3);
  j3.job_id = IUC(3);
  CopyStr(j3.job_name, 32, "baseline");
  CopyStr(j3.env_name, 32, "car_world");
  j3.seed = IUD(1);
  j3.episode_count = ISC(3);
  j3.max_frames = IUD(1000);
  j3.agent_count = ISC(8);
  j3.reward_threshold = FPC(0.3f);
  JCheck(JobEnqueue(&queue, &j3), "enqueue job 3");

  JCheck(JobQueueCount(&queue) == ISC(3), "count is 3 after 3 enqueues");
  JCheck(JobQueueCountByStatus(&queue, JobPending) == ISC(3),
         "3 pending");

  /* Dequeue: should get job 1 (first pending). */
  JobSpec out;
  JobSpecInit(&out);
  JCheck(JobDequeue(&queue, &out), "dequeue job");
  JCheck(out.job_id == IUC(1), "dequeued job 1");
  JCheck(out.job_name[0] == 'p' && out.job_name[1] == 'p',
         "job 1 name ppo_run");
  JCheck(out.seed == IUD(42), "job 1 seed 42");
  JCheck(out.episode_count == ISC(10), "job 1 episode_count 10");

  /* Job 1 is now Running in the queue. */
  JobSpec lookup;
  JCheck(JobLookup(&queue, IUC(1), &lookup), "lookup job 1");
  JCheck(lookup.status == JobRunning, "job 1 status Running");

  /* Dequeue: should get job 2. */
  JobSpecInit(&out);
  JCheck(JobDequeue(&queue, &out), "dequeue job 2");
  JCheck(out.job_id == IUC(2), "dequeued job 2");

  /* Mark job 2 as Done. */
  JCheck(JobSetStatus(&queue, IUC(2), JobDone), "set job 2 Done");
  JCheck(JobQueueCountByStatus(&queue, JobDone) == ISC(1), "1 Done");

  /* Dequeue: should get job 3. */
  JobSpecInit(&out);
  JCheck(JobDequeue(&queue, &out), "dequeue job 3");
  JCheck(out.job_id == IUC(3), "dequeued job 3");

  /* No more pending jobs. */
  JobSpecInit(&out);
  JCheck(!JobDequeue(&queue, &out), "dequeue fails when no pending");

  /* 2 Running, 1 Done. */
  JCheck(JobQueueCountByStatus(&queue, JobRunning) == ISC(2), "2 Running");
  JCheck(JobQueueCountByStatus(&queue, JobDone) == ISC(1), "1 Done (recheck)");

  /* Mark job 1 as Done, job 3 as Failed. */
  JCheck(JobSetStatus(&queue, IUC(1), JobDone), "set job 1 Done");
  JCheck(JobSetStatus(&queue, IUC(3), JobFailed), "set job 3 Failed");
  JCheck(JobQueueCountByStatus(&queue, JobDone) == ISC(2), "2 Done");
  JCheck(JobQueueCountByStatus(&queue, JobFailed) == ISC(1), "1 Failed");

  /* Lookup non-existent job. */
  JCheck(!JobLookup(&queue, IUC(99), &lookup), "lookup non-existent fails");

  /* Enqueue when full. */
  TJobQueue full;
  JobQueueInit(&full);
  for (ISC i = 0; i < JobQueueMax; ++i) {
    JobSpec f;
    JobSpecInit(&f);
    f.job_id = IUC(i + 100);
    JobEnqueue(&full, &f);
  }
  JCheck(JobQueueCount(&full) == JobQueueMax, "queue is full");
  JobSpec overflow;
  JobSpecInit(&overflow);
  overflow.job_id = IUC(999);
  JCheck(!JobEnqueue(&full, &overflow), "enqueue fails when full");

  /* Null safety. */
  JCheck(!JobEnqueue(NILP, &j1), "null queue enqueue fails");
  JCheck(!JobDequeue(NILP, &out), "null queue dequeue fails");
  JCheck(!JobLookup(NILP, IUC(1), &lookup), "null queue lookup fails");
  JCheck(JobQueueCount(NILP) == ISC(0), "null queue count is 0");

  ::_::StdOut() << "j_pass=" << j_pass << " j_fail=" << j_fail << '\n';
  return j_fail ? "GYM_JOB_QUEUE_FAIL: " : NILP;
}

}  //< namespace iGeekGymTest
#endif
