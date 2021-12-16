# libtaikey

## Development

- Built with CMake & Visual Studio Community 2022
- Packages using vcpkg (`./vcpkg integrate install` to use in Visual Studio)
- Edit CMakeSettings.json > `cmakeToolchain` to point to your local `vcpkg`

Boost 1.77.0:

```
./vcpkg install boost-locale[icu] boost-regex[icu] boost --triplet x64-windows --recurse

./vcpkg install boost-locale[icu] boost-regex[icu] boost --triplet x86-windows --recurse
```

utf8cpp lib:

```
./vcpkg install utfcpp:x64-windows

./vcpkg install utfcpp:x86-windows
```
