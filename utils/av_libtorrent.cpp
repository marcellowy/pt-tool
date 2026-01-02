#include "av_libtorrent.h"

#include <libtorrent/create_torrent.hpp>
#include <libtorrent/file_storage.hpp>
#include <libtorrent/bencode.hpp>
#include <filesystem>
#include <fstream>

#include "av_log.h"

namespace fs = std::filesystem;
namespace lt = libtorrent;

namespace av {
    namespace libtorrent {
        bool create_torrent(const std::string& in, const std::string& out_file) {
            lt::file_storage fs;

            // 1. 添加文件到存储结构
            lt::add_files(fs, in);

            // 2. 初始化创建对象
            lt::create_torrent ct(fs);
            //ct.add_tracker("http://tracker.example.com/announce");
            ct.set_creator("PT TOOL 1.0");
            
            fs::path p = in;
            fs::path parent = p.parent_path();
            lt::error_code ec;
            lt::set_piece_hashes(ct, parent.string(), ec); // 这里的路径要写 in 的父路径
            if (ec) {
                loge("error {}", ec.message());
                return false;
            }
            
            // 4. 将 Bencode 数据编码为字节流
            std::vector<char> torrent;
            lt::bencode(std::back_inserter(torrent), ct.generate());

            // 5. 写入文件
            std::ofstream out(out_file, std::ios::binary);
            out.write(torrent.data(), torrent.size());
            
            return true;
        }
    }
}
