/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// -*- c++ -*-

/*
 *  A few utilitary functions for similarity search:
 * - random generators
 * - optimized exhaustive distance and knn search functions
 * - some functions reimplemented from torch for speed
 */

#ifndef FAISS_utils_h
#define FAISS_utils_h

#include <random>
#include <stdint.h>

#include "Heap.h"

#ifdef _MSC_VER
#include <cmath>
#include "drand48.h"
#define finite(x) isfinite(x)
#endif

#if defined(_WIN32) || defined(_WIN64)
/* We are on Windows */
# define strtok_r strtok_s
#endif

namespace faiss {


/**************************************************
 * Get some stats about the system
**************************************************/


/// ms elapsed since some arbitrary epoch
double getmillisecs ();

/// get current RSS usage in kB
size_t get_mem_usage_kb ();


/**************************************************
 * Random data generation functions
 **************************************************/

/// random generator that can be used in multithreaded contexts
struct RandomGenerator {

    std::mt19937 mt;

    /// random positive integer
    int rand_int ();

    /// random int64_t
    int64_t rand_int64 ();

    /// generate random integer between 0 and max-1
    int rand_int (int max);

    /// between 0 and 1
    float rand_float ();

    double rand_double ();

    explicit RandomGenerator (int64_t seed = 1234);
};

/* Generate an array of uniform random floats / multi-threaded implementation */
void float_rand (float * x, size_t n, int64_t seed);
void float_randn (float * x, size_t n, int64_t seed);
void int64_rand (int64_t * x, size_t n, int64_t seed);
void byte_rand (uint8_t * x, size_t n, int64_t seed);

/* random permutation */
void rand_perm (int * perm, size_t n, int64_t seed);



 /*********************************************************
 * Optimized distance/norm/inner prod computations
 *********************************************************/


/// Squared L2 distance between two vectors
float fvec_L2sqr (
        const float * x,
        const float * y,
        size_t d);

/// inner product
float  fvec_inner_product (
        const float * x,
        const float * y,
        size_t d);

/// L1 distance
float fvec_L1 (
        const float * x,
        const float * y,
        size_t d);

float fvec_Linf (
        const float * x,
        const float * y,
        size_t d);


/// a balanced assignment has a IF of 1
double imbalance_factor (int n, int k, const int64_t *assign);

/// same, takes a histogram as input
double imbalance_factor (int k, const int *hist);

/** Compute pairwise distances between sets of vectors
 *
 * @param d     dimension of the vectors
 * @param nq    nb of query vectors
 * @param nb    nb of database vectors
 * @param xq    query vectors (size nq * d)
 * @param xb    database vectros (size nb * d)
 * @param dis   output distances (size nq * nb)
 * @param ldq,ldb, ldd strides for the matrices
 */
void pairwise_L2sqr (int64_t d,
                     int64_t nq, const float *xq,
                     int64_t nb, const float *xb,
                     float *dis,
                     int64_t ldq = -1, int64_t ldb = -1, int64_t ldd = -1);

/* compute the inner product between nx vectors x and one y */
void fvec_inner_products_ny (
        float * ip,         /* output inner product */
        const float * x,
        const float * y,
        size_t d, size_t ny);

/* compute ny square L2 distance bewteen x and a set of contiguous y vectors */
void fvec_L2sqr_ny (
        float * dis,
        const float * x,
        const float * y,
        size_t d, size_t ny);


/** squared norm of a vector */
float fvec_norm_L2sqr (const float * x,
                       size_t d);

/** compute the L2 norms for a set of vectors
 *
 * @param  ip       output norms, size nx
 * @param  x        set of vectors, size nx * d
 */
void fvec_norms_L2 (float * ip, const float * x, size_t d, size_t nx);

/// same as fvec_norms_L2, but computes square norms
void fvec_norms_L2sqr (float * ip, const float * x, size_t d, size_t nx);

/* L2-renormalize a set of vector. Nothing done if the vector is 0-normed */
void fvec_renorm_L2 (size_t d, size_t nx, float * x);


/* This function exists because the Torch counterpart is extremly slow
   (not multi-threaded + unexpected overhead even in single thread).
   It is here to implement the usual property |x-y|^2=|x|^2+|y|^2-2<x|y>  */
void inner_product_to_L2sqr (float * dis,
                             const float * nr1,
                             const float * nr2,
                             size_t n1, size_t n2);

/***************************************************************************
 * Compute a subset of  distances
 ***************************************************************************/

 /* compute the inner product between x and a subset y of ny vectors,
   whose indices are given by idy.  */
void fvec_inner_products_by_idx (
        float * ip,
        const float * x,
        const float * y,
        const int64_t *ids,
        size_t d, size_t nx, size_t ny);

/* same but for a subset in y indexed by idsy (ny vectors in total) */
void fvec_L2sqr_by_idx (
        float * dis,
        const float * x,
        const float * y,
        const int64_t *ids, /* ids of y vecs */
        size_t d, size_t nx, size_t ny);

/***************************************************************************
 * KNN functions
 ***************************************************************************/

// threshold on nx above which we switch to BLAS to compute distances
extern int distance_compute_blas_threshold;

/** Return the k nearest neighors of each of the nx vectors x among the ny
 *  vector y, w.r.t to max inner product
 *
 * @param x    query vectors, size nx * d
 * @param y    database vectors, size ny * d
 * @param res  result array, which also provides k. Sorted on output
 */
void knn_inner_product (
        const float * x,
        const float * y,
        size_t d, size_t nx, size_t ny,
        float_minheap_array_t * res);

/** Same as knn_inner_product, for the L2 distance */
void knn_L2sqr (
        const float * x,
        const float * y,
        size_t d, size_t nx, size_t ny,
        float_maxheap_array_t * res);



/** same as knn_L2sqr, but base_shift[bno] is subtracted to all
 * computed distances.
 *
 * @param base_shift   size ny
 */
void knn_L2sqr_base_shift (
         const float * x,
         const float * y,
         size_t d, size_t nx, size_t ny,
         float_maxheap_array_t * res,
         const float *base_shift);

/* Find the nearest neighbors for nx queries in a set of ny vectors
 * indexed by ids. May be useful for re-ranking a pre-selected vector list
 */
void knn_inner_products_by_idx (
        const float * x,
        const float * y,
        const int64_t *  ids,
        size_t d, size_t nx, size_t ny,
        float_minheap_array_t * res);

void knn_L2sqr_by_idx (const float * x,
                       const float * y,
                       const int64_t * ids,
                       size_t d, size_t nx, size_t ny,
                       float_maxheap_array_t * res);

/***************************************************************************
 * Range search
 ***************************************************************************/



/// Forward declaration, see AuxIndexStructures.h
struct RangeSearchResult;

/** Return the k nearest neighors of each of the nx vectors x among the ny
 *  vector y, w.r.t to max inner product
 *
 * @param x      query vectors, size nx * d
 * @param y      database vectors, size ny * d
 * @param radius search radius around the x vectors
 * @param result result structure
 */
void range_search_L2sqr (
        const float * x,
        const float * y,
        size_t d, size_t nx, size_t ny,
        float radius,
        RangeSearchResult *result);

/// same as range_search_L2sqr for the inner product similarity
void range_search_inner_product (
        const float * x,
        const float * y,
        size_t d, size_t nx, size_t ny,
        float radius,
        RangeSearchResult *result);





/***************************************************************************
 * Misc  matrix and vector manipulation functions
 ***************************************************************************/


/** compute c := a + bf * b for a, b and c tables
 *
 * @param n   size of the tables
 * @param a   size n
 * @param b   size n
 * @param c   restult table, size n
 */
void fvec_madd (size_t n, const float *a,
                float bf, const float *b, float *c);


/** same as fvec_madd, also return index of the min of the result table
 * @return    index of the min of table c
 */
int fvec_madd_and_argmin (size_t n, const float *a,
                           float bf, const float *b, float *c);


/* perform a reflection (not an efficient implementation, just for test ) */
void reflection (const float * u, float * x, size_t n, size_t d, size_t nu);


/** For k-means: update stage.
 *
 * @param x          training vectors, size n * d
 * @param centroids  centroid vectors, size k * d
 * @param assign     nearest centroid for each training vector, size n
 * @param k_frozen   do not update the k_frozen first centroids
 * @return           nb of spliting operations to fight empty clusters
 */
int km_update_centroids (
        const float * x,
        float * centroids,
        int64_t * assign,
        size_t d, size_t k, size_t n,
        size_t k_frozen);

/** compute the Q of the QR decomposition for m > n
 * @param a   size n * m: input matrix and output Q
 */
void matrix_qr (int m, int n, float *a);

/** distances are supposed to be sorted. Sorts indices with same distance*/
void ranklist_handle_ties (int k, int64_t *idx, const float *dis);

/** count the number of comon elements between v1 and v2
 * algorithm = sorting + bissection to avoid double-counting duplicates
 */
size_t ranklist_intersection_size (size_t k1, const int64_t *v1,
                                   size_t k2, const int64_t *v2);

/** merge a result table into another one
 *
 * @param I0, D0       first result table, size (n, k)
 * @param I1, D1       second result table, size (n, k)
 * @param keep_min     if true, keep min values, otherwise keep max
 * @param translation  add this value to all I1's indexes
 * @return             nb of values that were taken from the second table
 */
size_t merge_result_table_with (size_t n, size_t k,
                                int64_t *I0, float *D0,
                                const int64_t *I1, const float *D1,
                                bool keep_min = true,
                                int64_t translation = 0);



void fvec_argsort (size_t n, const float *vals,
                    size_t *perm);

void fvec_argsort_parallel (size_t n, const float *vals,
                    size_t *perm);


/// compute histogram on v
int ivec_hist (size_t n, const int * v, int vmax, int *hist);

/** Compute histogram of bits on a code array
 *
 * @param codes   size(n, nbits / 8)
 * @param hist    size(nbits): nb of 1s in the array of codes
 */
void bincode_hist(size_t n, size_t nbits, const uint8_t *codes, int *hist);


/// compute a checksum on a table.
size_t ivec_checksum (size_t n, const int *a);


/** random subsamples a set of vectors if there are too many of them
 *
 * @param d      dimension of the vectors
 * @param n      on input: nb of input vectors, output: nb of output vectors
 * @param nmax   max nb of vectors to keep
 * @param x      input array, size *n-by-d
 * @param seed   random seed to use for sampling
 * @return       x or an array allocated with new [] with *n vectors
 */
const float *fvecs_maybe_subsample (
       size_t d, size_t *n, size_t nmax, const float *x,
       bool verbose = false, int64_t seed = 1234);

/** Convert binary vector to +1/-1 valued float vector.
 *
 * @param d      dimension of the vector (multiple of 8)
 * @param x_in   input binary vector (uint8_t table of size d / 8)
 * @param x_out  output float vector (float table of size d)
 */
void binary_to_real(size_t d, const uint8_t *x_in, float *x_out);

/** Convert float vector to binary vector. Components > 0 are converted to 1,
 * others to 0.
 *
 * @param d      dimension of the vector (multiple of 8)
 * @param x_in   input float vector (float table of size d)
 * @param x_out  output binary vector (uint8_t table of size d / 8)
 */
void real_to_binary(size_t d, const float *x_in, uint8_t *x_out);


/** A reasonable hashing function */
uint64_t hash_bytes (const uint8_t *bytes, int64_t n);

/** Whether OpenMP annotations were respected. */
bool check_openmp();

} // namspace faiss


#endif /* FAISS_utils_h */
