#ifndef TEST_PUBLISH_SPORT_NAME_H_
#define TEST_PUBLISH_SPORT_NAME_H_

#include "defined.h"
#include "av_string.h"

#include "av_media_info.h"

using namespace av::media;

//
bool parseSportName(Source& obj);

//
bool parseMovieName(Source& obj);

//
bool parseTVSeriesName(Source& obj);

//
bool parseDiscoverName(Source& obj);

//
bool parseVarietyName(Source& obj);

//
bool parseCustomName(Source& obj);

#endif
