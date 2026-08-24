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
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        if (loss >= 0) {
            file << "Loss: " << std::fixed << std::setprecision(6) << loss << " | ";
        }
        file << message << std::endl;
        file.close();
    }
    
    void logError(const std::string& error) {
        std::string logFile = logsPath + "/error_logs.txt";
        std::ofstream file(logFile, std::ios::app);
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ERROR: ";
        file << error << std::endl;
        file.close();
    }
    
    void saveWeights(const std::vector<double>& weights, const std::string& filename = "weights.bin") {
        std::string path = logicPath + "/" + filename;
        std::ofstream file(path, std::ios::binary);
        
        size_t size = weights.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(reinterpret_cast<const char*>(weights.data()), size * sizeof(double));
        file.close();
    }
    
    std::vector<double> loadWeights(const std::string& filename = "weights.bin") {
        std::vector<double> weights;
        std::string path = logicPath + "/" + filename;
        
        if (!std::filesystem::exists(path)) {
            return weights;
        }
        
        std::ifstream file(path, std::ios::binary);
        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        weights.resize(size);
        file.read(reinterpret_cast<char*>(weights.data()), size * sizeof(double));
        file.close();
        
        return weights;
    }
    
    void saveStateGraph(const std::string& data, const std::string& filename = "state_graph.dat") {
        std::string path = logicPath + "/" + filename;
        std::ofstream file(path, std::ios::binary);
        file.write(data.data(), data.size());
        file.close();
    }
};

} // namespace System

#endif
