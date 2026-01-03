#include "mteam.h"

#include "fmt/format.h"
#include "fmt/ranges.h"

#include "av_path.h"
#include "av_log.h"
#include "av_libtorrent.h"
#include "av_http.h"
#include "av_tgbot.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

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
		const std::tstring& tgbot_chat_id ) :
		m_api_url(api_url),
		m_api_key(api_key),
		m_img_api_url(img_api_url),
		m_img_api_key(img_api_key),
		m_tgbot_token(tgbot_token),
		m_tgbot_chat_id(tgbot_chat_id)
	{
		
	}

	bool Mteam::publish(const av::media::Source& source) {
		m_external_source = source;
		//m_category = std::make_shared<Category>(m_source.category);
		Category category(source.category);
		//CategoryId category_id = category.getid();
		if (source.type == av::media::SourceType::File) {
			std::tstring torrent_dir;
			std::tstring title;
			std::tstring video_filename;
			std::tstring torrent_filename;
			size_t last_status = 0;

			// 命名
			parseName(torrent_dir, title, video_filename, torrent_filename);

			if (av::path::exists(torrent_dir)) {
				loge("dir {} exists", av::str::toA(torrent_dir));
				return false;
			}

			// 做种目录
			auto seed_dir = av::path::append(source.seed_dir, torrent_dir);
			if (!av::path::create_dir(seed_dir)) {
				loge("create dir {} failed", av::str::toA(seed_dir));
				return false;
			}

			// 移动视频,并修改名称
			auto video_file = av::path::append(seed_dir, video_filename);
			if (!av::path::move_file(m_external_source.fullpath, video_file)) {
				loge("move file{} to {} failed", av::str::toA(m_external_source.fullpath), av::str::toA(video_file));
				return false;
			}

			// 制作种子文件
			auto torrent_file = av::path::append(m_external_source.dir, torrent_filename);
			if (!av::libtorrent::create_torrent(av::str::toA(seed_dir), av::str::toA(torrent_file))) {
				loge("create torrent file failed, {}, {}", av::str::toA(seed_dir), av::str::toA(torrent_file));
				return false;
			}

			// 上传图片
			std::vector<std::tstring> img_url;
			UploadImg img(m_img_api_url, m_img_api_key);
			for (auto& i : m_external_source.screenshot_local) {
				std::tstring tmp;
				if (!img.Upload(i, tmp)) {
					loge("upload img {} failed", av::str::toA(i));
					continue;
				}
				if (!tmp.empty()) {
					img_url.push_back(tmp);
				}
			}
			std::tstring description = av::str::join(img_url, TEXT("\n"));
			if (!m_external_source.description.empty()) {
				// 组合豆瓣的描述
				description = m_external_source.description + TEXT("\n\n") + description;
			}

			// 发布到 m-team
			av::http::Client client;
			av::http::Response resp;
			av::http::FormData form;
			{
				auto category_id = category.getid();
				form.data[TEXT("category")] = av::str::toT(std::to_string(static_cast<int64_t>(category_id)));
				form.data[TEXT("name")] = title;
				form.data[TEXT("smallDescr")] = m_external_source.sub_title;
				form.data[TEXT("dmmCode")] = TEXT("");
				auto source_id = m_source.getid();
				form.data[TEXT("source")] = av::str::toT(std::to_string(static_cast<int64_t>(source_id)));
				auto standard_id = m_video_resolution.getid();
				form.data[TEXT("standard")] = av::str::toT(std::to_string(static_cast<int64_t>(standard_id)));
				form.data[TEXT("videoCodec")] = av::str::toT(std::to_string(static_cast<int64_t>(m_video_codec.getid())));
				form.data[TEXT("audioCodec")] = av::str::toT(std::to_string(static_cast<int64_t>(m_audio_codec.getid())));
				form.data[TEXT("team")] = av::str::toT(std::to_string(m_external_source.group_id));
				form.data[TEXT("imdb")] = m_external_source.imdb_link;
				char buff[2048];
				snprintf(buff, sizeof(buff)-1, "https://movie.douban.com/subject/%s/", av::str::toA(m_external_source.douban_id).c_str());
				form.data[TEXT("douban")] = av::str::toT(std::string(buff));
				form.data[TEXT("labelsNew")] = TEXT("");
				form.data[TEXT("mediainfo")] = m_external_source.mediainfo_text;
				form.data[TEXT("tags")] = TEXT("");
				form.data[TEXT("anonymous")] = TEXT("true");
				form.data[TEXT("aids")] = TEXT("");
				form.data[TEXT("descr")] = description;
				form.data[TEXT("mediaInfoAnalysisResult")] = TEXT("true");
				if (category.getid() == CategoryId::Movie) {
					form.data[TEXT("labels")] = TEXT("0");
					form.data[TEXT("labelsNew")] = TEXT("中配");
				}
			}
			av::http::Header header;
			header.data[TEXT("x-api-key")] = m_api_key;

			av::http::File file;
			file.data[TEXT("file")] = torrent_file;
			
			// 上传到网站
			auto url = m_api_url + TEXT("/api/torrent/createOredit");
			//auto url = m_api_url;
			logi("post url {}", av::str::toA(url));
			if (!client.postForm(url, std::make_tuple(header, form, file), resp)) {
				loge("send http failed");
				return false;
			}
			if (!resp.isOk()) {
				loge("post not ok");
				return false;
			}

			// 解析返回
			std::tstring response_id;
			std::tstring response_name;
			std::tstring response_sub_title;
			std::tstring response_create_date;
			try {
				json obj;
				auto j = obj.parse(av::str::toA( resp.body));
				if (!j.contains("code")) {
					logw("parse error {}", av::str::toA(resp.body));
					return false;
				}
				std::string message;
				if (j.contains("message") && j["message"].is_string()) {
					message = j["message"].get<std::string>();
				}

				auto code = j["code"].get<std::string>();
				if (code != "0") { // here, the website result string
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
			}
			catch (const json::parse_error& e) {
				logw("parse_error {}, {}", e.what(), av::str::toA(resp.body));
				return false;
			}
			catch (const std::exception& e) {
				logw("exception {}, {}", e.what(), av::str::toA(resp.body));
				return false;
			}
			
			if (!sendTGMessage(m_external_source.poster_img, img_url, response_id, response_name,
				response_sub_title, response_create_date)) {
				logw("sendTGMessage failed");
			}
		}
		return true;
	}

	bool Mteam::sendTGMessage(std::tstring& douban_poster_img,
		std::vector<std::tstring>& screenshots,
		std::tstring& publish_id,
		std::tstring& title,
		std::tstring& sub_title,
		std::tstring& create_date) {

		// 发送tg消息
		std::tstring text;
		{
			std::string text_tmp = R"(
🔗 链接: https://kp.m-team.cc/detail/%s
🔧 標題: %s
🎫 副標題: %s
⏲ 发布时间: %s
)";
			char buff[32768];
			sprintf(buff, text_tmp.c_str(), av::str::toA(publish_id), av::str::toA(title), av::str::toA(sub_title), av::str::toA(create_date));
			text = av::str::toT(std::string(buff));
		}

		// use douban img
		if (!douban_poster_img.empty()) {
			if (!av::tgbot::send_net_photo_message(av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
				av::str::toA(douban_poster_img), av::str::toA(text))) {
				logw("send_net_photo_message send failed!!! {}, {}, {}, {}", av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
					av::str::toA(douban_poster_img), av::str::toA(text));
				return false;
			}
			return true;
		}

		// use screenshots
		if (!screenshots.empty()) {
			auto img = screenshots[0];
			if (!av::tgbot::send_local_photo_message(av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
				av::str::toA(img), av::str::toA(text))) {
				logw("send_net_photo_message send failed!!! {}, {}, {}, {}", av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
					av::str::toA(img), av::str::toA(text));
				return false;
			}
		}

		// use defualt m-team logo
		std::tstring img = TEXT("https://static.m-team.cc/static/media/logo.80b63235eaf702e44a8d.png");
		if (!av::tgbot::send_net_photo_message(av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
			av::str::toA(img), av::str::toA(text))) {
			logw("send_net_photo_message send failed!!! {}, {}, {}, {}", av::str::toA(m_tgbot_token), av::str::toA(m_tgbot_chat_id),
				av::str::toA(img), av::str::toA(text));
			return false;
		}

		return true;
	}

	bool Mteam::parseName(std::tstring& torrent_dir, 
			std::tstring& title, 
			std::tstring& video_filename,
			std::tstring& torrent_filename
	) {
		m_video_codec.setSourceVideoCodec(m_external_source.video_codec);
		m_video_resolution.setSourceResolution(m_external_source.video_resolution);
		m_audio_codec.setSourceCodec(m_external_source.audio_codec);
		m_cateogry.setSourceCategory(m_external_source.category);
		m_source.setExternalSourceId(m_external_source.source_id);

		auto title_name_vec = av::str::split(m_external_source.name_eng, TEXT(" "));
		std::tstring new_name = av::str::join(title_name_vec, TEXT("."));
		for (auto& t : ex_include) {
			av::str::replace_all(new_name, t, TEXT(""));
		}

		// names
		std::vector<std::tstring> title_name;
		std::vector<std::tstring> video_name;
		std::vector<std::tstring> torrent_name;
		if (!m_external_source.title_prefix.empty()) {
			title_name.push_back(m_external_source.title_prefix);
			auto tmp = m_external_source.title_prefix;
			av::str::replace_all(tmp, TEXT(" "), TEXT("."));
			video_name.push_back(tmp);
			torrent_name.push_back(tmp);
		}
		title_name.push_back(m_external_source.name_eng);
		video_name.push_back(new_name);
		torrent_name.push_back(new_name);

		if (!m_external_source.season.empty()) {
			title_name.push_back(m_external_source.season);
			video_name.push_back(m_external_source.season);
			torrent_name.push_back(m_external_source.season);
		}

		auto source_text = m_source.getText();
		if (!source_text.empty()) {
			title_name.push_back(source_text);
			video_name.push_back(source_text);
			torrent_name.push_back(source_text);
		}

		title_name.push_back(m_video_resolution.getText());
		video_name.push_back(m_video_resolution.getText());
		torrent_name.push_back(m_video_resolution.getText());


		title_name.push_back(m_audio_codec.getText());
		{
			auto tmp = m_audio_codec.getText();
			av::str::replace_all(tmp, TEXT(" "), TEXT("."));
			video_name.push_back(tmp);
			torrent_name.push_back(tmp);
		}

		auto team_name = mteam::group::getText(m_external_source.group_id);
		title_name.push_back(m_video_codec.getText() + TEXT("-") + team_name);
		video_name.push_back(m_video_codec.getText());
		video_name.push_back(team_name);

		torrent_name.push_back(m_video_codec.getText());
		torrent_name.push_back(team_name);
		torrent_name.push_back(TEXT("torrent"));

		// 做种目录名
		torrent_dir = av::str::join(video_name, TEXT("."));
		// 标题
		title = av::str::join(title_name, TEXT(" "));
		// 视频文件名
		video_name.push_back(m_external_source.file_suffix);
		video_filename = av::str::join(video_name, TEXT("."));
		// 种子文件名
		torrent_filename = av::str::join(torrent_name, TEXT("."));
		return true;
	}

	Mteam::~Mteam()
	{

	}
}
