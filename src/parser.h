#ifndef __PARSER_H
#define __PARSER_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>

using stringvector_t = std::vector<std::string>;
typedef struct   {
    std::map<uint8_t, std::string> types;
    std::map<uint8_t, std::string> tasks;
    std::map<uint16_t, std::string> images;
} config_t;

void splitString(const std::string &str, stringvector_t &list);
bool readConfig(config_t & config, const char *confName);
void clearConfig(config_t & config);

#endif
