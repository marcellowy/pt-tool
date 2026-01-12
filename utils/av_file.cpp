//
// Created by Marcello on 2026/1/13.
//
#include "av_file.h"
#include "av_log.h"
#include <fstream>

namespace av {
    namespace file {
        bool readContent(const fs::path& path, std::string& content) {
            if (!fs::exists(path)) {
                logw("path {} not exists", path.string());
                return false;
            }
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) {
                logw("file {} not open");
                return false;
            }
            std::string line;
            while (std::getline(file, line)) {
                content += line;
            }
            return true;
        }
    }
}