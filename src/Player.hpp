#pragma once
#include "headers.hpp"
#include "SoundModule.hpp"

using namespace ftxui;
class Player {
private:
	SoundModule sm;
	std::wstring currentSongDuration = L"", currentlyPlaying = L"";
	int songProgressSliderValue = 0, volumeSliderValue = 50;
	bool userSongProgressDragging = false, userVolumeDragging = false;

	std::unordered_map<std::wstring, std::filesystem::path> musicList = {};
	ScreenInteractive screen = ScreenInteractive::Fullscreen();
	Component layout;

	std::thread timerThread;
	std::atomic<bool> stopUpdateTimer = false;

	std::unordered_map<std::wstring, std::filesystem::path> readMusicList();
	std::optional<std::wstring> recieveSongName(std::filesystem::path pathToSong);
public:
	Player();
	~Player();
};

