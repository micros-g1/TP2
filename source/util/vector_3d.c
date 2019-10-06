//
// Created by Rocío Parra on 10/5/2019.
//

#include "vector_3d.h"
#include <math.h>

vector_t v_add(vector_t v, vector_t w) {
    vector_t u = {v.x+w.x, v.y + w.y, v.z + w.z};
    return u;
}

vector_t v_scalar_product(float alpha, vector_t v)
{
    vector_t u = {alpha * v.x, alpha * v.y, alpha * v.z};
    return u;
}

vector_t v_substract(vector_t v, vector_t w)
{
    vector_t u = {v.x - w.x, v.y - w.y, v.z - w.z};
    return u;
}

vector_t v_invert(vector_t v)
{
    vector_t u = {-v.x, -v.y, -v.z};
    return u;
}


float v_norm (vector_t v)
{
    return sqrt(v_dot_product(v, v));
}

vector_t v_normalize(vector_t v)
{
    vector_t u = v_scalar_product(1.0/v_norm(v), v);
    return u;
}


float v_dot_product(vector_t a, vector_t b)
{
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

vector_t v_cross_product(vector_t a, vector_t b)
{
    vector_t c = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    return c;
}




ivector_t iv_add(ivector_t v, ivector_t w) {
    ivector_t u = {v.x+w.x, v.y + w.y, v.z + w.z};
    return u;
}

ivector_t iv_scalar_product(float alpha, ivector_t v)
{
    ivector_t u = {round(alpha * v.x), round(alpha * v.y), round(alpha * v.z)};
    return u;
}

ivector_t iv_substract(ivector_t v, ivector_t w)
{
    ivector_t u = {v.x - w.x, v.y - w.y, v.z - w.z};
    return u;
}

ivector_t iv_invert(ivector_t v)
{
    ivector_t u = {-v.x, -v.y, -v.z};
    return u;
}


float iv_norm (ivector_t v)
{
    return sqrt(iv_dot_product(v, v));
}

vector_t iv_normalize(ivector_t v)
{
	vector_t u = { v.x, v.y, v.z };
    u = v_scalar_product(1.0/v_norm(u), u);
    return u;
}


float iv_dot_product(ivector_t a, ivector_t b)
{
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

ivector_t iv_crossProduct(ivector_t a, ivector_t b)
{
    ivector_t c = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    return c;
}

