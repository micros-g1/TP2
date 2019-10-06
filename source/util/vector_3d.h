//
// Created by Rocío Parra on 10/5/2019.
//

#ifndef TP2_VECTOR_3D_H
#define TP2_VECTOR_3D_H

#include <stdint.h>

typedef struct {
    float x;
    float y;
    float z;
} vector_t;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} ivector_t;

vector_t v_add(vector_t v, vector_t w);
vector_t v_substract(vector_t v, vector_t w);
vector_t v_invert(vector_t v);
vector_t v_scalar_product(float alpha, vector_t v);
float v_dot_product(vector_t a, vector_t b);
float v_norm (vector_t v);
vector_t v_normalize(vector_t v);
vector_t v_cross_product(vector_t a, vector_t b);

ivector_t iv_add(ivector_t v, ivector_t w);
ivector_t iv_substract(ivector_t v, ivector_t w);
ivector_t iv_invert(ivector_t v);
ivector_t iv_scalar_product(float alpha, ivector_t v);
float iv_dot_product(ivector_t a, ivector_t b);
float iv_norm (ivector_t v);
vector_t iv_normalize(ivector_t v);
ivector_t iv_cross_product(ivector_t a, ivector_t b);





#endif //TP2_VECTOR_3D_H
