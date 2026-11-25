#include <filesystem>
#include <functional>
#include <utility>
#include "mteam.h"

#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_path.h"
#include "av_log.h"
#include "av_libtorrent.h"
#include "av_http.h"
#include "av_tgbot.h"
#include "av_async.h"
#include "av_utils.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;
namespace http_ = av::http;

namespace {
	static std::tstring getTmpDir(const av::media::Source& obj);

	std::tstring getTmpDir(const av::media::Source& obj) {
		std::tstring dir = av::path::get_exe_dir();
		dir = av::path::append(dir, TEXT("tmp"));
		dir = av::path::append(dir, obj.fullpath_md5);
		return dir;
	}
}

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

	Mteam::Mteam(const std::tstring& api_url,
		const std::tstring& api_key,
		const std::tstring& img_api_url,
		const std::tstring& img_api_key,
		const std::tstring& tgbot_token,
		const std::tstring& tgbot_chat_id,
		const bool& debug) : m_api_url(api_url),
		m_api_key(api_key),
		m_img_api_url(img_api_url),
		m_img_api_key(img_api_key),
		m_tgbot_token(tgbot_token),
		m_tgbot_chat_id(tgbot_chat_id),
		m_debug(debug) {
	}

	bool Mteam::publish(const av::media::Source& source) {
		Category category(source.category);

		//
		mteam::Source m_source; m_source.setExternalSourceId(source.source_id);
		mteam::video::Codec video_codec; video_codec.setSourceVideoCodec(source.video_codec);
		mteam::audio::Codec audio_codec; audio_codec.setSourceCodec(source.audio_codec);
		mteam::video::Resolution video_resolution; video_resolution.setSourceResolution(source.video_resolution);

		//
		PublishName publish_name;
		if (!name(source, publish_name)) {
			loge("get publish name failed");
			return false;
		}
		std::tstring title = publish_name.title;

		// 上传图片
		std::vector<std::tstring> img_url;
		if (!source.screenshot_local.empty()) {
			logi("source.screenshot_local size = {}", source.screenshot_local.size());
			UploadImg img(m_img_api_url, m_img_api_key);
			for (auto& i : source.screenshot_local) {
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
		}
		else {
			logw("m_external_source.screenshot_local size = {}", source.screenshot_local.size());
		}

		// description
		std::tstring description;

		// uploaded img_url
		if (!img_url.empty()) {
			description = description + av::str::join(img_url, TEXT("\n"));
		}

		// append douban description
		if (!source.description.empty()) {
			// 组合豆瓣的描述
			description = source.description + TEXT("\n\n") + description;
		}

		// 没有描述
		std::tstring warning_msg;
		if (description.empty()) {
			warning_msg = title + TEXT("\n");
			warning_msg += source.sub_title + TEXT("\n");
			warning_msg += TEXT("请补充描述");

			//
			description = TEXT("稍后补充截图");
		}


		// 发布到 m-team
		http_::Client client;
		http_::Form form; {
			auto category_id = category.getid();
			form.kv["category"] = std::to_string(static_cast<int64_t>(category_id));
			form.kv["name"] = av::str::toA(title);
			form.kv["smallDescr"] = av::str::toA(source.sub_title);
			form.kv["dmmCode"] = "";
			auto source_id = m_source.getid();
			form.kv["source"] = std::to_string(static_cast<int64_t>(source_id));
			auto standard_id = video_resolution.getid();
			form.kv["standard"] = std::to_string(static_cast<int64_t>(standard_id));
			form.kv["videoCodec"] = std::to_string(static_cast<int64_t>(video_codec.getid()));
			form.kv["audioCodec"] = std::to_string(static_cast<int64_t>(audio_codec.getid()));
			form.kv["team"] = std::to_string(source.group_id);
			form.kv["imdb"] = av::str::toA(source.imdb_link);
			if (!source.douban_id.empty()) {
				char buff[2048];
				snprintf(buff, sizeof(buff) - 1, "https://movie.douban.com/subject/%s/",
					av::str::toA(source.douban_id).c_str());
				form.kv["douban"] = std::string(buff);
			}
			else {
				form.kv["douban"] = "";
			}
			form.kv["labelsNew"] = "";
			form.kv["mediainfo"] = av::str::toA(source.mediainfo_text);
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
		form.file["file"] = av::str::toA(source.torrent_file);

		// 上传到网站
		auto url = m_api_url + TEXT("/api/torrent/createOredit");
		logi("post url {}", av::str::toA(url));
		av::http::Result resp;
		int max_retry = 0;
		while (true) {
			if (max_retry >= 5)
			{
				break;
			}
			max_retry++;

			//
			if (m_debug) {
				// debug
				resp = std::make_unique<av::http::Response>();
				resp->status = 200;
				resp->body = R"({
					"code": 0,
					"message": "success",
					"data": {
						"id": "123456",
						"name": "test name",
						"smallDescr": "test smallDescr",
						"createdDate": "2024-01-01 00:00:00"
					}
				})";
			}
			else {
				resp = client.post(av::str::toA(url), header, form);
			}

			//
			if (!resp) {
				loge("send http failed");
				std::this_thread::sleep_for(std::chrono::seconds(5));
				continue;
			}
			if (resp->status != 200) {
				loge("post not ok status code {}, body {}", resp->status, resp->body);
				std::this_thread::sleep_for(std::chrono::seconds(5));
				continue;
			}
			break;
		}

		//
		if (!resp) {
			loge("send http failed");
			return false;
		}
		if (resp->status != 200) {
			loge("post not ok status code {}, body {}", resp->status, resp->body);
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
					msg += source.sub_title + TEXT(" 失败\n");
					msg += TEXT("错误消息: ") + av::str::toT(message);
					sendTGWaringMessage(msg);
					return false;
				}
			}
			else if (j["code"].is_string()) {
				auto code = j["code"].get<std::string>();
				if (code != "0") {
					// here, the website result string
					logw("code {} message {}", code, message);
					std::tstring msg = TEXT("发布 ");
					msg += title + TEXT("\n");
					msg += source.sub_title + TEXT(" 失败\n");
					msg += TEXT("错误消息: ") + av::str::toT(message);
					sendTGWaringMessage(msg);
					return false;
				}
			}
			else {
				std::tstring msg = TEXT("发布 ");
				msg += title + TEXT("\n");
				msg += source.sub_title + TEXT(" 失败\n");
				msg += TEXT("错误消息: 无法解析返回结果(") + av::str::toT(resp->body) + TEXT(")");
				sendTGWaringMessage(msg);
				return false;
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
		}
		catch (const json::parse_error& e) {
			logw("parse_error {}, {}", e.what(), resp->body);
			return false;
		}
		catch (const std::exception& e) {
			logw("exception {}, {}", e.what(), resp->body);
			return false;
		}

		// 发送成功的tg消息
		if (!sendTGMessage(source.poster_img, source.screenshot_local, response_id, response_name,
			response_sub_title, response_create_date)) {
			logw("sendTGMessage failed");
		}

		// 如果有警告消息,发送tg警告消息
		if (!warning_msg.empty()) {
			sendTGWaringMessage(warning_msg);
		}

		return true;
	}

	bool Mteam::sendTGMessage(const std::tstring& douban_poster_img,
		const std::vector<std::tstring>& screenshots,
		const std::tstring& publish_id,
		const std::tstring& title,
		const std::tstring& sub_title,
		const std::tstring& create_date) {
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

	//
	bool Mteam::name(const av::media::Source& source, PublishName& publish_name) {
		mteam::video::Codec video_codec; video_codec.setSourceVideoCodec(source.video_codec);
		mteam::audio::Codec audio_codec; audio_codec.setSourceCodec(source.audio_codec);
		mteam::video::Resolution video_resolution; video_resolution.setSourceResolution(source.video_resolution);
		mteam::Category category; category.setSourceCategory(source.category);
		mteam::Source m_source; m_source.setExternalSourceId(source.source_id);

		auto title_name_vec = av::str::split(source.name_eng, TEXT(" "));
		std::tstring new_name = av::str::join(title_name_vec, TEXT("."));
		for (auto& t : ex_include) {
			av::str::replace_all(new_name, t, TEXT(""));
		}

		// names
		std::vector<std::tstring> title_name;
		std::vector<std::tstring> video_name;
		std::vector<std::tstring> torrent_name;
		std::vector<std::tstring> torrent_dir_;
		if (!source.title_prefix.empty()) {
			title_name.push_back(source.title_prefix);
			auto tmp = source.title_prefix;
			av::str::replace_all(tmp, TEXT(" "), TEXT("."));
			video_name.push_back(tmp);
			torrent_name.push_back(tmp);
			torrent_dir_.push_back(tmp);
		}
		title_name.push_back(source.name_eng);
		video_name.push_back(new_name);
		torrent_name.push_back(new_name);
		torrent_dir_.push_back(new_name);

		if (!source.season.empty()) {
			title_name.push_back(source.season);
			video_name.push_back(source.season);
			torrent_name.push_back(source.season);
			torrent_dir_.push_back(TEXT("Complete"));
		}

		title_name.push_back(source.year);
		video_name.push_back(source.year);
		torrent_name.push_back(source.year);
		torrent_dir_.push_back(source.year);

		auto source_text = m_source.getText();
		if (!source_text.empty()) {
			title_name.push_back(source_text);
			video_name.push_back(source_text);
			torrent_name.push_back(source_text);
			torrent_dir_.push_back(source_text);
		}

		title_name.push_back(video_resolution.getText());
		video_name.push_back(video_resolution.getText());
		torrent_name.push_back(video_resolution.getText());
		torrent_dir_.push_back(video_resolution.getText());


		title_name.push_back(audio_codec.getText()); {
			auto tmp = audio_codec.getText();
			av::str::replace_all(tmp, TEXT(" "), TEXT("."));
			video_name.push_back(tmp);
			torrent_name.push_back(tmp);
			torrent_dir_.push_back(tmp);
		}

		auto team_name = mteam::group::getText(source.group_id);
		title_name.push_back(video_codec.getText() + TEXT("-") + team_name);
		video_name.push_back(video_codec.getText() + TEXT("-") + team_name);
		torrent_dir_.push_back(video_codec.getText() + TEXT("-") + team_name);
		torrent_name.push_back(video_codec.getText() + TEXT("-") + team_name);

		// 做种目录名
		if (!source.season.empty()) {
			publish_name.torrent_dir = av::str::join(torrent_dir_, TEXT(".")); // 剧集
		}
		else {
			publish_name.torrent_dir = av::str::join(video_name, TEXT("."));
		}

		// 标题
		publish_name.title = av::str::join(title_name, TEXT(" "));

		// 视频文件名
		video_name.push_back(source.file_suffix);
		publish_name.video_filename = av::str::join(video_name, TEXT("."));

		// 种子文件名
		torrent_name.push_back(TEXT("torrent"));
		publish_name.torrent_filename = av::str::join(torrent_name, TEXT("."));
		return true;
	}

	void Mteam::sendTGWaringMessage(const std::tstring& msg) const {
		const std::string text = "失败警告: " + av::str::toA(msg);
		av::tgbot::send_message(av::str::toA(m_tgbot_token),
			av::str::toA(m_tgbot_chat_id),
			text);
	}

	Mteam::~Mteam() {
	}
}
