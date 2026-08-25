#include "../Math-Core/complex_matrix.hpp"
#include "../Math-Core/wave_engine.hpp"
#include "../Math-Core/activation.hpp"
#include "../Math-Core/wave_optimizer.hpp"
#include "../Tokenizer/cpp_tokenizer.hpp"
#include "../System/file_loader.hpp"
#include "../System/logger.hpp"

#include <iostream>
#include <csignal>
#include <vector>
#include <map>
#include <random>
#include <atomic>
#include <sstream>
#include <algorithm>

using namespace MathCore;
using namespace Tokenizer;
using namespace System;

std::atomic<bool> g_running{true};

extern "C" void signalHandler(int) {
    g_running.store(false, std::memory_order_relaxed);
}

std::string vocabToString(const std::map<std::string, int>& vocab) {
    std::stringstream ss;
    for (const auto& [word, id] : vocab) {
        ss << id << "=" << word << "\n";
    }
    return ss.str();
}

std::map<std::string, int> stringToVocab(const std::string& data) {
    std::map<std::string, int> vocab;
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            int id = std::stoi(line.substr(0, pos));
            std::string word = line.substr(pos + 1);
            vocab[word] = id;
        }
    }
    return vocab;
}

int runInference(Logger& logger) {
    std::cout << "=== INFERENCE MODE ===" << std::endl;
    
    auto weights = logger.loadWeights();
    if (weights.empty()) {
        std::cerr << "[ERROR] No trained weights found! Train first with ./quantum" << std::endl;
        return 1;
    }
    
    std::string vocabData = logger.loadStateGraph("vocab.dat");
    if (vocabData.empty()) {
        std::cerr << "[ERROR] No vocabulary found! Train first." << std::endl;
        return 1;
    }
    
    auto vocab = stringToVocab(vocabData);
    if (vocab.empty()) {
        std::cerr << "[ERROR] Empty vocabulary!" << std::endl;
        return 1;
    }
    
    std::map<int, std::string> idToToken;
    for (const auto& [word, id] : vocab) {
        idToToken[id] = word;
    }
    
    int vocabSize = static_cast<int>(vocab.size());
    std::cout << "Loaded vocabulary: " << vocabSize << " tokens" << std::endl;
    
    WaveEngine engine(static_cast<size_t>(vocabSize));
    WaveOptimizer optimizer;
    
    for (size_t i = 0; i < engine.tokens.size() && i * 2 < weights.size(); ++i) {
        WaveVector vec;
        vec.amplitude = Complex(weights[i * 2], 0.0);
        vec.phase = weights[i * 2 + 1];
        engine.setToken(i, static_cast<int>(i), vec);
    }
    
    std::cout << "\nGenerating code (50 tokens):" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    int currentId = 0;
    const int maxTokens = 50;
    
    for (int i = 0; i < maxTokens; ++i) {
        engine.negotiate(2);
        int nextId = optimizer.predictNextGreedy(engine);
        
        auto it = idToToken.find(nextId);
        if (it != idToToken.end()) {
            std::cout << it->second << " ";
        }
        
        if (nextId >= 0 && nextId < vocabSize) {
            WaveVector vec;
            vec.amplitude = Complex(1.0, 0.0);
            vec.phase = engine.tokens[nextId].state.phase;
            engine.setToken(static_cast<size_t>(nextId), nextId, vec);
        }
        currentId = nextId;
    }
    
    std::cout << "\n" << std::string(50, '-') << std::endl;
    return 0;
}

int runTraining() {
    std::cout << "=== TRAINING MODE ===" << std::endl;
    
    Logger logger;
    logger.logTraining("System initialized");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> ampDist(-0.5, 0.5);
    std::uniform_real_distribution<double> phaseDist(0.0, 2.0 * PI);
    
    FileLoader loader;
    auto files = loader.loadFromDirectory("Material");
    
    if (files.empty()) {
        std::cerr << "[ERROR] No files in Material/ directory!" << std::endl;
        logger.logError("Material/ is empty or missing");
        return 1;
    }
    
    std::cout << "Loaded " << files.size() << " source files" << std::endl;
    logger.logTraining("Loaded " + std::to_string(files.size()) + " files");
    
    CppTokenizer tokenizer;
    std::vector<Token> allTokens;
    
    for (const auto& file : files) {
        auto tokens = tokenizer.tokenize(file.content);
        allTokens.insert(allTokens.end(), tokens.begin(), tokens.end());
    }
    
    if (allTokens.empty()) {
        std::cerr << "[ERROR] No tokens extracted!" << std::endl;
        logger.logError("Tokenization produced no tokens");
        return 1;
    }
    
    std::cout << "Total tokens: " << allTokens.size() << std::endl;
    
    std::map<std::string, int> vocabulary;
    std::vector<WaveVector> tokenEmbeddings;
    
    for (const auto& token : allTokens) {
        if (vocabulary.find(token.value) == vocabulary.end()) {
            int id = static_cast<int>(vocabulary.size());
            vocabulary[token.value] = id;
            
            WaveVector vec;
            vec.amplitude = Complex(ampDist(gen), ampDist(gen));
            vec.phase = phaseDist(gen);
            tokenEmbeddings.push_back(vec);
        }
    }
    
    int vocabSize = static_cast<int>(vocabulary.size());
    std::cout << "Vocabulary size: " << vocabSize << std::endl;
    logger.logTraining("Vocabulary: " + std::to_string(vocabSize) + " tokens");
    
    if (vocabSize == 0) {
        std::cerr << "[ERROR] Empty vocabulary!" << std::endl;
        logger.logError("Vocabulary is empty");
        return 1;
    }
    
    logger.saveStateGraph(vocabToString(vocabulary), "vocab.dat");
    
    WaveEngine engine(static_cast<size_t>(vocabSize));
    WaveOptimizer optimizer(0.03);
    
    for (size_t i = 0; i < tokenEmbeddings.size() && i < engine.tokens.size(); ++i) {
        engine.setToken(i, static_cast<int>(i), tokenEmbeddings[i]);
    }
    
    const int epochs = 200;
    const int seqLength = 8;
    const int negSampleSize = 5;
    
    std::cout << "\nTraining started..." << std::endl;
    std::cout << "Epochs: " << epochs << ", SeqLength: " << seqLength << std::endl;
    
    double bestLoss = 1e10;
    
    for (int epoch = 0; epoch < epochs && g_running.load(); ++epoch) {
        double totalLoss = 0.0;
        int sampleCount = 0;
        int correctPredictions = 0;
        
        std::vector<size_t> startPositions;
        for (size_t i = 0; i + seqLength + 1 < allTokens.size(); i += seqLength) {
            startPositions.push_back(i);
        }
        std::shuffle(startPositions.begin(), startPositions.end(), gen);
        
        for (size_t pos : startPositions) {
            if (pos + seqLength >= allTokens.size()) continue;
            
            for (int j = 0; j < seqLength && pos + j < allTokens.size(); ++j) {
                std::string tokVal = allTokens[pos + j].value;
                auto it = vocabulary.find(tokVal);
                if (it != vocabulary.end()) {
                    int tokId = it->second;
                    engine.tokens[j].tokenId = tokId;
                    engine.tokens[j].state = tokenEmbeddings[tokId];
                }
            }
            
            engine.negotiate(3);
            
            int predictedId = optimizer.predictNextGreedy(engine);
            
            std::string targetVal = allTokens[pos + seqLength].value;
            auto targetIt = vocabulary.find(targetVal);
            if (targetIt == vocabulary.end()) continue;
            int targetId = targetIt->second;
            
            if (predictedId == targetId) {
                correctPredictions++;
            }
            
            auto probs = WaveOptimizer::softmax(engine.tokens);
            double loss = optimizer.computeLoss(predictedId, targetId, probs);
            totalLoss += loss;
            sampleCount++;
            
            optimizer.updateMatrix(engine.interactionMatrix, engine.tokens, predictedId, targetId);
            
            std::uniform_int_distribution<int> negDist(0, vocabSize - 1);
            for (int n = 0; n < negSampleSize; ++n) {
                int negId = negDist(gen);
                if (negId != targetId) {
                    for (size_t i = 0; i < engine.tokens.size(); ++i) {
                        if (i == static_cast<size_t>(negId)) {
                            engine.interactionMatrix.at(i, i).amplitude -= Complex(optimizer.learningRate * 0.05, 0.0);
                        }
                    }
                }
            }
        }
        
        double avgLoss = (sampleCount > 0) ? (totalLoss / sampleCount) : 0.0;
        double accuracy = (sampleCount > 0) ? (100.0 * correctPredictions / sampleCount) : 0.0;
        
        if (epoch % 20 == 0 || epoch == epochs - 1) {
            std::cout << "Epoch " << epoch 
                      << " | Loss: " << std::fixed << std::setprecision(4) << avgLoss
                      << " | Acc: " << std::setprecision(2) << accuracy << "%" 
                      << " | Samples: " << sampleCount << std::endl;
            logger.logTraining("Epoch " + std::to_string(epoch) + " Acc=" + std::to_string(accuracy) + "%", avgLoss);
        }
        
        if (avgLoss < bestLoss) {
            bestLoss = avgLoss;
        }
    }
    
    std::cout << "\nSaving model..." << std::endl;
    
    std::vector<double> weights;
    for (const auto& token : engine.tokens) {
        weights.push_back(token.state.magnitude());
        weights.push_back(token.state.phase);
    }
    logger.saveWeights(weights);
    
    logger.logTraining("Training complete. Best loss: " + std::to_string(bestLoss));
    std::cout << "Done! Best loss: " << bestLoss << std::endl;
    std::cout << "Run with --generate to produce code." << std::endl;
    
    return 0;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    Logger logger;
    
    if (argc > 1 && std::string(argv[1]) == "--generate") {
        return runInference(logger);
    }
    
    return runTraining();
}
