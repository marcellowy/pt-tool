#include <fstream>

#include "MediaInfo/MediaInfo.h"

#include "av_mediainfo.h"
#include "av_media_info.h"

#include "nlohmann/json.hpp"

#include "av_path.h"
#include "av_time.h"
#include "av_log.h"
#include "av_async.h"
#include "av_string.h"

namespace av {
    namespace mediainfo {

        MediaInfo::MediaInfo(const std::tstring& path) :m_video_path(path) {

        }

        MediaInfo::~MediaInfo() {

        }

        bool MediaInfo::parse() {
            MediaInfoLib::MediaInfo MI;
            if (!av::path::file_exists(m_video_path)) {
                loge("file {} not exists", av::str::toA(m_video_path));
                return false;
            }

            std::tstring tmp_name = av::str::toString(av::time::milliseconds());
            tmp_name.append(TEXT(".ts"));
            std::tstring dir;
            std::tstring filename;
            av::path::split(m_video_path, dir, filename);
            auto new_path = av::path::append(dir, tmp_name);
            if (av::path::file_exists(new_path)) {
                if (!av::path::remove_file(new_path)) {
                    loge("new tmp file {} exists, but can not remove", av::str::toA(new_path));
                    return false;
                }
            }

            // move to tmp file
            if (!av::path::move_file(m_video_path, new_path, true)) {
                loge("move file {} to {} failed", av::str::toA(m_video_path), av::str::toA(new_path));
                return false;
            }
            av::async::Exit exit([&new_path, this] {
                if (!av::path::move_file(new_path, m_video_path)) {
                    loge("move {} to {} file failed", av::str::toA(new_path), av::str::toA(m_video_path));
                }
            });

            // open
            size_t ret = MI.Open(av::str::toW(new_path));
            if (ret == 0) {
                std::tstring info = av::str::toT(MI.Inform());
                logw("open {} failed, info {}", av::str::toA(m_video_path), av::str::toA(info));
                return false;
            }
            MI.Option(__T("Complete"), __T("1"));
            MI.Option(__T("Output"), __T("JSON"));
            m_json = av::str::toT(MI.Inform());
            MI.Option(__T("Output"), __T("TEXT"));
            m_text = av::str::toT(MI.Inform());

            /*std::ofstream output_file("example.txt");
            if (!output_file) {
                std::cerr << "Error opening file!" << std::endl;
                return 1;
            }
            output_file << av::str::toA(m_json);
            output_file.close();*/

            nlohmann::json json;
            try {
                auto j = json.parse(av::str::toA(m_json));
                if (j.contains("media")) {
                    auto& media = j["media"];
                    if (media.contains("track") && media["track"].is_array()) {
                        for (const auto& track : media["track"]) {
                            if (track.contains("@type")) {
                                std::tstring type = av::str::toT(track["@type"].get<std::string>());
                                if (type == TEXT("Video")) {
                                    try {
                                        m_video.width = std::stoll(track["Width"].get<std::string>());
                                        m_video.height = std::stoll(track["Height"].get<std::string>());
                                    }
                                    catch (std::exception& e) {
                                        loge("exception err {}", e.what());
                                        return false;
                                    }
                                    if (track.contains("ScanType")) {
                                        auto scan_type = av::str::toT(track["ScanType"].get<std::string>());
                                        if (scan_type == TEXT("Interlaced")) {
                                            m_video.scan_type = av::media::ScanType::Interlaced;
                                        }
                                        else if (scan_type == TEXT("Progressive")) {
                                            m_video.scan_type = av::media::ScanType::Progressive;
                                        }
                                        else if (scan_type == TEXT("MBAFF")) {
                                            m_video.scan_type = av::media::ScanType::MBAFF;
                                        }
                                    }
                                    if (track.contains("Format")) {
                                        auto format = av::str::toT(track["Format"].get<std::string>());
                                        if (format == TEXT("AVC")) {
                                            m_video.codec = av::media::SourceVideoCodec::_h264;
                                        }
                                        else if (format == TEXT("MPEGVideo")) {
                                            m_video.codec = av::media::SourceVideoCodec::_mpeg2;
                                        }
                                        else if (format == TEXT("HEVC")) {
                                            m_video.codec = av::media::SourceVideoCodec::_h265;
                                        }
                                    }
                                }
                                else if (type == TEXT("Audio")) {
                                    auto format = av::str::toT("");
                                    if (track.contains("Format")) {
                                        format = av::str::toT(track["Format"].get<std::string>());
                                    }
                                    auto format_commercial_if_any = av::str::toT("");
                                    if (track.contains("Format_Commercial_IfAny")) {
                                        format_commercial_if_any = av::str::toT(track["Format_Commercial_IfAny"].get<std::string>());
                                    }
                                    auto format_profile = av::str::toT("");
                                    if (track.contains("Format_Profile")) {
                                        format_profile = av::str::toT(track["Format_Profile"].get<std::string>());
                                    }

                                    if (format == TEXT("AAC LC") || format == TEXT("AAC")) {
                                        m_audio.codec = av::media::SourceAudioCodec::_aac;
                                    }
                                    else if (format == TEXT("AC-3")) {
                                        if (format_commercial_if_any == TEXT("Dolby Digital")) {
                                            m_audio.codec = av::media::SourceAudioCodec::_ac3;
                                        }
                                    }
                                    else if (format == TEXT("OGG")) {
                                        m_audio.codec = av::media::SourceAudioCodec::_ogg;
                                    }
                                    else if (format == TEXT("MPEG Audio")) {
                                        if (format_profile == TEXT("Layer 1")) {
                                            m_audio.codec = av::media::SourceAudioCodec::_mp1;
                                        }
                                        else if (format_profile == TEXT("Layer 2")) {
                                            m_audio.codec = av::media::SourceAudioCodec::_mp2;
                                        }
                                        else if (format_profile == TEXT("Layer 3")) {
                                            m_audio.codec = av::media::SourceAudioCodec::_mp3;
                                        }
                                    }
                                    else if (format == TEXT("E-AC-3")) {
                                        if (format_commercial_if_any == TEXT("Dolby Digital Plus")) {
                                            m_audio.codec = av::media::SourceAudioCodec::_e_ac3_ddp;
                                        }
                                        else {
                                            m_audio.codec = av::media::SourceAudioCodec::_e_ac3_atoms;
                                        }
                                    }
                                    else if (format == TEXT("DTS")) {
                                        if (format_commercial_if_any == TEXT("DTS-HD Master Audio")) {
                                            m_audio.codec = av::media::SourceAudioCodec::_dts_hd_ma;
                                        }
                                        else {
                                            m_audio.codec = av::media::SourceAudioCodec::_dts;
                                        }
                                    }

                                }
                                else if (type == TEXT("General")) {
                                    // no any value need
                                }
                            }
                        }

                    }
                }
            }
            catch (std::exception& e) {
                loge("exception err {}", e.what());
                return false;
            }

            return true;
        }

        Video MediaInfo::getVideo() {
            return m_video;
        }

        Audio MediaInfo::getAudio() {
            return m_audio;
        }

        std::tstring MediaInfo::getText() {
            return m_text;
        }
    }
}

