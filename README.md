![Banner](banner.png)
# Simple and flexible version control and code syncing
# WORK IN PROGRESS
![latest-build](https://github.com/SiliconSloth/Metro/workflows/latest-build/badge.svg)
![all-tests](https://github.com/SiliconSloth/Metro/workflows/all-tests/badge.svg)

## Development Setup
Metro is written in C++ and uses CMake. It depends on libgit2 and libssh2 on all platforms,
and additionally OpenSSL on Windows and macOS.
For detailed build instructions, see the [wiki](https://siliconsloth.github.io/Metro/building/building.html).

## Status and Future
The basic functionality of Metro, including syncing, has been implemented, however it is not yet stable or user-friendly.
We are currently working on fixing bugs and generally improving the user experience.

## Information
See the [wiki](https://siliconsloth.github.io/Metro/) for details and info.

## Download

The latest version of Metro is alpha-1.1. You can download binaries from the
[Releases](https://github.com/SiliconSloth/Metro/releases) page, or build from source as above.

The Windows binaries depend on the Microsoft Visual C++ Redistributable for Visual Studio 2015, 2017 and 2019,
which you can download [here]( https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads).

Keep in mind that alpha builds are highly experimental and subject to change.
Use with caution at your own risk.

**[Download Metro alpha-1.1](https://github.com/SiliconSloth/Metro/releases/tag/alpha-1.1)**
