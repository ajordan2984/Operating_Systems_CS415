#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include<fstream>
using namespace std;

// Functions provided by Dr.Lewis
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
// My functions
double test(int);
void printM(int **, int);
void fill(int **arry, int input, int resized);
int pow2(int data);
void mulM(int*,int*,int*,int **A, int **B, int **Accum, int max);
void addM(int*, int*,int*, int **A, int **B, int **Accum, int max);
void subM(int *, int *,int *, int **A, int **B, int **Accum, int max);
int** createM(int msize);
void split(int *, int *,int *, int **A, int **B, int **joinedC, int max);
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
		out << "100:     " << test(5) << endl;
		//cout << "100 Finished." << endl;
		//out << "500:     " << test(500) << endl;
		//cout << "500 Finished." << endl;
		//out << "1000:    " << test(1000) << endl;
		//cout << "1000 Finished." << endl;
		//out << "5000:    " << test(5000) << endl;
		//cout << "5000 Finished." << endl;
		//out << "10000:   " << test(10000) << endl;
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
	fill(test1, input-1, resized);
	// 2nd matrix
	int **test2 = createM(resized);
	fill(test2, input-1, resized);
	// Resulting matrix
	int **result = createM(resized);

	int *test1_bounds = new int[2]{0, 0};
	int *test2_bounds = new int[2]{0, 0};
	int *result_bounds = new int[2]{0, 0};

	printM(test1, resized);
	printM(test2, resized);
	// Reset timer
	tmr.reset();
	doSomething();
	// Start MM
	split(test1_bounds, test2_bounds, result_bounds, test1, test2, result, resized);
	// find endtime
	doSomeMoreWork();
	double t = tmr.elapsed();

	printM(result, resized);
	// clean up
	deleteB(test1_bounds);
	deleteB(test2_bounds);
	deleteB(result_bounds);
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
void mulM(int *Abounds, int *Bbounds,int *AccumB, int **A, int **B, int **Accum, int max)
{
	// Abounds[row start][col start]
	// Bbounds[row start][col start]
	// Accumulator bounds[row start][col start]
	int Arow = Abounds[0];
	int Acol = Abounds[1];
	int Brow = Bbounds[0];
	int Bcol = Bbounds[1];
	int ACrow = AccumB[0];
	int ACcol = AccumB[1];
	
	Accum[ACrow][ACcol] = A[Arow][Acol] * B[Brow][Bcol];
}
void addM(int *Abounds, int *Bbounds, int *AccumB, int **A, int **B, int **Accum, int max)
{
	// Abounds[row start][col start]
	// Bbounds[row start][col start]
	// Accumulator bounds[row start][col start]
	int Arow = Abounds[0];
	int Acol = Abounds[1];
	int Brow = Bbounds[0];
	int Bcol = Bbounds[1];
	int ACrow = AccumB[0];
	int ACcol = AccumB[1];

	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			Accum[i + ACrow][j + ACcol] = A[i + Arow][j + Acol] + B[i + Brow][j + Bcol];
}
void subM(int *Abounds, int *Bbounds, int *AccumB, int **A, int **B, int **Accum, int max)
{
	// Abounds[row start][col start]
	// Bbounds[row start][col start]
	// Accumulator bounds[row start][col start]
	int Arow = Abounds[0];
	int Acol = Abounds[1];
	int Brow = Bbounds[0];
	int Bcol = Bbounds[1];
	int ACrow = AccumB[0];
	int ACcol = AccumB[1];

	for (int i = 0; i < max; i++)
		for (int j = 0; j < max; j++)
			Accum[i + ACrow][j + ACcol] = A[i + Arow][j + Acol] - B[i + Brow][j + Bcol];
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
void split(int *Astart, int *Bstart, int *Cstart, int **A, int **B, int **joinedC, int max)
{
	if ((max / 2) < 1)
		mulM(Astart,Bstart,Cstart,A, B, joinedC, max);

	else
	{
		int half = max / 2;
		int Arow = Astart[0];
		int Acol = Astart[1];
		int Brow = Bstart[0];
		int Bcol = Bstart[1];
		
		
		// Where corners of A start
		int *A11 = new int[2]{Arow, Acol};
		int *A12 = new int[2]{ Arow,half};
		int *A21 = new int[2]{ half,Acol};
		int *A22 = new int[2]{ half,half};
		// Where corners of B start
		int *B11 = new int[2]{Brow,Bcol};
		int *B12 = new int[2]{ Brow,half };
		int *B21 = new int[2]{ half,Bcol};
		int *B22 = new int[2]{ half,half };
		// Temp matrix to store inbetween answers
		int **temp1 = createM(half);
		int **temp2 = createM(half);
		// Where M1 - M7 row&column matricies start
		int *Mbounds = new int[2]{0, 0};
		int *tempb = new int[2]{0, 0};
		
		//M1
		int **M1 = createM(half);
		addM(A11, A22,tempb, A,A,temp1,half);
		addM(B11, B22,tempb, B,B,temp2, half);
		split(tempb, tempb, Mbounds,temp1, temp2, M1, half);

		//M2
		int **M2 = createM(half);
		addM(A21, A22,tempb,A,A,temp1,half);
		split(B11,tempb,Mbounds, B, temp1, M2, half);

		//M3
		int **M3 = createM(half);
		subM(B12, B22,tempb,B,B,temp1, half);
		split(A11,tempb,Mbounds, A, temp1, M3, half);

		//M4
		int **M4 = createM(half);
		subM(B21, B11,tempb,B, B,temp1,half);
		split(A22,tempb,Mbounds, A, temp1, M4, half);
	
		//M5
		int **M5 = createM(half);
		addM(A11, A12,tempb, A,A,temp1,half);
		split(B22,tempb,Mbounds, B, temp1, M5, half);

		//M6
		int **M6 = createM(half);
		subM(A21, A11,tempb, A,A,temp1,half);
		addM(B11, B12,tempb, B,B,temp2,half);
		split(tempb,tempb,Mbounds, temp1, temp2, M6, half);

		//M7
		int **M7 = createM(half);
		subM(A12, A22,tempb, A,A,temp1, half);
		addM(B21, B22,tempb, B,B,temp2, half);
		split(tempb, tempb,Mbounds,temp1, temp2, M7, half);

		int Crow = Cstart[0];
		int Ccol = Cstart[1];
		int *C11 = new int[2]{ Crow,Ccol};
		int *C12 = new int[2]{ Crow,half };
		int *C21 = new int[2]{ half,Ccol };
		int *C22 = new int[2]{ half,half };
		
		addM(Mbounds, Mbounds,tempb, M1, M7, temp1, half);// M1 + M7 
		subM(Mbounds, Mbounds,tempb, M4, M5, temp2, half);// M4 - M5
		addM(tempb,tempb, C11, temp1, temp2, joinedC, half);// C11
		addM(Mbounds, Mbounds, C12, M3, M5, joinedC, half);// C12 (M3 + M5)
		addM(Mbounds, Mbounds, C21, M2, M4, joinedC, half);// C21 (M2 + M4)
		subM(Mbounds, Mbounds,tempb, M1, M2, temp1, half);// M1 - M2
		addM(Mbounds, Mbounds,tempb, M3, M6, temp2, half);// M3 + M6
		addM(tempb, tempb, C22, temp1, temp2, joinedC, half);// C22 (M1 - M2 + M3 + M6)
		
		// Delete used matrix
		deleteM(M1, half);
		deleteM(M2, half);
		deleteM(M3, half);
		deleteM(M4, half);
		deleteM(M5, half);
		deleteM(M6, half);
		deleteM(M7, half);
		// Delete bounds
		deleteB(A11);
		deleteB(A12);
		deleteB(A21);
		deleteB(A22);
		deleteB(B11);
		deleteB(B12);
		deleteB(B21);
		deleteB(B22);
		deleteB(Mbounds);
		deleteB(C11);
		deleteB(C12);
		deleteB(C21);
		deleteB(C22);
	}

}//end void split
void deleteM(int** data, int S)
{
	for (int i = 0; i < S; i++)
		delete[] data[i];
	delete[] data;
	data = nullptr;
}
void deleteB(int* data)
{
	delete [] data;
	data = nullptr;
}
