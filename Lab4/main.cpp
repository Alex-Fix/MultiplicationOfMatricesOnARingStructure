#include "mpi.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <typeinfo>
#include <chrono>
#include<utility>

using namespace std;

#define N1 960
#define N2 768
#define N3 160
#define SETTINGS_FILE_NAME "appsettings.txt"
#define MAX_NAME_LENGTH 100
#define SETTINGS_COUNT 5

// template prototypes
template<typename T>
T** create_matrix(int height, int width);
template<typename T>
void delete_matrix(T **&matrix, int height);
template<typename T>
void matrix_multiply(T** A, T** B, T** C, int n1, int n2, int n3);
template<typename T>
void part_of_matrix_multiply(T** A, T** B, T** C, int n1, int n2, int n3, int cStartH, int cStartW);
template<typename T>
void read_matrix_from_file(const char* fileName, T** matrix, int height, int width);
template<typename T>
void read_part_of_matrix_from_file(const char* fileName, T** matrix, int height, int width, int startIndexH, int endIndexH, int startIndexW, int endIndexW);
template<typename T>
void print_matrix_to_file(const char* fileName, T** matrix, int height, int width);
template<typename T>
void move_chunk_to_matrix(T** C, T** Ctemp, int startIndexH, int endIndexH, int startIndexW, int endIndexW);
template<typename T>
void run_process_sync(char** fileNames);
template<typename T>
void run_process_0(char** fileNames);
template<typename T>
void run_process_1(char** fileNames);
template<typename T>
void run_process_2(char** fileNames);
template<typename T>
void run_process_3(char** fileNames);
template<typename T>
void run_process_4(char** fileNames);
template<typename T>
void run_process_5(char** fileNames);
template<typename T>
void run_process_6(char** fileNames);
template<typename T>
void run_process_7(char** fileNames);

// prototypes
char** load_settings();
void print_time(int procRank, long long nanoseconds, bool isSync);

int main(int *argc, char **argv)
{
	char **fileNames = load_settings();

	bool isReal = !strcmp(fileNames[0], "real");
	bool isSync = !strcmp(fileNames[4], "sync");
	
	if (isSync) 
	{
		if (isReal)
			run_process_sync<double>(fileNames);
		else
			run_process_sync<int>(fileNames);
	}
	else 
	{
		int ProcNum, ProcRank = 0;
		MPI_Init(argc, &argv);
		MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
		MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

		switch (ProcRank)
		{
		case 0:
			if (isReal)
				run_process_0<double>(fileNames);
			else
				run_process_0<int>(fileNames);
			break;
		case 1:
			if (isReal)
				run_process_1<double>(fileNames);
			else
				run_process_1<int>(fileNames);
			break;
		case 2:
			if (isReal)
				run_process_2<double>(fileNames);
			else
				run_process_2<int>(fileNames);
			break;
		case 3:
			if (isReal)
				run_process_3<double>(fileNames);
			else
				run_process_3<int>(fileNames);
			break;
		case 4:
			if (isReal)
				run_process_4<double>(fileNames);
			else
				run_process_4<int>(fileNames);
			break;
		case 5:
			if (isReal)
				run_process_5<double>(fileNames);
			else
				run_process_5<int>(fileNames);
			break;
		case 6:
			if (isReal)
				run_process_6<double>(fileNames);
			else
				run_process_6<int>(fileNames);
			break;
		case 7:
			if (isReal)
				run_process_7<double>(fileNames);
			else
				run_process_7<int>(fileNames);
			break;
		default:
			break;
		}

		MPI_Finalize();
	}
	
	delete_matrix(fileNames, SETTINGS_COUNT);

	return 0;
}

// templates
template<typename T>
T** create_matrix(int height, int width)
{
	T** matrix = new T * [height];
	for (int i = 0; i < height; i++)
	{
		matrix[i] = new T[width];
		for (int j = 0; j < width; j++)
			matrix[i][j] = 0;
	}
	return matrix;
}

template<typename T>
T** create_allocated_matrix(int height, int width) {
	T* data = new T[height * width];

	for (int i = 0; i < height * width; i++)
		data[i] = 0;

	T** array = new T*[height];

	for (int i = 0; i < height; i++)
		array[i] = &(data[width * i]);

	return array;
}

template<typename T>
void delete_matrix(T** &matrix, int height)
{
	for (int i = 0; i < height; i++)
		delete[] matrix[i];
	delete[] matrix;
}

template<typename T>
void delete_allocated_matrix(T** matrix, int height) {
	delete[] matrix[0];
	delete[] matrix;
}

template<typename T>
void matrix_multiply(T** A, T** B, T** C, int n1, int n2, int n3)
{
	for(int i = 0; i < n1; i++)
		for (int j = 0; j < n3; j++) 
		{
			C[i][j] = 0;
			for (int k = 0; k < n2; k++)
				C[i][j] += A[i][k] * B[k][j];
		}
}

template<typename T>
void part_of_matrix_multiply(T** A, T** B, T** C, int n1, int n2, int n3, int cStartH, int cStartW)
{
	for (int i = 0, cI = cStartH; i < n1; i++, cI++)
		for (int j = 0, cJ = cStartW; j < n3; j++, cJ++)
		{
			C[cI][cJ] = 0;
			for (int k = 0; k < n2; k++)
				C[cI][cJ] += A[i][k] * B[k][j];
		}
}

template<typename T>
void read_matrix_from_file(const char* fileName, T** matrix, int height, int width)
{
	ifstream fin;
	fin.open(fileName);

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			fin >> matrix[i][j];

	fin.close();
}

template<typename T>
void read_part_of_matrix_from_file(const char* fileName, T** matrix, int height, int width, int startIndexH, int endIndexH, int startIndexW, int endIndexW)
{
	T buffer;

	ifstream fin;
	fin.open(fileName);

	for (int i = 0, matrix_i = 0; i < height; i++) 
	{
		for (int j = 0, matrix_j = 0; j < width; j++)
			if (i >= startIndexH && i <= endIndexH && j >= startIndexW && j <= endIndexW)
				fin >> matrix[matrix_i][matrix_j++];
			else
				fin >> buffer;
		if (i >= startIndexH && i <= endIndexH)
			matrix_i++;
	}

	fin.close();
}

template<typename T>
void print_matrix_to_file(const char* fileName, T** matrix, int height, int width)
{
	ofstream fout;
	fout.open(fileName);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			fout << matrix[i][j];
			if (j != width - 1)
				fout << " ";
		}
		if (i != height - 1)
			fout << endl;
	}

	fout.close();
}

template<typename T>
void run_process_sync(char** fileNames)
{
	T** A = create_matrix<T>(N1, N2);
	T** B = create_matrix<T>(N2, N3);
	T** C = create_matrix<T>(N1, N3);

	read_matrix_from_file(fileNames[1], A, N1, N2);
	read_matrix_from_file(fileNames[2], B, N2, N3);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

	matrix_multiply(A, B, C, N1, N2, N3);

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(0, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), true);

	print_matrix_to_file(fileNames[3], C, N1, N3);

	delete_matrix(A, N1);
	delete_matrix(B, N2);
	delete_matrix(C, N1);
}

template<typename T>
void move_chunk_to_matrix(T** C, T** Ctemp, int startIndexH, int endIndexH, int startIndexW, int endIndexW) 
{
	for (int i = startIndexH, tempI = 0; i <= endIndexH; i++, tempI++)
		for (int j = startIndexW, tempJ = 0; j <= endIndexW; j++, tempJ++)
			C[i][j] = Ctemp[tempI][tempJ];
}

template<typename T>
void run_process_0(char** fileNames)
{
	MPI_Status status;
	char *goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1, N3);
	T** Ctemp = create_allocated_matrix<T>(N1, N3 / 8);

	int sizeB = N2 * (N3 / 8);

	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 0, 1 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 0, 1 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 1, 1, MPI_COMM_WORLD);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 1, i + 2, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 7, i, MPI_COMM_WORLD, &status);
		
		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, (i+1) * (N1/8), 0);
	}
	
	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(0, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_0.txt", C, N1 / 8, N3 / 8);

	for (int i = 1; i < 8; i++) {
		MPI_Recv(&(Ctemp[0][0]), N1 * (N3 / 8), dataType, i, 200, MPI_COMM_WORLD, &status);
		move_chunk_to_matrix<T>(C, Ctemp, 0, N1 - 1, i * (N3 / 8), (i+1) * (N3 / 8) - 1);
	}
	
	print_matrix_to_file(fileNames[3], C, N1, N3);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1);
	delete_allocated_matrix<T>(Ctemp, N1);
	delete goFlag;
}

template<typename T>
void run_process_1(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1/8, N3);

	int sizeB = N2 * (N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 1 * (N1 / 8), 2 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 2, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 1 * (N3 / 8), 2 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 2, 1, MPI_COMM_WORLD);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 2, i + 2, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 0, i + 2, MPI_COMM_WORLD, &status);

		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, (i + 1) * (N3 / 8));
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(1, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_1.txt", C, N1 / 8, N3 / 8);

	MPI_Send(&(C[0][0]), (N1 / 8) * N3, dataType, 0, 200, MPI_COMM_WORLD);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1/8);
	delete goFlag;
}

template<typename T>
void run_process_2(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1 / 8, N3);

	int sizeB = N2 * (N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 2 * (N1 / 8), 3 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 3, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 1, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 2 * (N3 / 8), 3 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 3, 1, MPI_COMM_WORLD);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 3, i + 2, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 1, i + 2, MPI_COMM_WORLD, &status);
		
		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, (i + 1) * (N3 / 8));
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(2, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_2.txt", C, N1 / 8, N3 / 8);

	MPI_Send(&(C[0][0]), (N1 / 8) * N3, dataType, 0, 200, MPI_COMM_WORLD);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_3(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1 / 8, N3);

	int sizeB = N2 * (N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 2, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 3 * (N1 / 8), 4 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 4, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 2, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 3 * (N3 / 8), 4 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 4, 1, MPI_COMM_WORLD);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 4, i + 2, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 2, i + 2, MPI_COMM_WORLD, &status);

		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, (i + 1) * (N3 / 8));
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(3, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_3.txt", C, N1 / 8, N3 / 8);

	MPI_Send(&(C[0][0]), (N1 / 8) * N3, dataType, 0, 200, MPI_COMM_WORLD);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_4(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1 / 8, N3);

	int sizeB = N2 * (N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 3, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 4 * (N1 / 8), 5 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 5, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 3, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 4 * (N3 / 8), 5 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 5, 1, MPI_COMM_WORLD);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 5, i + 2, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 3, i + 2, MPI_COMM_WORLD, &status);
		
		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, (i + 1) * (N3 / 8));
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(4, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_4.txt", C, N1 / 8, N3 / 8);

	MPI_Send(&(C[0][0]), (N1 / 8) * N3, dataType, 0, 200, MPI_COMM_WORLD);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_5(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1 / 8, N3);

	int sizeB = N2 * (N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 4, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 5 * (N1 / 8), 6 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 6, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 4, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 5 * (N3 / 8), 6 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 6, 1, MPI_COMM_WORLD);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 6, i + 2, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 4, i + 2, MPI_COMM_WORLD, &status);
		
		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, (i + 1) * (N3 / 8));
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(5, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_5.txt", C, N1 / 8, N3 / 8);

	MPI_Send(&(C[0][0]), (N1 / 8) * N3, dataType, 0, 200, MPI_COMM_WORLD);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_6(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1 / 8, N3);

	int sizeB = N2 * (N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 5, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 6 * (N1 / 8), 7 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 7, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 5, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 6 * (N3 / 8), 7 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 7, 1, MPI_COMM_WORLD);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 7, i + 2, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 5, i + 2, MPI_COMM_WORLD, &status);

		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, (i + 1) * (N3 / 8));
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(6, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_6.txt", C, N1 / 8, N3 / 8);

	MPI_Send(&(C[0][0]), (N1 / 8) * N3, dataType, 0, 200, MPI_COMM_WORLD);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_7(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;
	int dataType = typeid(T) == typeid(int) ? MPI_INT : MPI_DOUBLE;

	T** A = create_allocated_matrix<T>(N1 / 8, N2);
	T** B = create_allocated_matrix<T>(N2, N3 / 8);
	T** C = create_allocated_matrix<T>(N1 / 8, N3);

	int sizeB = N2 * (N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 6, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 7 * (N1 / 8), N1 - 1, 0, N2 - 1);
	MPI_Recv(goFlag, 1, MPI_CHAR, 6, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 7 * (N3 / 8), N3 - 1);

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, 0);

	for (int i = 0; i < 7; i++)
	{
		MPI_Send(&(B[0][0]), sizeB, dataType, 0, i, MPI_COMM_WORLD);
		MPI_Recv(&(B[0][0]), sizeB, dataType, 6, i + 2, MPI_COMM_WORLD, &status);

		part_of_matrix_multiply(A, B, C, N1 / 8, N2, N3 / 8, 0, (i + 1) * (N3 / 8));
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	print_time(7, chrono::duration_cast<chrono::nanoseconds>(end - start).count(), false);

	print_matrix_to_file("proc_7.txt", C, N1 / 8, N3 / 8);

	MPI_Send(&(C[0][0]), (N1 / 8) * N3, dataType, 0, 200, MPI_COMM_WORLD);

	delete_allocated_matrix<T>(A, N1 / 8);
	delete_allocated_matrix<T>(B, N2);
	delete_allocated_matrix<T>(C, N1 / 8);
	delete goFlag;
}

// functions
char** load_settings()
{
	char** fileNames = create_matrix<char>(SETTINGS_COUNT, MAX_NAME_LENGTH);

	ifstream fin;
	fin.open(SETTINGS_FILE_NAME);
	for (int i = 0; i < SETTINGS_COUNT; i++)
		fin >> fileNames[i];
	fin.close();

	return fileNames;
}

void print_time(int procRank, long long nanoseconds, bool isSync) {
	
	if (isSync)
	{
		cout << "The program worked in synchronous mode. Total execution time: " << nanoseconds << " ns. or " << nanoseconds / 1000000 << " ms." << endl;
		return;
	}

	cout << "The program worked in parallel. Process #"<< procRank << ". Total execution time: " << nanoseconds << " ns. or " << nanoseconds / 1000000 << " ms." << endl;
}