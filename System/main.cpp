#include "../Math-Core/complex_matrix.hpp"
#include "../Math-Core/wave_engine.hpp"
#include "../Math-Core/activation.hpp"
#include "../Tokenizer/cpp_tokenizer.hpp"
#include "../System/file_loader.hpp"
#include "../System/logger.hpp"

#include <iostream>
#include <csignal>
#include <vector>
#include <map>

using namespace MathCore;
using namespace Tokenizer;
using namespace System;

bool running = true;
Logger* globalLogger = nullptr;

void signalHandler(int sig) {
    std::cout << "\nShutting down gracefully..." << std::endl;
    if (globalLogger) {
        globalLogger->logTraining("Session ended by user");
    }
    running = false;
}

int main() {
    std::cout << "=== Math-Core Wave Intelligence ===" << std::endl;
    std::cout << "Initializing..." << std::endl;
    
    // Setup signal handler
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Initialize logger
    Logger logger;
    globalLogger = &logger;
    logger.logTraining("System initialized");
    
    // Load source files
    FileLoader loader;
    auto files = loader.loadFromDirectory("Material");
    
    std::cout << "Loaded " << files.size() << " source files" << std::endl;
    logger.logTraining("Loaded " + std::to_string(files.size()) + " files from Material/");
    
    // Tokenize all files
    CppTokenizer tokenizer;
    std::vector<Token> allTokens;
    
    for (const auto& file : files) {
        auto tokens = tokenizer.tokenize(file.content);
        allTokens.insert(allTokens.end(), tokens.begin(), tokens.end());
        std::cout << "Tokenized: " << file.path << " (" << tokens.size() << " tokens)" << std::endl;
    }
    
    std::cout << "Total tokens: " << allTokens.size() << std::endl;
    logger.logTraining("Total tokens extracted: " + std::to_string(allTokens.size()));
    
    // Build vocabulary and map to wave vectors
    std::map<std::string, int> vocabulary;
    std::vector<WaveVector> tokenEmbeddings;
    int vocabSize = 0;
    
    for (const auto& token : allTokens) {
        std::string key = token.value;
        if (vocabulary.find(key) == vocabulary.end()) {
            vocabulary[key] = vocabSize++;
            // Create random wave vector for new token
            WaveVector vec;
            vec.amplitude = Complex((rand() % 100) / 100.0, (rand() % 100) / 100.0);
            vec.phase = (rand() % 360) * M_PI / 180.0;
            tokenEmbeddings.push_back(vec);
        }
    }
    
    std::cout << "Vocabulary size: " << vocabSize << std::endl;
    logger.logTraining("Vocabulary built: " + std::to_string(vocabSize) + " unique tokens");
    
    // Initialize wave engine
    WaveEngine engine(vocabSize);
    for (size_t i = 0; i < tokenEmbeddings.size() && i < engine.tokens.size(); ++i) {
        engine.setToken(i, i, tokenEmbeddings[i]);
    }
    
    // Training loop
    std::cout << "\nStarting training..." << std::endl;
    int epoch = 0;
    double prevLoss = 999999.0;
    
    while (running) {
        // Run wave negotiations
        engine.negotiate(5);
        
        // Calculate pseudo-loss (resonance variance)
        double loss = 0.0;
        for (const auto& token : engine.tokens) {
            loss += token.state.magnitude();
        }
        loss /= engine.tokens.size();
        
        // Log progress
        if (epoch % 10 == 0) {
            std::cout << "Epoch " << epoch << " | Loss: " << loss << std::endl;
            logger.logTraining("Epoch " + std::to_string(epoch), loss);
        }
        
        // Simple convergence check
        if (std::abs(prevLoss - loss) < 0.0001 && epoch > 100) {
            std::cout << "Converged!" << std::endl;
            break;
        }
        prevLoss = loss;
        
        epoch++;
        
        // Small delay to prevent CPU burning
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Save results
    std::cout << "\nSaving results..." << std::endl;
    
    std::vector<double> weights;
    for (const auto& token : engine.tokens) {
        weights.push_back(token.state.magnitude());
        weights.push_back(token.state.phase);
    }
    logger.saveWeights(weights);
    
    // Save state graph (simple serialization)
    std::string graphData;
    for (const auto& [word, id] : vocabulary) {
        graphData += word + ":" + std::to_string(id) + "\n";
    }
    logger.saveStateGraph(graphData);
    
    logger.logTraining("Training completed. Epochs: " + std::to_string(epoch));
    std::cout << "Done! Results saved to Logic-Intelligence/" << std::endl;
    
    return 0;
}
