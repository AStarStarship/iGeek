// Copyright AStarship <https://astarship.net>.
namespace iGeek {
ISC core_fails = 0;

void CoreCheck(BOL ok, const CHA* what) {
  if (!ok) {
    core_fails += 1;
    D_COUT("\ncore FAIL:" << what);
  }
}

/* A minimal vectorized Gym: obs = [sin(step), cos(step)], one discrete
action, reward = 1 when action matches a target that flips every 8
steps. Deterministic per step index (no RNG). Allocates the batch
buffers in its ctor (the established Gym subclass pattern). */
class TCoreToyGym : public Gym {
 public:
  TCoreToyGym() : Gym("core_toy"), step_(0), target_(0) {
    env_count_ = 4;
    obs_len_ = 2;
    action_count_ = 2;
    observations_ = new FPC[env_count_ * obs_len_]();
    actions_ = new ISC[env_count_]();
    rewards_ = new FPC[env_count_]();
    terminals_ = new FPC[env_count_]();
    action_mask_ = new FPC[env_count_ * action_count_]();
  }

  ~TCoreToyGym() {
    delete[] observations_;
    delete[] actions_;
    delete[] rewards_;
    delete[] terminals_;
    delete[] action_mask_;
  }

  void ResetBatch() override {
    step_ = 0;
    target_ = 0;
    for (ISC b = 0; b < env_count_; ++b) {
      observations_[b * 2 + 0] = 0.0f;
      observations_[b * 2 + 1] = 1.0f;
      terminals_[b] = 0.0f;
      action_mask_[b * 2 + 0] = 1.0f;
      action_mask_[b * 2 + 1] = 1.0f;
    }
  }

  void StepBatch() override {
    for (ISC b = 0; b < env_count_; ++b) {
      ISC a = actions_[b];
      rewards_[b] = (a == target_) ? 1.0f : 0.0f;
      terminals_[b] = 0.0f;
      FPC s = FPC(step_) * 0.5f;
      observations_[b * 2 + 0] = FPC(sin(s));
      observations_[b * 2 + 1] = FPC(cos(s));
    }
    step_ += 1;
    if (step_ % 8 == 0) {
      target_ ^= 1;
    }
  }

 private:
  ISC step_;
  ISC target_;
};

inline const CHA* Core(const CHA* args) {
  //1. TTensor ops
  TTensor A = TTensorAlloc(2, 2);
  TTensor B = TTensorAlloc(2, 2);
  TTensor C = TTensorAlloc(2, 2);
  A.At(0, 0) = 1.0f; A.At(0, 1) = 2.0f;
  A.At(1, 0) = 3.0f; A.At(1, 1) = 4.0f;
  B.At(0, 0) = 5.0f; B.At(0, 1) = 6.0f;
  B.At(1, 0) = 7.0f; B.At(1, 1) = 8.0f;
  TTensorMatMul(A, B, C);
  CoreCheck(C.AtC(0, 0) == 19.0f, "matmul(0,0)");
  CoreCheck(C.AtC(0, 1) == 22.0f, "matmul(0,1)");
  CoreCheck(C.AtC(1, 0) == 43.0f, "matmul(1,0)");
  CoreCheck(C.AtC(1, 1) == 50.0f, "matmul(1,1)");
  TTensorFree(A); TTensorFree(B); TTensorFree(C);

  TTensor S = TTensorAlloc(1, 3);
  S.At(0, 0) = 1.0f; S.At(0, 1) = 1.0f; S.At(0, 2) = 1.0f;
  TTensorSoftmaxInPlace(S);
  FPC sx = S.AtC(0, 0) + S.AtC(0, 1) + S.AtC(0, 2);
  CoreCheck(sx > 0.999f && sx < 1.001f, "softmax sums to 1");
  TTensorFree(S);

  // 2. TLinearPolicy: forward deterministic, sampling valid
  TLinearPolicy policy(2, 2, IUD(12345));
  TTensor obs = TTensorAlloc(2, 2);
  obs.At(0, 0) = 0.1f; obs.At(0, 1) = 0.2f;
  obs.At(1, 0) = 0.3f; obs.At(1, 1) = 0.4f;
  TTensor logits = TTensorAlloc(2, 2);
  TTensor values = TTensorAlloc(2, 1);
  policy.Forward(obs, logits, values);
  TTensor logits2 = TTensorAlloc(2, 2);
  TTensor values2 = TTensorAlloc(2, 1);
  policy.Forward(obs, logits2, values2);
  BOL fwd_same = true;
  for (ISC i = 0; i < 4; ++i) {
    if (logits.AtC(i / 2, i % 2) != logits2.AtC(i / 2, i % 2)) 
      fwd_same = false;
  }
  CoreCheck(fwd_same, "linear forward deterministic");

  ISC acts[2];
  FPC logp[2];
  policy.SampleActions(logits, acts, logp);
  BOL acts_valid = (acts[0] >= 0 && acts[0] < 2) && 
                   (acts[1] >= 0 && acts[1] < 2);
  CoreCheck(acts_valid, "sampled actions in range");
  TTensorFree(obs); TTensorFree(logits); TTensorFree(values);
  TTensorFree(logits2); TTensorFree(values2);

  // 3. TPPO over the toy Gym: finite stats, no crash
  TCoreToyGym gym;
  gym.ResetBatch();

  TLinearPolicy ppolicy(2, 2, IUD(777));
  TPPO trainer(gym, ppolicy, /*horizon=*/8, /*epochs=*/1);
  TPPOStats st = trainer.TrainStep();
  BOL finite = isfinite(st.policy_loss_) && isfinite(st.value_loss_) &&
               isfinite(st.entropy_) && isfinite(st.approx_kl_);
  CoreCheck(finite, "ppo stats finite");

  // 4. Agent tiers
  // TTableAgent: history -> action via bound entries.
  {
    TTableAgent agent(/*action_count=*/2, /*history_len=*/1, /*percept_len=*/1);
    IUC key = 0;
    agent.SetEntry(&key, 1);
    IUC percept = 0;
    agent.OnPercept(&percept, 1);
    IUC a = agent.Action();
    CoreCheck(a == 1, "table agent lookup");
  }
  // TReflexAgent: condition-action on current percept.
  {
    TReflexAgent agent(/*action_count=*/2, /*percept_len=*/1, /*rule_cap=*/4);
    IUC cond = 0;
    BOL added = agent.AddRule(&cond, 1);
    CoreCheck(added, "reflex rule added");
    IUC percept = 0;
    agent.OnPercept(&percept, 1);
    IUC a = agent.Action();
    CoreCheck(a == 1, "reflex agent rule fires");
  }
  // TModelAgent: internal state accumulates.
  {
    TModelAgent agent(/*action_count=*/2, /*state_slots=*/4);
    IUC percept = 7;
    agent.OnPercept(&percept, 1);
    agent.OnPercept(&percept, 1);
    const AgentState* st_ = agent.State();
    CoreCheck(st_ != NILP, "model agent exposes state");
    if (st_ != NILP) {
      IUC w0 = st_->Get(0);
      CoreCheck(w0 != 0, "model agent state accumulates");
    }
  }

  // 5. LinearProbe end-to-end on a separable target
  {
    StateRecordSet records(/*cap=*/16);
    // Class 0: word0 in [0..3]; Class 1: word0 in [40..43]. Widely
    // separated so full-batch logistic descent converges quickly.
    for (ISC i = 0; i < 8; ++i) {
      IUC w[4] = {IUC(i), 1, 1, 1};
      AgentState as(4);
      for (ISC k = 0; k < 4; ++k) as.Set(k, w[k]);
      records.Push(as, 0);
    }
    for (ISC i = 0; i < 8; ++i) {
      IUC w[4] = {IUC(40 + i), 1, 1, 1};
      AgentState as(4);
      for (ISC k = 0; k < 4; ++k) as.Set(k, w[k]);
      records.Push(as, 1);
    }
    CoreCheck(records.Count() == 16, "records captured");
    LinearProbe probe(4);
    FPC acc = probe.Train(records, /*epochs=*/500, /*lr=*/0.5f);
    CoreCheck(acc > 0.99f, "linear probe trains to >99%");
    FPC eval_acc = probe.Accuracy(records);
    CoreCheck(eval_acc > 0.99f, "linear probe evaluates >99%");
  }

  if (core_fails != 0) {
    return "iGeek_core_test_failure";
  }
  D_COUT("\niGeek core: all checks passed\n");
  return NILP;
}

} //< namespace iGeek
