// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_AGENTSTATE_DECL
#define IGEEK_AGENTSTATE_DECL 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"

namespace _ {

/* AgentState — the internal-state memory model for iGeek agents.
 *
 * Decision (iGeek AGENT_PLAN.md section 3.5 #3, master-agent + low-level,
 * 2026-09-11): agent state = a FLAT fixed-size IUC word array wrapped in an
 * Autoject backed by ObjectFactoryHeap. The Autoject heap factory keeps the
 * state DLL-ready (a world can LoadDLL and the buffer survives the boundary);
 * snapshot = copy the buffer bytes, probe = a linear read over the words.
 *
 * Why a flat word array (not a nested Room): the plan notes that a nested
 * layout depends on template params and makes dump-to-bytes ambiguous. A flat
 * IUC slot array is unambiguous: slot i is always at a fixed offset, so a
 * snapshot is a memcpy of the whole buffer and a linear probe is a dot
 * product over the words.
 *
 * Autoject word-alignment: ObjectFactoryHeap word-aligns up (see
 * ASCIICrabs/Autoject.hxx). We therefore size the buffer in words and store
 * each IUC state slot in a whole IUW word. On a 64-bit host that wastes 4
 * bytes per slot, which is the explicit cost of word-alignment + simple
 * byte-copy snapshots (the plan accepts this).
 *
 * No C++ standard library: allocation goes through the Autoject RAMFactory
 * (new/delete[] are compiler builtins), never std::vector.
 */
class AgentState {
 public:

  /* @param slot_count Number of IUC state slots (word count). Must be >= 1.
   * The buffer is word-aligned by the RAMFactory. */
  explicit AgentState(ISC slot_count);
  ~AgentState();

  /* Copy is disabled: the buffer is heap-owned by the Autoject. Use
  Snapshot() to copy a word array out. */
  AgentState(const AgentState&) = delete;
  AgentState& operator=(const AgentState&) = delete;

  /* Number of IUC state slots. Immutable after construction. */
  ISC SlotCount() const { return slot_count_; }

  /* Read/write slot i (0..slot_count_-1). @pre 0 <= i < SlotCount(). */
  IUC Get(ISC i) const;
  void Set(ISC i, IUC value);

  /* Zero all slots (used to reset an agent's internal state). */
  void Clear();

  /* Snapshot: copy the slot_count_ IUC slots into out (must have at least
  slot_count_ elements). This is the "snapshot = copy the buffer bytes"
  mechanism from the plan — the caller stores these words in a Crabs record.
  @param out      Destination, at least SlotCount() IUC words. May be NILP
                  (then only the byte count is computed).
  @return         Number of IUC words copied. */
  ISC Snapshot(IUC* out) const;

  /* Restore slot_count_ slots from in (at least slot_count_ elements). */
  void Restore(const IUC* in);

  /* Byte size of the underlying word-aligned buffer (for records/debug). */
  IUD ByteSize() const;

  /* Raw word pointer (for direct probing). Do not retain; the Autoject owns
  it and it is invalidated by destruction. */
  const IUC* Words() const { return words_; }

 private:

  Autoject autoject_;   //< Heap-owned word buffer + RAMFactory.
  IUC* words_;          //< Pointer into the Autoject buffer (reinterpreted).
  ISC slot_count_;      //< Number of IUC slots (immutable).
};

}  // namespace _
#endif
