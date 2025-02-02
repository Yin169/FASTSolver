#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>  // 确保包含 functional.h
#include "../src/Obj/SparseObj.hpp"
#include "../src/Obj/DenseObj.hpp"
#include "../src/Obj/VectorObj.hpp"
#include "../src/Intergal/GaussianQuad.hpp"
#include "../src/ODE/RungeKutta.hpp"
#include "../src/LinearAlgebra/Preconditioner/ILU.hpp"
#include "../src/LinearAlgebra/Preconditioner/MultiGrid.hpp"
#include "../src/LinearAlgebra/Krylov/ConjugateGradient.hpp"
#include "../src/LinearAlgebra/Krylov/GMRES.hpp"
#include "../src/LinearAlgebra/Krylov/KrylovSubspace.hpp"
#include "../src/LinearAlgebra/Solver/IterSolver.hpp"
#include "../src/LinearAlgebra/Factorized/basic.hpp"
#include "../src/utils.hpp"

using namespace Quadrature;
namespace py = pybind11;

// 自定义异常
class fastsolverError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

PYBIND11_MODULE(fastsolver, m) {
    m.doc() = "Pybind11 interface for numerical solvers and linear algebra tools";

    // 注册自定义异常
    py::register_exception<fastsolverError>(m, "fastsolverError");

    // Basic Operations
    m.def("power_iter", &basic::powerIter<double, DenseObj<double>>, 
          "Power iteration method for computing the dominant eigenvalue of a matrix.",
          py::arg("A"), py::arg("b"), py::arg("max_iter"));

    m.def("rayleigh_quotient", &basic::rayleighQuotient<double, DenseObj<double>>, 
          "Compute the Rayleigh quotient for a given matrix and vector.",
          py::arg("A"), py::arg("b"));

    // Krylov Subspace
    m.def("arnoldi", &Krylov::Arnoldi<double, DenseObj<double>, VectorObj<double>>, 
          "Arnoldi iteration for generating an orthogonal basis of the Krylov subspace.",
          py::arg("A"), py::arg("Q"), py::arg("H"), py::arg("tol"));

    // Matrix-Vector Multiplication
    m.def("matvec_mul", [](const SparseMatrixCSC<double> &A, const VectorObj<double> &x) {
        return A * x;
    }, "Matrix-vector multiplication", py::arg("A"), py::arg("x"));

    // Matrix-Matrix Multiplication
    m.def("matmat_mul", [](const SparseMatrixCSC<double> &A, const SparseMatrixCSC<double> &B) {
        return A * B;
    }, "Matrix-matrix multiplication", py::arg("A"), py::arg("B"));

    // Conjugate Gradient Solver
    py::class_<ConjugateGrad<double, SparseMatrixCSC<double>, VectorObj<double>>>(m, "ConjugateGrad")
        .def(py::init<const SparseMatrixCSC<double>&, const VectorObj<double>&, int, double>())
        .def("solve", &ConjugateGrad<double, SparseMatrixCSC<double>, VectorObj<double>>::solve,
             "Solve the linear system using the Conjugate Gradient method.");

    // Iterative Solvers
    py::class_<GradientDescent<double, SparseMatrixCSC<double>, VectorObj<double>>>(m, "GradientDescent")
        .def(py::init<const SparseMatrixCSC<double>&, const VectorObj<double>&, int, double>())
        .def("solve", &GradientDescent<double, SparseMatrixCSC<double>, VectorObj<double>>::solve,
             "Solve the linear system using Gradient Descent.");

    // MultiGrid Solver
    py::class_<AlgebraicMultiGrid<double, VectorObj<double>>>(m, "AlgebraicMultiGrid")
        .def(py::init<>())
        .def("amgVCycle", &AlgebraicMultiGrid<double, VectorObj<double>>::amgVCycle,
             "Perform one V-cycle of Algebraic MultiGrid.",
             py::arg("A"), py::arg("b"), py::arg("x"), py::arg("levels"), py::arg("smoothingSteps"), py::arg("theta"));

    // LU Factorization
    m.def("pivot_lu", &basic::PivotLU<double, DenseObj<double>>, 
        "Perform LU decomposition with partial pivoting.",
        py::arg("A"), py::arg("P"));

    // GMRES Solver
    py::class_<GMRES<double, SparseMatrixCSC<double>, VectorObj<double>>>(m, "GMRES")
        .def(py::init<>())
        .def("solve", &GMRES<double, SparseMatrixCSC<double>, VectorObj<double>>::solve,
             "Solve the linear system using GMRES.",
             py::arg("A"), py::arg("b"), py::arg("x"), py::arg("maxIter"), py::arg("KrylovDim"), py::arg("tol"));

    // Numerical Integration
   py::class_<GaussianQuadrature<double>>(m, "GaussQuadrature")
        .def(py::init<int>())
        .def("integrate", &GaussianQuadrature<double>::integrate, 
             "Perform numerical integration of a function over the interval [a, b].",
             py::arg("f"), py::arg("a"), py::arg("b"))
        .def("getPoints", &GaussianQuadrature<double>::getPoints, 
             "Get the quadrature points.")
        .def("getWeights", &GaussianQuadrature<double>::getWeights, 
             "Get the quadrature weights.");

    py::class_<RungeKutta<double, VectorObj<double>, VectorObj<double>>>(m, "RK4")
        .def(py::init<>())
        .def("solve", &RungeKutta<double, VectorObj<double>, VectorObj<double>>::solve, 
            py::arg("y"), py::arg("f"), py::arg("h"), py::arg("n"), py::arg("callback") = nullptr);
        // .def("solve_adaptive", &RungeKutta<double, VectorObj<double>, VectorObj<double>>::solveAdaptive, 
        //     py::arg("y"), py::arg("f"), py::arg("h"), py::arg("tol"), py::arg("max_steps"));

    // Matrix/Vector Operations
    py::class_<VectorObj<double>>(m, "Vector")
        .def(py::init<int>())
        .def("__getitem__", [](const VectorObj<double> &self, size_t index) -> double {
            if (index >= self.size()) {
                throw py::index_error("Index out of range");
            }
            return self[index];
        })
        .def("__setitem__", [](VectorObj<double> &self, size_t index, double value) {
            if (index >= self.size()) {
                throw py::index_error("Index out of range");
            }
            self[index] = value;
        })
        .def("size", &VectorObj<double>::size)
        .def("norm", &VectorObj<double>::L2norm, "Compute the L2 norm of the vector.");

    // Dense Matrix
    py::class_<DenseObj<double>>(m, "DenseMatrix")
        .def(py::init<int, int>())
        .def("__setitem__", [](DenseObj<double> &self, std::pair<int, int> idx, double value) {
            self(idx.first, idx.second) = value;
        })
        .def("__getitem__", [](const DenseObj<double> &self, std::pair<int, int> idx) {
            return self(idx.first, idx.second);
        })
        .def("rows", &DenseObj<double>::getRows)
        .def("cols", &DenseObj<double>::getCols)
        .def("__len__", &DenseObj<double>::getRows);  // 返回矩阵的行数

    // Sparse Matrix
    py::class_<SparseMatrixCSC<double>>(m, "SparseMatrix")
        .def(py::init<int, int>())
        .def("addValue", &SparseMatrixCSC<double>::addValue)
        .def("finalize", &SparseMatrixCSC<double>::finalize)
        .def("rows", &SparseMatrixCSC<double>::getRows)
        .def("cols", &SparseMatrixCSC<double>::getCols);

    // Matrix Market File IO
    m.def("read_matrix_market", [](const std::string& filename, py::object matrix) {
        if (py::isinstance<DenseObj<double>>(matrix)) {
            auto& mat = matrix.cast<DenseObj<double>&>();
            utils::readMatrixMarket<double, DenseObj<double>>(filename, mat);
        } else if (py::isinstance<SparseMatrixCSC<double>>(matrix)) {
            auto& mat = matrix.cast<SparseMatrixCSC<double>&>();
            utils::readMatrixMarket<double, SparseMatrixCSC<double>>(filename, mat);
        } else {
            throw fastsolverError("Unsupported matrix type");
        }
    }, "Read matrix from Matrix Market format file",
       py::arg("filename"), py::arg("matrix"));
}