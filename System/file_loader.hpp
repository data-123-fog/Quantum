#ifndef FILE_LOADER_HPP
#define FILE_LOADER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace System {

struct SourceFile {
    std::string path;
    std::string content;
};

class FileLoader {
public:
    std::vector<SourceFile> loadFromDirectory(const std::string& dirPath) {
        std::vector<SourceFile> files;
        
        if (!std::filesystem::exists(dirPath)) {
            std::cerr << "Directory not found: " << dirPath << std::endl;
            return files;
        }
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".cc") {
                    SourceFile sf;
                    sf.path = entry.path().string();
                    sf.content = readFile(sf.path);
                    if (!sf.content.empty()) {
                        files.push_back(sf);
                    }
                }
            }
        }
        
        return files;
    }
    
private:
    std::string readFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open file: " << path << std::endl;
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        return content;
    }
};

} // namespace System

#endif
