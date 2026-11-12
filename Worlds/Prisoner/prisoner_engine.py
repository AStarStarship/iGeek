# Copyright AStarship <https://astarship.net>.
"""Prisoner's Dilemma game engine — Python port of PrisonerWorld.h.

Native C++ reference:
  Worlds/Prisoner/PrisonerWorld.h  — PrisonerEnvironment class

The native Step() uses an achieved/desired goal semantics that is not
standard PD.  This port implements the canonical payoff matrix:

    Both C   -> both get R
    Both D   -> both get P
    I C, you D -> I get S, you get T
    I D, you C -> I get T, you get S

Native C++ has no native Gym equivalent for the browser demo; this is a
direct Python port.  No ctypes bridge is used because the C++ Step()
semantics (agent_history_ as "achieved", agent_desired_ as "desired")
diverge from the standard PD payoff contract that the browser UI needs.
"""
import random


class PrisonerGame:
  """One session of repeated Prisoner's Dilemma."""

  C = 0
  D = 1

  def __init__(self, T=5, R=3, P=1, S=0, max_rounds=64, ai='tit-for-tat'):
    self.payoff_T, self.payoff_R = T, R
    self.payoff_P, self.payoff_S = P, S
    self.max_rounds = max_rounds
    self.ai_strategy = ai
    self._reset()

  def _reset(self):
    self.round = 0
    self.player_total = 0
    self.ai_total = 0
    self.history = []          # list of (player_action, ai_action, player_reward, ai_reward)
    self.done = False

  def _ai_action(self):
    if self.round == 0:
      if self.ai_strategy == 'random':
        return random.choice((self.C, self.D))
      if self.ai_strategy == 'always-defect':
        return self.D
      if self.ai_strategy == 'always-cooperate':
        return self.C
      return self.C
    last = self.history[-1][1]   # AI's last action
    if self.ai_strategy == 'random':
      return random.choice((self.C, self.D))
    if self.ai_strategy == 'always-defect':
      return self.D
    if self.ai_strategy == 'always-cooperate':
      return self.C
    if self.ai_strategy == 'grim-trigger':
      # Defect forever once the player has defected.
      if any(h[0] == self.D for h in self.history):
        return self.D
      return self.C
    # tit-for-tat (default): mirror the player's last move
    return self.history[-1][0]

  def _payoff(self, me, opp):
    if me == self.C and opp == self.C:
      return self.payoff_R
    if me == self.D and opp == self.D:
      return self.payoff_P
    if me == self.C and opp == self.D:
      return self.payoff_S
    return self.payoff_T       # me=D, opp=C

  def step(self, player_action):
    if self.done:
      raise ValueError('Episode is over; call reset().')
    if player_action not in (self.C, self.D):
      raise ValueError('Action must be 0 (Cooperate) or 1 (Defect).')
    ai_action = self._ai_action()
    player_reward = self._payoff(player_action, ai_action)
    ai_reward = self._payoff(ai_action, player_action)
    self.player_total += player_reward
    self.ai_total += ai_reward
    self.history.append((player_action, ai_action, player_reward, ai_reward))
    self.round += 1
    if self.round >= self.max_rounds:
      self.done = True
    return {
      'round': self.round,
      'player_action': player_action,
      'ai_action': ai_action,
      'player_reward': player_reward,
      'ai_reward': ai_reward,
      'player_total': self.player_total,
      'ai_total': self.ai_total,
      'done': self.done,
    }

  def reset(self, ai=None):
    if ai:
      self.ai_strategy = ai
    self._reset()

  def state(self):
    return {
      'round': self.round,
      'max_rounds': self.max_rounds,
      'player_total': self.player_total,
      'ai_total': self.ai_total,
      'done': self.done,
      'ai_strategy': self.ai_strategy,
      'payoffs': {'T': self.payoff_T, 'R': self.payoff_R,
                  'P': self.payoff_P, 'S': self.payoff_S},
      'history': [
        {'player': h[0], 'ai': h[1],
         'player_reward': h[2], 'ai_reward': h[3]}
        for h in self.history
      ],
      'payoff_matrix': {
        'CC': self.payoff_R,
        'DD': self.payoff_P,
        'CD': [self.payoff_S, self.payoff_T],   # [me, you]
        'DC': [self.payoff_T, self.payoff_S],
      },
    }
