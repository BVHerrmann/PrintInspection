#pragma once
#include <QtCore>

class ImageLineInformation
{
public:
	//std::chrono::time_point<std::chrono::steady_clock> timestamp;
	unsigned __int64 m_TimeStampInMuSec;

	bool tube_start = false;
	bool tube_end   = false;
	bool tube_found = false;

	bool error_font     = false;
	bool error_position = false;
	bool error_line     = false;
	bool error_height   = false;
	bool error_width    = false;
	bool no_format      = false;
};