// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekTensorHXX
#define _iGeekTensorHXX
#include "Tensor.h"
#include "../CrabsTK/Math/Blas.h"
namespace iGeek {
using namespace _;
using namespace CTK;

TensorFPC TensorFPCAlloc(ISC rows, ISC columns) {
  TensorFPC t;
  t.rows_ = rows;
  t.cols_ = columns;
  t.dtype_ = 0;
  ISC n = rows * columns;
  t.data_ = (n > 0) ? new FPC[n]() : _NilP;  // () zero-initializes.
  return t;
}

void TensorFPCFree(TensorFPC& t) {
  if (t.data_ == _NilP) {
    return;  // safe double-free.
  }
  delete[] t.data_;
  t.data_ = _NilP;
  t.rows_ = 0;
  t.cols_ = 0;
}

/* --- Elementwise / linear ops (CPU FPC) -------------------------------- */

void TensorFPCMatMul(TensorFPC A, TensorFPC B, TensorFPC& C) {
  // A: r x k, B: k x c, C: r x c.
  ISC r = A.rows_, k = A.cols_, c = B.cols_;
  for (ISC i = 0; i < r; ++i) {
    for (ISC j = 0; j < c; ++j) {
      FPC sum = 0.0f;
      for (ISC m = 0; m < k; ++m) {
        sum += A.At(i, m) * B.At(m, j);
      }
      C.At(i, j) = sum;
    }
  }
}

void TensorFPCSoftmaxInPlace(TensorFPC& X) {
  // Stable row softmax: subtract row max before exp.
  for (ISC i = 0; i < X.rows_; ++i) {
    FPC maxv = X.At(i, 0);
    for (ISC j = 1; j < X.cols_; ++j) {
      if (X.At(i, j) > maxv) maxv = X.At(i, j);
    }
    FPC sum = 0.0f;
    for (ISC j = 0; j < X.cols_; ++j) {
      FPC e = FPC(exp(X.At(i, j) - maxv));
      X.At(i, j) = e;
      sum += e;
    }
    if (sum > 0.0f) {
      for (ISC j = 0; j < X.cols_; ++j) X.At(i, j) /= sum;
    }
  }
}

void TensorFPCLayerNormInPlace(TensorFPC& X) {
  // Row-wise: x = (x - mean) / SquareRoot(var + eps). eps = 1e-5.
  const FPC eps = 1e-5f;
  for (ISC i = 0; i < X.rows_; ++i) {
    FPC mean = 0.0f;
    for (ISC j = 0; j < X.cols_; ++j) mean += X.At(i, j);
    mean /= FPC(X.cols_);
    FPC var = 0.0f;
    for (ISC j = 0; j < X.cols_; ++j) {
      FPC d = X.At(i, j) - mean;
      var += d * d;
    }
    var /= FPC(X.cols_);
    FPC inv = 1.0f / FPC(SquareRoot(var + eps));
    for (ISC j = 0; j < X.cols_; ++j) X.At(i, j) = (X.At(i, j) - mean) * inv;
  }
}

void TensorFPCReluInPlace(TensorFPC& X) {
  for (ISC i = 0; i < X.rows_; ++i) {
    for (ISC j = 0; j < X.cols_; ++j) {
      FPC v = X.At(i, j);
      X.At(i, j) = (v > 0.0f) ? v : 0.0f;
    }
  }
}

void TensorFPCAdd(TensorFPC A, TensorFPC B, TensorFPC& C) {
  for (ISC i = 0; i < A.rows_; ++i) {
    for (ISC j = 0; j < A.cols_; ++j) {
      C.At(i, j) = A.At(i, j) + B.At(i, j);
    }
  }
}

void TensorFPCAddBiasInPlace(TensorFPC& X, TensorFPC bias) {
  // X: [B, K], bias: [K, 1]. X[i, j] += bias[j, 0].
  ISC B = X.rows_, K = X.cols_;
  for (ISC i = 0; i < B; ++i) {
    for (ISC j = 0; j < K; ++j) {
      X.At(i, j) += bias.At(j, 0);
    }
  }
}

void TensorFPCScale(TensorFPC A, FPC s, TensorFPC& C) {
  for (ISC i = 0; i < A.rows_; ++i) {
    for (ISC j = 0; j < A.cols_; ++j) {
      C.At(i, j) = A.At(i, j) * s;
    }
  }
}

void TensorFPCRowLogSumExp(TensorFPC A, TensorFPC& out) {
  // out[i, 0] = log(sum_j exp(A[i, j])), stable (subtract row max).
  for (ISC i = 0; i < A.rows_; ++i) {
    FPC maxv = A.At(i, 0);
    for (ISC j = 1; j < A.cols_; ++j) {
      if (A.At(i, j) > maxv) maxv = A.At(i, j);
    }
    FPC sum = 0.0f;
    for (ISC j = 0; j < A.cols_; ++j) {
      sum += FPC(exp(A.At(i, j) - maxv));
    }
    out.At(i, 0) = maxv + FPC(log(sum));
  }
}

}       //< namespace iGeek
#endif  //< _iGeekTensorHXX
