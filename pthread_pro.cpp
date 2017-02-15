#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <chrono>
#include<fstream>
#include <stdio.h>
#include <pthread.h>
#include <string>


struct myargs{
	int **A;
	int **B;
	int **C;
	int *Adim;
	int *Bdim;
	int *Cdim;
	int max;
};

// My functions
void printM(int **, int);
void test(int);
void fill(int **arry, int input, int resized);
int pow2(int data);

void* addM(void *args);
void* subM(void *args);
void mulM(int **sub1, int **sub2, int **Accum, int max);
void* split(void *args);


int** createM(int msize);
void deleteM(int** data, int S);
void deleteB(int*);
void fill (myargs *ptr, int *Adim,int *Bdim,int **A,int **B, int **C, int *Cdim,int max);
using namespace std;

pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;

int main()
{

	ofstream outFILE;
	outFILE.open("results.txt");

	if (!outFILE)
	{
		cout << "Error->outfile not open." << endl;
		exit(1);
	}
	else{
		outFILE << "Data Sets| Time for Matrix Multiplication" << endl;
		test(100);
		cout << "100 Finished." << endl;
		test(500);
		cout << "500 Finished." << endl;
		test(1000);
		cout << "1000 Finished." << endl;
		test(4096);
		cout << "5000 Finished." << endl;
		//out << "10000:   " << test(10000) << endl;
		//cout << "10k Finished" << endl;
	}
	cout << "Finished execution and writing file." << endl;
	return 0;
}
void test(int input)
{
	int resized = pow2(input);
	// 1st matrix
	int **test1 = createM(resized);
	fill(test1, input - 1, resized);
	// 2nd matrix
	int **test2 = createM(resized);
	fill(test2, input - 1, resized);
	// Resulting matrix
	int **result = createM(resized);
	int *resultC = new int[2]{0, 0};

	// Start MM
	split(test1, test2, result, resultC, resized);


	//printM(result, resized);
	// clean up
	deleteB(resultC);
	deleteM(result, resized);
	deleteM(test1, resized);
	deleteM(test2, resized);
}
void printM(int **matrix, int max)
{
	//[row][col]
	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
		{
		cout << matrix[i][j] << " ";
		if ((j + 1) == max)
			cout << endl;
		}
}
void fill(int **arry, int input, int resized)
{
	for (int i = 0; i < resized; i++)
		for (int j = 0; j < resized; j++)
		{
		if (i > input || j>input)
			arry[i][j] = 0;
		else
			arry[i][j] = 2;
		}
}
int pow2(int data)
{
	int i = 2;
	while (i < data)
		i= i*2;
	return i;
}
void addM(void *args)
{

	myargs *dataset = (myargs*)args;

	// Abounds[row start][col start]
	// Bbounds[row start][col start]
	// Accumulator bounds[row start][col start]
	int Arow = dataset->Adim[0];
	int Acol = dataset->Adim[1];
	int Brow = dataset->Bdim[0];
	int Bcol = dataset->Bdim[1];
	int Crow = dataset->Cdim[0];
	int Ccol = dataset->Cdim[1];
	int max = dataset->max;


	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			dataset->C[i+Crow][j+Ccol] = dataset->A[i + Arow][j + Acol] + dataset->B[i + Brow][j + Bcol];
}
void subM(void *args)
{
	myargs *dataset = (myargs*)args;
	int Arow = dataset->Adim[0];
	int Acol = dataset->Adim[1];
	int Brow = dataset->Bdim[0];
	int Bcol = dataset->Bdim[1];
	int Crow = dataset->Cdim[0];
	int Ccol = dataset->Cdim[1];
	int max = dataset->max;

	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			dataset->C[i+Crow][j+Ccol] = dataset->A[i + Arow][j + Acol] - dataset->B[i + Brow][j + Bcol];
}
void mulM(int **A, int **B, int **C, int max)
{
	for (int i = 0; i < max; i++)
		for (int k = 0; k < max; k++)
			for (int j = 0; j < max; j++)
				C[i][j] += (A[i][k] * B[k][j]);
}
int** createM(int msize)
{
	int **temp = new int*[msize];
	for (int i = 0; i < msize; i++)
	{
		temp[i] = new int[msize];
		for (int j = 0; j < msize; j++)
			temp[i][j] = 0;
	}
	return temp;
}


//int **A, int **B, int **C, int *Cdim, int max

void split(void *args)
{
	myargs *dataset = (myargs*)args;
	int max = dataset->max;

	if ((max / 2) < 16)
		mulM(dataset->A,dataset->B,dataset->C,max);
	else
	{

		pthread_t Mpool[25]; // threads
		int pi =0;			// index of next live threads


		myargs *newargs = new myargs;
		myargs *newargs2 = new myargs;

		int half = max / 2;
		int **sub1 = createM(half);
		int **sub2 = createM(half);
		int *Zero1 = new int[2]{0, 0};
		int *Zero2 = new int[2]{0, 0};

		int **A11 = createM(half);
		int *A12_corner = new int[2]{0, half};
		int *A21_corner = new int[2]{half, 0};
		int **A22 = createM(half);

		int **B11 = createM(half);
		int *B12_corner = new int[2]{0, half};
		int *B21_corner = new int[2]{half, 0};
		int **B22 = createM(half);

		//[row][column]

		for (int i = 0; i < half; i++)
			for (int j = 0; j < half; j++)
			{
			A11[i][j] = dataset->A[i][j];
			A22[i][j] = dataset->A[i + half][j + half];
			B11[i][j] = dataset->B[i][j];
			B22[i][j] = dataset->B[i + half][j + half];
			}//end for spliting matrix




		//M1
		int **M1 = createM(half); // Create sub-matrix for use later

		fill (newargs,Zero1,Zero1, A11, A22, sub1, Zero1, half);
		pthread_create(&Mpool[0],NULL,&addM,newargs);

		fill (newargs2,Zero2,Zero2, B11, B22, sub2, Zero2, half);
		pthread_create(&Mpool[1],NULL,&addM,newargs2);

		pthread_join(Mpool[0],NULL);
		pthread_join(Mpool[1],NULL);

		fill (newargs,Zero1,Zero1, sub1, sub2, M1, Zero1, half);
		pthread_create(&Mpool[2],NULL,&split,newargs);
		pthread_join(Mpool[2],NULL);



		//M2
		int **M2 = createM(half);
		fill (newargs,A21_corner, Zero1,dataset->A, A22, sub1, Zero1, half);
		pthread_create(&Mpool[3],NULL,&addM,newargs);
		pthread_join(Mpool[3],NULL);

		fill (newargs,Zero1,Zero1,sub1, B11, M2, Zero1, half)
		pthread_create(&Mpool[3],NULL,&split,newargs); // (A21+A22)*B11

		//M3
		int **M3 = createM(half);
		fill (newargs,B12_corner, Zero1, dataset->B, B22, sub1,Zero1, half);
		pthread_create(&Mpool[4],NULL,&subM,newargs); // sub1 = B12 - B22
		pthread_join(Mpool[4],NULL);


		fill(newargs,Zero1,Zero1,A11, sub1, M3, Zero1, half);
		pthread_create(&Mpool[5],NULL,&split,newargs); // M3 = A11 * (B12 - B22)
		pthread_join(Mpool[5],NULL);

		//M4
		int **M4 = createM(half);
		fill(newargs,B21_corner, Zero1, dataset->B, B11, sub1, Zero1, half);
		pthread_create(&Mpool[6],NULL,&subM,newargs); // sub1 = B21 - B11
		pthread_join(Mpool[6],NULL);

		fill(newargs,Zero1,Zero1,A22, sub1, M4, Zero1, half);
		pthread_create(&Mpool[7],NULL,&split,newargs); // M4 = A22 * (B21 - B11)
		pthread_join(Mpool[7],NULL);


		//M5
		int **M5 = createM(half);
		fill(newargs,A12_corner, Zero1, dataset->A, A11, sub1, Zero1, half); // A21 + A11
		pthread_create(&Mpool[8],NULL,&addM,newargs);
		pthread_join(Mpool[8],NULL);
		fill(newargs,Zero1, Zero1, sub1, B22, M5, Zero1, half);
		pthread_create(&Mpool[9],NULL,&split,newargs);// (A21 + A11) * B22
		pthread_join(Mpool[9],NULL);



		//M6
		int **M6 = createM(half);
		fill(newargs,A21_corner, Zero1, dataset->A, A11, sub1, Zero1, half);
		pthread_create(&Mpool[10],NULL,&subM,newargs); // sub1 = A21 - A11

		fill(newargs2,B12_corner, Zero2, dataset->B, B11, sub2, Zero2, half);
		pthread_create(&Mpool[11],NULL,&addM,newargs2); // sub2 = B12 + B11

		pthread_join(Mpool[10],NULL);
		pthread_join(Mpool[11],NULL);

		fill(newargs,Zero1, Zero1,sub1, sub2, M6, Zero1, half);
		pthread_create(&Mpool[12],NULL,&split,newargs); // (A21 - A11) * (B12 + B11)
		pthread_join(Mpool[12],NULL);





		//M7
		int **M7 = createM(half);
		subM(A12_corner, Zero, A, A22, sub1, Zero, half);
		addM(B21_corner, Zero, B, B22, sub2, Zero, half);
		split(sub1, sub2, M7, Zero, half);

		// Clean up
		deleteM(A11, half);
		deleteM(A22, half);
		deleteM(B11, half);
		deleteM(B22, half);
		// delete corners
		deleteB(A12_corner);
		deleteB(A21_corner);
		deleteB(B12_corner);
		deleteB(B21_corner);
		//
		int crow = Cdim[0];
		int ccol = Cdim[1];

		int *C11 = new int[2]{crow, ccol};
		int *C12 = new int[2]{crow, half};
		int *C21 = new int[2]{half, ccol};
		int *C22 = new int[2]{half, half};

		addM(Zero, Zero, M1, M7, sub1, Zero, half);
		subM(Zero, Zero, M4, M5, sub2, Zero, half);
		// C11
		addM(Zero,Zero,sub1, sub2, C, C11, half);
		//C12
		addM(Zero, Zero, M3, M5, C, C12, half);
		//C21
		addM(Zero, Zero, M2, M4, C, C21, half);
		//C22
		subM(Zero, Zero, M1, M2, sub1, Zero, half);
		addM(Zero, Zero, M3, M6, sub2, Zero, half);
		//M1 - M2 + M3 + M6
		addM(Zero, Zero, sub1, sub2, C, C22, half);

		// Delete temp matrices
		deleteM(M1, half);
		deleteM(M2, half);
		deleteM(M3, half);
		deleteM(M4, half);
		deleteM(M5, half);
		deleteM(M6, half);
		deleteM(M7, half);
		deleteM(sub1, half);
		deleteM(sub2, half);
		// Delete Corners
		deleteB(C11);
		deleteB(C12);
		deleteB(C21);
		deleteB(C22);
		deleteB(Zero);
	}

}//end void split
void deleteM(int** data, int S)
{
	for (int i = 0; i < S; i++)
		delete[] data[i];
	delete[] data;
	data = nullptr;
}
void deleteB(int*data)
{
	delete[] data;
	data = nullptr;
}
void fill (myargs *ptr, int *Adim,int *Bdim,int **A,int **B, int **C, int *Cdim,int max)
{
	ptr->A = A;
	ptr->B = B;
	ptr->C = C;
	ptr->Adim = Adim;
	ptr->Bdim = Bdim;
	ptr->Cdim = Cdim;
	ptr->max = max;
}
