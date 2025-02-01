# FASTSolver: Advanced Scientific Computing Framework
## Overview
FASTSolver is a state-of-the-art scientific computing framework that delivers exceptional performance for numerical computations, CFD simulations, matrix operations, and scientific data visualization.
## Core Capabilities
### Advanced Numerical Methods
- Optimized linear algebra computations
- High-performance iterative solvers
  - GMRES (Generalized Minimal Residual)
  - CG (Conjugate Gradient)
- Adaptive multi-grid algorithms
- Robust ODE integration
- Advanced Newton-Raphson Implementation
  - Efficient nonlinear system resolution
  - Flexible Jacobian matrix handling
  - Seamless GMRES integration
  - Sophisticated convergence monitoring
### CFD Engine
- Advanced Lattice Boltzmann Implementation
  - Full D2Q9 and D3Q19 support
  - Comprehensive boundary condition library
  - Integrated IBM capabilities
- Interactive flow visualization
- Real-time performance analytics
### Development Tools
- Streamlined VTK export
- Dynamic flow visualization
- Comprehensive performance tracking
![JetFlow](https://github.com/Yin169/FASTSolver/blob/dev/doc/pic_1.png)
## System Prerequisites
- Modern C++ (17+)
- CMake 3.15 or newer
- Python 3.9+ (for optional features)
## Quick Start

## Installation

Clone the repository and run the setup script:

```bash
git clone https://github.com/Yin169/FASTSolver.git
cd FASTSolver
bash script_test.sh
```

For Python installation:

```bash
cd FASTSolver
python setup.py install
```

## Usage

Here's an example of how to use the `FASTSolver` module in Python:

```python
import fastsolver as fs

A = fs.SparseMatrix(2, 2)
A.addValue(0, 0, 4.0)
A.addValue(0, 1, 1.0)
A.addValue(1, 0, 1.0)
A.addValue(1, 1, 3.0)
A.finalize()

b = fs.Vector(2)
b[0] = 1.0
b[1] = 2.0

x = fs.Vector(2)
x[0] = 0.0
x[1] = 0.0

amg = fs.AlgebraicMultiGrid()

levels = 2
smoothing_steps = 10
theta = 0.5
amg.amgVCycle(A, b, x, levels, smoothing_steps, theta)

print("Solution x:", [x[i] for i in range(x.size())])
```

## Project Structure

```
FASTSolver
├── Doxyfile
├── LICENSE
├── README.md
├── application
│   ├── LatticeBoltz
│   │   └── LBMSolver.hpp
│   ├── Mesh
│   │   └── MeshObj.hpp
│   └── PostProcess
│       └── Visual.hpp
├── code
│   ├── test.ipynb
│   └── test.py
├── conanfile.txt
├── data
│   ├── Chem97ZtZ
│   │   └── Chem97ZtZ.mtx
│   └── Chem97ZtZ.tar
├── examples
│   ├── CylinderFlow.cpp
│   ├── JetFlow_2D.cpp
│   └── JetFlow_3D.cpp
├── python
│   └── pybind.cpp
├── script_test.sh
├── src
│   ├── Intergal
│   │   └── GaussianQuad.hpp
│   ├── LinearAlgebra
│   │   ├── Factorized
│   │   │   └── basic.hpp
│   │   ├── Krylov
│   │   │   ├── ConjugateGradient.hpp
│   │   │   ├── GMRES.hpp
│   │   │   └── KrylovSubspace.hpp
│   │   ├── Preconditioner
│   │   │   ├── LU.hpp
│   │   │   └── MultiGrid.hpp
│   │   └── Solver
│   │       └── IterSolver.hpp
│   ├── ODE
│   │   └── RungeKutta.hpp
│   ├── Obj
│   │   ├── DenseObj.hpp
│   │   ├── MatrixObj.hpp
│   │   ├── SparseObj.hpp
│   │   └── VectorObj.hpp
│   ├── PDEs
│   │   └── SpectralElementMethod.hpp
│   └── utils.hpp
└── test
    ├── ConjugateGradient_test.cpp
    ├── GMRES_test.cpp
    ├── GaussianQuad_test.cpp
    ├── KrylovSubspace_test.cpp
    ├── LBMSolver_test.cpp
    ├── LU_test.cpp
    ├── MeshObj_test.cpp
    ├── MultiGrid_test.cpp
    ├── RungeKutta_test.cpp
    ├── SparseMatrixCSCTest.cpp
    ├── SpectralElementMethod_test.cpp
    ├── Visual_test.cpp
    ├── basic_test.cpp
    ├── debuglogger.cpp
    ├── demo.cpp
    ├── itersolver_test.cpp
    ├── matrix_obj_test.cpp
    ├── test.cpp
    └── testfile.cpp
```

## Contributing

Contributions are welcome! Feel free to fork the repository and submit a pull request.

1. Fork the project
2. Create a feature branch (`git checkout -b feature-branch`)
3. Commit your changes (`git commit -am 'Add new feature'`)
4. Push to the branch (`git push origin feature-branch`)
5. Open a Pull Request

## License

This project is licensed under the MIT License.

## Acknowledgments

- Pybind11 for seamless C++ and Python integration
- Community contributions to numerical methods and solvers

## Citation

If you use FASTSolver in your research, please cite it as follows:

```markdown
@software{FASTSolver2024,
  author = {NG YIN CHEANG},
  title = {FASTSolver: High-Performance Scientific Computing Framework},
  year = {2024},
  url = {https://github.com/Yin169/FASTSolver}
}
```

