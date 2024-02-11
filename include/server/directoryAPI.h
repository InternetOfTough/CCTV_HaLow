#include <iostream>
#ifndef DIRECTORY_API_H
#define DIRECTORY_API_H



bool createDirectoryIfNotExists(const std::string &path);

bool writeMsgToFile( std::string *content, const std::string &filePath);

#endif