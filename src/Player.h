#pragma once
#include "headers.hpp"

using namespace ftxui;
class Player {
private:
	std::vector<std::pair<std::wstring, std::filesystem::path>> musicList = {};
	ScreenInteractive screen = ScreenInteractive::Fullscreen();
	Component layout;

	std::vector<std::pair<std::wstring, std::filesystem::path>> readMusicList();
	MP3_Header recieveHeader(uint8_t bytes[10]);
	std::wstring recieveSongName(std::filesystem::path pathToSong);
public:
	Player();
	void run();
};

