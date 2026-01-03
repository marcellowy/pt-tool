#ifndef AV_MEDIA_INFO_H_
#define AV_MEDIA_INFO_H_

#include "av_string.h"

namespace av {
	namespace media {
		// 
		enum class SourceCategory {
			Unknown = 0,
			Movie,			// 电影
			Sport,			// 体育
			TVSeries,		// 电视剧
			Discover,		// 纪录片
			Variety,		// 综艺节目
		};

		enum class SourceVideoResolution {
			Unknown = 0,
			_480,
			_720,
			_1080p,
			_1080i,
			_2k,
			_4k,
			_8k
		};

		enum class SourceVideoCodec {
			Unknown = 0,
			_h264,
			_h265,
			_vc1,
			_mpeg2,
			_xvid,
			_av1,
			_vp8,
			_vp9,
			_avs,
			_avs2,
			_cavs,
		};

		enum class SourceAudioCodec {
			Unknown = 0,
			_aac,
			_ac3,
			_dts,
			_dts_hd_ma,
			_e_ac3_ddp,
			_e_ac3_atoms,
			_true_hd,
			_true_hd_atmos,
			_lpcm,
			_wav,
			_flac,
			_ape,
			_mp1,
			_mp2,
			_mp3,
			_ogg,
			_other,
		};

		enum class SourceType {
			Unknown = 0,
			Dir,			// 目录
			File,			// 文件
		};

		enum class ScanType {
			Unknown,
			Interlaced,	// 1080i
			MBAFF,		// 1080p
			Progressive	// 1080p
		};

		enum class SourceId {
			Unknown,
			_WebDL,
			_Bluray,
			_Remux,
			_Encode,
			_HDTV,
			_TV,
			_DVD,
			_CD,
			_Other
		};

		inline SourceId from(int64_t id) {
			switch (id) {
			case static_cast<int64_t>(SourceId::Unknown):
				return SourceId::Unknown;
				break;
			case static_cast<int64_t>(SourceId::_Bluray):
				return SourceId::_Bluray;
				break;
			case static_cast<int64_t>(SourceId::_CD):
				return SourceId::_CD;
				break;
				case static_cast<int64_t>(SourceId::_DVD):
				return SourceId::_DVD;
				break;
			case static_cast<int64_t>(SourceId::_HDTV):
				return SourceId::_HDTV;
				break;
			case static_cast<int64_t>(SourceId::_Encode):
				return SourceId::_Encode;
			case static_cast<int64_t>(SourceId::_Remux):
				return SourceId::_Remux;
				break;
			case static_cast<int64_t>(SourceId::_Other):
				return SourceId::_Other;
				break; 
			case static_cast<int64_t>(SourceId::_TV):
				return SourceId::_TV;
				break;
			case static_cast<int64_t>(SourceId::_WebDL):
				return SourceId::_WebDL;
				break;
			}
			return SourceId::Unknown;
		}

		struct Source {

			// dirinfo
			std::tstring dir;							// 影片所在目录
			std::tstring name;							// 影片名称
			std::tstring fullpath;						// 完整路径
			std::tstring fullpath_md5;					// 完整路径md5
			SourceType type;							// 输入类型
			std::tstring file_suffix;					// 如果是文件,文件后缀

			int64_t group_id;							// 发布组id, 发布的网站必须有对应的id实现
			SourceId source_id;							// 影片来源
			SourceCategory category;					// 影片分类
			SourceVideoResolution video_resolution;		// 视频分辨率
			SourceVideoCodec video_codec;				// 视频格式
			SourceAudioCodec audio_codec;				// 音频格式

			// seed
			std::tstring seed_dir;						// 做种目录

			// baseinfo
			std::tstring year;
			std::tstring name_chs;
			std::tstring sub_title;
			std::tstring title_prefix;
			std::tstring douban_id;
			std::tstring season;
			std::tstring name_eng;
			std::tstring imdb_link;
			std::tstring description;
			std::tstring poster_img;

			// video screenshot
			std::vector<std::tstring> screenshot_local;

			// mediainfo
			std::tstring mediainfo_text;
		};
	}
}

#endif
