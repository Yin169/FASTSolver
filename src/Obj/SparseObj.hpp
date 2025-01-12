#ifndef SPARSEOBJ_HPP
#define SPARSEOBJ_HPP

#include <iostream>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <algorithm> // for std::lower_bound
#include <numeric>   // for std::accumulate

#include "MatrixObj.hpp"
#include "VectorObj.hpp"

template<typename TObj>
class VectorObj;

template<typename TObj>
class MatrixObj;

struct row_col_idx{
    int row;
    int col;
    size_t index;

    bool operator<(const row_col_idx& other) const {
        return row < other.col;
    }
};

template <typename TObj>
class SparseMatrixCSC : public MatrixObj<TObj> {
public:
    int _n, _m; // Number of rows and columns
    std::vector<TObj> values;      // Non-zero values
    std::vector<row_col_idx> row_indices_idx;  // Row indices of non-zeros
    std::vector<int> row_indices;  // Row indices of non-zeros
    std::vector<int> col_ptr;      // Column pointers
    std::vector<int> col_t_ptr;      // Column pointers

    // Constructor
    SparseMatrixCSC() = default;
    SparseMatrixCSC(int rows, int cols) : _n(rows), _m(cols), col_ptr(cols + 1, 0), col_t_ptr(cols + 1, 0) {}

    ~SparseMatrixCSC() = default;

    // Copy Constructor
    SparseMatrixCSC(const SparseMatrixCSC& other) = default;

    // Move Constructor
    SparseMatrixCSC(SparseMatrixCSC&& other) noexcept = default;

    // Copy Assignment
    SparseMatrixCSC& operator=(const SparseMatrixCSC& other) = default;

    // Move Assignment
    SparseMatrixCSC& operator=(SparseMatrixCSC&& other) noexcept = default;

    // Add a value to the sparse matrix
    void addValue(int row, int col, TObj value) {
        if (col >= _m || row >= _n) throw std::out_of_range("Row or column index out of range.");
        if (value == TObj()) return; // Skip zero values
        values.push_back(value);
        row_indices_idx.push_back({row, col, values.size() - 1});
        ++col_t_ptr[col + 1];
    }

    // Finalize column pointers after all values are added
    void finalize() {
        std::sort(row_indices_idx.begin(), row_indices_idx.end());
        std::vector<TObj> temp_values = values;
        for (int i = 0; i < row_indices_idx.size(); i++){
            values[i] = temp_values[row_indices_idx[i].index];
            row_indices.emplace_back(row_indices_idx[i].row);
        }
        std::partial_sum(col_t_ptr.begin(), col_t_ptr.end(), col_ptr.begin());
    }

    inline int getRows() const override { return _n; }
    inline int getCols() const override { return _m; }

    // Get a specific column
    VectorObj<TObj> getColumn(int index) const override {
        if (index < 0 || index >= _m) {
            throw std::out_of_range("Column index is out of range.");
        }
        VectorObj<TObj> column(_n);
        for (int i = 0; i < _n; ++i) {
            column[i] = (*this)(i, index);
        }
        return column;
    }

    // Element access with binary search
    TObj operator()(int row, int col) const {
        // for(int i = 0; i < values.size(); i++){
        //     std::cout << values[i] << " ";
        // }
        // std::cout << std::endl;
        // for(int i = 0; i < row_indices.size(); i++){
        //     std::cout << row_indices[i] << " ";
        // }
        // std::cout << std::endl;
        // for(int i = 0; i < col_t_ptr.size(); i++){
        //     std::cout << col_t_ptr[i] << " ";
        // }
        // std::cout << std::endl;
        // for(int i = 0; i < col_ptr.size(); i++){
        //     std::cout << col_ptr[i] << " ";
        // }
        // std::cout << std::endl;
        // std::cout << " ----------------- " << std::endl;
        if (row < 0 || row >= _n || col < 0 || col >= _m) throw std::out_of_range("Index out of range.");
        auto start = col_ptr[col];
        auto end = col_ptr[col + 1];
        auto it = std::lower_bound(row_indices.begin() + start, row_indices.begin() + end, row);
        // std::cout << "Start: " << start << " End: " << end << std::endl;
        // std::cout << "Row: " << row << " Col: " << col << std::endl;
        if (it != row_indices.begin() + end && *it == row) {
            return values[it - row_indices.begin()];
        }
        return TObj(); // Default value
    }

    template <typename Op>
    SparseMatrixCSC Operator(const SparseMatrixCSC& other, Op operation) const {
        if (_n != other._n || _m != other._m) throw std::invalid_argument("Matrices must be the same size for addition.");
        SparseMatrixCSC result(_n, _m);

        for (int col = 0; col < _m; ++col) {
            int a_pos = col_ptr[col], b_pos = other.col_ptr[col];
            std::unordered_map<int, TObj> col_data;

            while (a_pos < col_ptr[col + 1]) col_data[row_indices[a_pos++]] = operation(col_data[row_indices[a_pos]], values[a_pos]);
            while (b_pos < other.col_ptr[col + 1]) col_data[other.row_indices[b_pos++]] = operation(col_data[other.row_indices[b_pos]], other.values[b_pos]);

            for (const auto& [row, value] : col_data) {
                if (value != TObj()) result.addValue(row, col, value);
            }
        }
        result.finalize();
        return result;
    }

    // Optimized addition
    SparseMatrixCSC operator+(const SparseMatrixCSC& other) const {
        return Operator(other, std::plus<TObj>());
    }

    SparseMatrixCSC operator-(const SparseMatrixCSC& other) const {
        return Operator(other, std::minus<TObj>());
    }

    SparseMatrixCSC &operator*=(double scalar) {
        for (auto& e : values){ 
            e *= static_cast<TObj>(scalar);
        }
        return *this; 
    }

    SparseMatrixCSC operator*(double scalar){
        SparseMatrixCSC result = *this;
        result *= scalar;
        return result;
    }

    VectorObj<TObj> operator*(const VectorObj<TObj>& vector) const {
        // Check dimension compatibility
        if (_m != vector.size()) {
            throw std::invalid_argument("Sparse matrix columns must match vector size.");
        }

        // Result vector initialization
        VectorObj<TObj> result(_n, TObj(0));

        // Perform sparse matrix-vector multiplication
        for (int row = 0; row < _n; ++row) {
            for (int col = 0; col < _m; ++col) {
                result[row] += (*this)(row, col) * vector[col];
            }
        }
        return result;
    }


    // Optimized multiplication using CSC * CSR
    SparseMatrixCSC operator*(const SparseMatrixCSC& other) const {
        if (_m != other._n) throw std::invalid_argument("Matrix dimensions incompatible for multiplication.");
        SparseMatrixCSC result(_n, other._m);

        std::vector<std::vector<std::pair<int, TObj>>> rows(_n);
        for (int col = 0; col < other._m; ++col) {
            for (int k = other.col_ptr[col]; k < other.col_ptr[col + 1]; ++k) {
                int row_b = other.row_indices[k];
                TObj value_b = other.values[k];

                for (int p = col_ptr[row_b]; p < col_ptr[row_b + 1]; ++p) {
                    rows[row_indices[p]].emplace_back(col, values[p] * value_b);
                }
            }
        }

        for (int row = 0; row < _n; ++row) {
            std::unordered_map<int, TObj> temp_map;
            for (const auto& [col, value] : rows[row]) {
                temp_map[col] += value;
            }
            for (const auto& [col, value] : temp_map) {
                if (value != TObj()) result.addValue(row, col, value);
            }
        }

        result.finalize();
        return result;
    }
    // Transpose
    SparseMatrixCSC Transpose() const {
        SparseMatrixCSC result(_m, _n);
        result.col_ptr.resize(_n + 1, 0);
        result.row_indices.resize(values.size());
        result.values.resize(values.size());

        for (int i = 0; i < row_indices.size(); ++i) {
            ++result.col_ptr[row_indices[i] + 1];
        }

        // Compute cumulative sum to get column pointers
        std::partial_sum(result.col_ptr.begin(), result.col_ptr.end(), result.col_ptr.begin());

        // Populate row indices and values for the transpose
        for (int col = 0; col < _m; ++col) {
            for (int idx = col_ptr[col]; idx < col_ptr[col + 1]; ++idx) {
                int row = row_indices[idx];
                int dest_idx = result.col_ptr[row];

                result.row_indices[dest_idx] = col;        // Transpose row becomes column
                result.values[dest_idx] = values[idx];    // Copy the value
                ++result.col_ptr[row];
            }
        }

        // Adjust column pointers back by shifting right
        for (int i = _n; i > 0; --i) {
            result.col_ptr[i] = result.col_ptr[i - 1];
        }
        result.col_ptr[0] = 0;

        return result;
    }

};

#endif // SPARSEOBJ_HPP
