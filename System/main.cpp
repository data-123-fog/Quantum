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
#include <random>
#include <atomic>

using namespace MathCore;
using namespace Tokenizer;
using namespace System;

// ФИКС: атомарный флаг
std::atomic<bool> running{true};
Logger* globalLogger = nullptr;

// ФИКС: async-signal-safe handler (только write + _exit)
extern "C" void signalHandler(int sig) {
    running.store(false, std::memory_order_relaxed);
}

int main() {
    std::cout << "=== Math-Core Wave Intelligence ===" << std::endl;
    
    // ФИКС: инициализация случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    
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
    
    // ФИКС: проверка на пустую Material
    if (files.empty()) {
        std::cerr << "ERROR: No files in Material/ directory!" << std::endl;
        logger.logError("Material/ is empty or missing");
        return 1;
    }
    
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
    
    // ФИКС: проверка на пустые токены
    if (allTokens.empty()) {
        std::cerr << "ERROR: No tokens extracted!" << std::endl;
        logger.logError("Tokenization produced no tokens");
        return 1;
    }
    
    // Build vocabulary and map to wave vectors
    std::map<std::string, int> vocabulary;
    std::vector<WaveVector> tokenEmbeddings;
    int vocabSize = 0;
    
    std::uniform_real_distribution<double> ampDist(-1.0, 1.0);
    std::uniform_real_distribution<double> phaseDist(0.0, 2.0 * 3.14159265358979323846); // ФИКС: вместо M_PI
    
    for (const auto& token : allTokens) {
        std::string key = token.value;
        if (vocabulary.find(key) == vocabulary.end()) {
            vocabulary[key] = vocabSize++;
            WaveVector vec;
            vec.amplitude = Complex(ampDist(gen), ampDist(gen));
            vec.phase = phaseDist(gen);
            tokenEmbeddings.push_back(vec);
        }
    }
    
    std::cout << "Vocabulary size: " << vocabSize << std::endl;
    logger.logTraining("Vocabulary built: " + std::to_string(vocabSize) + " unique tokens");
    
    // ФИКС: проверка vocabSize
    if (vocabSize == 0) {
        std::cerr << "ERROR: Empty vocabulary!" << std::endl;
        logger.logError("Vocabulary is empty");
        return 1;
    }
    
    // Initialize wave engine
    WaveEngine engine(vocabSize);
    for (size_t i = 0; i < tokenEmbeddings.size() && i < engine.tokens.size(); ++i) {
        engine.setToken(i, i, tokenEmbeddings[i]);
    }
    
    // Training loop
    std::cout << "\nStarting training..." << std::endl;
    int epoch = 0;
    double prevLoss = 999999.0;
    
    while (running.load()) {
        engine.negotiate(5);
        
        // ФИКС: защита от деления на ноль
        double loss = 0.0;
        for (const auto& token : engine.tokens) {
            loss += token.state.magnitude();
        }
        loss /= static_cast<double>(engine.tokens.size());
        
        if (epoch % 10 == 0) {
            std::cout << "Epoch " << epoch << " | Loss: " << loss << std::endl;
            logger.logTraining("Epoch " + std::to_string(epoch), loss);
        }
        
        if (std::abs(prevLoss - loss) < 0.0001 && epoch > 100) {
            std::cout << "Converged!" << std::endl;
            break;
        }
        prevLoss = loss;
        epoch++;
    }
    
    // Save results
    std::cout << "\nSaving results..." << std::endl;
    
    std::vector<double> weights;
    for (const auto& token : engine.tokens) {
        weights.push_back(token.state.magnitude());
        weights.push_back(token.state.phase);
    }
    logger.saveWeights(weights);
    
    std::string graphData;
    for (const auto& [word, id] : vocabulary) {
        graphData += word + ":" + std::to_string(id) + "\n";
    }
    logger.saveStateGraph(graphData);
    
    logger.logTraining("Training completed. Epochs: " + std::to_string(epoch));
    std::cout << "Done! Results saved to Logic-Intelligence/" << std::endl;
    
    return 0;
}
