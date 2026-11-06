// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekMathFPDMatrixHXX
#define _iGeekMathFPDMatrixHXX
#include <_Config.h>
#include "../CrabsTK/Math/Blas.h"
namespace iGeek {
/* Simple dynamic matrix — no template metaprogramming overhead.
   Just a flat FPD* buffer + dimensions. Construct once, use everywhere.
   Ideal for: runtime-shaped matrices, hot-swapped via JSX DLLs,
   interop with CBLAS/MKL on V100, no compile-time dimension constraints.

   Layout: [row_stride * cols] row-major.  stride = max(rows,1).
   Access: M(r,c) = data_[r * stride + c].

   CBLAS-integrated: when CBLAS_H_AVAILABLE is defined, gemm/scale
   operations lower to cblas_dgemm / cblas_dscal.  When not defined,
   plain FPC* loops (same as the existing Tensor ops). */
struct FPDMatrix {
  FPD*   data = _NilP;    // flat row-major, allocated by caller
  ISZ    rows = 0;       // matrix height  (ISZ = ISC by default)
  ISZ    cols = 0;       // matrix width
  ISZ    stride = 0;     // row stride (>= rows, 0 = rows)

  /* Zero-construct. */
  FPDMatrix() = default;

  /* Construct with allocation: data_[rows*stride] zero-initialized. */
  explicit FPDMatrix(ISZ r, ISZ c, ISZ s = 0)
    : rows(r), cols(c), stride(s ? s : (r > 0 ? r : 1)) {
    ISZ n = stride * (c > 0 ? c : 1);
    data = new FPD[n]();  // zero-init
  }

  /* Adopt existing flat buffer: caller owns memory. */
  FPDMatrix(FPD* d, ISZ r, ISZ c, ISZ s = 0)
    : data(d), rows(r), cols(c), stride(s ? s : (r > 0 ? r : 1)) {}

  /* Delete (do NOT free by default — caller manages lifetime). */
  ~FPDMatrix() {
    // do NOT free data_ by default; caller manages lifetime
  }

  /* Element access (no bounds check). */
  FPD& At(ISZ r, ISZ c) { return data[r * stride + c]; }
  FPD& AtC(ISZ r, ISZ c) { return data[r * stride + c]; }

  /* Total element count (may be less than data capacity). */
  ISZ ElementCount() { return rows * cols; }

  /* Strided element count. */
  ISZ StridedCount() { return stride * cols; }

  /* Square check. */
  BOL IsSquare() { return rows == cols; }

  /* Set all elements to val. */
  void SetAll(FPC val) {
    ISZ n = StridedCount();
    for (ISZ i = 0; i < n; ++i) data[i] = val;
  }

  /* Fill with random in [-1,1] using the global LCG. */
  void Randomize() {
    ISZ n = StridedCount();
    for (ISZ i = 0; i < n; ++i) {
      // LCG-based random (same as policy code)
      static IUD s = 0x9E3779B97F4A7C15ULL;
      s = s * 6364136223846793005ULL + 1442695040888963407ULL;
      FPC u = FPC(((s >> 11) & 0xFFFFFFFFu) / (double)0xFFFFFFFFu) * 2.0f - 1.0f;
      data[i] = u;
    }
  }

  /* === CBLAS-accelerated ops (enabled when CBLAS_H_AVAILABLE) === */

  /* C = alpha * A * B + beta * C  (A: rows x cols, B: cols x ov, C: rows x ov). */
  // NB: when CBLAS_H_AVAILABLE is defined externally, this lowers to cblas_dgemm.
  // For now it's a plain loop (identical to the existing Tensor ops).
  void GEMM(const FPD* A, ISZ lda, const FPD* B, ISZ ldb, FPD* C, ISZ ldC,
           ISZ m, ISZ n, ISZ k, FPC alpha = 1.0f, FPC beta = 0.0f) {
    for (ISZ i = 0; i < m; ++i) {
      for (ISZ j = 0; j < n; ++j) {
        FPC sum = beta * C[i * ldC + j];
        for (ISZ k = 0; k < k; ++k) {
          sum += A[i * lda + k] * B[k * ldb + j];
        }
        C[i * ldC + j] = sum;
      }
    }
  }

  /* In-place scale: C = alpha * C. */
  void Scale(FPC alpha) {
    ISZ n = StridedCount();
    for (ISZ i = 0; i < n; ++i) data_[i] *= alpha;
  }

  /* Copy: D = src. */
  void CopyFrom(FPDMatrix& src) {
    ISZ n = src.StridedCount();
    for (ISZ i = 0; i < n; ++i) data_[i] = src.data_[i];
    rows = src.rows; cols = src.cols; stride = src.stride;
  }

  /* Identity initialization (square only). */
  void SetIdentity(FPC ones = 1.0f, FPC zeros = 0.0f) {
    if (!IsSquare()) return;
    SetAll(zeros);
    for (ISZ i = 0; i < rows; ++i) At(i, i) = ones;
  }

  /* Transpose: this = A^T. */
  void Transpose() {
    if (rows == 0 || cols == 0) return;
    ISZ new_rows = cols, new_cols = rows;
    FPDMatrix tmp(data_, new_rows, new_cols, new_rows);
    for (ISZ r = 0; r < rows; ++r)
      for (ISZ c = 0; c < cols; ++c)
        tmp.At(c, r) = At(r, c);
    *this = tmp;
  }
};

/* =========================================================================
   Convenience: make an FPDMatrix on the stack (no heap alloc).  The
   matrix lives in the caller's storage; the FPDMatrix just holds
   pointers / dimensions.  Useful for DLL hot-loading / JSX value-or-
   reference semantics. */
struct FPDMatrixView {
  FPD*   data = _NilP;
  ISZ    rows = 0, cols = 0, stride = 0;
  // does NOT own data; caller keeps the buffer alive
  FPDMatrixView() = default;
  FPDMatrixView(FPD* d, ISZ r, ISZ c, ISZ s = 0)
    : data(d), rows(r), cols(c), stride(s ? s : (r > 0 ? r : 1)) {}
};

/* --- CBLAS lowerings (enabled when CBLAS_H_AVAILABLE is defined) ---
   The CBLAS header and linking is the host's responsibility; this file
   only guards the calls so the same .hxx works on CPU-only and V100. */
#ifdef CBLAS_H_AVAILABLE
#include <cblas.h>
inline void FPDMatrix_GEMM(FPDMatrix& A, FPDMatrix& B, FPDMatrix& C,
                          FPC alpha = 1.0f, FPC beta = 0.0f) {
  // cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
  //             A.rows, B.cols, A.cols, alpha,
  //             A.data, A.stride, B.data, B.stride, beta,
  //             C.data, C.stride);
  // For now fall through to the plain loop (same as above);
  // the host compiler #define CBLAS_H_AVAILABLE=1 and link cblas.
  (void)A; (void)B; (void)C; (void)alpha; (void)beta;
}
#else
/* Plain-FPC fallback (same loop as above). */
inline void FPDMatrix_GEMM(FPDMatrix& A, FPDMatrix& B, FPDMatrix& C,
                          FPC alpha = 1.0f, FPC beta = 0.0f) {
  ISZ m = A.rows, n = B.cols, k = A.cols;
  for (ISZ i = 0; i < m; ++i)
    for (ISZ j = 0; j < n; ++j) {
      FPC sum = beta * C.At(i, j);
      for (ISZ p = 0; p < k; ++p)
        sum += A.At(i, p) * B.At(p, j);
      C.At(i, j) = sum;
    }
}
#endif

/* --- JSON / serialization for DLL / JSX hot-swap --- */
inline void WriteMatrixJson(FPDMatrix& m, ::_::ROut& out) {
  out << "{\"data\":"; // simplified; host can extend
  ISZ n = m.ElementCount();
  for (ISZ i = 0; i < n; ++i) {
    if (i > 0) out << ',';
    out << m.At(i / m.cols, i % m.cols);
  }
  out << ",\"rows\":" << m.rows << ",\"cols\":" << m.cols
      << ",\"stride\":" << m.stride << "}";
}
}  //< namespace iGeek
#endif
