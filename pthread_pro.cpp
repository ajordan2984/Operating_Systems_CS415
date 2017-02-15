//============================================================================
// Name        : pthreadex.cpp
// Author      : Andrew Jordan
// Version     : 20
// Copyright   : CS415 Freeware
// Description : Matrix Multiplication using Pthreads
// command line build with proper flags g++ pthreadx.cpp -o ex1.out -lpthread
// start program from command line: ./ex1.out
//============================================================================
#include <iostream>
#include <cmath>
#include <math.h>
#include <algorithm>
#include<fstream>
#include <stdio.h>
#include <pthread.h>
#include <string>
using namespace std;

struct data{
	pthread_mutex_t lock;
	int **A;
	int **B;
	int **C;
	int max;
};
struct deldata{
	pthread_mutex_t lock;
	int **data;
	int max;
};

//[row][col]
double test(int);
void printM(int **, int);
void fill(int **arry, int input, int resized);
int pow2(int data);
void *addM(void* args);
void *subM(void* args);
void mulM(int **A, int **B, int **C, int max);
void loadargs(data *ptr,int **A, int **B, int **C, int max);
int** createM(int); // creates a nXn matrix and returns a pointer
void *split(void*); // Recursive function to break matrix into small matrix
void delete_(deldata *ptr,int **data, int s); // Load struct with arguments to delete
void *deleteM(void*); // delete used 2D matrix


int main()
{
	ofstream out;
	out.open("results.txt");
	if (!out)
	{
		cout << "Error->outfile not open." << endl;
		exit(1);
	}
	else{
		out << "Data Sets| Time for Matrix Multiplication" << endl;
		out << "100:     " << test(15) << endl;
		//cout << "100 Finished." << endl;
		//out << "500:     " << test(500) << endl;
		//cout << "500 Finished." << endl;
		//out << "1000:    " << test(1000) << endl;
		//cout << "1000 Finished." << endl;
		//out << "5000:    " << test(4096) << endl;
		//cout << "5000 Finished." << endl;
		//out << "10000:   " << test(10000) << endl;
		//cout << "10k Finished" << endl;
	}
	cout << "Finished execution and writing file." << endl;
	return 0;
}
double test(int input)
{
	pthread_t MM[4];
	data *ptr = new data;

	int resized = pow2(input);
	// 1st matrix
	int **test1 = createM(resized);
	fill(test1, input - 1, resized);
	// 2nd matrix
	int **test2 = createM(resized);
	fill(test2, input - 1, resized);
	// Resulting matrix
	int **result = createM(resized);

	
	pthread_mutex_init(&ptr1->lock,NULL);
	ptr->A = test1;
	ptr->B = test2;
	ptr->C = result;
	ptr->max = resized;
	pthread_create(&MM[0],NULL,split,ptr);
	pthread_join(MM[0],NULL);

	//printM(result, resized);
	// clean up
	deldata *delptr = new deldata; // new struct to hold arguments to delete

	delete_(delptr,result,resized);
	pthread_create(&MM[1],NULL,&deleteM,delptr);
	delete_(delptr,test2,resized);
	pthread_create(&MM[2],NULL,&deleteM,delptr);
	delete_(delptr,test1,resized);
	pthread_create(&MM[3],NULL,&deleteM,delptr);
	for (int i=1;i<4;i++)
		pthread_join(MM[i],NULL);

	return 0.00;
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
	int i = 1;
	while (i < data)
		i = i * 2;
	return i;
}
void *addM(void *args)
{
	data *ptr = (data*)args;
	int max = ptr->max;
	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			ptr->C[i][j] = ptr->A[i][j] + ptr->B[i][j];
	pthread_exit(NULL);
}
void *subM(void *args)
{
	data *ptr = (data*)args;
	int max = ptr->max;
	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			ptr->C[i][j] = ptr->A[i][j] - ptr->B[i][j];
	pthread_exit(NULL);
}
void mulM(int **A, int **B, int **C, int max)
{
	for (int i = 0; i < max; i++)
		for (int k = 0; k < max; k++)
			for (int j = 0; j < max; j++)
				C[i][j] += (A[i][k] * B[k][j]);
}
void loadargs(data *ptr,int **A, int **B, int **C, int max)
{
	ptr->A = A;
	ptr->B = B;
	ptr->C = C;
	ptr->max = max;
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
void *split(void *args)
{
	data *startargs = (data*)args;
	pthread_mutex_lock(&startargs->lock);
	int max = startargs->max;

	if ((max / 2) < 16)
		mulM(startargs->A,startargs->B,startargs->C, max);
	else
	{
		data *ptr1 = new data;//Points to data for thread 1
		data *ptr2 = new data;//Points to data for thread 2
		pthread_mutex_init(&ptr1->lock,NULL); //Initialize lock1
		pthread_mutex_init(&ptr2->lock,NULL);// Initialzie lock2

		pthread_t Mpool[38]; //Number of threads used
		int half = max / 2;
		int **sub1 = createM(half);
		int **sub2 = createM(half);

		int **A11 = createM(half);
		int **A12 = createM(half);
		int **A21 = createM(half);
		int **A22 = createM(half);

		int **B11 = createM(half);
		int **B12 = createM(half);
		int **B21 = createM(half);
		int **B22 = createM(half);

		//[row][column]

		for (int i = 0; i < half; i++)
			for (int j = 0; j < half; j++)
			{
			A11[i][j] = startargs->A[i][j];
			A12[i][j] = startargs->A[i][j + half];
			A21[i][j] = startargs->A[i + half][j];
			A22[i][j] = startargs->A[i + half][j + half];

			B11[i][j] = startargs->B[i][j];
			B12[i][j] = startargs->B[i][j + half];
			B21[i][j] = startargs->B[i + half][j];
			B22[i][j] = startargs->B[i + half][j + half];
			}//end for spliting matrix

		//***************************************
		//Unlock the struct for passing args here
		//Cant unlock struct as C is used below
		//***************************************
		//pthread_mutex_unlock(&startargs->lock);

		//M1
		int **M1 = createM(half);
		// A11 + A22
		loadargs(ptr1,A11, A22, sub1, half);
		pthread_create(&Mpool[0],NULL,addM,ptr1);
		// B11 + B22
		loadargs(ptr2,B11, B22, sub2, half);
		pthread_create(&Mpool[1],NULL,addM,ptr2);
		pthread_join(Mpool[0],NULL);
		pthread_join(Mpool[1],NULL);
		// (A11 + A22) * (B11 + B22)
		loadargs(ptr1,sub1, sub2, M1, half);
		pthread_create(&Mpool[2],NULL,split,ptr1);
		pthread_join(Mpool[2],NULL);

		//M2
		int **M2 = createM(half);
		// A21 + A22
		loadargs(ptr1,A21, A22, sub1, half);
		pthread_create(&Mpool[3],NULL,addM,ptr1);
		//(A21 + A22)*B11
		loadargs(ptr1,sub1, B11, M2, half);
		pthread_create(&Mpool[4],NULL,split,ptr1);
		pthread_join(Mpool[4],NULL);

		//M3
		int **M3 = createM(half);
		// B12 - B22
		loadargs(ptr1,B12, B22, sub1, half);
		pthread_create(&Mpool[5],NULL,subM,ptr1);
		pthread_join(Mpool[5],NULL);
		// A11*(B12 - B22)
		loadargs(ptr1,A11, sub1, M3, half);
		pthread_create(&Mpool[6],NULL,split,ptr1);
		pthread_join(Mpool[6],NULL);

		int **M4 = createM(half);
		// B21 - B11
		loadargs(ptr1,B21, B11, sub1, half);
		pthread_create(&Mpool[7],NULL,subM,ptr1);
		pthread_join(Mpool[7],NULL);
		// A22*(B21 - B11)
		loadargs(ptr1,A22, sub1, M4, half);
		pthread_create(&Mpool[8],NULL,split,ptr1);
		pthread_join(Mpool[8],NULL);


		int **M5 = createM(half);
		// A12 + A11
		loadargs(ptr1,A12, A11, sub1, half);
		pthread_create(&Mpool[9],NULL,addM,ptr1);
		pthread_join(Mpool[9],NULL);
		// (A12 + A11)*B22
		loadargs(ptr1,sub1, B22, M5, half);
		pthread_create(&Mpool[10],NULL,split,ptr1);
		pthread_join(Mpool[10],NULL);


		//M6
		int **M6 = createM(half);
		// A21 - A11
		loadargs(ptr1,A21, A11, sub1, half);
		pthread_create(&Mpool[11],NULL,subM,ptr1);
		// B12 + B11
		loadargs(ptr2,B12, B11, sub2, half);
		pthread_create(&Mpool[12],NULL,addM,ptr2);
		// Join both threads
		pthread_join(Mpool[11],NULL);
		pthread_join(Mpool[12],NULL);
		// (A21 - A11)*(B12+B11)
		loadargs(ptr1,sub1, sub2, M6, half);
		pthread_create(&Mpool[13],NULL,split,ptr1);
		pthread_join(Mpool[13],NULL);


		//M7
		int **M7 = createM(half);
		// A12 - A22
		loadargs(ptr1,A12, A22, sub1, half);
		pthread_create(&Mpool[14],NULL,&subM,ptr1);
		// B21 + B22
		loadargs(ptr2,B21, B22, sub2, half);
		pthread_create(&Mpool[15],NULL,addM,ptr2);
		//Join threads
		pthread_join(Mpool[14],NULL);
		pthread_join(Mpool[15],NULL);
		//(A12 -A22)*(B21+B22)
		loadargs(ptr1,sub1, sub2, M7, half);
		pthread_create(&Mpool[16],NULL,split,ptr1);

		//Clean up;
		deldata *dptr = new deldata;
		pthread_mutex_init(&dptr->lock,NULL);

		delete_(dptr,A11,half);
		pthread_create(&Mpool[17],NULL,deleteM,dptr);
		delete_(dptr,A12,half);
		pthread_create(&Mpool[18],NULL,deleteM,dptr);
		delete_(dptr,A21,half);
		pthread_create(&Mpool[19],NULL,deleteM,dptr);
		delete_(dptr,A22,half);
		pthread_create(&Mpool[20],NULL,deleteM,dptr);
		delete_(dptr,B11,half);
		pthread_create(&Mpool[21],NULL,deleteM,dptr);
		delete_(dptr,B12,half);
		pthread_create(&Mpool[22],NULL,deleteM,dptr);
		delete_(dptr,B21,half);
		pthread_create(&Mpool[23],NULL,deleteM,dptr);
		delete_(dptr,B22,half);
		pthread_create(&Mpool[24],NULL,deleteM,dptr);
		for (int i = 16; i<25;i++)
		pthread_join(Mpool[i],NULL);

		int **C11 = createM(half);
		int **C12 = createM(half);
		int **C21 = createM(half);
		int **C22 = createM(half);
		
		// Create threads make C matrix
		pthread_t Cpool[8];
		// M1 + M7
		loadargs(ptr1,M1, M7, sub1, half);
		pthread_create(&Cpool[0],NULL,addM,ptr1);
		// M4 - M5
		loadargs(ptr2,M4, M5, sub2, half);
		pthread_create(&Cpool[1],NULL,subM,ptr2);
		pthread_join(Cpool[0],NULL);
	    pthread_join(Cpool[1],NULL);
		
		// C11 -> (M1 +M7)*(M4-M5)
		loadargs(ptr1,sub1, sub2, C11, half);
		pthread_create(&Cpool[2],NULL,addM,ptr1);
		
		//C12 -> M3 + M5
		loadargs(ptr1,M3, M5, C12, half);
		pthread_create(&Cpool[3],NULL,addM,ptr1);
	
		//C21 -> M2 + M4
		loadargs(ptr1,M2, M4, C21, half);
		pthread_create(&Cpool[4],NULL,addM,ptr1);
		
		// Wait on C11, C12, C21 to finish
		pthread_join(Cpool[2],NULL);
		pthread_join(Cpool[3],NULL);
		pthread_join(Cpool[4],NULL);
	
		//C22
		//M1 - M2
		loadargs(ptr1,M1, M2, sub1, half)
		pthread_create(&Cpool[5],NULL,subM,ptr1);
		//M3 + M6
		loadargs(ptr2,M3, M6, sub2, half)
		pthread_create(&Cpool[6],NULL,addM,ptr2);
		// Wait on 5 and 6
		pthread_join(Cpool[5],NULL);
		pthread_join(Cpool[6],NULL);

		//(M1 - M2) + (M3 + M6)
		loadargs(ptr1,sub1, sub2, C22, half)
		pthread_create(&Cpool[7],NULL,addM,ptr1);	
		pthread_join(Cpool[7],NULL);	
		
		// Delete used structs for passing args
		pthread_mutex_destroy(&ptr1->lock);
		pthread_mutex_destroy(&ptr2->lock);
		delete(ptr1);
		delete(ptr2);
				
	for (int i = 0; i<half; i++)
			for (int j = 0; j<half; j++)
			{
				startargs->C[i][j] = C11[i][j];
				startargs->C[i][j + half] = C12[i][j];
				startargs->C[i + half][j] = C21[i][j];
				startargs->C[i + half][j + half] = C22[i][j];
			}
		// Delete temp matrix M1 - M7
		delete_(dptr,M1,half);
		pthread_create(&Mpool[25],NULL,deleteM,dptr);
		delete_(dptr,M2,half);
		pthread_create(&Mpool[26],NULL,deleteM,dptr);
		delete_(dptr,M3,half);
		pthread_create(&Mpool[27],NULL,deleteM,dptr);
		delete_(dptr,M4,half);
		pthread_create(&Mpool[28],NULL,deleteM,dptr);
		delete_(dptr,M5,half);
		pthread_create(&Mpool[29],NULL,deleteM,dptr);
		delete_(dptr,M6,half);
		pthread_create(&Mpool[30],NULL,deleteM,dptr);
		delete_(dptr,M7,half);
		pthread_create(&Mpool[31],NULL,&deleteM,dptr);

		//**** 32 not used****

		// Delete temporary sub matricies
		delete_(dptr,sub1,half);
		pthread_create(&Mpool[32],NULL,deleteM,dptr);
		delete_(dptr,sub2,half);
		pthread_create(&Mpool[33],NULL,deleteM,dptr);

		// Delete Sub Matrix C11,C12,C21,C22
		delete_(dptr,C11,half);
		pthread_create(&Mpool[34],NULL,deleteM,dptr);
		delete_(dptr,C12,half);
		pthread_create(&Mpool[35],NULL,deleteM,dptr);
		delete_(dptr,C21,half);
		pthread_create(&Mpool[36],NULL,deleteM,dptr);
		delete_(dptr,C22,half);
		pthread_create(&Mpool[37],NULL,deleteM,dptr);

		for (int i = 25; i<38;i++)
		pthread_join(Mpool[i],NULL);
	}
		pthread_mutex_unlock(&startargs->lock);
		pthread_exit(0);
}//end void split

void delete_(deldata *ptr,int **data, int s)
{
	pthread_mutex_lock(&ptr->lock);
	ptr->data = data;
	ptr->max = s;
}
void *deleteM(void *args)
{
	deldata *ptr;
	ptr = (deldata*)args;
	//data = ptr->A;
	int S = ptr->max;
	for (int i = 0; i < S; i++)
		delete[] ptr->data[i];
	delete[] ptr->data;
	ptr->data = nullptr;
	pthread_mutex_unlock(&ptr->lock);
	pthread_exit(0);
}
