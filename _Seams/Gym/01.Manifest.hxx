// Copyright AStarship <https://astarship.net>.
//
// Gym Manifest seam unit: TManifest dump/load/validate round-trip.
// Gated on IGEEK_GYM_MANIFEST.
#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "../../GymManifest.hxx"
#include "List.hpp"
namespace iGeekGymTest {
using namespace iGeek;

ISC m_pass = 0;
ISC m_fail = 0;

inline void MCheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    m_pass++;
    ::_::StdOut() << "PASS " << what << '\n';
  } else {
    m_fail++;
    ::_::StdOut() << "FAIL " << what << '\n';
  }
}

/* A 2048-byte data buffer backing the string _CHA tuples.
   The TList struct is stack-allocated; its SCK map is self-contained.
   The string data values point into this buffer. */
alignas(16) static CHA s_str_data[2048];
ISC s_str_cursor = 0;

inline const CHA* ReserveStr(const CHA* src, ISN len) {
  /* Copy src into s_str_data, return pointer into the buffer. */
  ISN i = 0;
  while (i < len && src[i]) {
    s_str_data[s_str_cursor + i] = src[i];
    ++i;
  }
  s_str_data[s_str_cursor + i] = 0;
  s_str_cursor += i + 1;
  return &s_str_data[s_str_cursor - i - 1];
}

inline const CHA* Manifest(const CHA* args) {
  (void)args;
  A_TEST_BEGIN;
  ::_::StdOut() << Headingf("Testing Gym Manifest (dump/load/validate)...");

  s_str_cursor = 0;
  /* The TList struct must be at the start of a contiguous buffer.
     The SCK map is self-contained (on the stack), but data values are at
     offsets into the buffer. top = sizeof(TList) + total*(sizeof(ISC)+sizeof(DTB))
     = 16 + total*6. For 256 tuples: top=1552. Data: 12*4+195*1=243B.
     Total needed: ~1800B. Use 2048. */
  enum { FloorBytes = 2048, FloorTuples = 256 };
  alignas(16) IUW floor_mem[FloorBytes / sizeof(IUW)];
  TList<ISC, ISC, DTB>& floor =
      *reinterpret_cast<TList<ISC, ISC, DTB>*>(floor_mem);
  floor.bytes = ISC(FloorBytes);
  TListInit<ISC, ISC, DTB>(&floor, ISB(FloorTuples));

  /* Build a valid manifest. */
  TManifest m;
  TManifestInit(&m);
  CopyStr(m.name, ManifestNameLen, "car_world");
  CopyStr(m.version, ManifestVersionLen, "1.0.0");
  CopyStr(m.description, ManifestDescriptionLen, "Planar car driving");
  CopyStr(m.modality, ManifestModalityLen, "vector");
  CopyStr(m.world_name, ManifestWorldLen, "CarWorld");
  m.kind = ManifestKindEnv;
  m.domain = ManifestDomainRobotics;
  m.determinism = ManifestDeterminismSeeded;
  m.reward_low = FPC(-1.0f);
  m.reward_high = FPC(1.0f);
  m.reward_higher_is_better = true;
  m.obs_count = ISC(8);
  m.action_count = ISC(4);
  m.max_episode_steps = ISC(500);
  m.env_count_max = ISC(8);
  m.seam_number = ISC(IGEEK_CARWORLD_CORE);
  m.manifest_version = ISC(1);

  MCheck(TManifestValidate(&m), "in-memory manifest is valid");

  /* Dump to Floor. */
  MCheck(TManifestDump(&m, &floor), "dump to Floor");
  MCheck(ISC(floor.map.count) == ManifestTupleTotal,
         "Floor tuple count matches");

  /* Load back into a fresh manifest. */
  TManifest m2;
  TManifestInit(&m2);
  MCheck(TManifestLoad(&floor, &m2), "load from Floor");

  /* Verify all fields survived the round-trip. */
  MCheck(m2.name[0] == 'c' && m2.name[1] == 'a' && m2.name[2] == 'r',
         "name round-trip");
  MCheck(m2.version[0] == '1' && m2.version[1] == '.',
         "version round-trip");
  MCheck(m2.kind == ManifestKindEnv, "kind round-trip");
  MCheck(m2.domain == ManifestDomainRobotics, "domain round-trip");
  MCheck(m2.determinism == ManifestDeterminismSeeded, "determinism round-trip");
  MCheck(m2.reward_low == FPC(-1.0f), "reward_low round-trip");
  MCheck(m2.reward_high == FPC(1.0f), "reward_high round-trip");
  MCheck(m2.reward_higher_is_better == true, "reward_higher_is_better round-trip");
  MCheck(m2.obs_count == ISC(8), "obs_count round-trip");
  MCheck(m2.action_count == ISC(4), "action_count round-trip");
  MCheck(m2.max_episode_steps == ISC(500), "max_episode_steps round-trip");
  MCheck(m2.env_count_max == ISC(8), "env_count_max round-trip");
  MCheck(m2.seam_number == ISC(IGEEK_CARWORLD_CORE), "seam_number round-trip");
  MCheck(m2.manifest_version == ISC(1), "manifest_version round-trip");
  MCheck(m2.world_name[0] == 'C' && m2.world_name[1] == 'a' &&
         m2.world_name[2] == 'r',
         "world_name round-trip");
  MCheck(TManifestValidate(&m2), "loaded manifest is valid");

  /* Invalid manifests fail validation. */
  {
    TManifest bad;
    TManifestInit(&bad);
    CopyStr(bad.name, ManifestNameLen, "test");
    CopyStr(bad.version, ManifestVersionLen, "0.1");
    bad.kind = ManifestKindEnv;
    bad.domain = ManifestDomainGame;
    bad.determinism = ManifestDeterminismUnknown;
    bad.obs_count = ISC(0);
    bad.action_count = ISC(4);
    bad.max_episode_steps = ISC(100);
    bad.env_count_max = ISC(8);
    MCheck(!TManifestValidate(&bad), "obs_count=0 is invalid");
  }
  {
    TManifest bad;
    TManifestInit(&bad);
    CopyStr(bad.name, ManifestNameLen, "test");
    CopyStr(bad.version, ManifestVersionLen, "0.1");
    bad.kind = ManifestKindInvalid;
    bad.domain = ManifestDomainGame;
    bad.determinism = ManifestDeterminismUnknown;
    bad.obs_count = ISC(4);
    bad.action_count = ISC(4);
    bad.max_episode_steps = ISC(100);
    bad.env_count_max = ISC(8);
    MCheck(!TManifestValidate(&bad), "kind=Invalid is invalid");
  }
  {
    TManifest bad;
    TManifestInit(&bad);
    CopyStr(bad.name, ManifestNameLen, "test");
    CopyStr(bad.version, ManifestVersionLen, "0.1");
    bad.kind = ManifestKindEnv;
    bad.domain = ManifestDomainGame;
    bad.determinism = ManifestDeterminismUnknown;
    bad.reward_low = FPC(1.0f);
    bad.reward_high = FPC(0.0f);
    bad.obs_count = ISC(4);
    bad.action_count = ISC(4);
    bad.max_episode_steps = ISC(100);
    bad.env_count_max = ISC(8);
    MCheck(!TManifestValidate(&bad), "reward_low > reward_high is invalid");
  }

  /* Load from empty Floor fails. */
  {
    enum { EmptyBytes = 512, EmptyTuples = 64 };
    alignas(16) IUW empty_mem[EmptyBytes / sizeof(IUW)];
    TList<ISC, ISC, DTB>& empty_floor =
        *reinterpret_cast<TList<ISC, ISC, DTB>*>(empty_mem);
    empty_floor.bytes = ISC(EmptyBytes);
    TListInit<ISC, ISC, DTB>(&empty_floor, ISB(EmptyTuples));
    TManifest m3;
    TManifestInit(&m3);
    MCheck(!TManifestLoad(&empty_floor, &m3), "load from empty Floor fails");
  }

  ::_::StdOut() << "m_pass=" << m_pass << " m_fail=" << m_fail << '\n';
  return m_fail ? "GYM_MANIFEST_FAIL: " : NILP;
}

}  //< namespace iGeekGymTest
#endif
