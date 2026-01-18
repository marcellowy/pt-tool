//
// Created by Marcello on 2026/1/13.
//

#ifndef AV_FILE_H
#define AV_FILE_H

#include <filesystem>
#include <fstream>
#include <sstream>

#include "av_log.h"

namespace fs = std::filesystem;

namespace av::file {
    inline bool read(const fs::path& path, std::string& content) {
        if (!fs::exists(path)) {
            logw("path {} not exists", path.string());
            return false;
        }
        const std::ifstream file(path, std::ios::binary | std::ios::in);
        if (!file.is_open()) {
            logw("file {} not open");
            return false;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        content = ss.str();
        return true;
    }

};

#endif //AV_FILE_H
