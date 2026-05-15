# DesignerVersionChooser

[![Build](https://github.com/pharos-labs/DesignerVersionChooser/actions/workflows/build.yml/badge.svg)](https://github.com/pharos-labs/DesignerVersionChooser/actions/workflows/build.yml)

PharosVersionChooser is a trio of very small applications which detect copies of Pharos Designer, Pharos Expert, or Mosaic Designer installed on a (windows) computer, and provides a simple dialog to select them.

![Screenshot](./doc/screenshot.png)

# How to Use PharosVersionChooser
To use the tool, download the latest installer from the [Releases](https://github.com/djshield/PharosVersionChooser/releases) page.

Then run the application for Designer, Expert, or Mosaic Designer - a list of detected versions will be shown. You might wish to pin the applications to your taskbar or desktop for convenience.

# Building
The application is built using straight Windows API (no MFC, Qt or other toolkits) in order to keep it as absolutely minimal as possible.

You will need CMake and Visual Studio (any recent version should be fine) to build it.

To build:
- Clone the source
- From a command prompt:
  - Change to the source directory
  - Created a build directory and enter it, e.g `mkdir build`, `cd build`
- `cmake ..\` to configure
- `cmake --build . --config Release` to build a release binary
- `cpack -C release` to build the NSIS installer
