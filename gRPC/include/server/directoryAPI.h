#include <iostream>
#ifndef DIRECTORY_API_H
#define DIRECTORY_API_H



bool CreateDirectoryIfNotExists(const std::string &path, std::string *pi_name);

bool WriteMsgToFile( std::string *content, const std::string &filePath);

#endif