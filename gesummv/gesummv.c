#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

void gesummv(int n, double alpha, double beta, double * A, double * B, double * x, double * Ay)
{
	int i,j;
	double t1 = 0;
	double t2 = 0;

	/* Accelerator */
#pragma acc kernels copyin(A[0:n*n],B[0,n*n]) copyout(Ay[0:n])
{
#pragma acc loop independent vector(32)
	for (i = 0; i < n; i++)
	{
		t1 = 0;
		t2 = 0;
#pragma acc loop independent worker(32)
		for (j = 0; j < n; j++)
	    {
			t1 += A[i*n + j] * x[j];
			t2 += B[i*n + j] * x[j];
		}
		Ay[i] = alpha * t1 + beta * t2;
	}
}

int main(int argc, char **argv) 
{

	int n = atoi(*(argv + 1));

	struct timeval t_time1, t_time2;
	double time=0;

	double alpha = 0;
	double beta = 0;
	double* A = (double*)malloc(n*n * sizeof(double));
	double* B = (double*)malloc(n*n * sizeof(double));
	double* x = (double*)malloc(n * sizeof(double));
	double* Ay = (double*)malloc(n * sizeof(double));
	double* Hy = (double*)malloc(n * sizeof(double));

	if (A == NULL || B == NULL || x == NULL || Ay == NULL || Hy == NULL)
	{
		/* Something went wrong in the memory allocation here, fail gracefully */
		return(-10000);
	}
	
	/* Initialization */
	alpha = 435.32;
	beta = 123.13;
	for (int i = 0; i < n; i++)
	{
		x[i] = rand() / (1.0 + RAND_MAX);
		Ay[i]=0;
		for (int j = 0; j < n; j++)
		{
			A[i*n + j] = rand() / (1.0 + RAND_MAX);
			B[i*n + j] = rand() / (1.0 + RAND_MAX);
		}
	}

	/* Warm up */
	gesummv(n, alpha, beta, B, A, x , Hy);

	gettimeofday(&t_time1, NULL);
	gesummv(n, alpha, beta, A, B, x , Ay);
	gettimeofday(&t_time2, NULL);

	/* Free malloc'd memory to prevent leaks */
	free(A);
	free(B);
	free(x);
	free(Ay);
	free(Hy);

	time = (t_time2.tv_sec - t_time1.tv_sec) + (t_time2.tv_usec - t_time1.tv_usec)*1e-6;
	long long nflop = 6*n*n;
	float intensity = 0.25;
	double flops = nflop / time / 1000000000;

	printf("---gesummv---Size:%d\n", n);
	printf("---gesummv---Excution Time:%f s\n", time);
	printf("---gesummv---Arithmetic Intensity:%f\n", intensity);
	printf("---gesummv---Performance:%lf Gflops\n", flops);
	
	return EXIT_SUCCESS;
}