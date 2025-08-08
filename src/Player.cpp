#include "Player.h"

std::vector<std::pair<std::wstring, std::filesystem::path>> Player::readMusicList() {
    std::filesystem::path musicPath = appPath.wstring() + L"/music";
    std::vector<std::pair<std::wstring, std::filesystem::path>> musicList = { };

    for (const auto& entry : std::filesystem::directory_iterator(musicPath)) {
        if (std::filesystem::is_regular_file(entry.status()) && entry.path().extension() == L".mp3") {
            musicList.push_back({ recieveSongName(entry.path()), entry.path()});
        }
    }

    return musicList;
}

MP3_Header Player::recieveHeader(uint8_t bytes[10]) {
    MP3_Header header;
    std::memcpy(header.fileID, bytes, 3);
    header.verMajor = bytes[3];
    header.verMinor = bytes[4];
    header.flags = bytes[5];
    header.tagSize = ((bytes[6] & 0x7F) << 21 | (bytes[7] & 0x7F) << 14 | (bytes[8] & 0x7F) << 7 | (bytes[9] & 0x7F));
    return header;
}

std::wstring Player::recieveSongName(std::filesystem::path pathToSong) {
    std::ifstream file(pathToSong);
    if (!file) return L"";

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::array<uint8_t, 10> headerData;
    if (!file.read(reinterpret_cast<char*>(headerData.data()), 10)) return L"";
    MP3_Header header = recieveHeader(headerData.data());
    
    std::vector<uint8_t> data(header.tagSize);
    file.seekg(10, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(data.data()), header.tagSize)) return L"";

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
    auto name = Renderer([&] { return hbox(text(L"MP3 Player") | center | flex) | border | xflex; });
    auto terminalSize = Terminal::Size();

    auto musicList = readMusicList();
    int selected = 0;
    MenuOption option;

    std::vector<std::wstring> musicNames;
    for (const auto& [name, path] : musicList)
        musicNames.push_back(name);

    auto menu = Menu(&musicNames, &selected, option);
    auto button = Button(L"Refresh playlist!", [](){});
    
    auto controls = Container::Vertical({ menu, button });

    auto musicPane = Renderer(controls, [&] {
        return
            vbox(
                text("Music List") | center | bold,
                separator(),
                menu->Render(),
                filler(),
                button->Render()
            ) | border | size(WIDTH, EQUAL, terminalSize.dimx / 3);
        });

    auto playerPane = Renderer([&] {
        return
            vbox(
                text(L"")
            ) | border | size(WIDTH, EQUAL, 2 * terminalSize.dimx / 3);
        }
    );

    layout = Container::Vertical({ name, Container::Horizontal({musicPane, playerPane}) | flex });

    screen.Loop(layout);
}

void Player::run()
{
}
