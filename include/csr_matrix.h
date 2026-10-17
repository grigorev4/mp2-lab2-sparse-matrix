#pragma once

#include <cstdlib>
#include <iostream>
#include <random>
#include <cmath>

using namespace std;

template <typename ValT = double>
class CSR_matrix {
public:
    size_t n_rows = 0; // число строк
    size_t n_cols = 0; // число столбцов
    size_t nz = 0; // число ненулевых элементов
    size_t capacity = 0; // объем памяти для хранения значений
    size_t* RowIndex = nullptr;
    size_t* Col = nullptr;
    ValT* Val = nullptr;

    CSR_matrix(size_t _m = 1, size_t _n = 1) : n_rows(_m), n_cols(_n), Col(nullptr), Val(nullptr) {
        RowIndex = new size_t[n_rows + 1]();
    }

    CSR_matrix(size_t _m, size_t _n, size_t _nz) : n_rows(_m), n_cols(_n), nz(_nz) {
        RowIndex = new size_t[n_rows + 1] ();
        if (nz > 0) {
            Col = new size_t[nz];
            Val = new ValT[nz]();
        }
        else {
			Col = nullptr;
			Val = nullptr;
        }
        capacity = nz;
    }

    CSR_matrix(const CSR_matrix& copy) : n_rows(copy.n_rows), n_cols(copy.n_cols), nz(copy.nz), capacity(copy.capacity) {
        RowIndex = new size_t[n_rows + 1];
        std::copy(copy.RowIndex, copy.RowIndex + n_rows + 1, RowIndex);
        if (copy.nz > 0 && copy.Col != nullptr && copy.Val != nullptr) {
            Val = new ValT[nz];
            std::copy(copy.Val, copy.Val + nz, Val);
            Col = new size_t[nz];
            std::copy(copy.Col, copy.Col + nz, Col);
        }
        else {
            Val = nullptr;
			Col = nullptr;
        }
    }

    CSR_matrix(CSR_matrix&& mov) : n_rows(mov.n_rows), n_cols(mov.n_cols), nz(mov.nz), capacity(mov.capacity) {
        Col = mov.Col;
        RowIndex = mov.RowIndex;
        Val = mov.Val;

        mov.Col = nullptr;
        mov.RowIndex = nullptr;
        mov.Val = nullptr;
    }

    ~CSR_matrix() {
        delete[] Col;
        delete[] RowIndex;
        delete[] Val;
    }

    CSR_matrix& operator=(const CSR_matrix& copy) {
        if (this == &copy)
            return *this;

        if (n_rows != copy.n_rows) {
            delete[] RowIndex;
            RowIndex = new size_t[copy.n_rows + 1];
        }
        std::copy(copy.RowIndex, copy.RowIndex + copy.n_rows + 1, RowIndex);
        if (capacity < copy.nz) {
            delete[] Col;
            delete[] Val;

            Col = new size_t[copy.nz];
            Val = new ValT[copy.nz];
            capacity = copy.nz;
        }
        std::copy(copy.Col, copy.Col + copy.nz, Col);
        std::copy(copy.Val, copy.Val + copy.nz, Val);

        n_rows = copy.n_rows;
        n_cols = copy.n_cols;
        nz = copy.nz;

        return *this;
    }

    CSR_matrix& operator=(CSR_matrix&& mov) {
        if (this == &mov)
            return *this;

		std::swap(Col, mov.Col);
		std::swap(RowIndex, mov.RowIndex);
		std::swap(Val, mov.Val);
		std::swap(n_rows, mov.n_rows);
		std::swap(n_cols, mov.n_cols);
		std::swap(nz, mov.nz);
		std::swap(capacity, mov.capacity);

        return *this;
    }
    
    // вектора cols, rows, vals содержат корректное описание матрицы в формате CSR
    void set_from_vectors(const std::vector<size_t>& cols, const std::vector<size_t>& rows, 
        const std::vector<ValT>& vals) {

        size_t new_nz = cols.size();
        if (nz != new_nz) {
            delete[] Col;
            delete[] Val;
            nz = new_nz;
            Col = new size_t[new_nz];
            Val = new ValT[new_nz];
        }
		if (n_rows != rows.size() - 1) {
			delete[] RowIndex;
			RowIndex = new size_t[n_rows + 1];
            n_rows = rows.size() - 1;
			n_cols = std::max(*std::max_element(cols.begin(), cols.end()) + 1, n_rows);
		}
		std::copy(cols.begin(), cols.end(), Col);
		std::copy(vals.begin(), vals.end(), Val);
        std::copy(rows.begin(), rows.end(), RowIndex);
    }

    void generate_matrix(const size_t avg_degree, ValT minVal, ValT maxVal) {
        size_t new_nz = n_rows * avg_degree;
        if (avg_degree > n_cols) {
            throw std::invalid_argument("avg_degree cannot be greater than the number of columns");
        }

        if (nz != new_nz) {
            delete[] Col;
            delete[] Val;

            nz = new_nz;
            capacity = nz;

            Col = new size_t[nz];
            Val = new ValT[nz];
        }

        std::random_device rd;
        std::mt19937 gen(rd());

        using DistT = std::conditional_t<std::is_floating_point_v<ValT>,
            std::uniform_real_distribution<ValT>,
            std::uniform_int_distribution<ValT>>;
        DistT val_dist(minVal, maxVal);

        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        double p = static_cast<double>(avg_degree) / n_cols;

        size_t current_elem = 0;
        RowIndex[0] = 0;

        for (size_t i = 0; i < n_rows; ++i) {
            for (size_t j = 0; j < n_cols && current_elem < nz; ++j) {
                if (prob_dist(gen) < p) {
                    Col[current_elem] = j;
                    Val[current_elem] = val_dist(gen);
                    current_elem++;
                }
            }
            RowIndex[i + 1] = current_elem;
        }
        nz = RowIndex[n_rows];
        capacity = nz;
    }


	void print_coo() const {
		std::cout << "row  col val\n";
		for (size_t i = 0; i < n_rows; ++i)
			for (size_t j = RowIndex[i]; j < RowIndex[i + 1]; ++j)
				std::cout << i << " " << Col[j] << " " << Val[j] << '\n';
	}


    void print_dense() const {
		for (size_t i = 0; i < n_rows; ++i) {
			size_t k = 0;
			for (size_t j = RowIndex[i]; j < RowIndex[i + 1]; ++j, ++k) {
				while (k < Col[j]) {
					std::cout << 0 << " ";
					++k;
				}
				std::cout << Val[j] << " ";
			}
			while (k < n_cols) {
				std::cout << 0 << " ";
				++k;
			}
			std::cout << '\n';
		}
		std::cout << '\n';

    }

    // TODO:
	// сложение матриц в формате CSR
    CSR_matrix operator+(const CSR_matrix& other) const {
        if (n_rows != other.n_rows || n_cols != other.n_cols) {
            throw invalid_argument("Matrixi ne sovpadayut");
        }

        vector<size_t> res_row_index;
        vector<size_t> res_cols;
        vector<ValT> res_vals;
        res_row_index.push_back(0);

        for (size_t i = 0; i < n_rows; i++) {
            size_t pA = RowIndex[i];
            size_t endA = RowIndex[i + 1];
            size_t pB = other.RowIndex[i];
            size_t endB = other.RowIndex[i + 1];

            while (pA < endA && pB < endB) {
                if (Col[pA] < other.Col[pB]) {
                    res_cols.push_back(Col[pA]);
                    res_vals.push_back(Val[pA]);
                    pA++;
                }
                else if (Col[pA] > other.Col[pB]) {
                    res_cols.push_back(other.Col[pB]);
                    res_vals.push_back(other.Val[pB]);
                    pB++;
                }
                else {
                    ValT sum = Val[pA] + other.Val[pB];
                    if (abs(sum) > 1e-9) {
                        res_cols.push_back(Col[pA]);
                        res_vals.push_back(sum);
                    }
                    pA++;
                    pB++;
                }
            }

            while (pA < endA) {
                res_cols.push_back(Col[pA]);
                res_vals.push_back(Val[pA]);
                pA++;
            }

            while (pB < endB) {
                res_cols.push_back(other.Col[pB]);
                res_vals.push_back(other.Val[pB]);
                pB++;
            }

            res_row_index.push_back(res_cols.size());
        }

        CSR_matrix result(n_rows, n_cols, res_cols.size());
        result.set_from_vectors(res_cols, res_row_index, res_vals);
        return result;
    }

	// умножение матрицы на вектор
    std::vector<ValT> operator*(const std::vector<ValT>& x) const {
        if (n_cols != x.size()) {
            throw invalid_argument("Ne sovpadayut razmeri");
        }
        vector<ValT> y(n_rows, 0);

        for (size_t i = 0; i < n_rows; i++) {
            ValT sum = 0;
            for (size_t j = RowIndex[i]; j < RowIndex[i + 1]; j++) {
                sum += Val[j] * x[Col[j]];
            }
            y[i] = sum;
        }

        return y;
    }
	// транспонирование матрицы в формате CSR
    CSR_matrix transpose() const {
        CSR_matrix result(n_cols, n_rows, nz);
        
        if (nz == 0) {
            return result;
        }
        vector<size_t> col_counts(n_cols, 0);
        for (size_t i = 0; i < nz; i++) {
            col_counts[Col [i] ]++;
        }

        result.RowIndex[0] = 0;
        for (size_t i = 0; i < n_cols; i++) {
            result.RowIndex[i + 1] = result.RowIndex[i] + col_counts[i];
        }

        vector<size_t> current_pos(result.RowIndex, result.RowIndex + n_cols);
        for (size_t i = 0; i < n_rows; i++) {
            for (size_t j = RowIndex[i]; j < RowIndex[i + 1]; j++) {
                size_t old_col = Col[j];
                ValT val = Val[j];
                size_t new_row = old_col;
                size_t dest_idx = current_pos[new_row]++;
                result.Col[dest_idx] = i;
                result.Val[dest_idx] = val;
            }
        }
        return result;
    }
	// конвертер из координатного формата в формат CSR
    static CSR_matrix from_coo(size_t n_rows, size_t n_cols, const std::vector<size_t>& rows, const std::vector<size_t>& cols, const std::vector<ValT>& vals) {
        if (rows.size() != cols.size() || rows.size() != vals.size()) {
            throw invalid_argument("raznaya dlina");
        }

        size_t nz = rows.size();
        CSR_matrix result(n_rows, n_cols, nz);
        if (nz == 0) {
            return result;
        }

        vector<size_t> row_counts(n_rows, 0);
        for (size_t k = 0; k < nz; k++) {
            row_counts[rows[k]]++;
        }

        result.RowIndex[0] = 0;
        for (size_t i = 0; i < n_rows; i++) {
            result.RowIndex[i + 1] = result.RowIndex[i] + row_counts[i];
        }

        vector<size_t> current_pos(result.RowIndex, result.RowIndex + n_rows);
        for (size_t k = 0; k < nz; k++) {
            size_t r = rows[k];
            size_t c = cols[k];
            ValT v = vals[k];

            size_t dest_idx = current_pos[r]++;
            result.Col[dest_idx] = c;
            result.Val[dest_idx] = v;
        }

        return result;
    }
	
	// подумайте, как реализовать корректно для разных типов данных ValT
    bool operator==(const CSR_matrix& other) const {
        if (n_rows != other.n_rows || n_cols != other.n_cols || nz != other.nz)
            return false;

        for (size_t i = 0; i <= n_rows; i++) {
            if (RowIndex[i] != other.RowIndex[i])
                return false;
        }

        for (size_t i = 0; i < nz; i++) {
            if (Col[i] != other.Col[i])
                return false;
        }

        for (size_t i = 0; i < nz; i++) {
            if (is_floating_point_v<ValT>) {
                if (abs(Val[i] - other.Val[i]) > 1e-9)
                    return false;
            }
            else {
                if (Val[i] != other.Val[i])
                    return false;
            }
        }
        return true;
    }
};


