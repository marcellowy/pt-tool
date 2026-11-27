#include "publish.h"

#include <filesystem>
#include <fstream>
#include <chrono>

#include "boost/locale.hpp"

#include "av_libtorrent.h"
#include "av_log.h"
#include "av_string.h"
#include "av_path.h"
#include "av_async.h"
#include "av_time.h"
#include "av_mediainfo.h"
#include "av_media_info.h"
#include "av_pt_gen.h"
#include "av_ffmpeg.h"
#include "av_codec_stb_image_jpg.h"
#include "av_md5.h"
#include "av_translate.h"
#include "av_env.h"
#include "av_time.h"
#include "av_file.h"
#include "av_pt_gen_mteam.h"
#include "av_worker.h"
#include "av_tgbot.h"
#include "av_utils.h"
#include "error_code.h"
#include "nlohmann/json.hpp"


#include "config.h"
#include "parse_name.h"

static std::tstring getTmpDir(const Source& obj);

std::tstring getTmpDir(const Source& obj) {
	std::tstring dir = av::path::get_exe_dir();
	dir = av::path::append(dir, TEXT("tmp"));
	dir = av::path::append(dir, obj.fullpath_md5);
	return dir;
}

Publish::Publish() {
}

Publish::Publish(const std::tstring& dir) : m_dir(dir) {

}

Publish::Publish(std::shared_ptr<Site>& site, const std::tstring& dir, const std::tstring& tgbot_token, const std::tstring& tgbot_chat_id)
	: m_site(std::move(site)), m_dir(dir), m_tgbot_token(tgbot_token), m_tgbot_chat_id(tgbot_chat_id) {
}

Publish::~Publish() {
	stop();
}

void Publish::task() {
	logi("publish task start");
	PublishError error = PublishError::Success;
	auto arr = readDir(error);
	logi("read dir {} succ, count {}", av::str::toA(m_dir), arr.size());

	//
	auto& config = Config::instance();

	//
	for (auto& source : arr) {
		// rollback tasks
		{
			bool publish_success = false;
			std::vector<std::pair<std::string, std::function<void()>>> rollback_tasks;
			av::async::Exit exit_publish([&publish_success, &rollback_tasks, &config] {
				if (!publish_success || config.debug) {
					logi("publish failed, rollback tasks size = {}", rollback_tasks.size());

					// 倒序先进后出执行
					for (auto it = rollback_tasks.rbegin(); it != rollback_tasks.rend(); ++it) {
						try {
							if (it->second)
							{
								it->second();
								logi("rollback task \"{}\" succ", it->first);
							}
						}
						catch (const std::exception& e) {
							logw("rollback task \"{}\" failed, err: {}", it->first, e.what());
						}
					}
				}
				rollback_tasks.clear();
				});

			if (source.type == SourceType::File) {
				if (config.debug && config.debug_type != 0) {
					logi("debug type is {}, skip file {}", config.debug_type, av::str::toA(source.fullpath));
					continue;
				}
				if (!processFile(source, rollback_tasks, error)) {
					logw("process file failed, dir {}, name {}", av::str::toA(source.dir), av::str::toA(source.name));
					sendTGWarningMessage(av::str::toA(source.fullpath), error);
					continue;
				}
			}
			else if (source.type == SourceType::Dir) {
				if (config.debug && config.debug_type != 1) {
					logi("debug type is {}, skip dir {}", config.debug_type, av::str::toA(source.fullpath));
					continue;
				}
				const auto ret = processDir(source, rollback_tasks, error);
				if (ret == static_cast<int>(ErrorCode::ErrTimeNotReached)) {
					logw("{} time not reached", av::str::toA(source.name));
					continue;
				}
				if (ret != static_cast<int>(ErrorCode::Success)) {
					logw("process dir failed, dir {}, name {}", av::str::toA(source.dir), av::str::toA(source.name));
					sendTGWarningMessage(av::str::toA(source.fullpath), error);
					continue;
				}
			}
			logi("process dir/file succ, dir {}, name {}", av::str::toA(source.dir), av::str::toA(source.name));

			// check param
			if (source.audio_codec == SourceAudioCodec::Unknown ||
				source.category == SourceCategory::Unknown ||
				source.group_id == 0 ||
				source.name_eng.empty() ||
				source.source_id == SourceId::Unknown ||
				source.video_codec == SourceVideoCodec::Unknown ||
				source.video_resolution == SourceVideoResolution::Unknown
				) {
				logi(
					"audio_codec {}, category {}, group id {}, name eng {}, source id {}, video codec {}, video resolution {}",
					static_cast<int>(source.audio_codec),
					static_cast<int>(source.category),
					source.group_id,
					av::str::toA(source.name_eng),
					static_cast<int>(source.source_id),
					static_cast<int>(source.video_codec),
					static_cast<int>(source.video_resolution)
				);
				logw("{} param error, please check", av::str::toA(source.fullpath));

				// send tg warning
				sendTGWarningMessage(av::str::toA(source.fullpath), PublishError::ErrParam);
				continue;
			}

			// preprocess succ
			logi("begin publish");
			if (m_site->publish(source)) {
				if (!config.debug) {
					auto dir = getTmpDir(source);
					if (av::path::remove_dir_all(dir)) {
						logi("clean tmp dir {} succ", av::str::toA(dir));
					}
				}

				// if succ break,
				logi("publish succ, dir {}, name {}", av::str::toA(source.dir), av::str::toA(source.name));
				publish_success = true;
				break;
			}

			//
			sendTGWarningMessage(av::str::toA(source.fullpath), PublishError::ErrPublishFailed);

			// if incorrect, just use next file keep publish
			logi("publish failed, dir {}, name {}", av::str::toA(source.dir), av::str::toA(source.name));
		}
	}
}

bool Publish::start() {
	auto& config = Config::instance();
	for (auto& cycle : config.mteam.publish_cycle) {
		auto c = std::make_shared<av::cron::Cron>(cycle.name, cycle.pattern, [this] {
			logi("do task");

			// 如果有重复任务执行，就跳过本次执行，避免重复执行
			av::utils::ScopedTryFlag guard(m_scoped_flag);
			if (!guard) {
				logi("task is running, skip this time");
				return;
			};
			// has bug
			// sometimes task will block, so use sync to avoid block
			//av::worker::global_worker().sync([this] { task();}, 1000 * 60 * 10);    // 10 minutes
			task();
			});
		if (!c->start()) {
			logw("add cron {} failed!", av::str::toA(cycle.name));
			return false;
		}
		m_cron.push_back(c);
	}
	return true;
}

bool Publish::stop() {
	for (auto& c : m_cron) {
		c->stop();
	}
	m_cron.clear();
	return true;
}

bool Publish::getSiteType(Source& obj) {
	if (obj.type == SourceType::Dir) {
		return false;
	}

	av::async::Exit exit_filter_name([&obj] {
		av::str::replace_all(obj.name_chs, TEXT("(4k)"), TEXT("（4K）"));
		});

	std::tstring pre = TEXT("");

	// Sport
	pre = obj.name.substr(0, 2);
	if (pre == publishPrefixSport) {
		obj.category = av::media::SourceCategory::Sport;
		//obj.category_id = mteam::category::Id::Sport;
		if (!parseSportName(obj)) {
			loge("publishSportName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
			return false;
		}
		return true;
	}

	// Variety
	pre = obj.name.substr(0, 3);
	if (pre == publishPrefixVariety) {
		obj.category = av::media::SourceCategory::Variety;
		//obj.category_id = mteam::category::Id::TVSeries;
		if (!parseVarietyName(obj)) {
			loge("parseVarietyName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
			return false;
		}
		return true;
	}

	// Discover
	pre = obj.name.substr(0, 4);
	if (pre == publishPrefixDiscover) {
		obj.category = av::media::SourceCategory::Discover;
		//obj.category_id = mteam::category::Id::Discover;
		if (!parseDiscoverName(obj)) {
			loge("parseDiscoverName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
			return false;
		}
		return true;
	}

	// Custom
	pre = obj.name.substr(0, 5);
	if (pre == publishPrefixCustom) {
		if (!parseCustomName(obj)) {
			loge("parseCustomName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
			return false;
		}
		return true;
	}

	// Movie
	if (obj.name.find(TEXT("€")) != std::tstring::npos) {
		obj.category = av::media::SourceCategory::Movie;
		//obj.category_id = mteam::category::Id::Movie;
		if (!parseMovieName(obj)) {
			loge("parseMovieName failed! {},{}", av::str::toA(obj.dir), av::str::toA(obj.name));
			return false;
		}
		return true;
	}
	return false;
}

int Publish::processDir(Source& source,
	std::vector<std::pair<std::string, std::function<void()>>>& rollback_tasks,
	PublishError& error) {

	// config
	auto& config = Config::instance();
	logi("process {}", av::str::toA(source.fullpath));

	// 临时目录 
	auto tmp_dir = getTmpDir(source);
	if (!av::path::exists(tmp_dir)) {
		if (!av::path::create_dir(tmp_dir)) {
			loge("create dir {} failed", av::str::toA(tmp_dir));
			return false;
		}
	}

	//
	source.source_id = av::media::from(config.mteam.source_id);
	source.group_id = config.mteam.group_id;
	source.seed_dir = config.mteam.seed_dir;

	// media_info.json, media_info.txt
	auto media_info_json_path = av::path::append(source.fullpath, TEXT("media_info.json"));
	auto media_info_text_path = av::path::append(source.fullpath, TEXT("media_info.txt"));
	if (!av::path::exists(media_info_json_path) || !av::path::exists(media_info_text_path)) {
		logw("{} or {} not exists",
			av::str::toA(media_info_json_path), av::str::toA(media_info_text_path));
		error = PublishError::ErrNoMediaInfo;
		return -1;
	}

	// read file
	std::string json_content;
	fs::path json_path(media_info_json_path);
	if (!av::file::read(json_path, json_content)) {
		logw("read file {} failed", av::str::toA(media_info_json_path));
		error = PublishError::ErrNoMediaInfo;
		return -1;
	}
	std::string text_content;
	fs::path text_path(media_info_text_path);
	if (!av::file::read(text_path, text_content)) {
		logw("read file {} failed", av::str::toA(media_info_text_path));
		error = PublishError::ErrNoMediaInfo;
		return -1;
	}
	source.mediainfo_text = av::str::toT(text_content);
	logi("json content: \n{}", json_content);
	logi("text content: \n{}", text_content);

	// parse json file
	MediaInfo media_info;
	try {
		auto j = json::parse(json_content);
		media_info = j.get<MediaInfo>();
	}
	catch (const json::parse_error& e) {
		logw("json::parse_error {}", e.what());
		error = PublishError::ErrParseJson;
		return -1;
	}
	catch (const std::exception& e) {
		logw("std::exception {}", e.what());
		error = PublishError::ErrParseJson;
		return -1;
	}

	// check time
	int64_t diff_time_seconds;
	if (!av::time::diff_now(av::str::toA(media_info.created_time), diff_time_seconds)) {
		logw("diff time failed, time: {}", av::str::toA(media_info.created_time));
		error = PublishError::ErrParseCreatedTime;
		return -1;
	}

	if (diff_time_seconds < 24 * 3600) {
		// 24 hours
		logw("time not reached! wait time, {}", av::str::toA(source.fullpath));
		return static_cast<int>(ErrorCode::ErrTimeNotReached);
	}

	av::media::ScanType scan_type;
	if (media_info.scan_type == TEXT("Interlaced")) {
		scan_type = av::media::ScanType::Interlaced;
	}
	else if (media_info.scan_type == TEXT("MBAFF")) {
		scan_type = av::media::ScanType::MBAFF;
	}
	else if (media_info.scan_type == TEXT("Progressive")) {
		scan_type = av::media::ScanType::Progressive;
	}
	else {
		scan_type = av::media::ScanType::Unknown;
	}
	//
	setResolution(media_info.width, media_info.height, scan_type, source);

	// video codec
	if (av::utils::iequals(media_info.video_format, std::tstring(TEXT("AVS2")))) {
		source.video_codec = SourceVideoCodec::_avs2;
	}
	else if (av::utils::iequals(media_info.video_format, std::tstring(TEXT("CAVS")))) {
		source.video_codec = SourceVideoCodec::_cavs;
	}
	else if (av::utils::iequals(media_info.video_format, std::tstring(TEXT("AVS")))) {
		source.video_codec = SourceVideoCodec::_avs;
	}
	else if (av::utils::iequals(media_info.video_format, std::tstring(TEXT("MPEG2VIDEO")))) {
		source.video_codec = SourceVideoCodec::_mpeg2;
	}
	else if (av::utils::iequals(media_info.video_format, std::tstring(TEXT("h264")))) {
		source.video_codec = SourceVideoCodec::_h264;
	}
	else if (av::utils::iequals(media_info.video_format, std::tstring(TEXT("h265")))) {
		source.video_codec = SourceVideoCodec::_h265;
	}
	else if (av::utils::iequals(media_info.video_format, std::tstring(TEXT("AVS3")))) {
		source.video_codec = SourceVideoCodec::_avs3;
	}
	else {
		// default avs
		logw("video codec {} not support",
			av::str::toA(media_info.video_format));
		error = PublishError::ErrVideoCodecNotSupport;
		return -1;
	}

	// audio codec
	if (av::utils::iequals(media_info.audio_format, std::tstring(TEXT("AC3")))) {
		source.audio_codec = SourceAudioCodec::_ac3;
	}
	else if (av::utils::iequals(media_info.audio_format, std::tstring(TEXT("MP2"))))
	{
		source.audio_codec = SourceAudioCodec::_mp2;
	}
	else if (av::utils::iequals(media_info.audio_format, std::tstring(TEXT("MP3"))))
	{
		source.audio_codec = SourceAudioCodec::_mp3;
	}
	else if (av::utils::iequals(media_info.audio_format, std::tstring(TEXT("AAC"))))
	{
		source.audio_codec = SourceAudioCodec::_aac;
	}
	else if (av::utils::iequals(media_info.audio_format, std::tstring(TEXT("E-AC3(DDP)"))))
	{
		source.audio_codec = SourceAudioCodec::_e_ac3_ddp;
	}
	else if (av::utils::iequals(media_info.audio_format, std::tstring(TEXT("E-AC3 Atmos"))))
	{
		source.audio_codec = SourceAudioCodec::_e_ac3_atmos;
	}
	else {
		logw("audio codec {} not support",
			av::str::toA(media_info.audio_format));
		error = PublishError::ErrAudioCodecNotSupport;
		return -1;
	}

	// category
	source.category = SourceCategory::Unknown; // default
	if (media_info.category == 402) {
		source.category = SourceCategory::TVSeries;
	}
	else if (media_info.category == 404) {
		source.category = SourceCategory::Discover;
	}
	else if (media_info.category == 407) {
		source.category = SourceCategory::Sport;
	}
	else if (media_info.category == 419) {
		source.category = SourceCategory::Movie;
	}
	else {
		logw("category {} not support", media_info.category);
		error = PublishError::ErrCategoryNotSupport;
		return -1;
	}

	//
	source.title_prefix = media_info.channel;
	source.name_chs = media_info.title;
	source.sub_title = media_info.title;
	source.year = media_info.year;
	if (!media_info.douban_id.empty()) {
		source.douban_id = av::str::toT(media_info.douban_id);
	}

	//
	if (!source.douban_id.empty()) {
		av::ptgen::Douban douban_info;
		if (!m_site->douban(av::str::toA(source.douban_id), douban_info)) {
			loge("get douban info failed!");
			error = PublishError::ErrGetDoubanInfo;
			return -1;
		}

		// set douban info to obj
		source.year = av::str::toT(douban_info.year);
		source.name_chs = av::str::toT(douban_info.name_chs);
		source.name_eng = av::str::toT(douban_info.name_eng);
		source.sub_title = av::str::toT(douban_info.sub_title);
		source.imdb_link = av::str::toT(douban_info.imdb_link);
		source.description = av::str::toT(douban_info.description);
		source.poster_img = av::str::toT(douban_info.poster_img);
	}

	//
	tvname(source);

	// read dir
	int64_t ts_count = 0;
	std::vector<std::tstring> img_vec;
	for (const auto& entry : fs::directory_iterator(source.fullpath)) {
#ifdef _UNICODE
		std::tstring name = entry.path().filename().wstring();
		std::tstring ext = entry.path().extension().wstring();
#else
		std::tstring name = entry.path().filename().string();
		std::tstring ext = entry.path().extension().string();
#endif // _UNICODE
		std::tstring fullpath = av::path::append(source.fullpath, name);

		// process ts file
		if (av::utils::iequals(ext, std::tstring(TEXT(".ts")))) {
			ts_count++;
		}

		// process image file
		if (av::utils::iequals(ext, std::tstring(TEXT(".jpg"))) ||
			av::utils::iequals(ext, std::tstring(TEXT(".jpeg"))) ||
			av::utils::iequals(ext, std::tstring(TEXT(".png")))) {
			img_vec.push_back(fullpath);
		}

		// 判断文件是不是需要删除的文件或目录, 如果是就删除它
		auto& config = Config::instance();
		for (const auto& item : config.download.delete_in_dir) {
			const auto item_path = av::str::toA(item);
			if (av::utils::iequals(entry.path().filename().string(), item_path)) {
				try {
					if (entry.is_directory())
						fs::remove_all(entry.path());	// 删除目录 
					else
						fs::remove(entry.path());	// 删除文件
					logi("remove file {} success!", av::str::toA(entry.path()));
				}
				catch (const std::exception& e) {
					logw("remove file {} failed! {}", av::str::toA(entry.path()), e.what());
				}
				continue;
			}
		}
	}

	if (ts_count == 0) {
		// no ts
		logw("no ts file");
		error = PublishError::ErrNoTSFile;
		return -1;
	}

	// add english name
	if (source.name_eng.empty()) {
		av::translate::Translate t(config.rapidapi.key, config.rapidapi.host);
		if (!t.foo(source.name_chs, source.name_eng)) {
			loge("translate failed");
			error = PublishError::ErrTranslate;
			return -1;
		}
	}
	if (!empty(source.name_eng)) {
		capitalizeWords(source.name_eng);
	}

	auto move_image = [&source, &rollback_tasks, &img_vec]() -> bool {
		// create dir
		auto dir = getTmpDir(source);
		if (!av::path::exists(dir)) {
			if (!av::path::create_dir(dir)) {
				loge("create dir {} failed", av::str::toA(dir));
				return false;
			}
		}

		// move
		for (auto& img : img_vec) {
			fs::path i = img;
#ifdef _UNICODE
			std::tstring tmp_name = i.filename().wstring();
#else
			std::tstring tmp_name = i.filename().string();
#endif // _UNICODE

			auto tmp = av::path::append(dir, tmp_name);
			if (!av::path::move_file(img, tmp, true)) {
				loge("move file {} to {} failed", av::str::toA(img), av::str::toA(tmp));
				return false;
			}
			rollback_tasks.push_back(std::make_pair("rollback image", [img, tmp]() {
				if (!av::path::move_file(tmp, img, true)) {
					logw("rollback move file {} to {} failed", av::str::toA(tmp), av::str::toA(img));
				}
				}));
			source.screenshot_local.push_back(tmp);
		}
		logi("move_image succ");
		return true;
		};

	// move img
	if (!move_image()) {
		logw("move image failed");
		error = PublishError::ErrMoveImage;
		return -1;
	}

	//
	const auto media_info_tmp_json_file = av::path::append(tmp_dir, TEXT("media_info.json"));
	if (av::path::move_file(media_info_json_path, media_info_tmp_json_file, true)) {
		const std::string task_name = "rollback media_info.json -> " + av::str::toA(media_info_json_path);
		rollback_tasks.push_back(std::make_pair(task_name, [media_info_json_path, media_info_tmp_json_file] {
			av::path::move_file(media_info_tmp_json_file, media_info_json_path, true);
			}));
	}
	else {
		logw("move file {} to {} failed",
			av::str::toA(media_info_json_path),
			av::str::toA(media_info_tmp_json_file));
		error = PublishError::ErrMoveFile;
		return -1;
	}

	//
	const auto media_info_tmp_txt_file = av::path::append(tmp_dir, TEXT("media_info.txt"));
	if (av::path::move_file(media_info_text_path, media_info_tmp_txt_file, true)) {
		const std::string task_name = "rollback media_info.txt -> " + av::str::toA(media_info_text_path);
		rollback_tasks.push_back(std::make_pair(task_name, [media_info_text_path, media_info_tmp_txt_file] {
			av::path::move_file(media_info_tmp_txt_file, media_info_text_path, true);
			}));
	}
	else {
		logw("move file {} to {} failed",
			av::str::toA(media_info_text_path),
			av::str::toA(media_info_tmp_txt_file));
		error = PublishError::ErrMoveFile;
		return -1;
	}

	// 获取当前站点的发布名称
	PublishName publish_name;
	if (!m_site->name(source, publish_name)) {
		error = PublishError::ErrContactName;
		return -1;
	}

	// 移动目录为种子目录
	auto seed_dir = av::path::append(source.seed_dir, publish_name.torrent_dir);
	try {
		fs::rename(source.fullpath, seed_dir);
		//  添加回滚任务
		const std::string task_name = "rollback rename" + av::str::toA(source.fullpath);
		rollback_tasks.push_back(std::make_pair(task_name, [seed_dir, fullpath = source.fullpath] {
			fs::rename(seed_dir, fullpath);
			}));
	}
	catch (const fs::filesystem_error& e) {
		logw("rename {}, {} failed, err: {}", av::str::toA(source.fullpath), av::str::toA(seed_dir),
			e.what());
		return false;
	}
	catch (const std::exception& e) {
		logw("rename {}, {} failed, err: {}", av::str::toA(source.fullpath), av::str::toA(seed_dir),
			e.what());
		return false;
	}

	// 制作种子文件,直接放在临时目录下
	source.torrent_file = av::path::append(tmp_dir, publish_name.torrent_filename);
	logi("create torrent file, seed dir {}, torrent file {}",
		av::str::toA(seed_dir),
		av::str::toA(source.torrent_file));
	if (!av::libtorrent::create_torrent(av::str::toA(seed_dir), av::str::toA(source.torrent_file))) {
		loge("create torrent file failed, {}, {}", av::str::toA(seed_dir), av::str::toA(source.torrent_file));
		return false;
	}

	//
	logi("pre-process dir succ");
	return static_cast<int>(ErrorCode::Success);
}

bool Publish::processFile(Source& source,
	std::vector<std::pair<std::string, std::function<void()>>>& rollback_tasks,
	PublishError& error) {
	auto& config = Config::instance();
	logi("process {}", av::str::toA(source.fullpath));

	//
	auto tmp_dir = getTmpDir(source);
	if (!av::path::create_dir(tmp_dir)) {
		loge("create dir {} failed", av::str::toA(tmp_dir));
		return false;
	}

	//
	source.source_id = av::media::from(config.mteam.source_id);
	source.group_id = config.mteam.group_id;
	source.seed_dir = config.mteam.seed_dir;

	// site type
	if (!getSiteType(source)) {
		logw("dir {}, name {} get site type failed", av::str::toA(source.dir), av::str::toA(source.name));
		error = PublishError::ErrGetSiteType;
		return false;
	}

	// tv name
	tvname(source);

	// mediainfo
	av::mediainfo::MediaInfo m(source.fullpath);
	if (!m.parse()) {
		logw("get mediainfo failed, file {}", av::str::toA(source.fullpath));
		error = PublishError::ErrNoMediaInfo;
		return false;
	}

	// video resolution
	auto video_height = m.getVideo().height;
	auto video_width = m.getVideo().width;
	setResolution(video_width, video_height, m.getVideo().scan_type, source);

	// video codec
	source.video_codec = m.getVideo().codec;

	// audio codec
	source.audio_codec = m.getAudio().codec;

	// mediainfo text
	source.mediainfo_text = m.getText();

	// add douban info
	if (!source.douban_id.empty()) {
		av::ptgen::Douban douban_info;
		if (!m_site->douban(av::str::toA(source.douban_id), douban_info)) {
			loge("get douban info failed!");
			error = PublishError::ErrGetDoubanInfo;
			return false;
		}

		// set douban info to source
		source.year = av::str::toT(douban_info.year);
		source.name_chs = av::str::toT(douban_info.name_chs);
		source.name_eng = av::str::toT(douban_info.name_eng);
		source.sub_title = av::str::toT(douban_info.sub_title);
		source.imdb_link = av::str::toT(douban_info.imdb_link);
		source.description = av::str::toT(douban_info.description);
		source.poster_img = av::str::toT(douban_info.poster_img);
	}

	// add screenshot
	// 300, 360, 420, 480, 540, 600, 660, 720, 780, 840, 900, 960
	const std::vector<int64_t> capture_time = { 60, 120, 180, 240 };
	int capture_count = 0;
	av::codec::StbPNG stbPng([&source, &tmp_dir, &capture_count](void* data, int size) {
		// screenshots dir
		std::tstringstream oo;
		oo << TEXT("screenshot_") << capture_count << TEXT(".png");
		std::tstring filename = av::path::append(tmp_dir, oo.str());
		std::ofstream out_file(av::str::toA(filename), std::ios::binary);
		out_file.write(static_cast<char*>(data), size); // 写入数据到文件

		source.screenshot_local.push_back(filename);

		capture_count++;
		});
	logi("capture frame");
	if (!av::ffmpeg::captureFrame(source.fullpath, capture_time, stbPng)) {
		loge("capture frame failed");
		error = PublishError::ErrCaptureFrame;
		return false;
	}
	logi("capture frame succ, count {}", capture_count);

	// if capture frame failed
	if (source.screenshot_local.empty()) {
		logw("capture frame failed");
		// try use ffmpeg command capture
		std::vector<std::string> tt = { "00:00:30", "00:00:45", "00:01:00", "00:01:30" };
		std::vector<std::tstring> files;
		av::ffmpeg::captureFrame(source.fullpath, tt, tmp_dir, files);
		if (!files.empty()) {
			for (const auto& file : files) {
				source.screenshot_local.emplace_back(av::str::toT(file));
			}
		}
		else {
			logw("use ffmpeg command capture frame failed");
		}
		logi("capture screenshot local size {}", source.screenshot_local.size());
	}
	else {
		logi("capture screenshot local size {}", source.screenshot_local.size());
	}

	// add english name
	if (source.name_eng.empty()) {
		std::unique_ptr<av::translate::ITranslate> translate = nullptr;
		if (config.rapidapi.channel == 2) {
			translate = av::translate::TranslateFactory::create(config.rapidapi.key, config.rapidapi.host, av::translate::TranslateType::kFreeGoogleTranslate);
		}
		else {
			translate = av::translate::TranslateFactory::create(config.rapidapi.key, config.rapidapi.host, av::translate::TranslateType::kTranslate);
		}

		//
		if (!translate->foo(source.name_chs, source.name_eng)) {
			loge("translate failed");
			error = PublishError::ErrTranslate;
			return false;
		}
	}
	if (!empty(source.name_eng)) {
		capitalizeWords(source.name_eng);
	}

	// 获取当前站点的发布名称
	PublishName publish_name;
	if (!m_site->name(source, publish_name)) {
		error = PublishError::ErrContactName;
		return false;
	}

	//
	auto seed_dir = av::path::append(source.seed_dir, publish_name.torrent_dir);
	if (av::path::exists(publish_name.torrent_dir)) {
		loge("dir {} exists", av::str::toA(publish_name.torrent_dir));
		return false;
	}
	if (!av::path::create_dir(seed_dir)) {
		loge("create dir {} failed", av::str::toA(seed_dir));
		return false;
	}

	// 移动视频到目录, 并修改名称
	auto video_file = av::path::append(seed_dir, publish_name.video_filename);
	if (!av::path::move_file(source.fullpath, video_file)) {
		loge("move file{} to {} failed", av::str::toA(source.fullpath), av::str::toA(video_file));
		return false;
	}
	// 回滚移动的视频
	rollback_tasks.push_back(std::make_pair("rollback move video", [video_file, fullpath = source.fullpath] {
		av::path::move_file(video_file, fullpath, true);
		}));

	return true;
}

std::vector<Source> Publish::readDir(PublishError& error) {
	std::vector<Source> v;
	error = PublishError::Success;

	// check
	if (!av::path::dir_exists(m_dir)) {
		logw("{} not exists", av::str::toA(m_dir));
		error = PublishError::ErrDirNotExists;
		return v;
	}

	// read
	for (const auto& entry : fs::directory_iterator(m_dir)) {
		Source obj;
#ifdef _UNICODE
		obj.name = av::str::toT(entry.path().filename().wstring());
#else
		obj.name = av::str::toT(entry.path().filename().string());
#endif // _UNICODE
		if (obj.name.size() < 5) {
			continue;
		}
		//
		if (obj.name.find(TEXT("TPTV")) != std::tstring::npos) {
			// published
			continue;
		}
		if (obj.name.substr(0, 1) == TEXT(".")) {
			// tmp file
			continue;
		}

		//
		if (entry.is_regular_file()) {
			obj.type = SourceType::File;
#ifdef _UNICODE
			obj.file_suffix = entry.path().extension().wstring();
#else
			obj.file_suffix = entry.path().extension().string();
#endif
			av::str::replace(obj.file_suffix, TEXT("."), TEXT(""));
		}
		else if (entry.is_directory()) {
			obj.type = SourceType::Dir; // 目前传目录的只有AVS, AVS就特殊处理
		}
		obj.dir = m_dir;
		//logi("dir_ {}, obj.name {}, ", dir_, obj.name);
		try {
			obj.fullpath = av::path::append(m_dir, obj.name);
			std::string tmp_md5;
			av::hash::md5(av::str::toA(obj.fullpath), tmp_md5);
			obj.fullpath_md5 = av::str::toT(tmp_md5);
		}
		catch (const std::exception& e) {
			loge("exception {}", e.what());
		}

		v.emplace_back(obj);
	}
	return v;
}

void Publish::tvname(Source& source) {
	if (source.title_prefix.empty()) {
		return;
	}
	auto& tvs = Config::instance().tv_name;
	for (auto& tv : tvs) {
		if (tv.match == source.title_prefix) {
			source.title_prefix = tv.title_prefix;
			if (source.category != av::media::SourceCategory::Movie) {
				source.sub_title = tv.sub_title_prefix + TEXT(" | ") + source.sub_title;
			}
			return;
		}
	}
	source.title_prefix = TEXT("");
}

void Publish::setResolution(int64_t width, int64_t height, const av::media::ScanType& scan_type, Source& source) {
	source.video_resolution = SourceVideoResolution::_1080i; // default
	if (height == 4320) {
		source.video_resolution = SourceVideoResolution::_8k;
	}
	else if (height == 2160 || (width == 3840 && height >= 1500 && height <= 2160)) {
		source.video_resolution = SourceVideoResolution::_4k;
	}
	else if (height > 720 && height <= 1080) {
		switch (scan_type) {
		case av::media::ScanType::Interlaced:
		case av::media::ScanType::MBAFF:
		case av::media::ScanType::PAFF:
			source.video_resolution = SourceVideoResolution::_1080i;
			break;
		case av::media::ScanType::Progressive:
			source.video_resolution = SourceVideoResolution::_1080p;
			break;
		default:
			source.video_resolution = SourceVideoResolution::Unknown;
			break;
		}
	}
	else if (height <= 720 && height > 480) {
		source.video_resolution = SourceVideoResolution::_720;
	}
	else if (height <= 480) {
		source.video_resolution = SourceVideoResolution::_480;
	}
}

void Publish::capitalizeWords(std::tstring& s) {
	bool newWord = true;
	for (tchar& c : s) {
		if (std::isspace(c)) {
			newWord = true;
		}
		else if (newWord) {
			c = std::toupper(static_cast<tchar>(c));
			newWord = false;
		}
	}
}

void Publish::sendTGWarningMessage(const std::string& dir, PublishError error) {
	if (m_tgbot_token.empty() || m_tgbot_chat_id.empty()) {
		logi("tgbot token or chat id is empty, skip send tg warning message");
		return;
	}
	auto msg = publishErrorToString(error);
	const std::string text = "处理: " + dir + ", 错误: " + av::str::toA(msg);
	av::tgbot::send_message(av::str::toA(m_tgbot_token),
		av::str::toA(m_tgbot_chat_id),
		text);
}
