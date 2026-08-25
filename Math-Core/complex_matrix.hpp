#ifndef COMPLEX_MATRIX_HPP
#define COMPLEX_MATRIX_HPP

#include <complex>
#include <vector>
#include <cmath>
#include <random>
#include <stdexcept>

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
        Complex result = amplitude + other.amplitude * std::polar(1.0, other.phase - phase);
        double newPhase = std::arg(result);
        return WaveVector(result, newPhase);
    }
};

class ComplexMatrix {
public:
    std::vector<std::vector<WaveVector>> data;
    size_t rows, cols;

    ComplexMatrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(r, std::vector<WaveVector>(c));
    }

    void randomize(double scale = 1.0) {
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
        if (i >= rows || j >= cols) {
            throw std::out_of_range("ComplexMatrix::at index out of range");
        }
        return data[i][j];
    }

    const WaveVector& at(size_t i, size_t j) const {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("ComplexMatrix::at index out of range");
        }
        return data[i][j];
    }

    ComplexMatrix multiply(const ComplexMatrix& other) const {
        ComplexMatrix result(rows, other.cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < other.cols; ++j) {
                WaveVector sum;
                for (size_t k = 0; k < cols; ++k) {
                    sum = sum.interfere(data[i][k].interfere(other.data[k][j]));
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }
};

} // namespace MathCore

#endif
