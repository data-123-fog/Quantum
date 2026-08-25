#ifndef WAVE_OPTIMIZER_HPP
#define WAVE_OPTIMIZER_HPP

#include "complex_matrix.hpp"
#include "wave_engine.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace MathCore {

class WaveOptimizer {
public:
    double learningRate;
    
    explicit WaveOptimizer(double lr = 0.02) : learningRate(lr) {}
    
    static double cosineSim(const WaveVector& a, const WaveVector& b) {
        double magA = a.magnitude();
        double magB = b.magnitude();
        if (magA < 1e-10 || magB < 1e-10) return 0.0;
        
        double dot = a.amplitude.real() * b.amplitude.real() +
                     a.amplitude.imag() * b.amplitude.imag();
        return dot / (magA * magB);
    }
    
    static std::vector<double> softmax(const std::vector<WaveVector>& states) {
        std::vector<double> probs(states.size());
        double maxVal = -1e10;
        
        for (const auto& s : states) {
            maxVal = std::max(maxVal, s.magnitude());
        }
        
        double sum = 0.0;
        for (size_t i = 0; i < states.size(); ++i) {
            probs[i] = std::exp(states[i].magnitude() - maxVal);
            sum += probs[i];
        }
        
        if (sum < 1e-10) sum = 1e-10;
        for (auto& p : probs) p /= sum;
        return probs;
    }
    
    double computeLoss(int predictedId, int targetId, const std::vector<double>& probs) {
        if (targetId < 0 || targetId >= static_cast<int>(probs.size())) return 0.0;
        double p = probs[targetId];
        if (p < 1e-10) p = 1e-10;
        return -std::log(p);
    }
    
    void updateMatrix(ComplexMatrix& matrix,
                      const std::vector<TokenWave>& states,
                      int predictedId,
                      int targetId) {
        
        if (predictedId == targetId) return;
        
        for (size_t i = 0; i < states.size(); ++i) {
            double grad = (i == static_cast<size_t>(targetId)) ? learningRate : -learningRate * 0.1;
            
            for (size_t j = 0; j < states.size(); ++j) {
                if (i == j) continue;
                
                Complex& cell = matrix.at(i, j).amplitude;
                cell += Complex(grad * 0.1, 0.0);
                
                double maxVal = 2.0;
                if (std::abs(cell.real()) > maxVal) cell.real(maxVal * (cell.real() > 0 ? 1 : -1));
                if (std::abs(cell.imag()) > maxVal) cell.imag(maxVal * (cell.imag() > 0 ? 1 : -1));
            }
        }
    }
    
    int predictNext(const WaveEngine& engine) {
        auto probs = softmax(engine.tokens);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> dist(probs.begin(), probs.end());
        
        return dist(gen);
    }
    
    int predictNextGreedy(const WaveEngine& engine) {
        auto probs = softmax(engine.tokens);
        int bestIdx = 0;
        double bestProb = probs[0];
        
        for (size_t i = 1; i < probs.size(); ++i) {
            if (probs[i] > bestProb) {
                bestProb = probs[i];
                bestIdx = static_cast<int>(i);
            }
        }
        return bestIdx;
    }
};

} // namespace MathCore

#endif
