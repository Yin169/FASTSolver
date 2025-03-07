#ifndef RUNGE_KUTTA_HPP
#define RUNGE_KUTTA_HPP

#include "../Obj/VectorObj.hpp"
#include "../Obj/DenseObj.hpp"
#include "../Obj/SparseObj.hpp"
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <iostream>

template <typename TNum, 
          typename VectorType = VectorObj<TNum>, 
          typename MatrixType = DenseObj<TNum>>
class RungeKutta {
public:
    // Type aliases for clarity
    using StateVector = VectorType;
    using SystemMatrix = MatrixType;
    using SystemFunction = std::function<StateVector(const StateVector&)>;
    
    RungeKutta() = default;
    virtual ~RungeKutta() = default;
    
    // Prevent copying to avoid unintended performance impacts
    RungeKutta(const RungeKutta&) = delete;
    RungeKutta& operator=(const RungeKutta&) = delete;
    
    // Allow moving
    RungeKutta(RungeKutta&&) noexcept = default;
    RungeKutta& operator=(RungeKutta&&) noexcept = default;

    /**
     * @brief Solves the system using RK4 method
     * @param y Initial state vector, will contain the solution
     * @param f System function that computes the derivative
     * @param h Step size
     * @param n Number of steps
     * @param callback Optional callback for monitoring progress
     * @throws std::invalid_argument if parameters are invalid
     */
    void solve(StateVector& y,
              const SystemFunction& f,
              TNum h,
              size_t n,
              std::function<void(size_t, const StateVector&)> callback = nullptr) {
        // Parameter validation
        validateParameters(y, h, n);
        
        // Constants for the RK4 method
        constexpr TNum one_sixth = TNum(1) / TNum(6);
        constexpr TNum one_half = TNum(1) / TNum(2);
        constexpr TNum two = TNum(2);
        
        // Pre-allocate vectors to avoid repeated allocation
        StateVector k1(y.size());
        StateVector k2(y.size());
        StateVector k3(y.size());
        StateVector k4(y.size());
        StateVector temp(y.size());
        
        for(size_t i = 0; i < n; ++i) {
            // First stage
            k1 = f(y);
            
            // Second stage
            temp = y + k1 * (h * one_half);
            k2 = f(temp);
            
            // Third stage
            temp = y + k2 * (h * one_half);
            k3 = f(temp);
            
            // Fourth stage
            temp = y + k3 * h;
            k4 = f(temp);
            
            // Update solution
            y = y + (k1 + k2 * two + k3 * two + k4) * (h * one_sixth);
            
            // Call callback if provided
            if (callback) {
                callback(i, y);
            }
        }
    }
    
    /**
     * @brief Solves the system with adaptive step size control using embedded RK4(5) method
     * @param y Initial state vector, will contain the solution
     * @param f System function that computes the derivative
     * @param h Initial step size
     * @param tol Error tolerance
     * @param max_steps Maximum number of steps
     * @param safety_factor Safety factor for step size adjustment (default: 0.9)
     * @param min_scale Minimum scale factor for step size (default: 0.1)
     * @param max_scale Maximum scale factor for step size (default: 2.0)
     * @return Number of steps actually taken
     * @throws std::invalid_argument if parameters are invalid
     * @throws std::runtime_error if max_steps is reached before convergence
     */
    size_t solveAdaptive(StateVector& y,
                        const SystemFunction& f,
                        TNum h,
                        TNum tol,
                        size_t max_steps,
                        TNum safety_factor = 0.9,
                        TNum min_scale = 0.1,
                        TNum max_scale = 2.0) {
        validateParameters(y, h, max_steps);
        if (tol <= 0) {
            throw std::invalid_argument("Tolerance must be positive");
        }
        
        size_t steps_taken = 0;
        TNum current_h = h;
        StateVector y_temp = y;
        
        while (steps_taken < max_steps) {
            // Take two half steps
            solve(y_temp, f, current_h/2, 2);
            
            // Take one full step
            StateVector y_full = y;
            solve(y_full, f, current_h, 1);
            
            // Estimate error
            TNum error = (y_temp - y_full).L2norm();
            TNum scale = y_temp.L2norm();
            if (scale > 0) {
                error /= scale;
            }
            
            // Check if step is acceptable
            if (error < tol) {
                // Accept the step
                y = y_temp;
                steps_taken++;
                
                // Adjust step size up
                TNum factor = safety_factor * std::pow(tol/error, 0.2);
                factor = std::min(max_scale, std::max(min_scale, factor));
                current_h *= factor;
            } else {
                // Reject the step and reduce step size
                TNum factor = safety_factor * std::pow(tol/error, 0.25);
                factor = std::min(max_scale, std::max(min_scale, factor));
                current_h *= factor;
                y_temp = y;
            }
        }
        
        return steps_taken;
    }

private:
    /**
     * @brief Validates input parameters
     * @throws std::invalid_argument if parameters are invalid
     */
    void validateParameters(const StateVector& y, TNum h, size_t n) const {
        if (y.size() == 0) {
            throw std::invalid_argument("State vector must not be empty");
        }
        if (h <= 0) {
            throw std::invalid_argument("Step size must be positive");
        }
        if (n == 0) {
            throw std::invalid_argument("Number of steps must be positive");
        }
    }
};

#endif // RUNGE_KUTTA_HPP