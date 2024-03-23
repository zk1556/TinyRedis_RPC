#ifndef FILECREATOR_H
#define FILECREATOR_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
    文件创建类
*/
class FileCreator {
    static bool createDirectory(const std::string& path);

    static void createAllParentDirectories(const std::string& path);

    static void createFilesInDirectory(const std::string& path, const std::string& fileName, int numberOfFiles);

    static bool isFileExists(const std::string& path);
public:
    static void createFolderAndFiles(const std::string& folderPath, const std::string& fileName, int numberOfFiles);
};

bool FileCreator::createDirectory(const std::string& path) {
    struct stat st = {0};
    if (stat(path.c_str(), &st) == -1) {
        // 目录不存在，尝试创建
        if (mkdir(path.c_str(), 0755) != -1) {
            return true; // 目录创建成功
        } else {
            std::cerr << "Failed to create directory: " << path << std::endl;
            return false; // 创建目录失败
        }
    }
    return true; // 目录已存在
}

void FileCreator::createAllParentDirectories(const std::string& path) {
    size_t pos = 0;
    std::string currentPath;
    while ((pos = path.find_first_of("/\\", pos + 1)) != std::string::npos) {
        currentPath = path.substr(0, pos);
        createDirectory(currentPath);
    }
}

void FileCreator::createFilesInDirectory(const std::string& path, const std::string& fileName, int numberOfFiles) {
    for (int i = 0; i < numberOfFiles; ++i) {
        std::ostringstream filePath;
        filePath << path << "/" << fileName << i;

        if (isFileExists(filePath.str())) {
            continue;
        }
        std::ofstream file(filePath.str());
        if (file.is_open()) {
            file.close();
        } else {
            std::cerr << "Unable to create file: " << filePath.str() << std::endl;
        }
    }
}

void FileCreator::createFolderAndFiles(const std::string& folderPath, const std::string& fileName, int numberOfFiles) {
    createAllParentDirectories(folderPath);
    if (createDirectory(folderPath)) {
        
        createFilesInDirectory(folderPath, fileName, numberOfFiles);
    } else {
        std::cerr << "Unable to create or find folder: " << folderPath << std::endl;
    }
}

bool FileCreator::isFileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && !S_ISDIR(buffer.st_mode));
}


#endif 