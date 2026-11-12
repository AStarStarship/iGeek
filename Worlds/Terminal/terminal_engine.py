# Copyright AStarship <https://astarship.net>.
"""Terminal World simulation engine — Python port of TerminalWorld C++ code.

Native C++ reference:
  Worlds/Terminal/Terminal.h / Terminal.hxx — TerminalKernel with Star reducers
  for pwd/ls/echo/cd/clear/exit commands
  Worlds/Terminal/TerminalEnvironment.h — Gym wrapper (placeholder StepBatch)

This port implements a fully simulated terminal with server-side state.
No real shell access, no filesystem, no secret reads. All I/O is contained
within the Flask process; the "screen" is a 24×80 text grid rendered in HTML.

Native C++ StepBatch() is a PLACEHOLDER that executes one of 6 commands.
This port replaces it with real command routing and screen-state observation.
No native Gym equivalence — the native C++ terminal requires the full
ASCIICrabs/Crabs infrastructure (Boofer, BIn/BOut rings, operand stack)
that cannot be reasonably bridged to a browser demo.
"""
from __future__ import annotations

#: Terminal screen dimensions (fixed, no VLA, no heap growth)
TermScreenRows = 24
TermScreenCols = 80
TermScreenBytes = TermScreenRows * TermScreenCols

#: Max command line length
TermLineMax = 128
#: Max output bytes per command
TermOutputMax = 1024
#: Max cwd length
TermCwdMax = 64
#: Command history depth
TermHistoryMax = 16
#: Kernel operand-stack depth
TermStateMax = 8

#: Episode cap
TermWorldMaxSteps = 100

#: Action vocabulary (matching native C++ command chars)
#: p=pwd, l=ls, e=echo, c=cd, k=clear, x=exit
TermActionChars = ('p', 'l', 'e', 'c', 'k', 'x')
TermActionCount = len(TermActionChars)

#: Mapping from single-char codes to full command names
COMMAND_MAP = {
    'p': 'pwd',
    'l': 'ls',
    'e': 'echo',
    'c': 'cd',
    'k': 'clear',
    'x': 'exit',
}

#: Full command names allowed in the API
ALLOWED_COMMANDS = set(COMMAND_MAP.values())

#: Single-character codes for engine-internal use
ALLOWED_CHARS = set(TermActionChars)


def _handle_cd(state):
  """Handle cd <dir> — validate input, update cwd (simulated, no real fs)."""
  parts = state.line.split(None, 1)
  if len(parts) < 1 or not parts[0].strip():
    state.print_at('No directory specified')
    return
  dir_arg = parts[0].strip()
  if dir_arg == '/':
    state.cwd = '/'
  elif dir_arg == '..':
    parts_cwd = state.cwd.rstrip('/').split('/')
    if len(parts_cwd) <= 1:
      state.cwd = '/'
    else:
      state.cwd = '/'.join(parts_cwd[:-1]) or '/'
  elif dir_arg.startswith('/'):
    state.cwd = dir_arg if dir_arg else '/home/astar'
  # Relative paths: no real filesystem to navigate, keep current cwd
  state.print_at(state.cwd)


def _clear_screen(state):
  """Handle clear command - wipe screen and reset cursor."""
  state.screen = [' '] * TermScreenBytes
  state.cursor_row = 0
  state.cursor_col = 0
  state.print_at('')


class TermState:
  """Simulated terminal state — 24×80 screen + cursor + cwd + command line + history."""

  def __init__(self):
    self.screen = [' '] * TermScreenBytes
    self.cursor_row = 0
    self.cursor_col = 0
    self.cwd = '/home/astar'
    self.line = ''
    self.line_len = 0
    self.history = [0] * TermHistoryMax
    self.history_count = 0
    self.running = True

  def reset(self):
    """Reset to initial prompt state."""
    self.screen = [' '] * TermScreenBytes
    self.cursor_row = 0
    self.cursor_col = 0
    self.cwd = '/home/astar'
    self.line = ''
    self.line_len = 0
    self.history = [0] * TermHistoryMax
    self.history_count = 0
    self.print_at('TerminalWorld mini-Linux (Star-reducer kernel)\n')
    self.print_at('user@host:~/$ ')
    self.running = True

  def print_at(self, text):
    """Print text at cursor position; handle scrolling and line wrapping."""
    i = 0
    while i < len(text):
      ch = text[i]
      if ch == '\n':
        self.cursor_row += 1
        self.cursor_col = 0
        self._advance_cursor()
        i += 1
        continue
      if self.cursor_row >= TermScreenRows:
        self._scroll()
      if self.cursor_col >= TermScreenCols:
        self.cursor_row += 1
        self.cursor_col = 0
        self._advance_cursor()
        continue
      idx = self.cursor_row * TermScreenCols + self.cursor_col
      self.screen[idx] = ch
      self.cursor_col += 1
      i += 1
    self.cursor_row += 1
    self.cursor_col = 0
    self._advance_cursor()

  def _scroll(self):
    """Scroll screen up one row when cursor passes bottom."""
    if self.cursor_row >= TermScreenRows:
      for r in range(TermScreenRows - 1):
        start = r * TermScreenCols
        next_start = (r + 1) * TermScreenCols
        self.screen[start:next_start] = self.screen[next_start:next_start + TermScreenCols]
      self.cursor_row = TermScreenRows - 1
      self.cursor_col = 0

  def _advance_cursor(self):
    """Move cursor past edge; scroll if needed."""
    if self.cursor_col >= TermScreenCols:
      self.cursor_row += 1
      self.cursor_col = 0
      self._scroll()


class TerminalEngine:
  """Simulated terminal engine — routes commands to state mutators.

  The 6 supported commands (matching native C++ TerminalKernel reducers):
    pwd    -> print current cwd
    ls     -> list cwd (stub: print cwd + placeholder)
    echo   -> print the command text
    cd     -> change cwd (validated, no real fs)
    clear  -> wipe screen
    exit   -> terminate session (set running=False)
  """
  ALLOWED_CHARS = ALLOWED_CHARS
  ALLOWED_COMMANDS = ALLOWED_COMMANDS

  COMMAND_KEYS = {
    'p': 'pwd',
    'l': 'ls',
    'e': 'echo',
    'c': 'cd',
    'k': 'clear',
    'x': 'exit',
  }

  HANDLERS = {
    'pwd': lambda s: s.print_at(s.cwd),
    'ls': lambda s: s.print_at(f'{s.cwd}  (stub: no file table yet)'),
    'echo': lambda s: s.print_at(s.line.strip() or ''),
    'cd': _handle_cd,
    'clear': _clear_screen,
    'exit': lambda s: setattr(s, 'running', False),
  }

  _session_history_count = 0

  @staticmethod
  def step(state, action_char):
    """Execute one command action against the terminal state."""
    TerminalEngine._session_history_count += 1
    state.history_count = TerminalEngine._session_history_count

    if action_char not in TerminalEngine.ALLOWED_CHARS:
      return {
        'error': f'Invalid command: {action_char}',
        'screen_summary': TerminalEngine._screen_summary(state),
        'history_count': TerminalEngine._session_history_count,
      }

    key = TerminalEngine.COMMAND_KEYS[action_char]
    handler = TerminalEngine.HANDLERS[key]
    handler(state)
    screen_summary = TerminalEngine._screen_summary(state)

    return {
      'action': action_char,
      'cmd_name': key,
      'screen_summary': screen_summary,
      'cwd': state.cwd,
      'line_len': state.line_len,
      'history_count': TerminalEngine._session_history_count,
      'running': state.running,
      'done': not state.running,
    }

  @staticmethod
  def _screen_summary(state):
    """Return a 4-row pipe-separated summary of the terminal screen.

    Shows the first 4 non-empty rows from the top, padded with
    empty rows if the screen has fewer than 4 non-empty rows.
    """
    rows = []
    for r in range(TermScreenRows):
      row_start = r * TermScreenCols
      row_end = row_start + TermScreenCols
      row_chars = ''.join(state.screen[row_start:row_end]).rstrip()
      if row_chars:
        rows.append(row_chars)
      if len(rows) == 4:
        break
    while len(rows) < 4:
      rows.append('')
    return '|'.join(rows)

  @staticmethod
  def default_state():
    """Create the initial terminal state (the prompt)."""
    s = TermState()
    s.print_at('TerminalWorld mini-Linux (Star-reducer kernel)\n')
    s.print_at('user@host:~/$ ')
    return s


class TerminalWorld:
  """One episode of repeated terminal interaction.

  Each step the agent chooses an action (0..5 = pwd,ls,echo,cd,clear,exit).
  The engine routes the command to the state mutator and returns the
  resulting screen as the observation.
  """
  def __init__(self, max_steps=TermWorldMaxSteps):
    self.max_steps = max_steps
    self.round = 0
    self.running = True
    self.total_commands = 0
    self.state = TerminalEngine.default_state()
    self.history = []

  def reset(self):
    """Reset to a fresh episode."""
    TerminalEngine._session_history_count = 0
    self.round = 0
    self.running = True
    self.total_commands = 0
    self.state = TerminalEngine.default_state()
    self.history = []

  def step(self, action_idx):
    """Execute one step with action index (0..5).

    Args:
      action_idx: integer 0=pwd, 1=ls, 2=echo, 3=cd, 4=clear, 5=exit

    Returns:
      dict with observation, reward, terminal, info
    """
    if self.round >= self.max_steps:
      self.running = False

    action_char = TermActionChars[action_idx]
    result = TerminalEngine.step(self.state, action_char)

    self.round += 1
    self.total_commands += 1
    self.history.append({
      'round': self.round,
      'action': action_char,
      'cmd_name': result.get('cmd_name', ''),
      'screen_summary': result.get('screen_summary', ''),
      'running': result.get('running', True),
    })

    if action_char == 'x':
      reward = 1.0 if self.total_commands > 1 else -1.0
      self.running = False
    else:
      reward = -0.01
      if not result.get('running', True):
        reward = 1.0

    obs = {
      'screen': result.get('screen_summary', ''),
      'cwd': result.get('cwd', ''),
      'line_len': result.get('line_len', 0),
      'history_count': result.get('history_count', 0),
      'running': result.get('running', True),
      'done': result.get('done', True),
    }

    return {
      'observation': obs,
      'reward': reward,
      'terminal': result.get('done', False),
      'info': {
        'round': self.round,
        'action': action_char,
        'cmd_name': result.get('cmd_name', ''),
        'total_commands': self.total_commands,
      },
    }

  def to_dict(self):
    """Serialize world state for API response (without advancing)."""
    return {
      'observation': {
        'screen': TerminalEngine._screen_summary(self.state),
        'cwd': self.state.cwd,
        'line_len': self.state.line_len,
        'history_count': self.state.history_count,
        'running': self.state.running,
        'done': not self.state.running,
      },
      'round': self.round,
      'total_commands': self.total_commands,
    }
