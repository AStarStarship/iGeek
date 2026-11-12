# Copyright AStarship <https://astarship.net>.
"""Server-side play-chip games. No web dependencies or monetary features."""
import random
from collections import Counter
from itertools import combinations


def Evaluate(cards):
  """Comparable best-five score for five through seven distinct cards."""
  if not 5 <= len(cards) <= 7 or len(set(cards)) != len(cards):
    raise ValueError('Expected five to seven distinct cards')
  return max(Five(hand) for hand in combinations(cards, 5))


def Five(cards):
  values = sorted((14 if card[:-1] == 'A' else Ranks.index(card[:-1]) + 1
                   for card in cards), reverse=True)
  groups = sorted(((count, value) for value, count in Counter(values).items()),
                  reverse=True)
  flush = len({card[-1] for card in cards}) == 1
  straight = (5 if values == [14, 5, 4, 3, 2] else
              values[0] if len(set(values)) == 5 and values[0] - values[-1] == 4 else 0)
  if flush and straight:
    return (8, straight)
  if groups[0][0] == 4:
    return (7, groups[0][1], groups[1][1])
  if [group[0] for group in groups] == [3, 2]:
    return (6, groups[0][1], groups[1][1])
  if flush:
    return (5, *values)
  if straight:
    return (4, straight)
  category = {3: 3, 2: 1, 1: 0}[groups[0][0]]
  if len(groups) == 3 and groups[0][0] == groups[1][0] == 2:
    category = 2
  return (category, *(group[1] for group in groups))

Ranks = ('A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K')
BlackjackRules = ('Single deck; bet 10 play chips; blackjack pays 3:2; dealer stands '
                  'on soft 17; double on first two cards; no split or insurance.')


def Deck():
  deck = [rank + suit for suit in 'cdhs' for rank in Ranks]
  random.SystemRandom().shuffle(deck)
  return deck


def Total(cards):
  values = [11 if card[:-1] == 'A' else min(Ranks.index(card[:-1]) + 1, 10)
            for card in cards]
  total = sum(values)
  for value in values:
    if total > 21 and value == 11:
      total -= 10
  return total


class Table:
  def __init__(self, game='blackjack', deck=None):
    self.bankroll = 200
    self.opponent_bankroll = 200
    self.done = True
    self.game = game
    self.New(game, deck)

  def New(self, game, deck=None):
    if game not in ('blackjack', 'poker'):
      raise ValueError('Unknown game')
    if not self.done and game == self.game:
      raise ValueError('Finish the current round first')
    if game != self.game:
      self.bankroll = self.opponent_bankroll = 200
    minimum = 10 if game == 'blackjack' else 1
    dealer_minimum = 15 if game == 'blackjack' else 1
    if self.bankroll < minimum or self.opponent_bankroll < dealer_minimum:
      raise ValueError('Not enough play chips; start a new browser session')
    self.game = game
    self.deck = list(deck) if deck is not None else Deck()
    self.player = [self.Draw(), self.Draw()]
    self.opponent = [self.Draw(), self.Draw()]
    self.board = []
    self.done = False
    self.reveal = False
    self.phase = 'player'
    self.message = 'Your turn.'
    if game == 'poker':
      self.StartPoker()
      return
    self.bet = 10
    self.bankroll -= 10
    self.pot = 10
    if Total(self.player) == 21 or Total(self.opponent) == 21:
      reward = (0 if Total(self.player) == Total(self.opponent) else
                15 if Total(self.player) == 21 else -10)
      self.SettleBlackjack(reward)

  def Draw(self):
    return self.deck.pop(0)

  def Actions(self):
    if self.done:
      return []
    if self.game == 'poker':
      return self.PokerActions(0)
    actions = ['hit', 'stand']
    if (len(self.player) == 2 and self.bankroll >= self.bet and
        self.opponent_bankroll >= self.bet * 2):
      actions.append('double')
    return actions

  def Action(self, action):
    if not isinstance(action, str) or action not in self.Actions():
      raise ValueError('Illegal action')
    if self.game == 'poker':
      self.PokerAction(0, action)
      self.RunBot()
      return
    if action == 'double':
      self.bankroll -= self.bet
      self.pot += self.bet
      self.bet *= 2
    if action in ('hit', 'double'):
      self.player.append(self.Draw())
      if Total(self.player) > 21:
        self.SettleBlackjack(-self.bet)
        return
    if action in ('stand', 'double') or Total(self.player) == 21:
      while Total(self.opponent) < 17:
        self.opponent.append(self.Draw())
      player_total, dealer_total = Total(self.player), Total(self.opponent)
      reward = (self.bet if dealer_total > 21 or player_total > dealer_total else
                0 if player_total == dealer_total else -self.bet)
      self.SettleBlackjack(reward)

  def SettleBlackjack(self, reward):
    self.bankroll += self.pot + reward
    self.opponent_bankroll -= reward
    self.pot = 0
    self.done = self.reveal = True
    self.phase = 'done'
    self.message = 'Push.' if reward == 0 else f'Round complete: {reward:+} play chips.'

  def State(self):
    poker = self.game == 'poker'
    return dict(game=self.game, phase=self.phase, done=self.done,
                message=self.message, player=list(self.player),
                opponent=list(self.opponent) if self.reveal else
                ['back', 'back'] if poker else [self.opponent[0], 'back'],
                board=list(self.board), player_total=None if poker else Total(self.player),
                opponent_total=Total(self.opponent) if self.reveal and not poker else None,
                bankroll=self.bankroll, opponent_bankroll=self.opponent_bankroll,
                pot=self.pot, actions=self.Actions(), rules=PokerRules if poker else BlackjackRules)

  def StartPoker(self):
    self.button = 1 - getattr(self, 'button', 1)
    self.phase = 'preflop'
    self.committed = [0, 0]
    self.pot = 0
    self.raises = 1
    self.pending = {0, 1}
    self.actor = self.button
    effective = min(self.bankroll, self.opponent_bankroll)
    self.Pay(self.button, min(5, effective))
    self.Pay(1 - self.button, min(10, effective))
    self.RunBot()

  def Pay(self, actor, amount):
    if actor == 0:
      self.bankroll -= amount
    else:
      self.opponent_bankroll -= amount
    self.committed[actor] += amount
    self.pot += amount

  def PokerActions(self, actor):
    if self.done or actor != self.actor:
      return []
    owed = max(self.committed) - self.committed[actor]
    actions = ['fold', 'call'] if owed else ['check']
    limit = min(self.bankroll + self.committed[0],
                self.opponent_bankroll + self.committed[1])
    if self.raises < 4 and limit > max(self.committed):
      actions.append('raise')
    return actions

  def PokerAction(self, actor, action):
    if action == 'fold':
      self.FinishPoker(1 - actor, False)
      return
    target = max(self.committed)
    if action == 'raise':
      increment = 10 if self.phase in ('preflop', 'flop') else 20
      target = min(target + increment, self.bankroll + self.committed[0],
                   self.opponent_bankroll + self.committed[1])
      self.raises += 1
      self.pending = {1 - actor}
    else:
      self.pending.discard(actor)
    self.Pay(actor, target - self.committed[actor])
    self.actor = 1 - actor
    if not self.pending:
      self.AdvancePoker()

  def AdvancePoker(self):
    if self.bankroll == 0 or self.opponent_bankroll == 0:
      while len(self.board) < 5:
        self.board.append(self.Draw())
    if len(self.board) == 5:
      player_score = Evaluate(self.player + self.board)
      opponent_score = Evaluate(self.opponent + self.board)
      self.FinishPoker(None if player_score == opponent_score else
                       0 if player_score > opponent_score else 1, True)
      return
    count = 3 if not self.board else 1
    self.board.extend(self.Draw() for _ in range(count))
    self.phase = {3: 'flop', 4: 'turn', 5: 'river'}[len(self.board)]
    self.committed = [0, 0]
    self.raises = 0
    self.pending = {0, 1}
    self.actor = 1 - self.button

  def FinishPoker(self, winner, reveal):
    if winner is None:
      self.bankroll += self.pot // 2
      self.opponent_bankroll += self.pot // 2
      if self.pot % 2:
        if self.button == 1:
          self.bankroll += 1
        else:
          self.opponent_bankroll += 1
    elif winner == 0:
      self.bankroll += self.pot
    else:
      self.opponent_bankroll += self.pot
    self.pot = 0
    self.done = True
    self.reveal = reveal
    self.phase = 'done'
    self.message = ('Split pot.' if winner is None else
                    'You win the pot.' if winner == 0 else 'Bot wins the pot.')

  def RunBot(self):
    while not self.done and self.actor == 1:
      action = BotAction(tuple(self.opponent), tuple(self.board),
                         tuple(self.PokerActions(1)),
                         max(self.committed) - self.committed[1], self.pot)
      self.PokerAction(1, action)


PokerRules = ('Heads-up fixed-limit Hold\'em; alternating button; blinds 5/10; '
              'bets 10 preflop/flop, 20 turn/river; four bets per street including '
              'the big blind; short all-in raises count toward cap. Effective-stack '
              'caps prevent side pots. Best five of seven; ties split; odd chip to '
              'non-button. Folded hands stay hidden. Mode switches reset play chips.')


def BotAction(cards, board, actions, owed, pot):
  """Pure public-observation policy: receives only its own hole cards."""
  values = [14 if card[:-1] == 'A' else Ranks.index(card[:-1]) + 1 for card in cards]
  strength = (Evaluate(list(cards + board))[0] if board else
              2 if values[0] == values[1] else 1 if max(values) >= 12 else 0)
  if strength >= 2 and 'raise' in actions:
    return 'raise'
  if 'check' in actions:
    return 'check'
  if strength == 0 and owed > 10 and owed * 4 > pot:
    return 'fold'
  return 'call'
