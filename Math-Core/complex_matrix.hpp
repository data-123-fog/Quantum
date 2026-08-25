#ifndef WAVE_OPTIMIZER_HPP
#define WAVE_OPTIMIZER_HPP

#include <vector>
#include <cmath>
#include <algorithm>
#include "wave_engine.hpp"
#include "complex_matrix.hpp"

namespace MathCore {

class WaveOptimizer {
public:
    // Безопасный Softmax с защитой от переполнения
    static std::vector<double> softmax(const std::vector<MathCore::WaveEngine::TokenWave>& states) {
        if (states.empty()) return {};

        std::vector<double> expValues;
        expValues.reserve(states.size());

        double maxVal = states[0].state.magnitude();
        for (const auto& token : states) {
            double mag = token.state.magnitude();
            if (mag > maxVal) maxVal = mag;
        }

        double sumExp = 0.0;
        for (const auto& token : states) {
            double val = std::exp(token.state.magnitude() - maxVal);
            expValues.push_back(val);
            sumExp += val;
        }

        for (auto& val : expValues) {
            val /= (sumExp > 0.0 ? sumExp : 1.0);
        }

        return expValues;
    }

    // Обновление матрицы взаимодействия с учетом структуры WaveVector и std::complex
    void updateMatrix(MathCore::ComplexMatrix& matrix, 
                      const std::vector<MathCore::WaveEngine::TokenWave>& states, 
                      int predicted, 
                      int target) {
        
        double learningRate = 0.01;
        
        // Поля rows и cols публичные в ComplexMatrix
        size_t r = matrix.rows;
        size_t c = matrix.cols;

        for (size_t i = 0; i < r; ++i) {
            for (size_t j = 0; j < c; ++j) {
                auto& cell = matrix.at(i, j);
                double currentReal = cell.amplitude.real();
                double currentImag = cell.amplitude.imag();

                if (predicted != target) {
                    // Корректируем комплексную амплитуду при ошибке
                    cell.amplitude = Complex(currentReal - learningRate * 0.1, currentImag - learningRate * 0.1);
                } else {
                    // Усиливаем резонанс при совпадении
                    cell.amplitude = Complex(currentReal + learningRate * 0.05, currentImag);
                }
            }
        }
    }

    // Предсказание следующего токена
    int predictNextGreedy(const MathCore::WaveEngine& engine) {
        if (engine.tokens.empty()) return -1;
        
        auto probs = softmax(engine.tokens);
        auto maxIt = std::max_element(probs.begin(), probs.end());
        return static_cast<int>(std::distance(probs.begin(), maxIt));
    }
};

} // namespace MathCore

#endif // WAVE_OPTIMIZER_HPP
