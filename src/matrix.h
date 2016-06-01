/**
 * @file matrix.h
 *
 * Interface definitions for the matrix library.
 *
 * NOTE: Unlike C, which stores static arrays in row-major
 * order, this library stores matrices in column-major order.
 */
#ifndef MATRIX_H
#define MATRIX_H

#define COLOR 0
#define GRAYSCALE 1

#define IS_COLOR GRAYSCALE

typedef double precision;

typedef struct {
	precision *data;
	int numRows;
	int numCols;
} matrix_t;

#define elem(M, i, j) (M)->data[(j) * (M)->numRows + (i)]

// constructor, destructor functions
matrix_t * m_initialize (int rows, int rols);
matrix_t * m_identity (int rows);
matrix_t * m_zeros (int rows, int rols);
matrix_t * m_copy (matrix_t *M);
void m_free (matrix_t *M);

// I/O functions
void m_fprint (FILE *stream, matrix_t *M);
void m_fwrite (FILE *stream, matrix_t *M);
matrix_t * m_fscan (FILE *stream);
matrix_t * m_fread (FILE *stream);

// getter functions
matrix_t * m_matrix_multiply (matrix_t *A, matrix_t *B);
matrix_t * m_mean_column (matrix_t *M);
matrix_t * m_transpose (matrix_t *M);

// mutator functions
void m_normalize_columns (matrix_t *M, matrix_t *a);

/***************** Group 2 - Operations on a single matrix *******************/
/***** 2.0 - No return values, operate directly on M's data *****/
// 2.0.0
//	- Not element wise operation
//	- no extra inputs
void m_flipCols (matrix_t *M);
void m_normalize (matrix_t *M);
// THIS IS INCLUDED FOR GROUP 3 FOR TESTING TEMPORARILY
//void m_inverseMatrix (matrix_t *M); // Must be square matrix

// 2.0.1
//	- element wise math operation
//	- no extra inputs
void m_elem_truncate (matrix_t *M);
void m_elem_acos (matrix_t *M);
void m_elem_sqrt (matrix_t *M);
void m_elem_negate (matrix_t *M);
void m_elem_exp (matrix_t *M);
// 2.0.2
//	- element wise math operation
//	- has a second input operation relies on
void m_elem_pow (matrix_t *M, precision x);
void m_elem_mult (matrix_t *M, precision x);
void m_elem_divideByConst (matrix_t *M, precision x);
void m_elem_divideByMatrix (matrix_t *M, precision x);
void m_elem_add (matrix_t *M, precision x);

/***** 2.1 - returns a matrix, does not change input matrix M *****/
/***** No other inputs, except for m_reshape *****/
// 2.1.0
//	- returns row vector
matrix_t * m_sumCols (matrix_t *M);
matrix_t * m_meanCols (matrix_t *M);
// 2.1.1
//	- returns column vector
matrix_t * m_sumRows (matrix_t *M);
matrix_t * m_findNonZeros (matrix_t *M);
// 2.1.2
//	- reshapes data in matrix to new form
matrix_t * m_reshape (matrix_t *M, int newNumRows, int newNumCols);

// TEMPORARILY INCLUDED WHILE TESTING
void m_inverseMatrix (matrix_t *M);

// Group 3 - complex linear algebra functions of a single matrix
precision m_norm (matrix_t *M, int specRow);
matrix_t * m_sqrtm (matrix_t *M);
precision m_determinant (matrix_t *M);
matrix_t * m_cofactor (matrix_t *M);
matrix_t * m_covariance (matrix_t *M);

// Group 4 - ops with 2 matrices that return a matrix of same size
matrix_t * m_dot_subtract (matrix_t *A, matrix_t *B);
matrix_t * m_dot_add (matrix_t *A, matrix_t *B);
matrix_t * m_dot_division (matrix_t *A, matrix_t *B);

// Group 5 - ops with 2 matrices that return a matrix of diff size
matrix_t * m_matrix_division (matrix_t *A, matrix_t *B);
matrix_t * m_reorder_columns (matrix_t *M, matrix_t *V);

// Group 6 - other, doesn;t really fit in anywhere
void m_eigenvalues_eigenvectors (matrix_t *M, matrix_t **p_eigenvalues, matrix_t **p_eigenvectors);

void loadPPMtoMatrixCol (const char *filename, matrix_t *M, int specCol, unsigned char *pixels);
void writePPMgrayscale (const char *filename, matrix_t *M, int specCOl, int height, int width);

#endif