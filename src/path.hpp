#ifndef PATHING
#define PATHING

#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <dirent.h>

struct Path
{
    Path() { }

    Path(std::string path)
    {
        parse_nodes(path);
    }

    Path(Path& other)
    {
        nodes = other.nodes;
    }

    Path(Path&& other)
    {
        nodes = other.nodes;
    }

    Path& operator=(Path& other) = default;

    /// @brief Goes to the previous folder.
    Path back()
    {
        if (nodes.size() <= 1)
            return *this;

        Path result;
        
        for (size_t i = 0; i < nodes.size() - 1; ++i)
            result.nodes.push_back(nodes[i]);

        return result;
    }

    std::vector<std::string> get_files()
    {
        DIR *pDIR;
        struct dirent *entry;
        std::vector<std::string> fileNames;

        if(pDIR = opendir(to_string().c_str()))
        {
            while((entry = readdir(pDIR)))
            {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                    fileNames.push_back(entry->d_name);
            }
            closedir(pDIR);
        }

        return fileNames;
    }

    /// @brief Grabs the path to the executable's directory.
    static Path get_executable_path()
    {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return Path(std::string(buffer));
    }

    static Path get_working_dir()
    {
        char buffer[FILENAME_MAX];

        if (_getcwd(buffer, sizeof(buffer)) != NULL)
            return Path(std::string(buffer));

        return Path();
    }

    std::string to_string()
    {
        std::string result = "";

        for (size_t i = 0; i < nodes.size(); ++i)
        {
            if (i == nodes.size() - 1)
            {
                result += nodes[i];
            }
            else
            {
                result += nodes[i] + "\\";
            }
        }

        return result;
    }

    Path& operator/(const std::string& other)
    {
        this->nodes.push_back(other);
        return *this;
    }

    Path& operator/(const Path& other)
    {
        for (auto& node : other.nodes)
            this->nodes.push_back(node);
        return *this;
    }

private:
    std::vector<std::string> nodes;

    void parse_nodes(std::string path)
    {
        std::string node = "";

        for (size_t i = 0; i < path.size(); ++i)
        {
            if (path[i] == '/' || path[i] == '\\')
            {
                nodes.push_back(node);
                node = "";
            }
            else
            {
                node += path[i];
            }
        }

        if (node.size() != 0)
            nodes.push_back(node);
    }
};

#endif