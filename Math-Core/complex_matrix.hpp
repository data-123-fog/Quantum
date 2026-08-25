#ifndef COMPLEX_MATRIX_HPP
#define COMPLEX_MATRIX_HPP

#include <complex>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

namespace MathCore {

constexpr double PI = 3.14159265358979323846;

using Complex = std::complex<double>;

struct WaveVector {
    Complex amplitude;
    double phase;
    
    WaveVector() : amplitude(0.0, 0.0), phase(0.0) {}
    WaveVector(Complex a, double p) : amplitude(a), phase(p) {}
    
    double magnitude() const {
        return std::abs(amplitude);
    }
    
    WaveVector interfere(const WaveVector& other) const {
        Complex shifted = other.amplitude * std::polar(1.0, other.phase - phase);
        Complex result = amplitude + shifted;
        return WaveVector(result, std::arg(result));
    }
    
    void normalize() {
        double mag = magnitude();
        if (mag > 1e-10) {
            amplitude /= mag;
        }
    }
};

class ComplexMatrix {
public:
    std::vector<std::vector<WaveVector>> data;
    size_t rows, cols;
    
    ComplexMatrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(r, std::vector<WaveVector>(c));
    }
    
    void randomize(double scale = 0.1) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> dist(0.0, scale);
        
        for (auto& row : data) {
            for (auto& cell : row) {
                cell.amplitude = Complex(dist(gen), dist(gen));
                cell.phase = dist(gen) * 2.0 * PI;
            }
        }
    }
    
    WaveVector& at(size_t i, size_t j) {
        return data[i][j];
    }
    
    const WaveVector& at(size_t i, size_t j) const {
        return data[i][j];
    }
};

} // namespace MathCore

#endif
