#ifndef AV_MD5_H_
#define AV_MD5_H_

#include <string>

namespace av {
	namespace hash {
		bool md5(const std::string& in_str, std::string& out_str);
	}
}

#endif
