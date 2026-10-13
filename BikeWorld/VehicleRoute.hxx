// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_VEHICLE_ROUTE_HXX
#define CRABS_BIKEWORLD_VEHICLE_ROUTE_HXX
#include "VehicleRoute.h"
#include "VehicleMap.hxx"
#if SEAM >= CRABS_BIKEWORLD
namespace _ {
TVehicleGuide TVehicleGuidance(const TVehicleRoute& route, FPD x, FPD y,
    FPD heading, FPD lookahead) {
  TVehicleGuide guide;
  if (route.count < 2 || route.count > VehicleRouteMax) return guide;
  ISC nearest = 1;
  FPD best = 1e100, fraction = 0;
  for (ISC i = 1; i < route.count; ++i) {
    FPD dx = route.points[i].x - route.points[i - 1].x;
    FPD dy = route.points[i].y - route.points[i - 1].y;
    FPD length2 = dx * dx + dy * dy;
    FPD t = length2 > 1e-12 ? TBikeClamp(
        ((x - route.points[i - 1].x) * dx +
         (y - route.points[i - 1].y) * dy) / length2, 0, 1) : 0;
    FPD error = hypot(x - route.points[i - 1].x - t * dx,
                      y - route.points[i - 1].y - t * dy);
    if (error < best) { best = error; nearest = i; fraction = t; }
  }
  guide.cross_track = best;
  TVehiclePoint point = route.points[nearest - 1];
  point.x += fraction * (route.points[nearest].x - point.x);
  point.y += fraction * (route.points[nearest].y - point.y);
  FPD ahead = lookahead;
  BOL target_set = false;
  for (ISC i = nearest; i < route.count; ++i) {
    FPD length = hypot(route.points[i].x - point.x,
                       route.points[i].y - point.y);
    guide.remaining += length;
    if (!target_set && length >= ahead && length > 1e-12) {
      FPD t = ahead / length;
      guide.target_x = point.x + t * (route.points[i].x - point.x);
      guide.target_y = point.y + t * (route.points[i].y - point.y);
      target_set = true;
    }
    ahead -= length; point = route.points[i];
  }
  if (!target_set) { guide.target_x = point.x; guide.target_y = point.y; }
  FPD dx = guide.target_x - x, dy = guide.target_y - y;
  FPD lateral = -sin(heading) * dx + cos(heading) * dy;
  guide.curvature = 2 * lateral / TBikeMax(dx * dx + dy * dy, 1.0);
  return guide;
}
BOL AVehiclePlanner::Earlier(ISC a, ISC b) const {
  return priorities_[a] < priorities_[b] ||
      (priorities_[a] == priorities_[b] && a < b);
}
void AVehiclePlanner::HeapRaise(ISC node) {
  ISC index = heap_positions_[node];
  if (index < 0) {
    index = heap_count_++; heap_[index] = node;
    heap_positions_[node] = index;
  }
  while (index > 0) {
    ISC parent = (index - 1) / 2;
    if (!Earlier(node, heap_[parent])) break;
    heap_[index] = heap_[parent]; heap_positions_[heap_[index]] = index;
    index = parent;
  }
  heap_[index] = node; heap_positions_[node] = index;
}
ISC AVehiclePlanner::HeapPop() {
  ISC result = heap_[0], node = heap_[--heap_count_], index = 0;
  heap_positions_[result] = -1;
  if (heap_count_ == 0) return result;
  while (index * 2 + 1 < heap_count_) {
    ISC child = index * 2 + 1;
    if (child + 1 < heap_count_ && Earlier(heap_[child + 1], heap_[child]))
      ++child;
    if (!Earlier(heap_[child], node)) break;
    heap_[index] = heap_[child]; heap_positions_[heap_[index]] = index;
    index = child;
  }
  heap_[index] = node; heap_positions_[node] = index;
  return result;
}
BOL AVehiclePlanner::Plan(const TileMap& map, FPD sx, FPD sy,
    FPD gx, FPD gy, FPD clearance, TVehicleRoute& route) {
  route.count = 0;
  ISC width = map.SizeGridMax().x, height = map.SizeGridMax().y;
  FPD cell = map.GridSize(), contact;
  if (width <= 0 || height <= 0 || IUD(width) * IUD(height) > VehicleCellMax ||
      !isfinite(cell) || cell <= 0 || !isfinite(clearance) || clearance < 0 ||
      !isfinite(sx) || !isfinite(sy) || !isfinite(gx) || !isfinite(gy) ||
      sx < 0 || sy < 0 || gx < 0 || gy < 0 || sx >= width * cell ||
      gx >= width * cell || sy >= height * cell || gy >= height * cell ||
      TVehicleSweep(map, sx, sy, sx, sy, clearance, contact) ||
      TVehicleSweep(map, gx, gy, gx, gy, clearance, contact)) return false;
  route.points[0] = {sx, sy};
  if (!TVehicleSweep(map, sx, sy, gx, gy, clearance, contact)) {
    route.points[1] = {gx, gy}; route.count = 2; return true;
  }
  ISC start = ISC(floor(sy / cell)) * width + ISC(floor(sx / cell));
  ISC goal = ISC(floor(gy / cell)) * width + ISC(floor(gx / cell));
  ISC total = width * height;
  heap_count_ = 0;
  for (ISC i = 0; i < total; ++i) {
    FPD x = (i % width + 0.5) * cell, y = (i / width + 0.5) * cell;
    states_[i] = TVehicleSweep(map, x, y, x, y, clearance, contact) ? 3 : 0;
    costs_[i] = 1e100; parents_[i] = -1; heap_positions_[i] = -1;
  }
  FPD start_cx = (start % width + 0.5) * cell;
  FPD start_cy = (start / width + 0.5) * cell;
  FPD goal_cx = (goal % width + 0.5) * cell;
  FPD goal_cy = (goal / width + 0.5) * cell;
  if (states_[start] == 3 || states_[goal] == 3 ||
      TVehicleSweep(map, sx, sy, start_cx, start_cy, clearance, contact) ||
      TVehicleSweep(map, gx, gy, goal_cx, goal_cy, clearance, contact)) return false;
  costs_[start] = 0; priorities_[start] = 0;
  states_[start] = 1; HeapRaise(start);
  BOL found = false;
  while (heap_count_ > 0) {
    ISC node = HeapPop(); states_[node] = 2;
    if (node == goal) { found = true; break; }
    ISC x = node % width, y = node / width;
    ISC dx[4] = {1, 0, -1, 0}, dy[4] = {0, 1, 0, -1};
    for (ISC d = 0; d < 4; ++d) {
      ISC nx = x + dx[d], ny = y + dy[d];
      if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;
      ISC next = ny * width + nx;
      if (states_[next] >= 2 || costs_[node] + cell >= costs_[next]) continue;
      if (TVehicleSweep(map, (x + 0.5) * cell, (y + 0.5) * cell,
          (nx + 0.5) * cell, (ny + 0.5) * cell, clearance, contact)) continue;
      costs_[next] = costs_[node] + cell; parents_[next] = node;
      priorities_[next] = costs_[next] +
          (fabs(nx - goal % width) + fabs(ny - goal / width)) * cell;
      states_[next] = 1; HeapRaise(next);
    }
  }
  if (!found) return false;
  // Reuse the now-unused heap for the reverse parent chain.
  ISC length = 0;
  for (ISC node = goal; node >= 0; node = parents_[node])
    heap_[length++] = node;
  route.count = 1;
  ISC cursor = length;
  while (cursor > 0) {
    const TVehiclePoint& current = route.points[route.count - 1];
    if (route.count >= VehicleRouteMax) { route.count = 0; return false; }
    if (!TVehicleSweep(map, current.x, current.y, gx, gy, clearance, contact)) {
      route.points[route.count++] = {gx, gy}; return true;
    }
    ISC next = -1;
    for (ISC i = 0; i < cursor; ++i) {
      ISC node = heap_[i];
      FPD x = (node % width + 0.5) * cell, y = (node / width + 0.5) * cell;
      if (!TVehicleSweep(map, current.x, current.y, x, y, clearance, contact)) {
        next = i; route.points[route.count++] = {x, y}; break;
      }
    }
    if (next < 0) { route.count = 0; return false; }
    cursor = next;
  }
  route.count = 0; return false;
}
}  //< namespace _
#endif
#endif
