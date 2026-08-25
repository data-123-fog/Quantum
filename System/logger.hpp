#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace System {

class Logger {
public:
    std::string logicPath;
    std::string logsPath;
    
    Logger(const std::string& basePath = "Logic-Intelligence") {
        logicPath = basePath;
        logsPath = basePath + "/Logs";
        std::filesystem::create_directories(logsPath);
    }
    
    void logTraining(const std::string& message, double loss = -1.0) {
        std::string logFile = logsPath + "/train_logs.txt";
        std::ofstream file(logFile, std::ios::app);
        if (!file.is_open()) {
            std::cerr << "ERROR: Cannot open train_logs.txt" << std::endl;
            return;
        }
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        if (loss >= 0) {
            file << "Loss: " << std::fixed << std::setprecision(6) << loss << " | ";
        }
        file << message << std::endl;
    }
    
    void logError(const std::string& error) {
        std::string logFile = logsPath + "/error_logs.txt";
        std::ofstream file(logFile, std::ios::app);
        if (!file.is_open()) {
            std::cerr << "ERROR: Cannot open error_logs.txt" << std::endl;
            return;
        }
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ERROR: ";
        file << error << std::endl;
    }
    
    void saveWeights(const std::vector<double>& weights, const std::string& filename = "weights.bin") {
        std::string path = logicPath + "/" + filename;
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "ERROR: Cannot save weights to " << path << std::endl;
            return;
        }
        
        // Портируемый формат: фиксированный uint64_t + double[]
        uint64_t size = static_cast<uint64_t>(weights.size());
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(reinterpret_cast<const char*>(weights.data()), size * sizeof(double));
        if (!file) {
            std::cerr << "ERROR: Failed to write weights" << std::endl;
        }
    }
    
    std::vector<double> loadWeights(const std::string& filename = "weights.bin") {
        std::vector<double> weights;
        std::string path = logicPath + "/" + filename;
        
        if (!std::filesystem::exists(path)) {
            return weights;
        }
        
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "ERROR: Cannot load weights from " << path << std::endl;
            return weights;
        }
        
        uint64_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        weights.resize(static_cast<size_t>(size));
        file.read(reinterpret_cast<char*>(weights.data()), size * sizeof(double));
        
        if (!file) {
            std::cerr << "ERROR: Failed to read weights" << std::endl;
            weights.clear();
        }
        
        return weights;
    }
    
    void saveStateGraph(const std::string& data, const std::string& filename = "state_graph.dat") {
        std::string path = logicPath + "/" + filename;
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "ERROR: Cannot save state graph to " << path << std::endl;
            return;
        }
        file.write(data.data(), data.size());
        if (!file) {
            std::cerr << "ERROR: Failed to write state graph" << std::endl;
        }
    }
};

} // namespace System

#endif
