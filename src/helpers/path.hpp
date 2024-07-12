#ifndef PATHING
#define PATHING

#include <vector>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
#endif

#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <dirent.h>

/**
 * Used for path manipulation.
 */
class Path
{
    std::vector<std::string> nodes;

    /**
     * Parsers the string path into nodes.
     */
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

public:
    Path() { }
    Path(std::string path) { parse_nodes(path); }
    Path(Path& other) { nodes = other.nodes; }
    Path(Path&& other) { nodes = other.nodes; }
    Path& operator=(Path& other) = default;

    /**
     * Removes the top folder name.
     */
    Path back()
    {
        if (nodes.size() <= 1)
            return *this;

        Path result;
        
        for (size_t i = 0; i < nodes.size() - 1; ++i)
            result.nodes.push_back(nodes[i]);

        return result;
    }

    std::string get_file_name()
    {
        if (nodes.size() > 0)
            return nodes[0];
        
        return "";
    }

    /**
     * Returns all of the files in the paths directory.
     */
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

    /**
     * Gets the path of the executable.
     */
    static Path get_executable_path()
    {
#if _WIN32
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return Path(std::string(buffer));
#elif __linux__
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        const char *path;
        if (count != -1) {
            path = dirname(result);
        }
        return Path(std::string(result));
#endif
    }

    /**
     * Gets the current working directory.
     */
    static Path get_working_dir()
    {
        char buffer[FILENAME_MAX];

#if _WIN32
        if (_getcwd(buffer, sizeof(buffer)) != NULL)
#elif __linux__
        if (getcwd(buffer, sizeof(buffer)) != NULL)
#endif
            return Path(std::string(buffer));

        return Path();
    }

    /**
     * Prints the path as string.
     */
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

    /**
     * Appends a node to the path.
     */
    Path& operator/(const std::string& other)
    {
        this->nodes.push_back(other);
        return *this;
    }

    /**
     * Combines two paths.
     */
    Path& operator/(const Path& other)
    {
        for (auto& node : other.nodes)
            this->nodes.push_back(node);
        return *this;
    }
};

#endif