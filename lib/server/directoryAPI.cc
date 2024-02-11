#include <filesystem>
#include <fstream>
#include "directoryAPI.h"
namespace fs = std::filesystem;

bool createDirectoryIfNotExists(const std::string &path)
{
    try
    {
        if (!fs::exists(path))
        {
            fs::create_directories(path);
        }
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }
}

bool writeMsgToFile( std::string *content, const std::string &filePath)
{
    try
    {
        std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
        if (file.is_open())
        {
            file << *content;
            file.close();
            return true;
        }
        else
        {
            std::cerr << "Error opening file for writing" << std::endl;
            return false;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing content to file: " << e.what() << std::endl;
        return false;
    }
}
