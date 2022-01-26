# Khíín Tâigí Phah Jī Hoat

## Development

Khiin consists of separate projects in the following folders:

- `engine`: The cross-platform IME engine that plugs in to each app. (c++17)
- `windows`: A win32 Text Services Framework [TSF] application. (c++17)

### Dependencies

- utf8cpp v3.2.1 (BSL-1.0)
    - Header only, included in `third_party`
    - From [nemtrif/utfcpp](https://github.com/nemtrif/utfcpp)
- unilib v3.2.0 (MPL-2.0)
    - Modified to be header only, included in `third_party`
    - From [ufal/unilib](https://github.com/ufal/unilib)

The following packages can be installed with `vcpkg`:

- Boost 1.77.0 (BSL-1.0)
    - Specified libraries only
- SQLiteCpp 3.1.1 (MIT)
- sqlite3
- protobuf [v3.18.0](https://github.com/protocolbuffers/protobuf/releases/tag/v3.18.0)
- GTest v1.11.0

```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
./vcpkg install boost-log boost-range boost-algorithm sqlitecpp protobuf gtest --triplet=x64-windows-static-md
```

For release you will need the x86 packages:

```
./vcpkg install boost-log boost-range boost-algorithm sqlitecpp protobuf gtest --triplet=x86-windows-static-md
```

Notes:

- Don't forget to integrate if using Visual Studio: `./vcpkg integrate install`

### Building

#### Protobuf

Protobuf generated files are included in the repo. You only need to link
to the protobuf static library to build.

To re-build protobuf generated files after modification, install `protoc`
and use the following command:

```
protoc.exe -I=proto --cpp_out=proto/proto proto/messages.proto
```

#### Engine

The `engine` folder contains a `cmake` project. For building in
Visual Studio, update the file `CmakeSettings.json > cmakeToolchain`
to point to your `vcpkg.cmake` file.

#### Windows

See the `windows/KhiinWindows.sln` Visual Studio solution. Currently built using
Visual Studio Community 2022.

For now you still need to manually register the DLL for use. Using an elevated
prompt or powershell, go to the `windows/x64/Debug` folder and run:

```
regsvr32.exe KhiinPJH.dll
```

You should unregister when you are not actively using it, since things may change
and break at this early stage:

```
regsvr32.exe /u KhiinPJH.dll
```
