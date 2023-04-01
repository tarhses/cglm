
#ifndef cglm_simd_wasm_h
#define cglm_simd_wasm_h
#include "intrin.h"
#ifdef CGLM_SIMD_WASM
#include <wasm_simd128.h>

#define glmm_load(p)      wasm_v128_load(p)
#define glmm_store(p, a)  wasm_v128_store(p, (a))

#define glmm_set1(x) wasm_f32x4_splat(x)
#define glmm_128     v128_t

#define glmm_shuff1(xmm, z, y, x, w) wasm_i32x4_shuffle(xmm, xmm, w, x, y, z)

#define glmm_splat(x, lane) glmm_shuff1(x, lane, lane, lane, lane)

#define glmm_splat_x(x) glmm_splat(x, 0)
#define glmm_splat_y(x) glmm_splat(x, 1)
#define glmm_splat_z(x) glmm_splat(x, 2)
#define glmm_splat_w(x) glmm_splat(x, 3)

#define _mm_cvtss_f32(v) wasm_f32x4_extract_lane(v, 0)

static inline glmm_128 __attribute__((__always_inline__, __nodebug__))
_mm_movehl_ps(glmm_128 __a, glmm_128 __b)
{
  return wasm_i32x4_shuffle(__a, __b, 6, 7, 2, 3);
}

static inline
glmm_128
glmm_abs(glmm_128 x) {
  return wasm_v128_andnot(x, wasm_f32x4_const_splat(-0.0f));
}

static inline
glmm_128
glmm_vhadd(glmm_128 v) {
  glmm_128 x0;
  x0 = wasm_f32x4_add(v,  glmm_shuff1(v, 0, 1, 2, 3));
  x0 = wasm_f32x4_add(x0, glmm_shuff1(x0, 1, 0, 0, 1));
  return x0;
}

static inline
glmm_128
glmm_vhadds(glmm_128 v) {
  glmm_128 shuf, sums;
  shuf = glmm_shuff1(v, 2, 3, 0, 1);
  sums = wasm_f32x4_add(v, shuf);
  shuf = _mm_movehl_ps(shuf, sums);
  sums = wasm_i32x4_shuffle(sums, wasm_f32x4_add(sums, shuf), 4, 1, 2, 3);
  return sums;
}

static inline
float
glmm_hadd(glmm_128 v) {
  return _mm_cvtss_f32(glmm_vhadds(v));
}

static inline
glmm_128
glmm_vhmin(glmm_128 v) {
  glmm_128 x0, x1, x2;
  x0 = glmm_shuff1(v, 2, 3, 2, 3);     /* [2, 3, 2, 3] */
  x1 = wasm_f32x4_pmin(x0, v);   /* [0|2, 1|3, 2|2, 3|3] */
  x2 = glmm_splat(x1, 1);              /* [1|3, 1|3, 1|3, 1|3] */
  return wasm_f32x4_pmin(x1, x2);
}

static inline
float
glmm_hmin(glmm_128 v) {
  return _mm_cvtss_f32(glmm_vhmin(v));
}

static inline
glmm_128
glmm_vhmax(glmm_128 v) {
  glmm_128 x0, x1, x2;
  x0 = glmm_shuff1(v, 2, 3, 2, 3);     /* [2, 3, 2, 3] */
  x1 = wasm_f32x4_pmax(x0, v);   /* [0|2, 1|3, 2|2, 3|3] */
  x2 = glmm_splat(x1, 1);              /* [1|3, 1|3, 1|3, 1|3] */
  // _mm_max_ss
  return wasm_i32x4_shuffle(x1, wasm_f32x4_pmax(x1, x2), 4, 1, 2, 3);
}

static inline
float
glmm_hmax(glmm_128 v) {
  return wasm_f32x4_extract_lane(glmm_vhmax(v), 0);
}

static inline
glmm_128
glmm_vdots(glmm_128 a, glmm_128 b) {
  return glmm_vhadds(wasm_f32x4_mul(a, b));
}

static inline
glmm_128
glmm_vdot(glmm_128 a, glmm_128 b) {
  glmm_128 x0;
  x0 = wasm_f32x4_mul(a, b);
  x0 = wasm_f32x4_add(x0, glmm_shuff1(x0, 1, 0, 3, 2));
  return wasm_f32x4_add(x0, glmm_shuff1(x0, 0, 1, 0, 1));
}

static inline
float
glmm_dot(glmm_128 a, glmm_128 b) {
  return _mm_cvtss_f32(glmm_vdots(a, b));
}

static inline
float
glmm_norm(glmm_128 a) {
  glmm_128 x0;
  x0 = glmm_vhadds(wasm_f32x4_mul(a, a));
  return _mm_cvtss_f32(wasm_i32x4_shuffle(x0, wasm_f32x4_sqrt(x0),4, 1, 2, 3));
}

static inline
float
glmm_norm2(glmm_128 a) {
  return _mm_cvtss_f32(glmm_vhadds(wasm_f32x4_mul(a, a)));
}

static inline
float
glmm_norm_one(glmm_128 a) {
  return _mm_cvtss_f32(glmm_vhadds(glmm_abs(a)));
}

static inline
float
glmm_norm_inf(glmm_128 a) {
  return _mm_cvtss_f32(glmm_vhmax(glmm_abs(a)));
}

static inline
glmm_128
glmm_load3(float v[3]) {
  glmm_128 xy = wasm_v128_load64_zero(v);
  return wasm_f32x4_replace_lane(xy, 2, v[2]);
}

static inline
void
glmm_store3(float v[3], glmm_128 vx) {
  wasm_v128_store64_lane(v, vx, 0);
  wasm_v128_store32_lane(&v[2], vx, 2);
}

static inline
glmm_128
glmm_div(glmm_128 a, glmm_128 b) {
  return wasm_f32x4_div(a, b);
}

static inline
glmm_128
glmm_fmadd(glmm_128 a, glmm_128 b, glmm_128 c) {
  return wasm_f32x4_add(c, wasm_f32x4_mul(a, b));
}

static inline
glmm_128
glmm_fnmadd(glmm_128 a, glmm_128 b, glmm_128 c) {
  return wasm_f32x4_sub(c, wasm_f32x4_mul(a, b));
}

static inline
glmm_128
glmm_fmsub(glmm_128 a, glmm_128 b, glmm_128 c) {
  return wasm_f32x4_sub(wasm_f32x4_mul(a, b), c);
}

static inline
glmm_128
glmm_fnmsub(glmm_128 a, glmm_128 b, glmm_128 c) {
  return wasm_v128_xor(wasm_f32x4_add(wasm_f32x4_mul(a, b), c),
                       wasm_f32x4_const_splat(-0.0f));
}

#endif
#endif /* cglm_simd_wasm_h */
