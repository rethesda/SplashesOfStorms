# Splashes of Storms

SKSE plugin that adds splashes to surfaces, and real ripples on water, when raining

* **[Download on Nexus Mods!](https://www.nexusmods.com/skyrimspecialedition/mods/72115)**
* [SKSEVR version](https://www.nexusmods.com/skyrimspecialedition/mods/73111)

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/) or newer
	* Desktop development with C++

## User Requirements
* [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
	* Needed for SSE/AE
* [VR Address Library for SKSEVR](https://www.nexusmods.com/skyrimspecialedition/mods/58101)
	* Needed for VR

## Building
```
git clone https://github.com/powerof3/SplashesOfStorms.git
cd SplashesOfStorms
git submodule update --init --recursive
```

### SSE (1.5.97)
```
cmake --preset vs2022-se
cmake --build --preset vs2022-se
```
### AE (1.6.1170+)
```
cmake --preset vs2022-ae
cmake --build --preset vs2022-ae
```
### VR
```
cmake --preset vs2022-vr
cmake --build --preset vs2022-vr
```

Replace `vs2022` with `vs2026` to build with Visual Studio 2026.

## License
[GPL-3.0](LICENSE)
