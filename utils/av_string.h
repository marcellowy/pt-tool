#ifndef AV_STRING_H_
#define AV_STRING_H_

#include "av_base.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "fmt/ranges.h"
#include "fmt/format.h"

namespace std {
#if defined(UNICODE) || defined(_UNICODE)
	typedef wstring tstring;
	typedef wstringstream tstringstream;
#else
	typedef string tstring;
	typedef stringstream tstringstream;
#endif
}

typedef wchar_t wchar;
#if defined(UNICODE) || defined(_UNICODE)
typedef wchar_t tchar;
#else
typedef char    tchar;
#endif

namespace av {
namespace str {
	std::string toA(const std::string& str);
	std::string toA(const std::wstring& str);
	std::wstring toW(const std::string& str);
	std::wstring toW(const std::wstring& str);
	std::tstring toT(const std::string& str);
	std::tstring toT(const std::wstring& str);
	std::string toUtf8(const std::string& str);
	std::string toUtf8(const std::wstring& str);
	std::string toUnicodeA(const std::string& str);
	std::wstring toUnicodeW(const std::string& str);
	std::string urlEncode(const std::string& in);
	std::string urlDecode(const std::string& in);

	std::string toUpper(const std::string& lower);
	std::string toLower(const std::string& upper);

	void replace(std::tstring& str, const std::tstring& old_str, const std::tstring& new_str);
	void replace_all(std::tstring& str, const std::tstring& old_str, const std::tstring& new_str);
	void remove_suffix(std::tstring& s, const std::tstring& suffix);
#ifdef _WIN32
	std::tstring trim(const std::tstring& s);
#endif // _WIN32
	std::string trim(const std::string& s);
	
	template<typename T>
	std::tstring toString(T v) {
#if defined(UNICODE) || defined(_UNICODE)
		return std::to_wstring(v);
#else
		return std::to_string(v);
#endif // _WIN32
	}

	std::vector<std::tstring> split(const std::tstring s, std::tstring delimiter);
	std::tstring join(const std::vector<std::tstring>& vec, const std::tstring& sep);
}
};

#endif
