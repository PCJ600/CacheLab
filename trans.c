/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32) { /* 按8*8分块, 利用局部变量减少对角线访问的miss */
        int a1, a2, a3, a4, a5, a6, a7, a8;
        for(int i = 0; i < M; i += 8) {
            for(int j = 0; j < N; j += 8) {
                for(int k = i; k < i + 8; ++k) {
                    a1 = A[k][j];
                    a2 = A[k][j+1];
                    a3 = A[k][j+2];
                    a4 = A[k][j+3];
                    a5 = A[k][j+4];
                    a6 = A[k][j+5];
                    a7 = A[k][j+6];
                    a8 = A[k][j+7];

                    B[j][k] = a1;
                    B[j+1][k] = a2;
                    B[j+2][k] = a3;
                    B[j+3][k] = a4;
                    B[j+4][k] = a5;
                    B[j+5][k] = a6;
                    B[j+6][k] = a7;
                    B[j+7][k] = a8;
                }
            }
        }
    } else if(M == 64) {    /* 非常难! */
        /*
        int i, j, x, y;
        int x1, x2, x3, x4, x5, x6, x7, x8;
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (x = i; x < i + 4; ++x) {
                    x1 = A[x][j]; x2 = A[x][j+1]; x3 = A[x][j+2]; x4 = A[x][j+3];
                    x5 = A[x][j+4]; x6 = A[x][j+5]; x7 = A[x][j+6]; x8 = A[x][j+7];

                    B[j][x] = x1; B[j+1][x] = x2; B[j+2][x] = x3; B[j+3][x] = x4;
                    B[j][x+4] = x5; B[j+1][x+4] = x6; B[j+2][x+4] = x7; B[j+3][x+4] = x8;
                }
                for (y = j; y < j + 4; ++y) {
                    x1 = A[i+4][y]; x2 = A[i+5][y]; x3 = A[i+6][y]; x4 = A[i+7][y];
                    x5 = B[y][i+4]; x6 = B[y][i+5]; x7 = B[y][i+6]; x8 = B[y][i+7];

                    B[y][i+4] = x1; B[y][i+5] = x2; B[y][i+6] = x3; B[y][i+7] = x4;
                    B[y+4][i] = x5; B[y+4][i+1] = x6; B[y+4][i+2] = x7; B[y+4][i+3] = x8;
                }
                for (x = i + 4; x < i + 8; ++x) {
                    x1 = A[x][j+4]; x2 = A[x][j+5]; x3 = A[x][j+6]; x4 = A[x][j+7];
                    B[j+4][x] = x1; B[j+5][x] = x2; B[j+6][x] = x3; B[j+7][x] = x4;
                }
            }
        }*/

        // 4*4分块 1667 Misses
        int i, j, l;
        int a1, a2, a3, a4, a5, a6, a7, a8;
        for(i = 0; i < M; i += 4) {
            for(j = 0; j < N; j += 4) {
                for(l = i; l < i + 4; l += 2) {
                    a1 = A[l][j];
                    a2 = A[l][j+1];
                    a3 = A[l][j+2];
                    a4 = A[l][j+3];
                    a5 = A[l+1][j];
                    a6 = A[l+1][j+1];
                    a7 = A[l+1][j+2];
                    a8 = A[l+1][j+3];

                    B[j][l] = a1;
                    B[j+1][l] = a2;
                    B[j+2][l] = a3;
                    B[j+3][l] = a4;
                    B[j][l+1] = a5;
                    B[j+1][l+1] = a6;
                    B[j+2][l+1] = a7;
                    B[j+3][l+1] = a8;
                }
            }
        }
    } else if(M == 61) {    /* 8*8分块, 先处理64 * 56 */
        int i, j, a1, a2, a3, a4, a5, a6, a7, a8;
        for(j = 0; j < 56; j += 8) {
            for(i = 0; i < 64; ++i) {
                a1 = A[i][j];
                a2 = A[i][j+1];
                a3 = A[i][j+2];
                a4 = A[i][j+3];
                a5 = A[i][j+4];
                a6 = A[i][j+5];
                a7 = A[i][j+6];
                a8 = A[i][j+7];
                
                B[j][i] = a1;
                B[j+1][i] = a2;
                B[j+2][i] = a3;
                B[j+3][i] = a4;
                B[j+4][i] = a5;
                B[j+5][i] = a6;
                B[j+6][i] = a7;
                B[j+7][i] = a8;
            }
        }
        // 处理其余部分，简单转置
        for(i = 0; i < 64; ++i) {
            for(j = 56; j < 61; ++j) {
                a1 = A[i][j];
                B[j][i] = a1;
            }
        }
        for(i = 64; i < 67; ++i) {
            for(j = 0; j < 61; ++j) {
                a1 = A[i][j];
                B[j][i] = a1;
            }
        }
    } else {
        for(int i = 0; i < M; ++i) {
            for(int j = 0; j < N; ++j) {
                B[i][j] = A[j][i];
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

