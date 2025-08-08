#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

// STD HEADERS

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <fstream>
#include <codecvt>

// UI HEADERS

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>

// SOUND PROCESSING HEADERS

extern "C" {
	#include "vendor/minimp3/minimp3.h"
	#include "vendor/minimp3/minimp3_ex.h"
}

// GLOBAL VARS

static const std::filesystem::path appPath = std::filesystem::current_path();

#pragma pack(push, 1)
struct MP3_Header {
	char fileID[3];
	uint8_t verMajor;
	uint8_t verMinor;
	uint8_t flags;
	uint32_t tagSize;
};
#pragma pack(pop)