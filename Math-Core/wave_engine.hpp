#ifndef WAVE_ENGINE_HPP
#define WAVE_ENGINE_HPP

#include "complex_matrix.hpp"
#include <thread>
#include <vector>
#include <functional>
#include <future>
#include <atomic>

namespace MathCore {

class WaveEngine {
public:
    struct TokenWave {
        int tokenId;
        WaveVector state;
        std::vector<WaveVector> connections;
    };
    
    std::vector<TokenWave> tokens;
    ComplexMatrix interactionMatrix;
    
    WaveEngine(size_t tokenCount) : interactionMatrix(tokenCount, tokenCount) {
        tokens.resize(tokenCount);
        interactionMatrix.randomize(0.1);
    }
    
    void negotiate(size_t iterations = 10) {
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 1;  // ФИКС: защита от 0
        
        for (size_t iter = 0; iter < iterations; ++iter) {
            std::vector<WaveVector> newStates(tokens.size());
            
            // Пул потоков (переиспользуем, не создаём каждый раз)
            std::vector<std::future<void>> futures;
            size_t batchSize = tokens.size() / numThreads;
            if (batchSize == 0) batchSize = 1;
            
            for (size_t t = 0; t < tokens.size(); t += batchSize) {
                futures.push_back(std::async(std::launch::async, [this, &newStates, t, batchSize]() {
                    size_t end = std::min(t + batchSize, tokens.size());
                    for (size_t i = t; i < end; ++i) {
                        WaveVector resonance = tokens[i].state;
                        for (size_t j = 0; j < tokens.size(); ++j) {
                            if (i != j) {
                                WaveVector interaction = tokens[j].state;
                                interaction.amplitude *= interactionMatrix.at(i, j).amplitude;
                                resonance = resonance.interfere(interaction);
                            }
                        }
                        newStates[i] = resonance;
                    }
                }));
            }
            
            for (auto& f : futures) f.get();
            
            for (size_t i = 0; i < tokens.size(); ++i) {
                tokens[i].state = newStates[i];
            }
        }
    }
    
    void setToken(size_t idx, int id, const WaveVector& state) {
        if (idx < tokens.size()) {
            tokens[idx].tokenId = id;
            tokens[idx].state = state;
        }
    }
};

} // namespace MathCore

#endif
