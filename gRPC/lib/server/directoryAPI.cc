#include <filesystem>
#include <fstream>
#include "directoryAPI.h"
namespace fs = std::filesystem;

bool CreateDirectoryIfNotExists(const std::string &path, std::string *pi_name)
{
    std::string fullPath = path + (*pi_name);
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

bool WriteMsgToFile(std::string *content, const std::string &file_path)
{
    try
    {
        std::ofstream file(file_path, std::ios::binary | std::ios::trunc);
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
