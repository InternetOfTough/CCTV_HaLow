#include <filesystem>
#include <fstream>
#include "directoryAPI.h"
namespace fs = std::filesystem;

bool createDirectoryIfNotExists(const std::string &path, std::string *piName)
{
    std::string fullPath = path + (*piName);
    try
    {
        if (!fs::exists(fullPath))
        {
            fs::create_directories(fullPath);
        }
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }
}

bool writeMsgToFile(std::string *content, const std::string &filePath)
{
    try
    {
        std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
        if (file.is_open() && content != nullptr)
        {
            file << *content;
            file.close();
            return true;
        }
        else
        {
            file.close();
            std::cerr << "Error opening file for writing" << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing content to file: " << e.what() << std::endl;
        return false;
    }
    if (content != nullptr)
        delete content;
}
