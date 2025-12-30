#ifndef AV_TRANSLATE_H_
#define AV_TRANSLATE_H_

#include "av_string.h"

// Example code
//std::tstring text;
//av::translate::Translate t(TEXT("key...."), TEXT("google-translate113.p.rapidapi.com"));
//if (!t.foo(TEXT("中国"), text)) {
//	loge("translate error");
//	return 0;
//}
//else {
//	logi("translate success {}", av::str::toA(text));
//}

namespace av {
	namespace translate {
		class Translate
		{
		public:
			Translate(const std::tstring& rapidapi_key, const std::tstring& rapidapi_host);
			~Translate() = default;
			bool foo(const std::tstring& source_text, std::tstring& text);
		private:
			std::tstring m_rapidapi_url = TEXT("https://google-translate113.p.rapidapi.com");
			std::tstring m_rapidapi_key;
			std::tstring m_rapidapi_host;
		};

	}
}

#endif