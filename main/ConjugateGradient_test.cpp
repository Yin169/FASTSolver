#include <gtest/gtest.h>
#include "ConjugateGradient.hpp"
#include "MatrixObj.hpp"
#include "VectorObj.hpp"

// Helper function to compare two vectors for near equality
template<typename T>
bool areVectorsNear(const VectorObj<T>& v1, const VectorObj<T>& v2, double tol = 1e-6) {
    if (v1.get_row() != v2.get_row() || v1.get_col() != v2.get_col()) return false;
    for (int i = 0; i < v1.get_row(); ++i) {
        if (std::fabs(v1[i] - v2[i]) > tol) return false;
    }
    return true;
}

// Test fixture for Conjugate Gradient tests
template<typename T>
class ConjugateGradientTest : public ::testing::Test {
protected:
    MatrixObj<T> A, P;
    VectorObj<T> b, x_exact;

    void SetUp() override {
        // Create a symmetric positive definite matrix A
        A = MatrixObj<T>(3, 3);
        A(0, 0) = 5; A(0, 1) = 0; A(0, 2) = 1;
        A(1, 0) = 0; A(1, 1) = 2; A(1, 2) = 0;
        A(2, 0) = 1; A(2, 1) = 0; A(2, 2) = 3;

        P = MatrixObj<T>(3, 3);
        P(0, 0) = 1; P(0, 1) = 0; P(0, 2) = 0;
        P(1, 0) = 0; P(1, 1) = 1; P(1, 2) = 0;
        P(2, 0) = 0; P(2, 1) = 0; P(2, 2) = 1;

        // Create the right-hand side vector b
        b = VectorObj<T>(3);
        b[0] = 6; b[1] = 2; b[2] = 4;

        // The exact solution to Ax = b
        x_exact = VectorObj<T>(3);
        x_exact[0] = 1; x_exact[1] = 1; x_exact[2] = 1;
    }
};

using TestTypes = ::testing::Types<double>;
TYPED_TEST_SUITE(ConjugateGradientTest, TestTypes);

// Test case to check if the Conjugate Gradient solver solves a system correctly
TYPED_TEST(ConjugateGradientTest, SolvesSystemCorrectly) {
    ConjugateGrad<TypeParam> solver(this->P, this->A, this->b, 1000, 1e-12);
    solver.callUpdate();

    EXPECT_TRUE(areVectorsNear(solver.x, this->x_exact));
}

// Test case to verify that the residual is below tolerance
TYPED_TEST(ConjugateGradientTest, ResidualBelowTolerance) {
    ConjugateGrad<TypeParam> solver(this->P, this->A, this->b, 1000, 1e-6);
    solver.callUpdate();

    VectorObj<TypeParam> residual = this->b - (this->A * solver.x);
    EXPECT_LT(residual.L2norm(), 1e-6);
}

// Test case for non-convergence due to low max iterations
TYPED_TEST(ConjugateGradientTest, DoesNotConvergeWithinMaxIterations) {
    ConjugateGrad<TypeParam> solver(this->P, this->A, this->b, 2, 1e-12);
    solver.callUpdate();

    VectorObj<TypeParam> residual = this->b - (this->A * solver.x);
    EXPECT_GT(residual.L2norm(), 1e-12);
}

// // Test case to check handling of singular matrices
// TYPED_TEST(ConjugateGradientTest, HandlesSingularMatrix) {
//     this->A(2, 2) = 0; // Make the matrix singular

//     ConjugateGrad<TypeParam> solver(this->P, this->A, this->b, 1000, 1e-6);
//     EXPECT_THROW(solver.callUpdate(), std::runtime_error);
// }

// // Test case for ill-conditioned matrices
// TYPED_TEST(ConjugateGradientTest, HandlesIllConditionedMatrix) {
//     this->A(0, 0) = 1e10; // Increase condition number

//     ConjugateGrad<TypeParam> solver(this->P, this->A, this->b, 1000, 1e-8);
//     solver.callUpdate();

//     EXPECT_TRUE(areVectorsNear(solver.x, this->x_exact, 1e-6));
// }

// Test case for zero RHS vector
TYPED_TEST(ConjugateGradientTest, SolvesZeroRHS) {
    VectorObj<TypeParam> zero_b(this->b.get_row());
    ConjugateGrad<TypeParam> solver(this->P, this->A, zero_b, 1000, 1e-12);
    solver.callUpdate();

    VectorObj<TypeParam> zero_x(this->b.get_row());
    EXPECT_TRUE(areVectorsNear(solver.x, zero_x));
}

// // Test case with an initial guess
// TYPED_TEST(ConjugateGradientTest, SolvesWithInitialGuess) {
//     VectorObj<TypeParam> initial_guess(this->b.get_row(), 2.0); // Non-zero initial guess
//     ConjugateGrad<TypeParam> solver(this->P, this->A, this->b, 1000, 1e-12);
//     solver.x = initial_guess; // Set initial guess
//     solver.callUpdate();

//     EXPECT_TRUE(areVectorsNear(solver.x, this->x_exact));
// }

// // Test case for non-SPD matrix
// TYPED_TEST(ConjugateGradientTest, HandlesNonSPDMatrix) {
//     this->A(0, 2) = -1; // Make the matrix non-SPD

//     ConjugateGrad<TypeParam> solver(this->P, this->A, this->b, 1000, 1e-12);
//     EXPECT_THROW(solver.callUpdate(), std::runtime_error);
// }

// Main function to run the tests
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}