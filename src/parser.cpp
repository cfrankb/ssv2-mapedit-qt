#include <QDebug>
#include <cstring>
#include <stdint.h>
#include <cstdio>
#include "parser.h"
#include "shared/qtgui/qfilewrap.h"

void parseSectionOptions(const std::string section, config_t &config, const stringvector_t& list, const int line)
{
    if (section == "x-types" || section == "tasks") {
        if (list.size() == 2) {
            auto const & val = list[0];
            auto const & key = std::strtoul(list[1].c_str(), nullptr, 16);
            if (section == "x-types") {
                config.types[key] = val;
            } else if (section == "tasks") {
                config.tasks[key] = val;
            }
        } else {
            qDebug("list on line %d has %zu params but should be %d", line, list.size(), 2);
        }
    } else if (section == "images") {
        if (list.size() == 2) {
            auto const & val = list[1];
            auto const & key = std::strtoul(list[0].c_str(), nullptr, 16);
            config.images[key] = val;
        } else {
            qDebug("list on line %d has %zu params but should be %d", line, list.size(), 2);
        }
    } else if (section == "types") {
    } else {
        qDebug("unknown section: %s on line %d", section.c_str(), line);
    }
}

char *parseLine(config_t & config, int &line, std::string &section, char *p)
{
    ++line;
    char *e = strstr(p, "\n");
    if (e)
    {
        *e = '\0';
    }
    char *m = strstr(p, "\r");
    if (m)
    {
        *m = '\0';
    }
    if (m > e)
    {
        e = m;
    }

    char *c = strstr(p, "#");
    if (c)
    {
        *c = '\0';
    }
    int n = strlen(p);
    if (n)
    {
        char *t = p + n - 1;
        while (t > p && isspace(*t))
        {
            *t = '\0';
            --t;
        }
    }

    while (isspace(*p))
    {
        ++p;
    }
    if (*p == '[')
    {
        ++p;
        char *t = strstr(p, "]");
        if (t)
        {
            *t = '\0';
        }
        else
        {
            printf("no section delimiter on line %d\n", line);
        }
        section = p;
    }
    else if (*p)
    {
        stringvector_t list;
        splitString(p, list);
        if (list.size() == 0)
        {
            printf("empty list on line %d\n", line);
        } else if (section.size() == 0) {
            qDebug("noname section");
           // general options
        } else {
            parseSectionOptions(section, config, list, line);
        }
    }
    return e ? ++e : nullptr;
}

void splitString(const std::string &str, stringvector_t &list)
{
    int i = 0;
    unsigned int j = 0;
    bool inQuote = false;
    while (j < str.length())
    {
        if (str[j] == '"') {
            inQuote = !inQuote;
        }
        if (!inQuote && isspace(str[j]))
        {
            list.emplace_back(str.substr(i, j - i));
            while (isspace(str[j]) && j < str.length())
            {
                ++j;
            }
            i = j;
            continue;
        }
        ++j;
    }
    list.emplace_back(str.substr(i, j - i));
}

bool readConfig(config_t & config, const char *confName)
{
    qDebug("parsing: %s", confName);

    QFileWrap file;
    if (file.open(confName, "rb"))
    {
        size_t size = file.getSize();
        char *data = new char[size + 1];
        data[size] = 0;
        file.read(data, size);
        file.close();

        char *p = data;
        std::string section;
        int line = 0;
        while (p && *p)
        {
            p = parseLine(config, line, section, p);
        }
        delete[] data;
        return true;
    }
    return false;
}

void clearConfig(config_t & config)
{
    config.images.clear();
    config.tasks.clear();
    config.types.clear();
}
