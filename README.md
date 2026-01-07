# Command Line Player

Command Line Player is a terminal-based MP3 music player for Windows, built with C++. It features a sophisticated and interactive Terminal User Interface (TUI) powered by FTXUI, providing a modern music playback experience directly in your console.

## Features

-   **Interactive TUI**: A rich, mouse-driven interface for seamless navigation and control within the terminal.
-   **Core Playback Controls**: Standard play, pause, and stop functionalities.
-   **Volume Management**: Adjust volume with a vertical slider or +/- buttons. Press-and-hold on the buttons accelerates the volume change.
-   **Song Progress and Seeking**: A visual progress slider shows playback position. You can click to seek, or use the `◀◀` and `▶▶` buttons to jump backward or forward by 10 seconds (or 30 seconds with `Ctrl`+click).
-   **Playback Modes**:
    -   **Normal**: Plays through the playlist once.
    -   **Repeat All (`⟳`)**: Loops the entire playlist.
    -   **Repeat One (`↻`)**: Repeats the current song.
    -   **Shuffle (`⤨`)**: Plays songs in a random order.
-   **Playlist Management**: Automatically discovers MP3 files and sub-directories from a `music` folder. A "Refresh playlist!" button re-scans the directory.
-   **ID3 Tag Support**: Intelligently parses ID3v2 tags to display song titles and artists (`TPE1` and `TIT2`). If tags are not present, it defaults to the filename.

## Getting Started

### Prerequisites

* Windows OS
* A C++20 compatible compiler (tested with MSVC)
* **FTXUI Library**: Required for the terminal user interface
* **SDL2 Library**: Required for audio output

#### Installing FTXUI (without Visual Studio project files)

FTXUI is installed using **vcpkg** and does not require a Visual Studio project configuration.

1. Install vcpkg if it is not already installed:

   ```sh
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   bootstrap-vcpkg.bat
   ```

2. Install FTXUI:

   ```sh
   vcpkg install ftxui
   ```

3. Make sure the vcpkg toolchain is integrated:

   ```sh
   vcpkg integrate install
   ```

FTXUI will then be automatically available to MSVC projects.

---

### SDL2 Setup

SDL2 must be installed manually.

1. Download the **SDL2 development library for Visual C++** from the official SDL website.
2. Extract the archive to a directory of your choice.
3. The project expects a **static build** (`SDL2-static.lib`).

---

### Building

This project is configured as a **Visual Studio 2022 (x64)** application.

1. **Clone the repository:**

   ```sh
   git clone https://github.com/Atomex148/Command-Line-Player.git
   cd Command-Line-Player
   ```

2. **Configure SDL2 paths:**

   * Open `src/CLP.vcxproj` in a text editor or Visual Studio.
   * Update:

     * `<AdditionalIncludeDirectories>` to point to `SDL2/include`
     * `<AdditionalLibraryDirectories>` to point to `SDL2/lib/x64`

3. **Build the project:**

   * Open `CLP.vcxproj` in Visual Studio
   * Select `Release` and `x64`
   * Build the solution

### Building

This project is configured as a Visual Studio 2022 solution for Windows (x64).

1.  **Clone the repository:**
    ```sh
    git clone https://github.com/atomex148/command-line-player.git
    cd command-line-player
    ```

2.  **Configure Dependencies:**
    -   Download the SDL2 development library for Visual C++.
    -   Open `src/CLP.vcxproj` in a text editor.
    -   Update the `<AdditionalIncludeDirectories>` and `<AdditionalLibraryDirectories>` to point to the `include` and `lib` folders of your SDL2 installation.

3.  **Build the project:**
    -   Open the `CLP.vcxproj` file with Visual Studio.
    -   Select the `Release` and `x64` configuration.
    -   Build the solution (F7 or Build > Build Solution).

## Usage

1.  After a successful build, locate the executable (e.g., in `x64/Release/CLP.exe`).
2.  Create a directory named `music` in the same folder as the executable.
3.  Place your `.mp3` files into the `music` directory.
4.  Run the executable.
5.  Use your mouse to interact with the player controls and music list.

## Code Overview

-   `main.cpp`: The main entry point which instantiates and runs the `Player`.
-   `Player.hpp` / `Player.cpp`: The core of the application. It uses FTXUI to construct the TUI, manages component layout, and handles user input events for all controls (buttons, sliders, etc.). It orchestrates the `SoundModule` and `FilesystemModule`.
-   `SoundModule.hpp` / `SoundModule.cpp`: A multi-threaded module for handling all audio-related tasks. It uses `minimp3` to decode MP3 data and SDL2 to manage the audio device and playback buffer. It controls playback state (playing, paused), volume, and seeking logic.
-   `FilesystemModule.h` / `FilesystemModule.cpp`: Responsible for file system interactions. It scans the `music` directory, identifies MP3 files by parsing their headers, and extracts song metadata from ID3v2 tags.
-   `ButtonStyles.h` / `ButtonStyles.cpp`: Contains helper functions to create custom-styled buttons for FTXUI, enabling features like the mutually exclusive playback mode toggles.
-   `vendor/minimp3/`: Contains the single-header `minimp3` library for MP3 decoding.
