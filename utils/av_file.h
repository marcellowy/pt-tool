//
// Created by Marcello on 2026/1/13.
//

#ifndef AV_FILE_H
#define AV_FILE_H

#include <filesystem>

namespace fs = std::filesystem;

namespace av {
    namespace file {
        // readfile for UTF-8
        bool readContent(const fs::path& path, std::string& content);
    };
};

#endif //AV_FILE_H
