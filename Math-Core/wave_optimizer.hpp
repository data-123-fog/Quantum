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
    // Вычисление вероятностей через Softmax на основе амплитуд токенов
    static std::vector<double> softmax(const std::vector<MathCore::WaveEngine::TokenWave>& states) {
        std::vector<double> expValues;
        double sumExp = 0.0;

        for (const auto& token : states) {
            // Берем модуль (амплитуду) комплексного состояния токена
            double val = std::exp(token.state.magnitude());
            expValues.push_back(val);
            sumExp += val;
        }

        for (auto& val : expValues) {
            val /= (sumExp > 0.0 ? sumExp : 1.0);
        }

        return expValues;
    }

    // Обновление матрицы взаимодействия (обучение)
    void updateMatrix(MathCore::ComplexMatrix& matrix, 
                      const std::vector<MathCore::WaveEngine::TokenWave>& states, 
                      int predicted, 
                      int target) {
        
        float learningRate = 0.01f;
        size_t numStates = states.size();

        for (size_t i = 0; i < numStates; ++i) {
            for (size_t j = 0; j < numStates; ++j) {
                if (predicted != target) {
                    // Корректируем фазу и амплитуду при ошибке
                    matrix.at(i, j).real -= learningRate * 0.1f;
                    matrix.at(i, j).imag -= learningRate * 0.1f;
                } else {
                    // Усиливаем резонанс при совпадении
                    matrix.at(i, j).real += learningRate * 0.05f;
                }
            }
        }
    }

    // Предсказание следующего токена (Жадный выбор)
    int predictNextGreedy(const MathCore::WaveEngine& engine) {
        auto probs = softmax(engine.tokens);
        auto maxIt = std::max_element(probs.begin(), probs.end());
        return std::distance(probs.begin(), maxIt);
    }
};

} // namespace MathCore

#endif // WAVE_OPTIMIZER_HPP
