// Copyright AStarship <https://astarship.net>.

// This file replaces the Script2-era _release.inl, which was a
// one-line redirect to <script2/_debug.inl> (the Script2 build used
// a single debug/release path rather than separate seam files).
//
// Release macros (R_*, D_THIS, etc.) are now in
// ASCIICrabs/_Release.h, included via _ConfigFooter.h when
// SEAM < CRABS_RELEASE.
