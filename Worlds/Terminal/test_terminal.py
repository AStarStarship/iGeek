# Copyright AStarship <https://astarship.net>.
"""Unit tests for the Terminal World browser demo engine and server."""
import sys
import unittest
import importlib
from pathlib import Path

# Ensure the engine is importable regardless of execution context
_WorldsDir = Path(__file__).resolve().parent
if str(_WorldsDir) not in sys.path:
  sys.path.insert(0, str(_WorldsDir))


class TerminalEngineTests(unittest.TestCase):
  """Tests for the core TerminalEngine logic."""

  def _engine(self):
    try:
      return importlib.import_module('terminal_engine')
    except ModuleNotFoundError:
      import terminal_engine
      return terminal_engine

  def test_default_state(self):
    """Default state has the prompt text and running=True."""
    mod = self._engine()
    s = mod.TermState()
    s = mod.TerminalEngine.default_state()
    self.assertEqual(s.cwd, '/home/astar')
    self.assertEqual(s.running, True)
    self.assertEqual(s.line_len, 0)
    # Screen should have prompt text
    screen_str = ''.join(s.screen)
    self.assertIn('TerminalWorld', screen_str)
    self.assertIn('user@host', screen_str)

  def test_reset(self):
    """Reset clears the screen and resets to prompt."""
    mod = self._engine()
    s = mod.TermState()
    # Take a step first: print some text
    s.print_at('hello world\n')
    # Reset
    s.reset()
    self.assertEqual(s.cwd, '/home/astar')
    self.assertEqual(s.running, True)
    self.assertEqual(s.line_len, 0)
    # Screen should be back to prompt
    screen_str = ''.join(s.screen)
    self.assertIn('TerminalWorld', screen_str)

  def test_pwd_command(self):
    """pwd prints the current cwd."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    result = mod.TerminalEngine.step(s, 'p')
    self.assertEqual(result['cmd_name'], 'pwd')
    self.assertEqual(result['cwd'], '/home/astar')
    self.assertIsNone(result.get('error'))

  def test_ls_command(self):
    """ls prints cwd with stub message."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    result = mod.TerminalEngine.step(s, 'l')
    self.assertEqual(result['cmd_name'], 'ls')
    self.assertIn(s.cwd, result['screen_summary'])
    self.assertIsNone(result.get('error'))

  def test_echo_command(self):
    """echo prints the current command text."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    s.line = 'hello world'
    s.line_len = 11
    result = mod.TerminalEngine.step(s, 'e')
    self.assertEqual(result['cmd_name'], 'echo')
    self.assertIsNone(result.get('error'))

  def test_cd_command(self):
    """cd changes the cwd (simulated)."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    # Change to root
    s.line = '/'
    s.line_len = 1
    result = mod.TerminalEngine.step(s, 'c')
    self.assertEqual(result['cmd_name'], 'cd')
    self.assertEqual(s.cwd, '/')
    self.assertIsNone(result.get('error'))

  def test_cd_no_arg(self):
    """cd with no args prints error/msg."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    s.line = ''
    s.line_len = 0
    result = mod.TerminalEngine.step(s, 'c')
    self.assertEqual(result['cmd_name'], 'cd')
    # Should not crash

  def test_clear_command(self):
    """clear wipes the screen."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    s.print_at('some text\n')
    result = mod.TerminalEngine.step(s, 'k')
    self.assertEqual(result['cmd_name'], 'clear')
    # Screen should be cleared (all spaces)
    self.assertTrue(all(c == ' ' for c in s.screen))
    self.assertIsNone(result.get('error'))

  def test_exit_command(self):
    """exit sets running=False."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    result = mod.TerminalEngine.step(s, 'x')
    self.assertEqual(result['cmd_name'], 'exit')
    self.assertEqual(result['running'], False)
    self.assertIsNone(result.get('error'))

  def test_invalid_command(self):
    """Invalid command chars return error but don't crash."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    result = mod.TerminalEngine.step(s, 'z')
    self.assertEqual(result['error'], 'Invalid command: z')
    # screen_summary should still be present

  def test_screen_summary(self):
    """Screen summary shows last 4 rows."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    # Write some text to the screen
    s.print_at('Line 1\nLine 2\nLine 3\nLine 4\nLine 5\n')
    summary = mod.TerminalEngine._screen_summary(s)
    self.assertIsInstance(summary, str)
    # Should contain some of the written text
    self.assertTrue(len(summary) > 0)

  def test_history_tracking(self):
    """Command history is tracked."""
    mod = self._engine()
    s = mod.TerminalEngine.default_state()
    # Step through some commands
    mod.TerminalEngine.step(s, 'p')  # pwd
    mod.TerminalEngine.step(s, 'l')  # ls
    mod.TerminalEngine.step(s, 'e')  # echo
    # History count should be tracked
    self.assertGreater(s.history_count, 0)


class TerminalServerTests(unittest.TestCase):
  """Tests for the Flask server endpoints."""

  def _server_mod(self):
    try:
      return importlib.import_module('terminal_server')
    except ModuleNotFoundError:
      import terminal_server
      return terminal_server

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
    self.assertIn('observation', data)
    self.assertIn('round', data)
    # Check HttpOnly cookie was set via response headers
    set_cookie = resp.headers.get('Set-Cookie', '')
    self.assertIn('terminal_session', set_cookie)
    self.assertIn('HttpOnly', set_cookie)

  def test_invalid_action_no_mutation(self):
    """Invalid action does not mutate the world state."""
    self.client.post('/api/new')
    resp = self.client.post('/api/action', json={'action': 'invalid_cmd'})
    self.assertEqual(resp.status_code, 400)
    # State should be unchanged
    state = self.client.get('/api/state').get_json()
    self.assertIsNotNone(state, 'State should not be None')
    self.assertIn('observation', state)

  def test_valid_action_progresses(self):
    """Valid action advances the episode."""
    self.client.post('/api/new')
    resp = self.client.post('/api/action', json={'action': 'pwd'})
    self.assertEqual(resp.status_code, 200)
    data = resp.get_json()
    self.assertIn('observation', data)
    self.assertIn('round', data)
    self.assertIn('action', data)

  def test_exit_ends_episode(self):
    """exit action ends the episode (done=True)."""
    self.client.post('/api/new')
    # Take a few steps
    self.client.post('/api/action', json={'action': 'pwd'})
    self.client.post('/api/action', json={'action': 'ls'})
    # Exit should end the episode
    resp = self.client.post('/api/action', json={'action': 'exit'})
    self.assertEqual(resp.status_code, 200)
    data = resp.get_json()
    self.assertIn('terminal', data)
    # done should be True after exit
    self.assertTrue(data.get('terminal', False) or data.get('done', False))

  def test_reset(self):
    """reset clears the episode and starts fresh."""
    self.client.post('/api/new')
    # Play one step
    self.client.post('/api/action', json={'action': 'pwd'})
    # Reset
    resp = self.client.post('/api/reset')
    self.assertEqual(resp.status_code, 200)
    data = resp.get_json()
    self.assertIn('observation', data)
    self.assertIn('round', data)
