#include <filesystem>

#include "mteam.h"

#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_path.h"
#include "av_log.h"
#include "av_libtorrent.h"
#include "av_http.h"
#include "av_tgbot.h"
#include "av_async.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;
namespace http_ = av::http;

namespace mteam {
	static std::vector<std::tstring> ex_include = {
		TEXT("\\"),
		TEXT("/"),
		TEXT(":"),
		TEXT("*"),
		TEXT("?"),
		TEXT("\""),
		TEXT("<"),
		TEXT(">"),
		TEXT("|"),
	};

	Mteam::Mteam(const std::tstring &api_url,
	             const std::tstring &api_key,
	             const std::tstring &img_api_url,
	             const std::tstring &img_api_key,
	             const std::tstring &tgbot_token,
	             const std::tstring &tgbot_chat_id) : m_api_url(api_url),
	                                                  m_api_key(api_key),
	                                                  m_img_api_url(img_api_url),
	                                                  m_img_api_key(img_api_key),
	                                                  m_tgbot_token(tgbot_token),
	                                                  m_tgbot_chat_id(tgbot_chat_id) {
	}

	bool Mteam::publish(const av::media::Source &source) {
		m_external_source = source;
		Category category(source.category);

		// all names
		std::tstring torrent_dir;
		std::tstring title;
		std::tstring video_filename;
		std::tstring torrent_filename;

		// 命名
		parseName(torrent_dir, title, video_filename, torrent_filename);

		// 做种目录
		auto seed_dir = av::path::append(source.seed_dir, torrent_dir);
		if (source.type == av::media::SourceType::File) {
			if (av::path::exists(torrent_dir)) {
				loge("dir {} exists", av::str::toA(torrent_dir));
				return false;
			}
			if (!av::path::create_dir(seed_dir)) {
				loge("create dir {} failed", av::str::toA(seed_dir));
				return false;
			}

			// 移动视频到目录, 并修改名称
			auto video_file = av::path::append(seed_dir, video_filename);
			if (!av::path::move_file(m_external_source.fullpath, video_file)) {
				loge("move file{} to {} failed", av::str::toA(m_external_source.fullpath), av::str::toA(video_file));
				return false;
			}
		} else if (source.type == av::media::SourceType::Dir) {
			// 如果是目录,修改名称即可
			try {
				fs::rename(m_external_source.fullpath, seed_dir);
			} catch (const fs::filesystem_error &e) {
				logw("rename {}, {} failed, err: {}", av::str::toA(m_external_source.fullpath), av::str::toA(seed_dir),
				     e.what());
				return false;
			}
			catch (const std::exception &e) {
				logw("rename {}, {} failed, err: {}", av::str::toA(m_external_source.fullpath), av::str::toA(seed_dir),
				     e.what());
				return false;
			}
		}

		// 制作种子文件
		auto torrent_file = av::path::append(m_external_source.dir, torrent_filename);
		if (!av::libtorrent::create_torrent(av::str::toA(seed_dir), av::str::toA(torrent_file))) {
			loge("create torrent file failed, {}, {}", av::str::toA(seed_dir), av::str::toA(torrent_file));
			return false;
		}
		av::async::Exit exit_delete_torrent_file([&torrent_file] {
			if (av::path::remove_file(torrent_file)) {
				logi("clean {} succ", av::str::toA(torrent_file));
			}
		});


		// 上传图片
		std::vector<std::tstring> img_url;
		UploadImg img(m_img_api_url, m_img_api_key);
		for (auto &i: m_external_source.screenshot_local) {
			std::tstring tmp;
			if (!img.Upload(i, tmp)) {
				loge("upload img {} failed", av::str::toA(i));
				continue;
			}
			if (!tmp.empty()) {
				tmp = av::str::toT(fmt::format("![]({})", av::str::toA(tmp)));
				img_url.push_back(tmp);
			}
		}
		std::tstring description = av::str::join(img_url, TEXT("\n"));
		if (!m_external_source.description.empty()) {
			// 组合豆瓣的描述
			description = m_external_source.description + TEXT("\n\n") + description;
		}

		// 发布到 m-team
		http_::Client client;
		http_::Form form; {
			auto category_id = category.getid();
			form.kv["category"] = std::to_string(static_cast<int64_t>(category_id));
			form.kv["name"] = av::str::toA(title);
			form.kv["smallDescr"] = av::str::toA(m_external_source.sub_title);
			form.kv["dmmCode"] = "";
			auto source_id = m_source.getid();
			form.kv["source"] = std::to_string(static_cast<int64_t>(source_id));
			auto standard_id = m_video_resolution.getid();
			form.kv["standard"] = std::to_string(static_cast<int64_t>(standard_id));
			form.kv["videoCodec"] = std::to_string(static_cast<int64_t>(m_video_codec.getid()));
			form.kv["audioCodec"] = std::to_string(static_cast<int64_t>(m_audio_codec.getid()));
			form.kv["team"] = std::to_string(m_external_source.group_id);
			form.kv["imdb"] = av::str::toA(m_external_source.imdb_link);
			char buff[2048];
			snprintf(buff, sizeof(buff) - 1, "https://movie.douban.com/subject/%s/",
			         av::str::toA(m_external_source.douban_id).c_str());
			form.kv["douban"] = std::string(buff);
			form.kv["labelsNew"] = "";
			form.kv["mediainfo"] = av::str::toA(m_external_source.mediainfo_text);
			form.kv["tags"] = "";
			form.kv["anonymous"] = "true";
			form.kv["aids"] = "";
			form.kv["descr"] = av::str::toA(description);
			form.kv["mediaInfoAnalysisResult"] = "true";
			if (category.getid() == CategoryId::Movie) {
				form.kv["labels"] = "0";
				form.kv["labelsNew"] = "中配";
			}
		}
		http_::Header header;
		header.kv["x-api-key"] = av::str::toA(m_api_key);
		form.file["file"] = av::str::toA(torrent_file);

		// 上传到网站
		auto url = m_api_url + TEXT("/api/torrent/createOredit");
		logi("post url {}", av::str::toA(url));
		const auto resp = client.post(av::str::toA(url), header, form);
		if (!resp) {
			loge("send http failed");
			return false;
		}
		if (resp->status != 200) {
			loge("post not ok");
			return false;
		}

		// 解析返回
		std::tstring response_id = TEXT("");
		std::tstring response_name = TEXT("");
		std::tstring response_sub_title = TEXT("");
		std::tstring response_create_date = TEXT("");
		try {
			json obj;
			auto j = obj.parse(resp->body);
			if (!j.contains("code")) {
				logw("parse error {}", resp->body);
				return false;
			}
			std::string message;
			if (j.contains("message") && j["message"].is_string()) {
				message = j["message"].get<std::string>();
			}

			// 网站返回不标准,所以这里要分两次判断
			if (j["code"].is_number_integer()) {
				// 这可能是返回错误
				const auto code_i = j["code"].get<int>();
				if (code_i != 0) {
					std::tstring msg = TEXT("发布 ");
					msg += title + TEXT("\n");
					msg += m_external_source.sub_title + TEXT(" 失败\n");
					msg += TEXT("错误消息: ") + av::str::toT(message);
					sendTGWaringMessage(msg);
					return false;
				}
			}

			auto code = j["code"].get<std::string>();
			if (code != "0") {
				// here, the website result string
				logw("code {} message {}", code, message);
			}

			if (j.contains("data")) {
				json data = j["data"];
				if (data.contains("id") && data["id"].is_string()) {
					response_id = av::str::toT(data["id"].get<std::string>());
				}
				if (data.contains("name") && data["name"].is_string()) {
					response_name = av::str::toT(data["name"].get<std::string>());
				}
				if (data.contains("smallDescr") && data["smallDescr"].is_string()) {
					response_sub_title = av::str::toT(data["smallDescr"].get<std::string>());
				}
				if (data.contains("createdDate") && data["createdDate"].is_string()) {
					response_create_date = av::str::toT(data["createdDate"].get<std::string>());
				}
			}
		} catch (const json::parse_error &e) {
			logw("parse_error {}, {}", e.what(), resp->body);
			return false;
		}
		catch (const std::exception &e) {
			logw("exception {}, {}", e.what(), resp->body);
			return false;
		}

		if (!sendTGMessage(m_external_source.poster_img, m_external_source.screenshot_local, response_id, response_name,
		                   response_sub_title, response_create_date)) {
			logw("sendTGMessage failed");
		}
		return true;
	}

	bool Mteam::sendTGMessage(std::tstring &douban_poster_img,
	                          std::vector<std::tstring> &screenshots,
	                          std::tstring &publish_id,
	                          std::tstring &title,
	                          std::tstring &sub_title,
	                          std::tstring &create_date) {
		// 发送tg消息
		std::tstring text; {
			std::string text_tmp = R"(
🔗 链接: https://kp.m-team.cc/detail/%s
🔧 標題: %s
🎫 副標題: %s
⏲ 发布时间: %s
)";
			char buff[32768];
			sprintf(buff, text_tmp.c_str(), av::str::toA(publish_id).c_str(), av::str::toA(title).c_str(),
			        av::str::toA(sub_title).c_str(), av::str::toA(create_date).c_str());
			text = av::str::toT(std::string(buff));
		}

		// use douban img
		if (!douban_poster_img.empty()) {
			logi("try use douban poster img: {}", av::str::toA(douban_poster_img));
			while (true) {
				if (!av::tgbot::send_net_photo_message(av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
				                                       av::str::toA(douban_poster_img), av::str::toA(text))) {
					logw("send_net_photo_message send failed!!! {}, {}, {}, {}", av::str::toA(m_tgbot_token),
					     av::str::toA(m_tgbot_chat_id),
					     av::str::toA(douban_poster_img), av::str::toA(text));
					break;
				}
				return true;
			}
		}

		// use screenshots
		if (!screenshots.empty()) {
			auto img = screenshots[0];
			logi("try use screenshots img: {}", av::str::toA(img));
			while (true) {
				if (!av::tgbot::send_local_photo_message(av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
				                                         av::str::toA(img), av::str::toA(text))) {
					logw("send_net_photo_message send failed!!! {}, {}, {}, {}", av::str::toA(m_tgbot_token),
					     av::str::toA(m_tgbot_chat_id),
					     av::str::toA(img), av::str::toA(text));
					break;
				}
				return true;
			}
		}

		// use defualt m-team logo
		std::tstring img = TEXT("https://static.m-team.cc/static/media/logo.80b63235eaf702e44a8d.png");
		logi("try use default img: {}", av::str::toA(img));
		if (!av::tgbot::send_net_photo_message(av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
		                                       av::str::toA(img), av::str::toA(text))) {
			logw("send_net_photo_message send failed!!! {}, {}, {}, {}", av::str::toA(m_tgbot_token),
			     av::str::toA(m_tgbot_chat_id),
			     av::str::toA(img), av::str::toA(text));
		}

		return true;
	}

	bool Mteam::parseName(std::tstring &torrent_dir,
	                      std::tstring &title,
	                      std::tstring &video_filename,
	                      std::tstring &torrent_filename
	) {
		m_video_codec.setSourceVideoCodec(m_external_source.video_codec);
		m_video_resolution.setSourceResolution(m_external_source.video_resolution);
		m_audio_codec.setSourceCodec(m_external_source.audio_codec);
		m_category.setSourceCategory(m_external_source.category);
		m_source.setExternalSourceId(m_external_source.source_id);

		auto title_name_vec = av::str::split(m_external_source.name_eng, TEXT(" "));
		std::tstring new_name = av::str::join(title_name_vec, TEXT("."));
		for (auto &t: ex_include) {
			av::str::replace_all(new_name, t, TEXT(""));
		}

		// names
		std::vector<std::tstring> title_name;
		std::vector<std::tstring> video_name;
		std::vector<std::tstring> torrent_name;
		std::vector<std::tstring> torrent_dir_;
		if (!m_external_source.title_prefix.empty()) {
			title_name.push_back(m_external_source.title_prefix);
			auto tmp = m_external_source.title_prefix;
			av::str::replace_all(tmp, TEXT(" "), TEXT("."));
			video_name.push_back(tmp);
			torrent_name.push_back(tmp);
			torrent_dir_.push_back(tmp);
		}
		title_name.push_back(m_external_source.name_eng);
		video_name.push_back(new_name);
		torrent_name.push_back(new_name);
		torrent_dir_.push_back(new_name);

		if (!m_external_source.season.empty()) {
			title_name.push_back(m_external_source.season);
			video_name.push_back(m_external_source.season);
			torrent_name.push_back(m_external_source.season);
			torrent_dir_.push_back(TEXT("Complete"));
		}

		title_name.push_back(m_external_source.year);
		video_name.push_back(m_external_source.year);
		torrent_name.push_back(m_external_source.year);
		torrent_dir_.push_back(m_external_source.year);

		auto source_text = m_source.getText();
		if (!source_text.empty()) {
			title_name.push_back(source_text);
			video_name.push_back(source_text);
			torrent_name.push_back(source_text);
			torrent_dir_.push_back(source_text);
		}

		title_name.push_back(m_video_resolution.getText());
		video_name.push_back(m_video_resolution.getText());
		torrent_name.push_back(m_video_resolution.getText());
		torrent_dir_.push_back(m_video_resolution.getText());


		title_name.push_back(m_audio_codec.getText()); {
			auto tmp = m_audio_codec.getText();
			av::str::replace_all(tmp, TEXT(" "), TEXT("."));
			video_name.push_back(tmp);
			torrent_name.push_back(tmp);
			torrent_dir_.push_back(tmp);
		}

		auto team_name = mteam::group::getText(m_external_source.group_id);
		title_name.push_back(m_video_codec.getText() + TEXT("-") + team_name);
		video_name.push_back(m_video_codec.getText() + TEXT("-") + team_name);
		torrent_dir_.push_back(m_video_codec.getText() + TEXT("-") + team_name);
		torrent_name.push_back(m_video_codec.getText() + TEXT("-") + team_name);

		// 做种目录名
		if (!m_external_source.season.empty()) {
			torrent_dir = av::str::join(torrent_dir_, TEXT(".")); // 剧集
		} else {
			torrent_dir = av::str::join(video_name, TEXT("."));
		}

		// 标题
		title = av::str::join(title_name, TEXT(" "));

		// 视频文件名
		video_name.push_back(m_external_source.file_suffix);
		video_filename = av::str::join(video_name, TEXT("."));

		// 种子文件名
		torrent_name.push_back(TEXT("torrent"));
		torrent_filename = av::str::join(torrent_name, TEXT("."));
		return true;
	}

	void Mteam::sendTGWaringMessage(const std::tstring &msg) const {
		const std::string text = "失败警告: " + av::str::toA(msg);
		av::tgbot::send_message(av::str::toA(m_tgbot_token),
		                        av::str::toA(m_tgbot_chat_id),
		                        text);
	}

	Mteam::~Mteam() {
	}
}
