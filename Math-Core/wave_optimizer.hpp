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
    // Безопасный Softmax с защитой от переполнения (Numerical Stability)
    static std::vector<double> softmax(const std::vector<MathCore::WaveEngine::TokenWave>& states) {
        if (states.empty()) return {};

        std::vector<double> expValues;
        expValues.reserve(states.size());

        // Находим максимум для стабилизации экспоненты
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

    // Обновление матрицы взаимодействия с защитой границ
    void updateMatrix(MathCore::ComplexMatrix& matrix, 
                      const std::vector<MathCore::WaveEngine::TokenWave>& states, 
                      int predicted, 
                      int target) {
        
        float learningRate = 0.01f;
        
        // Ограничиваем циклы реальным размером матрицы, чтобы не вылететь за границы памяти
        size_t rows = matrix.rows();
        size_t cols = matrix.cols();

        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (predicted != target) {
                    matrix.at(i, j).real -= learningRate * 0.1f;
                    matrix.at(i, j).imag -= learningRate * 0.1f;
                } else {
                    matrix.at(i, j).real += learningRate * 0.05f;
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
