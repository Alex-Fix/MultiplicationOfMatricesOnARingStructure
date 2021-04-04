#include <fstream>
#include <ctime>

using namespace std;

#define N1 960
#define N2 768
#define N3 159

void generate_matrixes();
void generate_matrix(const char* fileName, int height, int width, bool isDefaultValue, int defaultValue, bool isRandomValue, bool isReal, int randomMin, int randomMax);

int main() 
{
	generate_matrixes();
	return 0;
}

void generate_matrixes() 
{
	generate_matrix("MatrixA1.txt", N1, N2, true, 1, false, false, NULL, NULL);
	generate_matrix("MatrixB1.txt", N2, N3, true, 1, false, false, NULL, NULL);
	generate_matrix("MatrixA2.txt", N1, N2, true, 1, false, false, NULL, NULL);
	generate_matrix("MatrixB2.txt", N2, N3, true, 3, false, false, NULL, NULL);
	generate_matrix("MatrixA3.txt", N1, N2, false, NULL, true, false, 1, 9);
	generate_matrix("MatrixB3.txt", N2, N3, false, NULL, true, false, 1, 9);
	generate_matrix("MatrixA4.txt", N1, N2, false, NULL, true, false, 10, 99);
	generate_matrix("MatrixB4.txt", N2, N3, false, NULL, true, false, 10, 99);
	generate_matrix("MatrixA5.txt", N1, N2, false, NULL, true, true, 100, 9999);
	generate_matrix("MatrixB5.txt", N2, N3, false, NULL, true, true, 100, 9999);
}

void generate_matrix(const char* fileName, int height, int width, bool isDefaultValue, int defaultValue, bool isRandomValue, bool isReal, int randomMin, int randomMax)
{
	srand(time(NULL));
	ofstream fout;
	fout.open(fileName);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (isDefaultValue)
				fout << defaultValue;
			else if (isRandomValue)
				if (isReal) 
				{
					double fraction = (rand() % 1000) / 1000.0;
					int integer = rand() % (randomMax - 1) + randomMin;
					fout << integer + fraction;
				}
				else
					fout << rand() % randomMax + randomMin;
			else
				fout << 0;
			if (j != width - 1)
				fout << " ";
		}
		if (i != height - 1)
			fout << endl;
	}
	fout.close();
}