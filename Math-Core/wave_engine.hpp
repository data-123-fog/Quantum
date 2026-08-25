#ifndef WAVE_ENGINE_HPP
#define WAVE_ENGINE_HPP

#include "complex_matrix.hpp"
#include <vector>
#include <future>
#include <algorithm>

namespace MathCore {

class WaveEngine {
public:
    struct TokenWave {
        int tokenId;
        WaveVector state;
    };
    
    std::vector<TokenWave> tokens;
    ComplexMatrix interactionMatrix;
    
    explicit WaveEngine(size_t tokenCount) : interactionMatrix(tokenCount, tokenCount) {
        tokens.resize(tokenCount);
        interactionMatrix.randomize(0.05);
    }
    
    void negotiate(size_t iterations = 5) {
        if (tokens.empty()) return;
        
        for (size_t iter = 0; iter < iterations; ++iter) {
            std::vector<WaveVector> newStates(tokens.size());
            
            size_t numThreads = std::max(1u, std::thread::hardware_concurrency());
            size_t batchSize = std::max(size_t(1), tokens.size() / numThreads);
            std::vector<std::future<void>> futures;
            
            for (size_t t = 0; t < tokens.size(); t += batchSize) {
                size_t start = t;
                size_t end = std::min(t + batchSize, tokens.size());
                
                futures.push_back(std::async(std::launch::async, [this, &newStates, start, end]() {
                    for (size_t i = start; i < end; ++i) {
                        WaveVector resonance = tokens[i].state;
                        for (size_t j = 0; j < tokens.size(); ++j) {
                            if (i == j) continue;
                            WaveVector interaction = tokens[j].state;
                            interaction.amplitude *= interactionMatrix.at(i, j).amplitude;
                            resonance = resonance.interfere(interaction);
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
