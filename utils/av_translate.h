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

		enum class TranslateType {
			kNone,
			kFreeGoogleTranslate,
			kTranslate
		};

		class ITranslate
		{
		public:
			virtual ~ITranslate() = default;
			virtual bool foo(const std::tstring& source_text, std::tstring& text) = 0;
		};

		class TranslateFactory
		{
		public:
			static std::unique_ptr<ITranslate> create(const std::tstring& rapidapi_key, const std::tstring& rapidapi_host, TranslateType type = TranslateType::kTranslate);
		};

		// Blud icon Free Google Translate API
		class FreeGoogleTranslate : public ITranslate
		{
		public:
			FreeGoogleTranslate(const std::tstring& rapidapi_key, const std::tstring& rapidapi_host);
			~FreeGoogleTranslate() = default;
			bool foo(const std::tstring& source_text, std::tstring& text) override;
		private:
			std::tstring m_rapidapi_url = TEXT("https://free-google-translator.p.rapidapi.com/external-api/free-google-translator?query=%s&to=en&from=auto");
			std::tstring m_rapidapi_key;
			std::tstring m_rapidapi_host;
		};


		// Black icon Translate API
		class Translate : public ITranslate
		{
		public:
			Translate(const std::tstring& rapidapi_key, const std::tstring& rapidapi_host);
			~Translate() = default;
			bool foo(const std::tstring& source_text, std::tstring& text) override;
		private:

			std::tstring m_rapidapi_url = TEXT("https://google-translate113.p.rapidapi.com/api/v1/translator/text");
			std::tstring m_rapidapi_key;
			std::tstring m_rapidapi_host;
		};

	}
}

#endif
