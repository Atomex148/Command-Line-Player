#pragma once
#include "headers.hpp"
#include "ButtonStyles.h"
#include "SoundModule.hpp"
#include "FilesystemModule.h"

using namespace ftxui;
class Player {
private:
	SoundModule sm;
	FilesystemModule fm;

	int selectedSongIndex = 0;
	std::vector<std::wstring> musicNames;
	std::wstring currentSongDuration = L"", currentlyPlaying = L"";
	int songProgressSliderValue = 0, volumeSliderValue = 50;
	bool userSongProgressDragging = false, userVolumeDragging = false;
	std::shared_ptr<ButtonGroupState> groupStates = std::make_shared<ButtonGroupState>();

	ScreenInteractive screen = ScreenInteractive::Fullscreen();
	Component layout;

	std::thread timerThread;
	std::atomic<bool> stopUpdateTimer = false, soundButtonHoldingUp = false, soundButtonHoldingDown = false;
	std::chrono::steady_clock::time_point holdingStart;
	short volumeChangeStep = 200, volumeChangedTimes = 0;
	std::random_device rd;

	void handleSongEnding();
public:
	Player();
	~Player();
};

