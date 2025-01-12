#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <algorithm>
#include "DenseObj.hpp"
#include "SparseObj.hpp"

namespace utils{

template <typename T = double, typename MatrixType = DenseObj<T>>
void setMatrixValue(MatrixType& H, int i, int j, T value) {
	if constexpr (std::is_same_v<MatrixType, SparseMatrixCSC<T>>) {
        H.addValue(i, j, value);
        H.finalize();
    } else {
        H(i, j) = value;
    }
}

template <typename T, typename MatrixType>
void readfile(std::string filename, MatrixType& matrix) {
	// Open the file:
	std::ifstream fin(filename);
    if (!fin.is_open()) {
        std::cerr << "Unable to open" << std::endl;
        return;
    }

	// Declare variables:
	int M, N, L;

	// Ignore headers and comments:
	while (fin.peek() == '%') fin.ignore(2048, '\n');

	// Read defining parameters:
	fin >> M >> N >> L;

	std::cout << "M: " << M << " N: " << N << " L: " << L << std::endl;	
	// Create your matrix:

	matrix = MatrixType(M, N);
	std::cout << "Matrix created " << matrix.getRows() << " " << matrix.getCols() << std::endl; 

	// Read the data
	for (int l = 0; l < L; l++)
	{	if (l%1000 == 0){
			std::cout << "Reading line: " << l << std::endl;
		}
		int m, n;
		double data;
		fin >> m >> n >> data;
		setMatrixValue(matrix, m-1, n-1, data);
	}

	fin.close();
}

} // namespace utils
#endif // UTILS_HPP