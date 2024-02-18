#include <iostream>
#ifndef DIRECTORY_API_H
#define DIRECTORY_API_H



bool createDirectoryIfNotExists(const std::string &path,std::string *piName);

bool writeMsgToFile( std::string *content, const std::string &filePath);

#endif