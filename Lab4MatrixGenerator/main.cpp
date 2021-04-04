#include <fstream>
#include <ctime>

using namespace std;

void generate_matrixes();
void generate_matrix(const char* fileName, int heigth, int width, bool useDefaultValue, int defaultValue, bool useRandomValue, int randomMin, int randomMax);

int main() 
{
	generate_matrixes();
	return 0;
}

void generate_matrixes() 
{
	generate_matrix("MatrixA1.txt", 960, 768, true, 1, false, NULL, NULL);
	generate_matrix("MatrixB1.txt", 768, 159, true, 1, false, NULL, NULL);
	generate_matrix("MatrixA2.txt", 960, 768, true, 1, false, NULL, NULL);
	generate_matrix("MatrixB2.txt", 768, 159, true, 3, false, NULL, NULL);
	generate_matrix("MatrixA3.txt", 960, 768, false, NULL, true, 1, 9);
	generate_matrix("MatrixB3.txt", 768, 159, false, NULL, true, 1, 9);
	generate_matrix("MatrixA4.txt", 960, 768, false, NULL, true, 10, 99);
	generate_matrix("MatrixB4.txt", 768, 159, false, NULL, true, 10, 99);
	generate_matrix("MatrixA5.txt", 960, 768, false, NULL, true, 100, 9999);
	generate_matrix("MatrixB5.txt", 768, 159, false, NULL, true, 100, 9999);
}

void generate_matrix(const char* fileName, int heigth, int width, bool useDefaultValue, int defaultValue, bool useRandomValue, int randomMin, int randomMax)
{
	srand(time(NULL));
	ofstream fout;
	fout.open(fileName);
	for (int i = 0; i < heigth; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (useDefaultValue)
				fout << defaultValue;
			else if (useRandomValue)
				fout << rand() % randomMax + randomMin;
			else
				fout << 0;
			if (j != width - 1)
				fout << " ";
		}
		if (i != heigth - 1)
			fout << endl;
	}
	fout.close();
}