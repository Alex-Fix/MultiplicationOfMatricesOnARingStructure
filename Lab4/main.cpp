#include "mpi.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <typeinfo>

using namespace std;

#define N1 960
#define N2 768
#define N3 159
#define SETTINGS_FILE_NAME "appsettings.txt"
#define MAX_NAME_LENGTH 100
#define SETTINGS_COUNT 4

// template prototypes
template<typename T>
T** create_matrix(int height, int width);
template<typename T>
void delete_matrix(T **&matrix, int height);
template<typename T>
void matrix_multiply(T** A, T** B, T** C, int n1, int n2, int n3);
template<typename T>
void read_matrix_from_file(const char* fileName, T** matrix, int height, int width);
template<typename T>
void read_part_of_matrix_from_file(const char* fileName, T** matrix, int height, int width, int startIndexH, int endIndexH, int startIndexW, int endIndexW);
template<typename T>
void print_matrix_to_file(const char* fileName, T** matrix, int height, int width);
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

int main(int *argc, char **argv)
{
	char **fileNames = load_settings();

	bool isReal = !strcmp(fileNames[0], "real");

	int ProcNum, ProcRank;
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
void delete_matrix(T** &matrix, int height)
{
	for (int i = 0; i < height; i++)
		delete[] matrix[i];
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
void run_process_0(char** fileNames)
{
	MPI_Status status;
	char *goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1, N3);

	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 0, 1 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 0, 1 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 1, 1, MPI_COMM_WORLD);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1);
	delete goFlag;
}

template<typename T>
void run_process_1(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1/8, N3/8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 1 * (N1 / 8), 2 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 2, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 1 * (N3 / 8), 2 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 2, 1, MPI_COMM_WORLD);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1/8);
	delete goFlag;
}

template<typename T>
void run_process_2(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1 / 8, N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 2 * (N1 / 8), 3 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 3, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 1, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 2 * (N3 / 8), 3 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 3, 1, MPI_COMM_WORLD);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_3(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1 / 8, N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 2, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 3 * (N1 / 8), 4 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 4, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 2, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 3 * (N3 / 8), 4 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 4, 1, MPI_COMM_WORLD);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_4(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1 / 8, N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 3, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 4 * (N1 / 8), 5 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 5, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 3, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 4 * (N3 / 8), 5 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 5, 1, MPI_COMM_WORLD);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_5(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1 / 8, N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 4, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 5 * (N1 / 8), 6 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 6, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 4, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 5 * (N3 / 8), 6 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 6, 1, MPI_COMM_WORLD);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_6(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1 / 8, N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 5, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 6 * (N1 / 8), 7 * (N1 / 8) - 1, 0, N2 - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 7, 0, MPI_COMM_WORLD);
	MPI_Recv(goFlag, 1, MPI_CHAR, 5, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 6 * (N3 / 8), 7 * (N3 / 8) - 1);
	MPI_Send(goFlag, 1, MPI_CHAR, 7, 1, MPI_COMM_WORLD);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1 / 8);
	delete goFlag;
}

template<typename T>
void run_process_7(char** fileNames)
{
	MPI_Status status;
	char* goFlag = new char;

	T** A = create_matrix<T>(N1 / 8, N2);
	T** B = create_matrix<T>(N2, N3 / 8);
	T** C = create_matrix<T>(N1 / 8, N3 / 8);

	MPI_Recv(goFlag, 1, MPI_CHAR, 6, 0, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[1], A, N1, N2, 7 * (N1 / 8), N1 - 1, 0, N2 - 1);
	MPI_Recv(goFlag, 1, MPI_CHAR, 6, 1, MPI_COMM_WORLD, &status);
	read_part_of_matrix_from_file<T>(fileNames[2], B, N2, N3, 0, N2 - 1, 7 * (N3 / 8), N3 / 8 - 1);

	delete_matrix<T>(A, N1 / 8);
	delete_matrix<T>(B, N2);
	delete_matrix<T>(C, N1 / 8);
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