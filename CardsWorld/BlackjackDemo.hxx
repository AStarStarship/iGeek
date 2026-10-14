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
#include "../Gym.hxx"
#include "../PPO.hxx"
#if SEAM >= CARDSWORLD_CORE
namespace _ {
/** Headless one-decision Blackjack adapter; owns flat batch buffers. */
class TBlackjackDemoGym : public Gym {
 public:
  enum { Batch = 8, Features = 30 };
  BlackjackEnv envs[Batch];
  FPC obs[Batch * Features] = {}, reward[Batch] = {}, done[Batch] = {};
  ISC action[Batch] = {};
  IUD rng;
  explicit TBlackjackDemoGym(IUD seed) : Gym("blackjack-demo"), rng(seed) {
    env_count_ = Batch; obs_len_ = Features; action_count_ = 2;
    observations_ = obs; rewards_ = reward; terminals_ = done;
    actions_ = action;
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
    for (ISC i = 0; i < Features; ++i) obs[row * Features + i] = 0;
    const CardsWorld::Hand& hand = env.Game().PlayerHand();
    ISC total = env.Game().HandValue(hand), hard = 0;
    for (ISC i = 0; i < hand.CardCount(); ++i)
      hard += hand.PeekCard(i)->PipValue() == 1 ? 1 :
              hand.PeekCard(i)->PointValue();
    const CardsWorld::Card* up_card = env.Game().DealerHand().PeekCard(0);
    ISC up = up_card->PipValue() == 1 ? 1 : up_card->PointValue();
    obs[row * Features + total - 4] = 1;
    obs[row * Features + 18 + up - 1] = 1;
    obs[row * Features + 28 + (total > hard ? 1 : 0)] = 1;
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
  FPD Mean() const { return sum / FPD(count); }
  FPD Error() const {
    return sqrt((sum_sq / FPD(count) - Mean() * Mean()) / FPD(count - 1));
  }
};
/** @param mode 0 greedy policy, 1 uniform random, 2 hit-below-17. */
inline TBlackjackDemoScore TBlackjackDemoEval(TLinearPolicy& policy,
    IUD seed, ISC batches, ISC mode) {
  TBlackjackDemoGym gym(seed);
  TBlackjackDemoScore score;
  IUD action_rng = seed ^ 0xabcdef123456789ULL;
  TTensor obs{gym.obs, gym.Batch, gym.Features, 0};
  TTensor logits = TTensorAlloc(gym.Batch, 2);
  TTensor values = TTensorAlloc(gym.Batch, 1);
  for (ISC batch = 0; batch < batches; ++batch) {
    if (mode == 0) policy.Forward(obs, logits, values);
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
  TTensorFree(logits); TTensorFree(values);
  return score;
}
inline void TBlackjackDemoTrain(TPPO& trainer, ISC updates) {
  for (ISC i = 0; i < updates; ++i) trainer.TrainStep();
}
}  //< namespace _
#endif
#endif
