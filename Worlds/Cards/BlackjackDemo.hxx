// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BLACKJACK_DEMO_HXX
#define CRABS_BLACKJACK_DEMO_HXX
#include <_Config.h>
#include <_Package.hxx>
#include "Card.hxx"
#include "CardStack.hxx"
#include "Deck.hxx"
#include "Hand.hxx"
#include "Player.hxx"
#include "Dealer.hxx"
#include "Blackjack.hxx"
#include "BlackjackEnv.hxx"
#include "../../Gym.hxx"
#include "../../PPO.hxx"
#if _Seam >= _iGeekCardsWorldBlackjack
namespace iGeek {
namespace CardsWorld {
using namespace ::_;

/** Headless one-decision Blackjack adapter; owns flat batch buffers. */
class TBlackjackDemoGym : public Gym {
 public:
  enum { Batch = 8, Features = 30 };
  BlackjackEnv envs[Batch];
  FPC observation[Batch * Features] = {}, reward[Batch] = {}, done[Batch] = {};
  ISC action[Batch] = {};
  IUD rng;
  explicit TBlackjackDemoGym(IUD seed) : Gym("blackjack-demo"), rng(seed) {
    env_count_ = Batch; observation_len_ = Features; action_count_ = 2;
    observations_ = observation; rewards_ = reward; terminals_ = done;
    actions_ = action;
    /* The manifest is the in-Room contract a joining agent reads first. */
    TSPrint(manifest_.name, ISW(ManifestNameLen), "blackjack-demo");
    TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
    manifest_.kind = ManifestKindEnv;
    manifest_.domain = ManifestDomainGame;
    TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
            "One-decision Blackjack; hit or stand on a flat batch of tables.");
    TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
    manifest_.reward_low = FPC(-1.0);
    manifest_.reward_high = FPC(1.0);
    manifest_.reward_higher_is_better = true;
    manifest_.determinism = ManifestDeterminismSeeded;
    manifest_.observation_count = ISC(Features);
    manifest_.action_count = ISC(action_count_);
    manifest_.max_episode_steps = ISC(1);  //< One decision per episode.
    manifest_.environment_count_max = ISC(Batch);
    TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "CardsWorld");
    manifest_.seam_number = _iGeekCardsWorldBlackjackDemo;
    manifest_.manifest_version = ISC(1);
    ResetBatch();
  }
  /** Seeded uniform Fisher-Yates, independent of the legacy random helper. */
  IUC Draw(ISC bound) {
    IUC value, limit = IUC(-IUC(bound)) % IUC(bound);
    do {
      rng = rng * 6364136223846793005ULL + 1442695040888963407ULL;
      value = IUC(rng >> 32);
    } while (value < limit);
    return value % IUC(bound);
  }
  /** Replenish a fresh single deck; condition on a playable initial hand. */
  void Deal(ISC row) {
    BlackjackEnv& env = envs[row];
    do {
      CardsWorld::Dealer& dealer = env.Game().GetDealer();
      ISC order[52];
      for (ISC i = 0; i < 52; ++i) order[i] = i;
      for (ISC i = 51; i > 0; --i) {
        ISC j = ISC(Draw(i + 1)), temp = order[i];
        order[i] = order[j]; order[j] = temp;
      }
      while (!dealer.GetStock().IsEmpty()) dealer.GetStock().TakeNextCard();
      for (ISC i = 0; i < 52; ++i)
        dealer.GetStock().AddCard(dealer.GetDeck().GetCard(order[i]));
      env.Reset();
    } while (env.Done());
    for (ISC i = 0; i < Features; ++i) observation[row * Features + i] = 0;
    CardsWorld::Hand hand = env.Game().PlayerHand();
    ISC total = env.Game().HandValue(hand), hard = 0;
    for (ISC i = 0; i < hand.CardCount(); ++i)
      hard += hand.PeekCard(i)->PipValue() == 1 ? 1 :
              hand.PeekCard(i)->PointValue();
    CardsWorld::Card* up_card = env.Game().DealerHand().PeekCard(0);
    ISC up = up_card->PipValue() == 1 ? 1 : up_card->PointValue();
    observation[row * Features + total - 4] = 1;
    observation[row * Features + 18 + up - 1] = 1;
    observation[row * Features + 28 + (total > hard ? 1 : 0)] = 1;
  }
  void ResetBatch() override {
    for (ISC i = 0; i < Batch; ++i) {
      Deal(i); reward[i] = 0; done[i] = 0;
    }
  }
  void StepBatch() override {
    for (ISC i = 0; i < Batch; ++i) {
      reward[i] = FPC(envs[i].Step(action[i]));
      if (!envs[i].Done()) reward[i] += FPC(envs[i].Step(1));
      done[i] = 1;
      Deal(i);  // Next initial observation; preserve terminal reward.
    }
  }
};
/** Episode return moments, not per-step training-loss diagnostics. */
struct TBlackjackDemoScore
{
  ISC count = 0;
  FPD sum = 0, sum_sq = 0;
  FPD Mean() { return sum / FPD(count); }
  FPD Error() {
    return SquareRoot((sum_sq / FPD(count) - Mean() * Mean()) / FPD(count - 1));
  }
};
/** @param mode 0 greedy policy, 1 uniform random, 2 hit-below-17. */
inline TBlackjackDemoScore TBlackjackDemoEval(TLinearPolicy& policy,
    IUD seed, ISC batches, ISC mode) {
  TBlackjackDemoGym gym(seed);
  TBlackjackDemoScore score;
  IUD action_rng = seed ^ 0xabcdef123456789ULL;
  TensorFPC observation{gym.observation, gym.Batch, gym.Features, 0};
  TensorFPC logits = TensorFPCAlloc(gym.Batch, 2);
  TensorFPC values = TensorFPCAlloc(gym.Batch, 1);
  for (ISC batch = 0; batch < batches; ++batch) {
    if (mode == 0) policy.Forward(observation, logits, values);
    for (ISC i = 0; i < gym.Batch; ++i) {
      action_rng = action_rng * 6364136223846793005ULL +
                   1442695040888963407ULL;
      gym.action[i] = mode == 0 ? ISC(logits.At(i, 1) > logits.At(i, 0)) :
          mode == 1 ? ISC((action_rng >> 32) & 1) :
          ISC(gym.envs[i].Game().HandValue(gym.envs[i].Game().PlayerHand()) >= 17);
    }
    gym.StepBatch();
    for (ISC i = 0; i < gym.Batch; ++i) {
      FPD reward = FPD(gym.reward[i]);
      ++score.count; score.sum += reward; score.sum_sq += reward * reward;
    }
  }
  TensorFPCFree(logits); TensorFPCFree(values);
  return score;
}
inline void TBlackjackDemoTrain(PPO& trainer, ISC updates) {
  for (ISC i = 0; i < updates; ++i) trainer.TrainStep();
}
  
}       //< namespace CardsWorld
}       //< namespace iGeek
#endif  //< _Seam >= _iGeekCardsWorldBlackjackDemo
#endif  //< _iGeekCardsWorldBlackjackDemoHXX
