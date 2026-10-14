// Copyright AStarship <https://astarship.net>.

// This file replaces the Script2-era _debug.inl, which was a one-line
// redirect to <script2/_debug.inl> - a tree that no longer exists.
//
// Debug macros (D_COUT, D_RUN_TEST, D_ASSERT, etc.) are now in
// ASCIICrabs/_Debug.h, included via _ConfigFooter.h when
// SEAM >= CRABS_RELEASE.
