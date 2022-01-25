# Khíín Tâigí Phah Jī Hoat

## Development

Khiin consists of separate projects in the following folders:

- `engine`: The cross-platform IME engine that plugs in to each app. (c++17)
- `windows`: A win32 Text Services Framework [TSF] application. (c++17)

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
- protobuf [v3.18.0](https://github.com/protocolbuffers/protobuf/releases/tag/v3.18.0)

### Installing for development

Following packages can be installed with vcpkg:

```
./vcpkg install boost:x64-windows
./vcpkg install sqlitecpp:x64-windows
./vcpkg install protobuf:x64-windows
```

Notes:

- You will also need the `x86-windows` triplets for a production build.
- Don't forget to integrate if using Visual Studio: `./vcpkg integrate install`

### Building

#### Protobuf

Protobuf generated files are included in the repo. You only need to link
to the protobuf static library to build.

To re-build protobuf generated files after modification, use the following command:

```
protoc.exe -I=proto --cpp_out=proto/proto proto/messages.proto
```

#### Engine

The `engine` folder contains a standard `cmake` project. For building in
Visual Studio, don't forget to update the file `CmakeSettings.json::cmakeToolchain`
to point to your `vcpkg.cmake` file.
