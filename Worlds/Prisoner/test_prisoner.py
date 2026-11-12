# Copyright AStarship <https://astarship.net>.
"""Unit tests for the Prisoner's Dilemma browser demo engine and server."""
import random
import sys
import unittest
import importlib
from pathlib import Path

# Ensure the engine is importable regardless of execution context
_WorldsDir = Path(__file__).resolve().parent
if str(_WorldsDir) not in sys.path:
  sys.path.insert(0, str(_WorldsDir))


class PrisonerEngineTests(unittest.TestCase):
  """Tests for the core PrisonerGame engine logic."""

  def _engine(self):
    try:
      return importlib.import_module('prisoner_engine')
    except ModuleNotFoundError:
      import prisoner_engine
      return prisoner_engine

  def test_payoff_matrix(self):
    """Standard PD: T>R>P>S. Round 0: AI cooperates (tit-for-tat first move C)."""
    g = self._engine().PrisonerGame()
    # Round 0: AI cooperates (tit-for-tat default, first move C)
    r = g.step(g.C)
    self.assertEqual(r['player_reward'], 3)  # both C -> R=3
    self.assertEqual(r['ai_reward'], 3)
    # Round after: player D, AI C -> T=5 for player
    r2 = g.step(g.D)
    self.assertEqual(r2['player_reward'], 5)  # Temptation

  def test_defection_pays_temptation(self):
    g = self._engine().PrisonerGame(T=5, R=3, P=1, S=0)
    # On round 0, AI cooperates (tit-for-tat default, first move C)
    r = g.step(g.C)
    self.assertEqual(r['player_reward'], 3)
    # Now player defects
    r2 = g.step(g.D)
    # Player D, AI C -> T=5
    self.assertEqual(r2['player_reward'], 5)

  def test_cooperate_against_defect_sucker(self):
    """Player D then C against AI that mirrors player's last move (tit-for-tat)."""
    g = self._engine().PrisonerGame(T=5, R=3, P=1, S=0)
    # Round 0: AI cooperates (tit-for-tat first move C)
    r = g.step(g.D)
    # Player D, AI C -> T=5 for player
    self.assertEqual(r['player_reward'], 5)
    # After player D, AI's tit-for-tat mirrors player's last D
    r2 = g.step(g.C)
    # Player C, AI D (mirror) -> S=0 for player
    self.assertEqual(r2['player_reward'], 0)

  def test_random_ai(self):
    g = self._engine().PrisonerGame(ai='random')
    # Just verify it doesn't crash and returns valid rewards
    for _ in range(10):
      r = g.step(random.choice((g.C, g.D)))
      self.assertIn(r['player_reward'], (0, 1, 3, 5))
      self.assertIn(r['ai_reward'], (0, 1, 3, 5))

  def test_grim_trigger(self):
    g = self._engine().PrisonerGame(ai='grim-trigger')
    # First few rounds should cooperate
    for _ in range(5):
      r = g.step(g.C)
      self.assertEqual(r['player_reward'], 3)
    # Now player defects once
    r = g.step(g.D)
    self.assertEqual(r['player_reward'], 5)  # Temptation payoff
    # After that, AI should always defect
    r2 = g.step(g.C)
    self.assertEqual(r2['player_reward'], 0)  # Sucker payoff

  def test_always_cooperate(self):
    g = self._engine().PrisonerGame(ai='always-cooperate')
    for _ in range(10):
      r = g.step(g.C)
      # Always C vs C -> R=3 always
      self.assertEqual(r['player_reward'], 3)

  def test_always_defect(self):
    """Player always C vs AI always D: player gets S=0 each round (sucker)."""
    g = self._engine().PrisonerGame(ai='always-defect')
    for _ in range(10):
      r = g.step(g.C)
      # Player C vs AI always D -> S=0 each round (sucker payoff)
      self.assertEqual(r['player_reward'], 0)

  def test_episode_ends_at_max_rounds(self):
    g = self._engine().PrisonerGame(max_rounds=3)
    # Play 3 rounds
    for _ in range(3):
      r = g.step(g.C)
    # After 3 rounds, done should be True
    self.assertTrue(g.state()['done'])

  def test_reset(self):
    g = self._engine().PrisonerGame()
    r1 = g.step(g.C)
    self.assertFalse(r1['done'])
    g.reset()
    r2 = g.step(g.C)
    # After reset, round should be 1 again
    self.assertEqual(r2['round'], 1)

  def test_invalid_action_raises(self):
    g = self._engine().PrisonerGame()
    with self.assertRaises(ValueError):
      g.step(2)  # invalid action

  def test_total_rewards_accumulate(self):
    g = self._engine().PrisonerGame(max_rounds=5)
    # All cooperate: 5 rounds * R=3 = 15
    for _ in range(5):
      r = g.step(g.C)
    self.assertEqual(g.player_total, 15)
    self.assertEqual(g.ai_total, 15)
    self.assertTrue(g.state()['done'])

  def test_history_tracked(self):
    g = self._engine().PrisonerGame(max_rounds=3)
    g.step(g.C)
    g.step(g.D)
    g.step(g.C)
    h = g.state()['history']
    self.assertEqual(len(h), 3)
    # History entries from state() are dicts with 'player_reward' key
    self.assertEqual(h[0]['player_reward'], 3)  # first round: both C -> R=3
    self.assertEqual(h[0]['ai_reward'], 3)
    # Second round: player D, AI C (tit-for-tat mirrors first C) -> T=5
    self.assertEqual(h[1]['player_reward'], 5)
    # Third round: player C, AI D (mirror) -> S=0
    self.assertEqual(h[2]['player_reward'], 0)


class PrisonerServerTests(unittest.TestCase):
  """Tests for the Flask server endpoints."""

  def _server_mod(self):
    try:
      return importlib.import_module('prisoner_server')
    except ModuleNotFoundError:
      import prisoner_server
      return prisoner_server

  def setUp(self):
    server_mod = self._server_mod()
    self.app = server_mod.CreateApp()
    self.client = self.app.test_client()

  def test_manifest(self):
    resp = self.client.get('/api/manifest')
    self.assertEqual(resp.status_code, 200)
    m = resp.get_json()
    self.assertIn('name', m)
    self.assertIn('version', m)
    self.assertIn('rules', m)
    self.assertIn('observation', m)
    self.assertIn('actions', m)

  def test_new_session(self):
    resp = self.client.post('/api/new')
    self.assertEqual(resp.status_code, 200)
    data = resp.get_json()
    self.assertIn('done', data)
    self.assertEqual(data['round'], 0)
    # Check HttpOnly cookie was set via response headers
    set_cookie = resp.headers.get('Set-Cookie', '')
    self.assertIn('prisoner_session', set_cookie)
    self.assertIn('HttpOnly', set_cookie)

  def test_invalid_action_no_mutation(self):
    # Start a new session
    self.client.post('/api/new')
    # Try invalid action
    resp = self.client.post('/api/action', json={'action': 'cheat'})
    self.assertEqual(resp.status_code, 400)
    # State should be unchanged — no mutation from invalid input
    state = self.client.get('/api/state').get_json()
    # round should still be 0
    self.assertIsNotNone(state, 'State should not be None after invalid action')
    self.assertEqual(state['round'], 0)

  def test_valid_action_progresses_episode(self):
    self.client.post('/api/new')
    # Cooperate on round 0
    resp = self.client.post('/api/action', json={'action': 'cooperate'})
    self.assertEqual(resp.status_code, 200)
    data = resp.get_json()
    self.assertEqual(data['round'], 1)
    self.assertIn('player_reward', data)
    self.assertIn('ai_reward', data)

  def test_reset(self):
    self.client.post('/api/new')
    # Play one step
    self.client.post('/api/action', json={'action': 'cooperate'})
    # Reset
    resp = self.client.post('/api/reset')
    self.assertEqual(resp.status_code, 200)
    data = resp.get_json()
    # After reset the round counter may vary by implementation;
    # the important thing is that it doesn't crash and returns state
    self.assertIn('round', data)
    self.assertIn('done', data)
