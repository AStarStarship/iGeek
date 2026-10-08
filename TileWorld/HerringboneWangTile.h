// Copyright AStarship <https://astarship.net>.

#ifndef INCLUDE_STB_HWANG_H
#define INCLUDE_STB_HWANG_H

#ifdef STB_HBWANG_STATIC
#define STBHW_EXTERN static
#else
#ifdef __cplusplus
#define STBHW_EXTERN extern "C"
#else
#define STBHW_EXTERN extern
#endif
#endif

typedef struct stbhw_tileset stbhw_tileset;

// returns description of last error produced by any function (not thread-safe)
STBHW_EXTERN const char *stbhw_get_last_error(void);

// build a tileset from an image that conforms to a template created by this
// library. (you allocate storage for stbhw_tileset and function fills it out;
// memory for individual tiles are malloc()ed).
// returns non-zero on success, 0 on error
STBHW_EXTERN ISN stbhw_build_tileset_from_image(stbhw_tileset *ts,
                                                unsigned char *pixels,
                                                ISN stride_in_bytes, ISN w,
                                                ISN h);

// free a tileset built by stbhw_build_tileset_from_image
STBHW_EXTERN void stbhw_free_tileset(stbhw_tileset *ts);

// generate a map that is w * h pixels (3-bytes each)
// returns non-zero on success, 0 on error
// not thread-safe (uses a global data structure to avoid memory management)
// weighting should be NULL, as non-NULL weighting is currently untested
STBHW_EXTERN ISN stbhw_generate_image(stbhw_tileset *ts, ISN **weighting,
                                      unsigned char *pixels,
                                      ISN stride_in_bytes, ISN w, ISN h);

//////////////////////////////////////
//
// TILESET DATA STRUCTURE
//
// if you use the image-to-tileset system from this file, you
// don't need to worry about these data structures. but if you
// want to build/load a tileset yourself, you'll need to fill
// these out.

typedef struct {
  // the edge or vertex constraints, according to diagram below
  signed char a, b, c, d, e, f;

  // The herringbone wang tile data; it is a bitmap which is either
  // w=2*short_sidelen,h=short_sidelen, or w=short_sidelen,h=2*short_sidelen.
  // it is always RGB, stored row-major, with no padding between rows.
  // (allocate stbhw_tile structure to be large enough for the pixel data)
  unsigned char pixels[1];
} stbhw_tile;

struct stbhw_tileset {
  ISN is_corner;
  ISN num_color[6];  // number of colors for each of 6 edge types or 4 corner
                     // types
  ISN short_side_len;
  stbhw_tile **h_tiles;
  stbhw_tile **v_tiles;
  ISN num_h_tiles, max_h_tiles;
  ISN num_v_tiles, max_v_tiles;
};

///////////////  TEMPLATE GENERATOR  //////////////////////////

// when requesting a template, you fill out this data
typedef struct {
  ISN is_corner;       // using corner colors or edge colors?
  ISN short_side_len;  // rectangles is 2n x n, n = short_side_len
  ISN num_color[6];    // see below diagram for meaning of the index to this;
                       // 6 values if edge (!is_corner), 4 values if is_corner
                       // legal numbers: 1..8 if edge, 1..4 if is_corner
  ISN num_vary_x;      // additional number of variations along x axis in the
                       // template
  ISN num_vary_y;      // additional number of variations along y axis in the
                       // template
  ISN corner_type_color_template[4][4];
  // if corner_type_color_template[s][t] is non-zero, then any
  // corner of type s generated as color t will get a little
  // corner sample markup in the template image data

} stbhw_config;

// computes the size needed for the template image
STBHW_EXTERN void stbhw_get_template_size(stbhw_config *c, ISN *w, ISN *h);

// generates a template image, assuming data is 3*w*h bytes long, RGB format
STBHW_EXTERN ISN stbhw_make_template(stbhw_config *c, unsigned char *data,
                                     ISN w, ISN h, ISN stride_in_bytes);

#endif  // INCLUDE_STB_HWANG_H

// TILE CONSTRAINT TYPES
//
// there are 4 "types" of corners and 6 types of edges.
// you can configure the tileset to have different numbers
// of colors for each type of color or edge.
//
// corner types:
//
//                     0---*---1---*---2---*---3
//                     |       |               |
//                     *       *               *
//                     |       |               |
//     1---*---2---*---3       0---*---1---*---2
//     |               |       |
//     *               *       *
//     |               |       |
//     0---*---1---*---2---*---3
//
//
//  edge types:
//
//     *---2---*---3---*      *---0---*
//     |               |      |       |
//     1               4      5       1
//     |               |      |       |
//     *---0---*---2---*      *       *
//                            |       |
//                            4       5
//                            |       |
//                            *---3---*
//
// TILE CONSTRAINTS
//
// each corner/edge has a color; this shows the name
// of the variable containing the color
//
// corner constraints:
//
//                        a---*---d
//                        |       |
//                        *       *
//                        |       |
//     a---*---b---*---c  b       e
//     |               |  |       |
//     *               *  *       *
//     |               |  |       |
//     d---*---e---*---f  c---*---f
//
//
//  edge constraints:
//
//     *---a---*---b---*      *---a---*
//     |               |      |       |
//     c               d      b       c
//     |               |      |       |
//     *---e---*---f---*      *       *
//                            |       |
//                            d       e
//                            |       |
//                            *---f---*
//

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                       IMPLEMENTATION SECTION                             //
//                                                                          //

#ifdef STB_HERRINGBONE_WANG_TILE_IMPLEMENTATION

#include <stdlib.h>  // malloc
#include <string.h>  // memcpy

#ifndef STB_HBWANG_RAND
#include <stdlib.h>
#define STB_HBWANG_RAND() (rand() >> 4)
#endif

#ifndef STB_HBWANG_ASSERT
#include <assert.h>
#define STB_HBWANG_ASSERT(x) assert(x)
#endif

// map size
#ifndef STB_HBWANG_MAX_X
#define STB_HBWANG_MAX_X 100
#endif

#ifndef STB_HBWANG_MAX_Y
#define STB_HBWANG_MAX_Y 100
#endif

// global variables for color assignments
// @MEMORY change these to just store last two/three rows
//         and keep them on the stack
static signed char c_color[STB_HBWANG_MAX_Y + 6][STB_HBWANG_MAX_X + 6];
static signed char v_color[STB_HBWANG_MAX_Y + 6][STB_HBWANG_MAX_X + 5];
static signed char h_color[STB_HBWANG_MAX_Y + 5][STB_HBWANG_MAX_X + 6];

static const char *stbhw_error;
STBHW_EXTERN const char *stbhw_get_last_error(void) {
  const char *temp = stbhw_error;
  stbhw_error = 0;
  return temp;
}

/////////////////////////////////////////////////////////////
//
//  SHARED TEMPLATE-DESCRIPTION CODE
//
//  Used by both template generator and tileset parser; by
//  using the same code, they are locked in sync and we don't
//  need to try to do more sophisticated parsing of edge color
//  markup or something.

typedef void stbhw__process_rect(struct stbhw__process *p, ISN xpos, ISN ypos,
                                 ISN a, ISN b, ISN c, ISN d, ISN e, ISN f);

typedef struct stbhw__process {
  stbhw_tileset *ts;
  stbhw_config *c;
  stbhw__process_rect *process_h_rect;
  stbhw__process_rect *process_v_rect;
  unsigned char *data;
  ISN stride, w, h;
} stbhw__process;

static void stbhw__process_h_row(stbhw__process *p, ISN xpos, ISN ypos, ISN a0,
                                 ISN a1, ISN b0, ISN b1, ISN c0, ISN c1, ISN d0,
                                 ISN d1, ISN e0, ISN e1, ISN f0, ISN f1,
                                 ISN variants) {
  ISN a, b, c, d, e, f, v;

  for (v = 0; v < variants; ++v)
    for (f = f0; f <= f1; ++f)
      for (e = e0; e <= e1; ++e)
        for (d = d0; d <= d1; ++d)
          for (c = c0; c <= c1; ++c)
            for (b = b0; b <= b1; ++b)
              for (a = a0; a <= a1; ++a) {
                p->process_h_rect(p, xpos, ypos, a, b, c, d, e, f);
                xpos += 2 * p->c->short_side_len + 3;
              }
}

static void stbhw__process_v_row(stbhw__process *p, ISN xpos, ISN ypos, ISN a0,
                                 ISN a1, ISN b0, ISN b1, ISN c0, ISN c1, ISN d0,
                                 ISN d1, ISN e0, ISN e1, ISN f0, ISN f1,
                                 ISN variants) {
  ISN a, b, c, d, e, f, v;

  for (v = 0; v < variants; ++v)
    for (f = f0; f <= f1; ++f)
      for (e = e0; e <= e1; ++e)
        for (d = d0; d <= d1; ++d)
          for (c = c0; c <= c1; ++c)
            for (b = b0; b <= b1; ++b)
              for (a = a0; a <= a1; ++a) {
                p->process_v_rect(p, xpos, ypos, a, b, c, d, e, f);
                xpos += p->c->short_side_len + 3;
              }
}

static void stbhw__get_template_info(stbhw_config *c, ISN *w, ISN *h,
                                     ISN *h_count, ISN *v_count) {
  ISN size_x, size_y;
  ISN horz_count, vert_count;

  if (c->is_corner) {
    ISN horz_w =
        c->num_color[1] * c->num_color[2] * c->num_color[3] * c->num_vary_x;
    ISN horz_h =
        c->num_color[0] * c->num_color[1] * c->num_color[2] * c->num_vary_y;

    ISN vert_w =
        c->num_color[0] * c->num_color[3] * c->num_color[2] * c->num_vary_y;
    ISN vert_h =
        c->num_color[1] * c->num_color[0] * c->num_color[3] * c->num_vary_x;

    ISN horz_x = horz_w * (2 * c->short_side_len + 3);
    ISN horz_y = horz_h * (c->short_side_len + 3);

    ISN vert_x = vert_w * (c->short_side_len + 3);
    ISN vert_y = vert_h * (2 * c->short_side_len + 3);

    horz_count = horz_w * horz_h;
    vert_count = vert_w * vert_h;

    size_x = horz_x > vert_x ? horz_x : vert_x;
    size_y = 2 + horz_y + 2 + vert_y;
  } else {
    ISN horz_w =
        c->num_color[0] * c->num_color[1] * c->num_color[2] * c->num_vary_x;
    ISN horz_h =
        c->num_color[3] * c->num_color[4] * c->num_color[2] * c->num_vary_y;

    ISN vert_w =
        c->num_color[0] * c->num_color[5] * c->num_color[1] * c->num_vary_y;
    ISN vert_h =
        c->num_color[3] * c->num_color[4] * c->num_color[5] * c->num_vary_x;

    ISN horz_x = horz_w * (2 * c->short_side_len + 3);
    ISN horz_y = horz_h * (c->short_side_len + 3);

    ISN vert_x = vert_w * (c->short_side_len + 3);
    ISN vert_y = vert_h * (2 * c->short_side_len + 3);

    horz_count = horz_w * horz_h;
    vert_count = vert_w * vert_h;

    size_x = horz_x > vert_x ? horz_x : vert_x;
    size_y = 2 + horz_y + 2 + vert_y;
  }
  if (w) *w = size_x;
  if (h) *h = size_y;
  if (h_count) *h_count = horz_count;
  if (v_count) *v_count = vert_count;
}

STBHW_EXTERN void stbhw_get_template_size(stbhw_config *c, ISN *w, ISN *h) {
  stbhw__get_template_info(c, w, h, NULL, NULL);
}

static ISN stbhw__process_template(stbhw__process *p) {
  ISN i, j, k, q, ypos;
  ISN size_x, size_y;
  stbhw_config *c = p->c;

  stbhw__get_template_info(c, &size_x, &size_y, NULL, NULL);

  if (p->w < size_x || p->h < size_y) {
    stbhw_error = "image too small for configuration";
    return 0;
  }

  if (c->is_corner) {
    ypos = 2;
    for (k = 0; k < c->num_color[2]; ++k) {
      for (j = 0; j < c->num_color[1]; ++j) {
        for (i = 0; i < c->num_color[0]; ++i) {
          for (q = 0; q < c->num_vary_y; ++q) {
            stbhw__process_h_row(p, 0, ypos, 0, c->num_color[1] - 1, 0,
                                 c->num_color[2] - 1, 0, c->num_color[3] - 1, i,
                                 i, j, j, k, k, c->num_vary_x);
            ypos += c->short_side_len + 3;
          }
        }
      }
    }
    ypos += 2;
    for (k = 0; k < c->num_color[3]; ++k) {
      for (j = 0; j < c->num_color[0]; ++j) {
        for (i = 0; i < c->num_color[1]; ++i) {
          for (q = 0; q < c->num_vary_x; ++q) {
            stbhw__process_v_row(p, 0, ypos, 0, c->num_color[0] - 1, 0,
                                 c->num_color[3] - 1, 0, c->num_color[2] - 1, i,
                                 i, j, j, k, k, c->num_vary_y);
            ypos += (c->short_side_len * 2) + 3;
          }
        }
      }
    }
    assert(ypos == size_y);
  } else {
    ypos = 2;
    for (k = 0; k < c->num_color[3]; ++k) {
      for (j = 0; j < c->num_color[4]; ++j) {
        for (i = 0; i < c->num_color[2]; ++i) {
          for (q = 0; q < c->num_vary_y; ++q) {
            stbhw__process_h_row(p, 0, ypos, 0, c->num_color[2] - 1, k, k, 0,
                                 c->num_color[1] - 1, j, j, 0,
                                 c->num_color[0] - 1, i, i, c->num_vary_x);
            ypos += c->short_side_len + 3;
          }
        }
      }
    }
    ypos += 2;
    for (k = 0; k < c->num_color[3]; ++k) {
      for (j = 0; j < c->num_color[4]; ++j) {
        for (i = 0; i < c->num_color[5]; ++i) {
          for (q = 0; q < c->num_vary_x; ++q) {
            stbhw__process_v_row(p, 0, ypos, 0, c->num_color[0] - 1, i, i, 0,
                                 c->num_color[1] - 1, j, j, 0,
                                 c->num_color[5] - 1, k, k, c->num_vary_y);
            ypos += (c->short_side_len * 2) + 3;
          }
        }
      }
    }
    assert(ypos == size_y);
  }
  return 1;
}

/////////////////////////////////////////////////////////////
//
//  MAP GENERATOR
//

static void stbhw__draw_pixel(unsigned char *output, ISN stride, ISN x, ISN y,
                              unsigned char c[3]) {
  memcpy(output + y * stride + x * 3, c, 3);
}

static void stbhw__draw_h_tile(unsigned char *output, ISN stride, ISN xmax,
                               ISN ymax, ISN x, ISN y, stbhw_tile *h, ISN sz) {
  ISN i, j;
  for (j = 0; j < sz; ++j)
    if (y + j >= 0 && y + j < ymax)
      for (i = 0; i < sz * 2; ++i)
        if (x + i >= 0 && x + i < xmax)
          stbhw__draw_pixel(output, stride, x + i, y + j,
                            &h->pixels[(j * sz * 2 + i) * 3]);
}

static void stbhw__draw_v_tile(unsigned char *output, ISN stride, ISN xmax,
                               ISN ymax, ISN x, ISN y, stbhw_tile *h, ISN sz) {
  ISN i, j;
  for (j = 0; j < sz * 2; ++j)
    if (y + j >= 0 && y + j < ymax)
      for (i = 0; i < sz; ++i)
        if (x + i >= 0 && x + i < xmax)
          stbhw__draw_pixel(output, stride, x + i, y + j,
                            &h->pixels[(j * sz + i) * 3]);
}

// randomly choose a tile that fits constraints for a given spot, and update the
// constraints
static stbhw_tile *stbhw__choose_tile(stbhw_tile **list, ISN numlist,
                                      signed char *a, signed char *b,
                                      signed char *c, signed char *d,
                                      signed char *e, signed char *f,
                                      ISN **weighting) {
  ISN i, n, m = 1 << 30, pass;
  for (pass = 0; pass < 2; ++pass) {
    n = 0;
    // pass #1:
    //   count number of variants that match this partial set of constraints
    // pass #2:
    //   stop on randomly selected match
    for (i = 0; i < numlist; ++i) {
      stbhw_tile *h = list[i];
      if ((*a < 0 || *a == h->a) && (*b < 0 || *b == h->b) &&
          (*c < 0 || *c == h->c) && (*d < 0 || *d == h->d) &&
          (*e < 0 || *e == h->e) && (*f < 0 || *f == h->f)) {
        if (weighting)
          n += weighting[0][i];
        else
          n += 1;
        if (n > m) {
          // use list[i]
          // update constraints to reflect what we placed
          *a = h->a;
          *b = h->b;
          *c = h->c;
          *d = h->d;
          *e = h->e;
          *f = h->f;
          return h;
        }
      }
    }
    if (n == 0) {
      stbhw_error = "couldn't find tile matching constraints";
      return NULL;
    }
    m = STB_HBWANG_RAND() % n;
  }
  STB_HBWANG_ASSERT(0);
  return NULL;
}

static ISN stbhw__match(ISN x, ISN y) {
  return c_color[y][x] == c_color[y + 1][x + 1];
}

static ISN stbhw__weighted(ISN num_options, ISN *weights) {
  ISN k, total, choice;
  total = 0;
  for (k = 0; k < num_options; ++k) total += weights[k];
  choice = STB_HBWANG_RAND() % total;
  total = 0;
  for (k = 0; k < num_options; ++k) {
    total += weights[k];
    if (choice < total) break;
  }
  STB_HBWANG_ASSERT(k < num_options);
  return k;
}

static ISN stbhw__change_color(ISN old_color, ISN num_options, ISN *weights) {
  if (weights) {
    ISN k, total, choice;
    total = 0;
    for (k = 0; k < num_options; ++k)
      if (k != old_color) total += weights[k];
    choice = STB_HBWANG_RAND() % total;
    total = 0;
    for (k = 0; k < num_options; ++k) {
      if (k != old_color) {
        total += weights[k];
        if (choice < total) break;
      }
    }
    STB_HBWANG_ASSERT(k < num_options);
    return k;
  } else {
    ISN offset = 1 + STB_HBWANG_RAND() % (num_options - 1);
    return (old_color + offset) % num_options;
  }
}

// generate a map that is w * h pixels (3-bytes each)
// returns 1 on success, 0 on error
STBHW_EXTERN ISN stbhw_generate_image(stbhw_tileset *ts, ISN **weighting,
                                      unsigned char *output, ISN stride, ISN w,
                                      ISN h) {
  ISN sidelen = ts->short_side_len;
  ISN xmax = (w / sidelen) + 6;
  ISN ymax = (h / sidelen) + 6;
  if (xmax > STB_HBWANG_MAX_X + 6 || ymax > STB_HBWANG_MAX_Y + 6) {
    stbhw_error = "increase STB_HBWANG_MAX_X/Y";
    return 0;
  }

  if (ts->is_corner) {
    ISN i, j, ypos;
    ISN *cc = ts->num_color;

    for (j = 0; j < ymax; ++j) {
      for (i = 0; i < xmax; ++i) {
        ISN p = (i - j + 1) & 3;  // corner type
        if (weighting == NULL || weighting[p] == 0 || cc[p] == 1)
          c_color[j][i] = STB_HBWANG_RAND() % cc[p];
        else
          c_color[j][i] = stbhw__weighted(cc[p], weighting[p]);
      }
    }
#ifndef STB_HBWANG_NO_REPITITION_REDUCTION
    // now go back through and make sure we don't have adjancent 3x2 vertices
    // that are identical, to avoid really obvious repetition (which happens
    // easily with extreme weights)
    for (j = 0; j < ymax - 3; ++j) {
      for (i = 0; i < xmax - 3; ++i) {
        // ISN p = (i-j+1) & 3; // corner type   // unused, not sure what the
        // intent was so commenting it out
        STB_HBWANG_ASSERT(i + 3 < STB_HBWANG_MAX_X + 6);
        STB_HBWANG_ASSERT(j + 3 < STB_HBWANG_MAX_Y + 6);
        if (stbhw__match(i, j) && stbhw__match(i, j + 1) &&
            stbhw__match(i, j + 2) && stbhw__match(i + 1, j) &&
            stbhw__match(i + 1, j + 1) && stbhw__match(i + 1, j + 2)) {
          ISN p = ((i + 1) - (j + 1) + 1) & 3;
          if (cc[p] > 1)
            c_color[j + 1][i + 1] = stbhw__change_color(
                c_color[j + 1][i + 1], cc[p], weighting ? weighting[p] : NULL);
        }
        if (stbhw__match(i, j) && stbhw__match(i + 1, j) &&
            stbhw__match(i + 2, j) && stbhw__match(i, j + 1) &&
            stbhw__match(i + 1, j + 1) && stbhw__match(i + 2, j + 1)) {
          ISN p = ((i + 2) - (j + 1) + 1) & 3;
          if (cc[p] > 1)
            c_color[j + 1][i + 2] = stbhw__change_color(
                c_color[j + 1][i + 2], cc[p], weighting ? weighting[p] : NULL);
        }
      }
    }
#endif

    ypos = -1 * sidelen;
    for (j = -1; ypos < h; ++j) {
      // a general herringbone row consists of:
      //    horizontal left block, the bottom of a previous vertical, the top of
      //    a new vertical
      ISN phase = (j & 3);
      // displace horizontally according to pattern
      if (phase == 0) {
        i = 0;
      } else {
        i = phase - 4;
      }
      for (;; i += 4) {
        ISN xpos = i * sidelen;
        if (xpos >= w) break;
        // horizontal left-block
        if (xpos + sidelen * 2 >= 0 && ypos >= 0) {
          stbhw_tile *t = stbhw__choose_tile(
              ts->h_tiles, ts->num_h_tiles, &c_color[j + 2][i + 2],
              &c_color[j + 2][i + 3], &c_color[j + 2][i + 4],
              &c_color[j + 3][i + 2], &c_color[j + 3][i + 3],
              &c_color[j + 3][i + 4], weighting);
          if (t == NULL) return 0;
          stbhw__draw_h_tile(output, stride, w, h, xpos, ypos, t, sidelen);
        }
        xpos += sidelen * 2;
        // now we're at the end of a previous vertical one
        xpos += sidelen;
        // now we're at the start of a new vertical one
        if (xpos < w) {
          stbhw_tile *t = stbhw__choose_tile(
              ts->v_tiles, ts->num_v_tiles, &c_color[j + 2][i + 5],
              &c_color[j + 3][i + 5], &c_color[j + 4][i + 5],
              &c_color[j + 2][i + 6], &c_color[j + 3][i + 6],
              &c_color[j + 4][i + 6], weighting);
          if (t == NULL) return 0;
          stbhw__draw_v_tile(output, stride, w, h, xpos, ypos, t, sidelen);
        }
      }
      ypos += sidelen;
    }
  } else {
    // @TODO edge-color repetition reduction
    ISN i, j, ypos;
    memset(v_color, -1, sizeof(v_color));
    memset(h_color, -1, sizeof(h_color));

    ypos = -1 * sidelen;
    for (j = -1; ypos < h; ++j) {
      // a general herringbone row consists of:
      //    horizontal left block, the bottom of a previous vertical, the top of
      //    a new vertical
      ISN phase = (j & 3);
      // displace horizontally according to pattern
      if (phase == 0) {
        i = 0;
      } else {
        i = phase - 4;
      }
      for (;; i += 4) {
        ISN xpos = i * sidelen;
        if (xpos >= w) break;
        // horizontal left-block
        if (xpos + sidelen * 2 >= 0 && ypos >= 0) {
          stbhw_tile *t = stbhw__choose_tile(
              ts->h_tiles, ts->num_h_tiles, &h_color[j + 2][i + 2],
              &h_color[j + 2][i + 3], &v_color[j + 2][i + 2],
              &v_color[j + 2][i + 4], &h_color[j + 3][i + 2],
              &h_color[j + 3][i + 3], weighting);
          if (t == NULL) return 0;
          stbhw__draw_h_tile(output, stride, w, h, xpos, ypos, t, sidelen);
        }
        xpos += sidelen * 2;
        // now we're at the end of a previous vertical one
        xpos += sidelen;
        // now we're at the start of a new vertical one
        if (xpos < w) {
          stbhw_tile *t = stbhw__choose_tile(
              ts->v_tiles, ts->num_v_tiles, &h_color[j + 2][i + 5],
              &v_color[j + 2][i + 5], &v_color[j + 2][i + 6],
              &v_color[j + 3][i + 5], &v_color[j + 3][i + 6],
              &h_color[j + 4][i + 5], weighting);
          if (t == NULL) return 0;
          stbhw__draw_v_tile(output, stride, w, h, xpos, ypos, t, sidelen);
        }
      }
      ypos += sidelen;
    }
  }
  return 1;
}

static void stbhw__parse_h_rect(stbhw__process *p, ISN xpos, ISN ypos, ISN a,
                                ISN b, ISN c, ISN d, ISN e, ISN f) {
  ISN len = p->c->short_side_len;
  stbhw_tile *h = (stbhw_tile *)malloc(sizeof(*h) - 1 + 3 * (len * 2) * len);
  ISN i, j;
  ++xpos;
  ++ypos;
  h->a = a, h->b = b, h->c = c, h->d = d, h->e = e, h->f = f;
  for (j = 0; j < len; ++j)
    for (i = 0; i < len * 2; ++i)
      memcpy(h->pixels + j * (3 * len * 2) + i * 3,
             p->data + (ypos + j) * p->stride + (xpos + i) * 3, 3);
  STB_HBWANG_ASSERT(p->ts->num_h_tiles < p->ts->max_h_tiles);
  p->ts->h_tiles[p->ts->num_h_tiles++] = h;
}

static void stbhw__parse_v_rect(stbhw__process *p, ISN xpos, ISN ypos, ISN a,
                                ISN b, ISN c, ISN d, ISN e, ISN f) {
  ISN len = p->c->short_side_len;
  stbhw_tile *h = (stbhw_tile *)malloc(sizeof(*h) - 1 + 3 * (len * 2) * len);
  ISN i, j;
  ++xpos;
  ++ypos;
  h->a = a, h->b = b, h->c = c, h->d = d, h->e = e, h->f = f;
  for (j = 0; j < len * 2; ++j)
    for (i = 0; i < len; ++i)
      memcpy(h->pixels + j * (3 * len) + i * 3,
             p->data + (ypos + j) * p->stride + (xpos + i) * 3, 3);
  STB_HBWANG_ASSERT(p->ts->num_v_tiles < p->ts->max_v_tiles);
  p->ts->v_tiles[p->ts->num_v_tiles++] = h;
}

STBHW_EXTERN ISN stbhw_build_tileset_from_image(stbhw_tileset *ts,
                                                unsigned char *data, ISN stride,
                                                ISN w, ISN h) {
  ISN i, h_count, v_count;
  unsigned char header[9];
  stbhw_config c = {0};
  stbhw__process p = {0};

  // extract binary header

  // remove encoding that makes it more visually obvious it encodes actual data
  for (i = 0; i < 9; ++i) header[i] = data[w * 3 - 1 - i] ^ (i * 55);

  // extract header info
  if (header[7] == 0xc0) {
    // corner-type
    c.is_corner = 1;
    for (i = 0; i < 4; ++i) c.num_color[i] = header[i];
    c.num_vary_x = header[4];
    c.num_vary_y = header[5];
    c.short_side_len = header[6];
  } else {
    c.is_corner = 0;
    // edge-type
    for (i = 0; i < 6; ++i) c.num_color[i] = header[i];
    c.num_vary_x = header[6];
    c.num_vary_y = header[7];
    c.short_side_len = header[8];
  }

  if (c.num_vary_x < 0 || c.num_vary_x > 64 || c.num_vary_y < 0 ||
      c.num_vary_y > 64)
    return 0;
  if (c.short_side_len == 0) return 0;
  if (c.num_color[0] > 32 || c.num_color[1] > 32 || c.num_color[2] > 32 ||
      c.num_color[3] > 32)
    return 0;

  stbhw__get_template_info(&c, NULL, NULL, &h_count, &v_count);

  ts->is_corner = c.is_corner;
  ts->short_side_len = c.short_side_len;
  memcpy(ts->num_color, c.num_color, sizeof(ts->num_color));

  ts->max_h_tiles = h_count;
  ts->max_v_tiles = v_count;

  ts->num_h_tiles = ts->num_v_tiles = 0;

  ts->h_tiles = (stbhw_tile **)malloc(sizeof(*ts->h_tiles) * h_count);
  ts->v_tiles = (stbhw_tile **)malloc(sizeof(*ts->v_tiles) * v_count);

  p.ts = ts;
  p.data = data;
  p.stride = stride;
  p.process_h_rect = stbhw__parse_h_rect;
  p.process_v_rect = stbhw__parse_v_rect;
  p.w = w;
  p.h = h;
  p.c = &c;

  // load all the tiles out of the image
  return stbhw__process_template(&p);
}

STBHW_EXTERN void stbhw_free_tileset(stbhw_tileset *ts) {
  ISN i;
  for (i = 0; i < ts->num_h_tiles; ++i) free(ts->h_tiles[i]);
  for (i = 0; i < ts->num_v_tiles; ++i) free(ts->v_tiles[i]);
  free(ts->h_tiles);
  free(ts->v_tiles);
  ts->h_tiles = NULL;
  ts->v_tiles = NULL;
  ts->num_h_tiles = ts->max_h_tiles = 0;
  ts->num_v_tiles = ts->max_v_tiles = 0;
}

//////////////////////////////////////////////////////////////////////////////
//
//               GENERATOR
//
//

// shared code

static void stbhw__set_pixel(unsigned char *data, ISN stride, ISN xpos,
                             ISN ypos, unsigned char color[3]) {
  memcpy(data + ypos * stride + xpos * 3, color, 3);
}

static void stbhw__stbhw__set_pixel_whiten(unsigned char *data, ISN stride,
                                           ISN xpos, ISN ypos,
                                           unsigned char color[3]) {
  unsigned char c2[3];
  ISN i;
  for (i = 0; i < 3; ++i) c2[i] = (color[i] * 2 + 255) / 3;
  memcpy(data + ypos * stride + xpos * 3, c2, 3);
}

static unsigned char stbhw__black[3] = {0, 0, 0};

// each edge set gets its own unique color variants
// used http://phrogz.net/css/distinct-colors.html to generate this set,
// but it's not very good and needs to be revised

static unsigned char stbhw__color[7][8][3] = {
    {
        {255, 51, 51},
        {143, 143, 29},
        {0, 199, 199},
        {159, 119, 199},
        {0, 149, 199},
        {143, 0, 143},
        {255, 128, 0},
        {64, 255, 0},
    },
    {
        {235, 255, 30},
        {255, 0, 255},
        {199, 139, 119},
        {29, 143, 57},
        {143, 0, 71},
        {0, 143, 143},
        {0, 99, 199},
        {143, 71, 0},
    },
    {
        {0, 149, 199},
        {143, 0, 143},
        {255, 128, 0},
        {64, 255, 0},
        {255, 191, 0},
        {51, 255, 153},
        {0, 0, 143},
        {199, 119, 159},
    },
    {
        {143, 0, 71},
        {0, 143, 143},
        {0, 99, 199},
        {143, 71, 0},
        {255, 190, 153},
        {0, 255, 255},
        {128, 0, 255},
        {255, 51, 102},
    },
    {
        {255, 191, 0},
        {51, 255, 153},
        {0, 0, 143},
        {199, 119, 159},
        {255, 51, 51},
        {143, 143, 29},
        {0, 199, 199},
        {159, 119, 199},
    },
    {
        {255, 190, 153},
        {0, 255, 255},
        {128, 0, 255},
        {255, 51, 102},
        {235, 255, 30},
        {255, 0, 255},
        {199, 139, 119},
        {29, 143, 57},
    },

    {{40, 40, 40},
     {90, 90, 90},
     {150, 150, 150},
     {200, 200, 200},
     {255, 90, 90},
     {160, 160, 80},
     {50, 150, 150},
     {200, 50, 200}},
};

static void stbhw__draw_hline(unsigned char *data, ISN stride, ISN xpos,
                              ISN ypos, ISN color, ISN len, ISN slot) {
  ISN i;
  ISN j = len * 6 / 16;
  ISN k = len * 10 / 16;
  for (i = 0; i < len; ++i)
    stbhw__set_pixel(data, stride, xpos + i, ypos, stbhw__black);
  if (k - j < 2) {
    j = len / 2 - 1;
    k = j + 2;
    if (len & 1) ++k;
  }
  for (i = j; i < k; ++i)
    stbhw__stbhw__set_pixel_whiten(data, stride, xpos + i, ypos,
                                   stbhw__color[slot][color]);
}

static void stbhw__draw_vline(unsigned char *data, ISN stride, ISN xpos,
                              ISN ypos, ISN color, ISN len, ISN slot) {
  ISN i;
  ISN j = len * 6 / 16;
  ISN k = len * 10 / 16;
  for (i = 0; i < len; ++i)
    stbhw__set_pixel(data, stride, xpos, ypos + i, stbhw__black);
  if (k - j < 2) {
    j = len / 2 - 1;
    k = j + 2;
    if (len & 1) ++k;
  }
  for (i = j; i < k; ++i)
    stbhw__stbhw__set_pixel_whiten(data, stride, xpos, ypos + i,
                                   stbhw__color[slot][color]);
}

//                 0--*--1--*--2--*--3
//                 |     |           |
//                 *     *           *
//                 |     |           |
//     1--*--2--*--3     0--*--1--*--2
//     |           |     |
//     *           *     *
//     |           |     |
//     0--*--1--*--2--*--3
//
// variables while enumerating (no correspondence between corners
// of the types is implied by these variables)
//
//     a-----b-----c      a-----d
//     |           |      |     |
//     |           |      |     |
//     |           |      |     |
//     d-----e-----f      b     e
//                        |     |
//                        |     |
//                        |     |
//                        c-----f
//

unsigned char stbhw__corner_colors[4][4][3] = {
    {
        {255, 0, 0},
        {200, 200, 200},
        {100, 100, 200},
        {255, 200, 150},
    },
    {
        {0, 0, 255},
        {255, 255, 0},
        {100, 200, 100},
        {150, 255, 200},
    },
    {
        {255, 0, 255},
        {80, 80, 80},
        {200, 100, 100},
        {200, 150, 255},
    },
    {
        {0, 255, 255},
        {0, 255, 0},
        {200, 120, 200},
        {255, 200, 200},
    },
};

ISN stbhw__corner_colors_to_edge_color[4][4] = {
    // 0   1   2   3
    {
        0,
        1,
        4,
        9,
    },  // 0
    {
        2,
        3,
        5,
        10,
    },  // 1
    {
        6,
        7,
        8,
        11,
    },  // 2
    {
        12,
        13,
        14,
        15,
    },  // 3
};

#define stbhw__c2e stbhw__corner_colors_to_edge_color

static void stbhw__draw_clipped_corner(unsigned char *data, ISN stride,
                                       ISN xpos, ISN ypos, ISN w, ISN h, ISN x,
                                       ISN y) {
  static unsigned char template_color[3] = {167, 204, 204};
  ISN i, j;
  for (j = -2; j <= 1; ++j) {
    for (i = -2; i <= 1; ++i) {
      if ((i == -2 || i == 1) && (j == -2 || j == 1))
        continue;
      else {
        if (x + i < 1 || x + i > w) continue;
        if (y + j < 1 || y + j > h) continue;
        stbhw__set_pixel(data, stride, xpos + x + i, ypos + y + j,
                         template_color);
      }
    }
  }
}

static void stbhw__edge_process_h_rect(stbhw__process *p, ISN xpos, ISN ypos,
                                       ISN a, ISN b, ISN c, ISN d, ISN e,
                                       ISN f) {
  ISN len = p->c->short_side_len;
  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos, a, len, 2);
  stbhw__draw_hline(p->data, p->stride, xpos + len + 1, ypos, b, len, 3);
  stbhw__draw_vline(p->data, p->stride, xpos, ypos + 1, c, len, 1);
  stbhw__draw_vline(p->data, p->stride, xpos + 2 * len + 1, ypos + 1, d, len,
                    4);
  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos + len + 1, e, len, 0);
  stbhw__draw_hline(p->data, p->stride, xpos + len + 1, ypos + len + 1, f, len,
                    2);
}

static void stbhw__edge_process_v_rect(stbhw__process *p, ISN xpos, ISN ypos,
                                       ISN a, ISN b, ISN c, ISN d, ISN e,
                                       ISN f) {
  ISN len = p->c->short_side_len;
  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos, a, len, 0);
  stbhw__draw_vline(p->data, p->stride, xpos, ypos + 1, b, len, 5);
  stbhw__draw_vline(p->data, p->stride, xpos + len + 1, ypos + 1, c, len, 1);
  stbhw__draw_vline(p->data, p->stride, xpos, ypos + len + 1, d, len, 4);
  stbhw__draw_vline(p->data, p->stride, xpos + len + 1, ypos + len + 1, e, len,
                    5);
  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos + 2 * len + 1, f, len,
                    3);
}

static void stbhw__corner_process_h_rect(stbhw__process *p, ISN xpos, ISN ypos,
                                         ISN a, ISN b, ISN c, ISN d, ISN e,
                                         ISN f) {
  ISN len = p->c->short_side_len;

  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos, stbhw__c2e[a][b], len,
                    2);
  stbhw__draw_hline(p->data, p->stride, xpos + len + 1, ypos, stbhw__c2e[b][c],
                    len, 3);
  stbhw__draw_vline(p->data, p->stride, xpos, ypos + 1, stbhw__c2e[a][d], len,
                    1);
  stbhw__draw_vline(p->data, p->stride, xpos + 2 * len + 1, ypos + 1,
                    stbhw__c2e[c][f], len, 4);
  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos + len + 1,
                    stbhw__c2e[d][e], len, 0);
  stbhw__draw_hline(p->data, p->stride, xpos + len + 1, ypos + len + 1,
                    stbhw__c2e[e][f], len, 2);

  if (p->c->corner_type_color_template[1][a])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len * 2, len, 1,
                               1);
  if (p->c->corner_type_color_template[2][b])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len * 2, len,
                               len + 1, 1);
  if (p->c->corner_type_color_template[3][c])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len * 2, len,
                               len * 2 + 1, 1);

  if (p->c->corner_type_color_template[0][d])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len * 2, len, 1,
                               len + 1);
  if (p->c->corner_type_color_template[1][e])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len * 2, len,
                               len + 1, len + 1);
  if (p->c->corner_type_color_template[2][f])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len * 2, len,
                               len * 2 + 1, len + 1);

  stbhw__set_pixel(p->data, p->stride, xpos, ypos, stbhw__corner_colors[1][a]);
  stbhw__set_pixel(p->data, p->stride, xpos + len, ypos,
                   stbhw__corner_colors[2][b]);
  stbhw__set_pixel(p->data, p->stride, xpos + 2 * len + 1, ypos,
                   stbhw__corner_colors[3][c]);
  stbhw__set_pixel(p->data, p->stride, xpos, ypos + len + 1,
                   stbhw__corner_colors[0][d]);
  stbhw__set_pixel(p->data, p->stride, xpos + len, ypos + len + 1,
                   stbhw__corner_colors[1][e]);
  stbhw__set_pixel(p->data, p->stride, xpos + 2 * len + 1, ypos + len + 1,
                   stbhw__corner_colors[2][f]);
}

static void stbhw__corner_process_v_rect(stbhw__process *p, ISN xpos, ISN ypos,
                                         ISN a, ISN b, ISN c, ISN d, ISN e,
                                         ISN f) {
  ISN len = p->c->short_side_len;

  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos, stbhw__c2e[a][d], len,
                    0);
  stbhw__draw_vline(p->data, p->stride, xpos, ypos + 1, stbhw__c2e[a][b], len,
                    5);
  stbhw__draw_vline(p->data, p->stride, xpos + len + 1, ypos + 1,
                    stbhw__c2e[d][e], len, 1);
  stbhw__draw_vline(p->data, p->stride, xpos, ypos + len + 1, stbhw__c2e[b][c],
                    len, 4);
  stbhw__draw_vline(p->data, p->stride, xpos + len + 1, ypos + len + 1,
                    stbhw__c2e[e][f], len, 5);
  stbhw__draw_hline(p->data, p->stride, xpos + 1, ypos + 2 * len + 1,
                    stbhw__c2e[c][f], len, 3);

  if (p->c->corner_type_color_template[0][a])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len, len * 2, 1,
                               1);
  if (p->c->corner_type_color_template[3][b])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len, len * 2, 1,
                               len + 1);
  if (p->c->corner_type_color_template[2][c])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len, len * 2, 1,
                               len * 2 + 1);

  if (p->c->corner_type_color_template[1][d])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len, len * 2,
                               len + 1, 1);
  if (p->c->corner_type_color_template[0][e])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len, len * 2,
                               len + 1, len + 1);
  if (p->c->corner_type_color_template[3][f])
    stbhw__draw_clipped_corner(p->data, p->stride, xpos, ypos, len, len * 2,
                               len + 1, len * 2 + 1);

  stbhw__set_pixel(p->data, p->stride, xpos, ypos, stbhw__corner_colors[0][a]);
  stbhw__set_pixel(p->data, p->stride, xpos, ypos + len,
                   stbhw__corner_colors[3][b]);
  stbhw__set_pixel(p->data, p->stride, xpos, ypos + 2 * len + 1,
                   stbhw__corner_colors[2][c]);
  stbhw__set_pixel(p->data, p->stride, xpos + len + 1, ypos,
                   stbhw__corner_colors[1][d]);
  stbhw__set_pixel(p->data, p->stride, xpos + len + 1, ypos + len,
                   stbhw__corner_colors[0][e]);
  stbhw__set_pixel(p->data, p->stride, xpos + len + 1, ypos + 2 * len + 1,
                   stbhw__corner_colors[3][f]);
}

// generates a template image, assuming data is 3*w*h bytes long, RGB format
STBHW_EXTERN ISN stbhw_make_template(stbhw_config *c, unsigned char *data,
                                     ISN w, ISN h, ISN stride_in_bytes) {
  stbhw__process p;
  ISN i;

  p.data = data;
  p.w = w;
  p.h = h;
  p.stride = stride_in_bytes;
  p.ts = 0;
  p.c = c;

  if (c->is_corner) {
    p.process_h_rect = stbhw__corner_process_h_rect;
    p.process_v_rect = stbhw__corner_process_v_rect;
  } else {
    p.process_h_rect = stbhw__edge_process_h_rect;
    p.process_v_rect = stbhw__edge_process_v_rect;
  }

  for (i = 0; i < p.h; ++i) memset(p.data + i * p.stride, 255, 3 * p.w);

  if (!stbhw__process_template(&p)) return 0;

  if (c->is_corner) {
    // write out binary information in first line of image
    for (i = 0; i < 4; ++i) data[w * 3 - 1 - i] = c->num_color[i];
    data[w * 3 - 1 - i] = c->num_vary_x;
    data[w * 3 - 2 - i] = c->num_vary_y;
    data[w * 3 - 3 - i] = c->short_side_len;
    data[w * 3 - 4 - i] = 0xc0;
  } else {
    for (i = 0; i < 6; ++i) data[w * 3 - 1 - i] = c->num_color[i];
    data[w * 3 - 1 - i] = c->num_vary_x;
    data[w * 3 - 2 - i] = c->num_vary_y;
    data[w * 3 - 3 - i] = c->short_side_len;
  }

  // make it more obvious it encodes actual data
  for (i = 0; i < 9; ++i) p.data[p.w * 3 - 1 - i] ^= i * 55;

  return 1;
}
#endif  // STB_HBWANG_IMPLEMENTATION
