// Copyright AStarship <https://astarship.net>.
// Tensor.hxx — implementations of the TTensor ops declared in Tensor.h.
//
// NO C++ standard library. Math (exp/sqrt/log) comes from the C library
// <math.h> (C, not C++ std) — the same pragmatic choice as the rest of the
// single-TU g++ build. Allocation uses new[]/delete[] (compiler builtins,
// no <new> header required).
#pragma once
#include "Tensor.h"
#include <math.h>

namespace _ {

/* --- Allocation -------------------------------------------------------- */

TTensor TTensorAlloc(ISC rows, ISC cols) {
  TTensor t;
  t.rows_ = rows;
  t.cols_ = cols;
  t.dtype_ = 0;
  ISC n = rows * cols;
  t.data_ = (n > 0) ? new FPC[n]() : NILP;  // () zero-initializes.
  return t;
}

void TTensorFree(TTensor& t) {
  if (t.data_ == NILP) {
    return;  // safe double-free.
  }
  delete[] t.data_;
  t.data_ = NILP;
  t.rows_ = 0;
  t.cols_ = 0;
}

/* --- Elementwise / linear ops (CPU FPC) -------------------------------- */

void TTensorMatMul(const TTensor& A, const TTensor& B, TTensor& C) {
  // A: r x k, B: k x c, C: r x c.
  ISC r = A.rows_, k = A.cols_, c = B.cols_;
  for (ISC i = 0; i < r; ++i) {
    for (ISC j = 0; j < c; ++j) {
      FPC sum = 0.0f;
      for (ISC m = 0; m < k; ++m) {
        sum += A.AtC(i, m) * B.AtC(m, j);
      }
      C.At(i, j) = sum;
    }
  }
}

void TTensorSoftmaxInPlace(TTensor& X) {
  // Stable row softmax: subtract row max before exp.
  for (ISC i = 0; i < X.rows_; ++i) {
    FPC maxv = X.At(i, 0);
    for (ISC j = 1; j < X.cols_; ++j) {
      if (X.At(i, j) > maxv) maxv = X.At(i, j);
    }
    FPC sum = 0.0f;
    for (ISC j = 0; j < X.cols_; ++j) {
      FPC e = (FPC)exp(X.At(i, j) - maxv);
      X.At(i, j) = e;
      sum += e;
    }
    if (sum > 0.0f) {
      for (ISC j = 0; j < X.cols_; ++j) X.At(i, j) /= sum;
    }
  }
}

void TTensorLayerNormInPlace(TTensor& X) {
  // Row-wise: x = (x - mean) / sqrt(var + eps). eps = 1e-5.
  const FPC eps = 1e-5f;
  for (ISC i = 0; i < X.rows_; ++i) {
    FPC mean = 0.0f;
    for (ISC j = 0; j < X.cols_; ++j) mean += X.At(i, j);
    mean /= (FPC)X.cols_;
    FPC var = 0.0f;
    for (ISC j = 0; j < X.cols_; ++j) {
      FPC d = X.At(i, j) - mean;
      var += d * d;
    }
    var /= (FPC)X.cols_;
    FPC inv = 1.0f / (FPC)sqrt(var + eps);
    for (ISC j = 0; j < X.cols_; ++j) X.At(i, j) = (X.At(i, j) - mean) * inv;
  }
}

void TTensorReluInPlace(TTensor& X) {
  for (ISC i = 0; i < X.rows_; ++i) {
    for (ISC j = 0; j < X.cols_; ++j) {
      FPC v = X.At(i, j);
      X.At(i, j) = (v > 0.0f) ? v : 0.0f;
    }
  }
}

void TTensorAdd(const TTensor& A, const TTensor& B, TTensor& C) {
  for (ISC i = 0; i < A.rows_; ++i) {
    for (ISC j = 0; j < A.cols_; ++j) {
      C.At(i, j) = A.AtC(i, j) + B.AtC(i, j);
    }
  }
}

void TTensorScale(const TTensor& A, FPC s, TTensor& C) {
  for (ISC i = 0; i < A.rows_; ++i) {
    for (ISC j = 0; j < A.cols_; ++j) {
      C.At(i, j) = A.AtC(i, j) * s;
    }
  }
}

void TTensorRowLogSumExp(const TTensor& A, TTensor& out) {
  // out[i, 0] = log(sum_j exp(A[i, j])), stable (subtract row max).
  for (ISC i = 0; i < A.rows_; ++i) {
    FPC maxv = A.AtC(i, 0);
    for (ISC j = 1; j < A.cols_; ++j) {
      if (A.AtC(i, j) > maxv) maxv = A.AtC(i, j);
    }
    FPC sum = 0.0f;
    for (ISC j = 0; j < A.cols_; ++j) {
      sum += (FPC)exp(A.AtC(i, j) - maxv);
    }
    out.At(i, 0) = maxv + (FPC)log(sum);
  }
}

}  // namespace _
