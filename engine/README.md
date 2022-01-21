# libtaikey

## Development

- Built with CMake & Visual Studio Community 2022
- Packages using vcpkg (`./vcpkg integrate install` to use in Visual Studio)
- Edit CMakeSettings.json > `cmakeToolchain` to point to your local `vcpkg`

## Dependencies

- Boost 1.77.0 (BSL-1.0)
    - Header only libraries
- SQLiteCpp 3.1.1 (MIT)
- utf8cpp v3.2.1 (BSL-1.0)
    - Header only, included in `externals`
    - From [nemtrif/utfcpp](https://github.com/nemtrif/utfcpp)
- unilib v3.2.0 (MPL-2.0)
    - Modified to be header only, included in `externals`
    - From [ufal/unilib](https://github.com/ufal/unilib)

### Installing for development

You can install Boost and SQLiteCpp (including sqlite3) with vcpkg:

```
./vcpkg install boost:x64-windows
./vcpkg install sqlitecpp:x64-windows
./vcpkg install boost:x86-windows
./vcpkg install sqlitecpp:x86-windows
```
