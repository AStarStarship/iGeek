// Copyright AStarship <https://astarship.net>.
// ExoArena (memory model) — the ASCIICrabs arena that backs the Exo/WebGL
// demo. The RGBA8 texture pixels live in a Room -> Wall -> root List slab;
// there is no C heap. The GL layer (ExoArenaWeb.cpp) uploads Pixels() with
// ExoCreateImageRGBA and draws them with ExoImagePattern.
#pragma once
#ifndef CRABSTK_IGEEK_EXOARENA_HPP
#define CRABSTK_IGEEK_EXOARENA_HPP
#include <ASCIICrabs/Room.hpp>
#include <ASCIICrabs/Wall.hpp>

namespace iGeek {

/* Owns the arena Room and an RGBA8 texture whose bytes live in the Room's
 * Wall root List. The template parameters are the canonical ROM_A aliases, so
 * ROM_P and the sanctioned LST / WAL_P / LST_P forms are usable. */
template<ROM_A>
class AExoArena {
 public:
  AExoArena() = default;

  /* Build the Room + Wall + root List, then fill a tex_w x tex_h RGBA8
   * procedural plate into the slab. @return false if it can't fit. */
  BOL Init(ISC tex_w = 64, ISC tex_h = 64) {
    if (tex_w < 1 || tex_h < 1) return false;
    tex_w_ = tex_w;
    tex_h_ = tex_h;
    tex_bytes_ = tex_w * tex_h * 4;
    if (!TWallInit(&wall_, ISW(tex_bytes_ + WallSlack))) return false;
    if (room_.AddWall(&wall_) != &wall_) return false;
    root_ = TWallRoot(&wall_);
    if (!root_) return false;
    tex_index_ = TListAlloc<LST_P>(root_, DTB(_CHA), ISZ(tex_bytes_));
    if (tex_index_ < 0) return false;
    tex_pixels_ = TListValue<LST_P, IUA>(root_, tex_index_);
    if (!tex_pixels_) return false;
    FillTexture();
    return true;
  }

  const IUA* Pixels() const { return tex_pixels_; }
  ISC Width() const { return tex_w_; }
  ISC Height() const { return tex_h_; }
  ISC Bytes() const { return tex_bytes_; }
  ISY Index() const { return tex_index_; }
  ISW WallBytes() { return TWallGetSizeBytes(&wall_); }
  IUD RoomBytes() { return room_.GetBytes(); }

 private:
  enum { WallSlack = 4096 };

  /* A tileable arena plate: dark checker plus a warm inner disc, RGBA8. */
  void FillTexture() {
    ISC cx = tex_w_ / 2;
    ISC cy = tex_h_ / 2;
    ISC r2 = (tex_w_ * tex_w_) / 9;
    for (ISC y = 0; y < tex_h_; ++y) {
      for (ISC x = 0; x < tex_w_; ++x) {
        ISC i = (y * tex_w_ + x) * 4;
        ISC dx = x - cx;
        ISC dy = y - cy;
        BOL disc = (dx * dx + dy * dy) < r2;
        BOL check = BOL(((x >> 3) ^ (y >> 3)) & 1);
        tex_pixels_[i + 0] = disc ? IUB(60) : (check ? IUB(26) : IUB(15));
        tex_pixels_[i + 1] = disc ? IUB(220) : (check ? IUB(150) : IUB(110));
        tex_pixels_[i + 2] = disc ? IUB(200) : (check ? IUB(105) : IUB(70));
        tex_pixels_[i + 3] = IUB(255);
      }
    }
  }

  TRoom<ROM_P> room_;
  TWall<WAL_P> wall_;
  LST* root_ = _NilP;
  IUA* tex_pixels_ = _NilP;
  ISY tex_index_ = ISY(-1);
  ISC tex_w_ = 0;
  ISC tex_h_ = 0;
  ISC tex_bytes_ = 0;
};

}  //< namespace iGeek
#endif  //< CRABSTK_IGEEK_EXOARENA_HPP
