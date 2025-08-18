#include "Player.hpp"

std::unordered_map<std::wstring, std::filesystem::path> Player::readMusicList() {
    std::filesystem::path musicPath = appPath.wstring() + L"\\music";
    std::unordered_map<std::wstring, std::filesystem::path> musicList = { };

    for (const auto& entry : std::filesystem::directory_iterator(musicPath)) {
        if (std::filesystem::is_regular_file(entry.status()) && entry.path().extension() == L".mp3") {
            std::optional<std::wstring> songName = recieveSongName(entry.path());
            if (songName) musicList[*songName] = entry.path();
        }
    }

    return musicList;
}

std::optional<std::wstring> Player::recieveSongName(std::filesystem::path pathToSong) {
    std::ifstream file(pathToSong, std::ios::binary);
    if (!file) return L"";

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::array<uint8_t, 10> headerData;
    if (!file.read(reinterpret_cast<char*>(headerData.data()), 10)) return std::nullopt;
    MP3_Header header = recieveHeader(headerData.data());
    if (std::memcmp(header.fileID, "ID3", 3) != 0) return std::nullopt;
    
    std::vector<uint8_t> data(header.tagSize);
    file.seekg(10, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(data.data()), header.tagSize)) return std::nullopt;

    size_t pos = 0;
    std::pair<std::wstring, std::wstring> songArtistAndName;
    while (pos < data.size()) {
        std::string tag(reinterpret_cast<char*>(&data[pos]), 4);

        uint32_t sectionSize = ((data[pos + 4] << 24) | (data[pos + 5] << 16) | (data[pos + 6] << 8) | (data[pos + 7]));

        if (!std::isalnum(tag[0])) break;
        if (tag != "TIT2" && tag != "TPE1") {
            pos += sectionSize + 10;
            continue;
        }

        uint32_t sectionStart = pos + 10;
        uint8_t encoding = data[sectionStart];
        std::wstring extractedText;

        switch (encoding) {
            case 0: {
                extractedText = std::wstring(data.begin() + sectionStart + 1, data.begin() + sectionStart + sectionSize);
                break;
            }

            case 1: {
                const wchar_t* namePtr = reinterpret_cast<wchar_t*>(&data[sectionStart + 1]);
                uint32_t textSize = sectionSize - 1;

                uint16_t bom = ((data[sectionStart + 1] << 8) | data[sectionStart + 2]);
                if (bom == 0xFEFF || bom == 0xFFFE) {
                    textSize -= 1;
                    namePtr += 1;
                }

                uint32_t charCount = textSize / 2;
                extractedText = std::wstring(namePtr, charCount);
                break;
            }
        
            case 2: {
                const wchar_t* namePtr = reinterpret_cast<wchar_t*>(&data[sectionStart + 1]);
                uint32_t charCount = (sectionSize - 1) / 2;
                extractedText = std::wstring(namePtr, charCount);
                break;
            }

            case 3: {
                std::string utf8str(reinterpret_cast<const char*>(&data[sectionStart + 1]), sectionSize - 1);
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                extractedText = converter.from_bytes(utf8str);
                break;
            }
        }

        (tag == "TIT2") ? songArtistAndName.second = extractedText : songArtistAndName.first = extractedText;
        pos += sectionSize + 10;
    }

    return songArtistAndName.first + L" - " + songArtistAndName.second;
}


Player::Player() {
    if (!std::filesystem::exists(appPath.wstring() + L"\\music")) {
        std::filesystem::create_directory(appPath.wstring() + L"\\music");
    }

    auto name = Renderer([&] { return hbox(text(L"MP3 Player") | center | flex | bold) | border | xflex; });
    auto terminalSize = Terminal::Size();

    musicList = readMusicList();
    int selected = 0;

    std::vector<std::wstring> musicNames;
    for (const auto& [name, path] : musicList)
        musicNames.push_back(name);

    auto menu = Menu(&musicNames, &selected, MenuOption::Vertical());
    auto refreshButton = Button(L"Refresh playlist!", [&](){
        selected = 0;
        musicNames.clear();
        musicList = readMusicList();
        for (const auto& [name, path] : musicList)
            musicNames.push_back(name);

        });
    
    auto musicPaneControls = Container::Vertical({ menu, refreshButton });

    auto musicPane = Renderer(musicPaneControls, [&] {
        return
            vbox(
                text("Music List") | center | bold,
                separator(),
                menu->Render(),
                filler(),
                refreshButton->Render()
            ) | border | size(WIDTH, EQUAL, terminalSize.dimx / 3);
        });

    auto playButton = Button(L"▶", [&] {
        if (!musicNames.empty()) {
            currentlyPlaying = musicNames[selected];
            sm.play(this->musicList[musicNames[selected]]);
        }
    });
    auto pauseButton = Button(L"∥", [&] {
        sm.pause();
    });
    auto stopButton = Button(L"■", [&] {
        currentlyPlaying.clear();
        sm.stop();
    });

    Box volumeSliderBox;
    auto volumeSlider = Slider<int>({.value = &volumeSliderValue, .min = 0,.max = 100, .increment = 1, .direction = Direction::Up}) | 
        CatchEvent([&](Event event) {
            auto mouse = event.mouse();
            if (mouse.x >= volumeSliderBox.x_min && mouse.x <= volumeSliderBox.x_max &&
                mouse.y >= volumeSliderBox.y_min && mouse.y <= volumeSliderBox.y_max || userVolumeDragging)
            {
                if (event.is_mouse()) {
                    if (event.mouse().button == Mouse::Left) {
                        if (event.mouse().motion == Mouse::Pressed) {
                            userVolumeDragging = true;
                        }
                        else if (event.mouse().motion == Mouse::Released) {
                            if (userVolumeDragging) {
                                sm.changeVolume(volumeSliderValue);
                                userVolumeDragging = false;
                            }
                        }
                    }
                }
            }
            return false;
        }) |
        Renderer([&](Element e) {
            return e | reflect(volumeSliderBox);
        });
    auto volumeUpButton = Button(L" +", [&] {
        if (volumeSliderValue < 100) {
            volumeSliderValue++;
            sm.changeVolume(volumeSliderValue);
        }
        
    });
    auto volumeDownButton = Button(L" -", [&] {
        if (volumeSliderValue > 0) {
            volumeSliderValue--;
            sm.changeVolume(volumeSliderValue);
        }
    });

    Box songProgressSliderBox;
    auto songProgressSlider = Slider("", &songProgressSliderValue, 0, 100, 1) | 
        CatchEvent([&](Event event) {
            auto mouse = event.mouse();
            if (mouse.x >= songProgressSliderBox.x_min && mouse.x <= songProgressSliderBox.x_max &&
                mouse.y >= songProgressSliderBox.y_min && mouse.y <= songProgressSliderBox.y_max || userSongProgressDragging)
            {
                if (event.is_mouse()) {
                    if (event.mouse().button == Mouse::Left) {
                        if (event.mouse().motion == Mouse::Pressed) {
                            userSongProgressDragging = true;
                        }
                        else if (event.mouse().motion == Mouse::Released) {
                            if (userSongProgressDragging) {
                                sm.seekTo(songProgressSliderValue);
                                userSongProgressDragging = false;
                            }
                        }
                    }
                }
            }
            return false;
        }) | 
        Renderer([&](Element e) {
            return e | reflect(songProgressSliderBox);
        });

    auto playerPaneControls = Container::Vertical({ playButton, pauseButton, stopButton, songProgressSlider, volumeUpButton,
                                                    volumeDownButton, volumeSlider });

    auto playerPane = Renderer(playerPaneControls, [&] {
        return
            vbox(
                filler(),
                vbox(
                    songProgressSlider->Render() | flex | border,
                    text(currentlyPlaying) | center,
                    text(sm.fromDoubleToTime(sm.getTimeElapsed()) + L"/" + sm.fromDoubleToTime(sm.getSongDuration())) | center
                ),
                filler(),
                hbox(
                    vbox(
                        filler() | size(HEIGHT, EQUAL, 2),
                        hbox(
                            playButton->Render() | flex,
                            pauseButton->Render() | flex | bold,
                            stopButton->Render() | flex | color(Color::Red)
                        ) | xflex,
                        filler() | size(HEIGHT, EQUAL, 1)
                    ) | flex | size(HEIGHT, EQUAL, 5),
                    vbox(
                        volumeUpButton->Render() | flex,
                        text(std::to_wstring(volumeSliderValue)) | center,
                        volumeDownButton->Render() | flex
                    ) | size(WIDTH, EQUAL, 5),
                    volumeSlider->Render() | size(HEIGHT, EQUAL, 5) | border
                )
            ) | border | size(WIDTH, EQUAL, 2 * terminalSize.dimx / 3);
        }
    );
    layout = Container::Vertical({ name, Container::Horizontal({musicPane, playerPane}) | flex });

    timerThread = std::thread([&] {
        while (!stopUpdateTimer.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (!userSongProgressDragging) songProgressSliderValue = sm.getProgress() * 100;
            screen.Post(Event::Custom);
        }
    });

    screen.Loop(layout);
}

Player::~Player() {
    stopUpdateTimer.store(true);
    if (timerThread.joinable()) timerThread.join();
}

