#include <gtest/gtest.h>
#include "csr_matrix.h"

using namespace std;

// 1. Проверка создания матрицы
TEST(TestMatrix, sozdat_matricu)
{
    ASSERT_NO_THROW(CSR_matrix<double> m(10, 10, 20));
}

// 2. Проверка генератора случайных чисел
TEST(TestMatrix, sgenerirovat_matricu)
{
    CSR_matrix<double> m(10, 10);
    ASSERT_NO_THROW(m.generate_matrix(3, 0.5, 2.5));
}

// 3. Заполнение матрицы из готовых векторов
TEST(TestMatrix, zapolnit_iz_vektorov)
{
    CSR_matrix<double> m(5, 5);
    vector<size_t> cols = { 0, 2, 4, 1, 3 };
    vector<size_t> rows = { 0, 2, 2, 4, 5 };
    vector<double> vals = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    ASSERT_NO_THROW(m.set_from_vectors(cols, rows, vals));
}

// 4. Транспонирование матрицы
TEST(TestMatrix, transponirovat_matricu)
{
    vector<size_t> rows = { 0 };
    vector<size_t> cols = { 1 };
    vector<double> vals = { 5.0 };

    auto mat = CSR_matrix<double>::from_coo(2, 3, rows, cols, vals);
    auto mat_t = mat.transpose();

    EXPECT_EQ(mat_t.n_rows, 3);
    EXPECT_EQ(mat_t.n_cols, 2);
    EXPECT_EQ(mat_t.nz, 1);
}

// 5. Сложение
TEST(TestMatrix, slozhenie_matric)
{
    vector<size_t> r1 = { 0, 1 };
    vector<size_t> c1 = { 0, 2 };
    vector<double> v1 = { 1.0, 2.0 };
    auto m1 = CSR_matrix<double>::from_coo(2, 2, r1, c1, v1);

    vector<size_t> r2 = { 0, 1 };
    vector<size_t> c2 = { 0, 1 };
    vector<double> v2 = { 3.0, 4.0 };
    auto m2 = CSR_matrix<double>::from_coo(2, 2, r2, c2, v2);

    ASSERT_NO_THROW({
        CSR_matrix<double> sum = m1 + m2;
        EXPECT_EQ(sum.nz, 3);
        });
}

// 6. Умножение матрицы на вектор
Test(TestMatrix, umnozhenie_na_vektor)
{
    vector<size_t> rows = { 0, 1 };
    vector<size_t> cols = { 0, 1 };
    vector<double> vals = { 2.0, 3.0 };
    auto mat = CSR_matrix<double>::from_coo(2, 2, rows, cols, vals);

    vector<double> x = { 4.0, 5.0 };
    vector<double> result;
    ASSERT_NO_THROW(result = mat * x);

    EXPECT_EQ(result.size(), 2);
    EXPECT_DOUBLE_EQ(result[0], 8.0);
    EXPECT_DOUBLE_EQ(result[1], 15.0);
}

// 7. Проверка ошибок и исключений
TEST(TestMatrix, proverka_oshibok)
{
    CSR_matrix<double> m1(2, 2);
    CSR_matrix<double> m3(3, 3);

    EXPECT_THROW(m1 + m3, invalid_argument);

    vector<double> bad_x = { 1.0, 2.0, 3.0 };
    EXPECT_THROW(m1 * bad_x, invalid_argument);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}