A playing cards game environment for [iGeek](https://iGeek.com).

Status: **experimental** — BlackjackGym is standalone (does not inherit
the base `Gym` class); no manifest. TToyGym is a test-only Gym subclass.
PPO diagnostics exist but algorithm correctness is not validated.

## Browser demo

Run from the iGeek root:

    python3 Worlds/Cards/cards_server.py

Open `http://localhost:5003` (or the host's LAN address). The development
server binds `0.0.0.0`; use only on a trusted LAN, not the public Internet.

- Blackjack: hit, stand, double; dealer stands on soft 17; natural pays 3:2.
  Fixed 10-chip bet; no splitting or insurance.
- Poker: heads-up fixed-limit Texas Hold'em against a heuristic (not trained)
  opponent. Blinds 5/10, betting limits 10/20, four bets per street. Effective
  stacks cap bets; ties split the pot. Opponent cards stay hidden until showdown.
- Play chips only. Each browser cookie has its own server-side table. Switching
  game resets balances; restarting the server clears sessions. If either stack
  cannot fund another round, switch games to restart with fresh play chips.
- Generated local SVG deck: 52 faces plus a card back; no external image/CDN
  dependency. Regenerate with `python3 Worlds/Cards/generate_cards.py`.
- `GET /api/manifest` describes the browser environment's rules, observations,
  legal actions, and reward accounting.

This is a separate Python implementation, not a binding to the native C++
BlackjackGym and not a native Gym/Environment integration. The experimental
native status above still applies.

Run the retained backend tests:

    python3 -m unittest discover -s Worlds/Cards -p 'test_cards.py' -v

## License

Copyright [AStarship™](https://astarship.net).
