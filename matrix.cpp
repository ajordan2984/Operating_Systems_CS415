#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include<fstream>
using namespace std;
class Timer{
public:
	Timer() : beg_(clock_::now()) {}
	void reset() { beg_ = clock_::now(); }
	double elapsed() const {
		return std::chrono::duration_cast<second_>
			(clock_::now() - beg_).count();
	}
private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1>> second_;
	std::chrono::time_point<clock_> beg_;
};
void mySleep(unsigned long timeInSeconds)
{
	std::chrono::milliseconds timeInMS(timeInSeconds);
	std::this_thread::sleep_for(timeInMS);
}
void doSomething() { mySleep(3); };
void doSomeMoreWork() { mySleep(4); };
//[row][col]
void printM(int **, int);
double test(int);
void fill(int **arry, int input, int resized);
int pow2(int data);
void addM(int *Adim, int *Bdim, int **A, int **B, int **C, int max);
void addM(int **A, int **B, int **C, int *Cdim, int max);
void subM(int *Adim, int *Bdim, int **A, int **B, int **C, int max);
void subM(int **A, int **B, int **C, int *Cdim, int max);
void mulM(int **sub1, int **sub2, int **Accum, int max);
int** createM(int msize);
void split(int **A, int**B, int **C, int *Cdim, int max);
void deleteM(int** data, int S);
void deleteB(int*);

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
		out << "100:     " << test(100) << endl;
		cout << "100 Finished." << endl;
		out << "500:     " << test(500) << endl;
		cout << "500 Finished." << endl;
		out << "1000:    " << test(1000) << endl;
		cout << "1000 Finished." << endl;
		out << "5000:    " << test(4096) << endl;
		cout << "5000 Finished." << endl;
		//out << "10000:   " << test(10000) << endl; 
		//cout << "10k Finished" << endl;
	}
	cout << "Finished execution and writing file." << endl;
	return 0;
}
double test(int input)
{
	int resized = pow2(input);
	Timer tmr;
	// 1st matrix
	int **test1 = createM(resized);
	fill(test1, input - 1, resized);
	// 2nd matrix
	int **test2 = createM(resized);
	fill(test2, input - 1, resized);
	// Resulting matrix
	int **result = createM(resized);
	int *resultC = new int[2]{0, 0};
	// Reset timer
	tmr.reset();
	doSomething();
	// Start MM
	split(test1, test2, result, resultC, resized);
	// find endtime
	doSomeMoreWork();
	double t = tmr.elapsed();

	//printM(result, resized);
	// clean up
	deleteB(resultC);
	deleteM(result, resized);
	deleteM(test1, resized);
	deleteM(test2, resized);
	return t;
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
	while (pow(2, i) < data)
		i++;
	return pow(2, i);
}
void addM(int **A, int **B, int **C, int *Cdim, int max)
{
	int Crow = Cdim[0];
	int Ccol = Cdim[1];
	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			C[i + Crow][j + Ccol] = A[i][j] + B[i][j];
}
void addM(int *Adim, int *Bdim, int **A, int **B, int **C, int max)
{
	// Abounds[row start][col start]
	// Bbounds[row start][col start]
	// Accumulator bounds[row start][col start]
	int Arow = Adim[0];
	int Acol = Adim[1];
	int Brow = Bdim[0];
	int Bcol = Bdim[1];

	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			C[i][j] = A[i + Arow][j + Acol] + B[i + Brow][j + Bcol];
}
void subM(int *Adim, int *Bdim, int **A, int **B, int **C, int max)
{
	int Arow = Adim[0];
	int Acol = Adim[1];
	int Brow = Bdim[0];
	int Bcol = Bdim[1];
	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			C[i][j] = A[i + Arow][j + Acol] - B[i + Brow][j + Bcol];
}
void subM(int **A, int **B, int **C, int *Cdim, int max)
{
	int Crow = Cdim[0];
	int Ccol = Cdim[1];
	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			C[i + Crow][j + Ccol] = A[i][j] - B[i][j];
}
void mulM(int **sub1, int **sub2, int **Accum, int max)
{
	for (int i = 0; i < max; i++)
		for (int k = 0; k < max; k++)
			for (int j = 0; j < max; j++)
				Accum[i][j] += (sub1[i][k] * sub2[k][j]);
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
void split(int **A, int **B, int **C, int *Cdim, int max)
{
	if ((max / 2) < 16)
		mulM(A, B, C, max);
	else
	{
		int half = max / 2;
		int **sub1 = createM(half);
		int **sub2 = createM(half);
		int *Zero = new int[2]{0, 0};

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
			A11[i][j] = A[i][j];
			A22[i][j] = A[i + half][j + half];
			B11[i][j] = B[i][j];
			B22[i][j] = B[i + half][j + half];
			}//end for spliting matrix

		//M1
		int **M1 = createM(half);
		addM(Zero, Zero, A11, A22, sub1, half);
		addM(Zero, Zero, B11, B22, sub2, half);
		split(sub1, sub2, M1, Zero, half);

		//M2
		int **M2 = createM(half);
		addM(A21_corner, Zero, A, A22, sub1, half);
		split(sub1, B11, M2, Zero, half);

		//M3
		int **M3 = createM(half);
		subM(B12_corner, Zero, B, B22, sub1, half);
		split(A11, sub1, M3, Zero, half);

		//M4
		int **M4 = createM(half);
		subM(B21_corner, Zero, B, B11, sub1, half);
		split(A22, sub1, M4, Zero, half);

		//M5
		int **M5 = createM(half);
		addM(A12_corner, Zero, A, A11, sub1, half);
		split(sub1, B22, M5, Zero, half);

		//M6
		int **M6 = createM(half);
		subM(A21_corner, Zero, A, A11, sub1, half);
		addM(B12_corner, Zero, B, B11, sub2, half);
		split(sub1, sub2, M6, Zero, half);

		//M7
		int **M7 = createM(half);
		subM(A12_corner, Zero, A, A22, sub1, half);
		addM(B21_corner, Zero, B, B22, sub2, half);
		split(sub1, sub2, M7, Zero, half);

		// Clean up
		deleteM(A11, half);
		deleteM(A22, half);
		deleteM(B11, half);
		deleteM(B22, half);
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

		addM(Zero, Zero, M1, M7, sub1, half);
		subM(Zero, Zero, M4, M5, sub2, half);
		// C11
		addM(sub1, sub2, C, C11, half);
		//C12
		addM(M3, M5, C, C12, half);
		//C21
		addM(M2, M4, C, C21, half);
		//C22
		subM(Zero, Zero, M1, M2, sub1, half);
		addM(Zero, Zero, M3, M6, sub2, half);
		//M1 - M2 + M3 + M6
		addM(sub1, sub2, C, C22, half);

		// Delete temp matricies
		deleteM(M1, half);
		deleteM(M2, half);
		deleteM(M3, half);
		deleteM(M4, half);
		deleteM(M5, half);
		deleteM(M6, half);
		deleteM(M7, half);
		deleteM(sub1, half);
		deleteM(sub2, half);
		// Delete Coners
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
