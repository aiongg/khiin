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
regsvr32.exe /s KhiinPJH.dll    # /s for silent install
```

To register the x86 (32-bit) DLL, use an elevated 32-bit cmd.exe
prompt (`C:\Windows\SysWOW64\cmd.exe`), go to the `windows/x86/Debug`
folder and run:

```
C:\Windows\SysWOW64\regsvr32.exe /s KhiinPJH.dll
```

You should unregister when you are not actively developing, since things may change
and break at this early stage:

```
regsvr32.exe /s /u KhiinPJH.dll     # /u to unregister

// In a 32-bit cmd.exe
C:\Windows\SysWOW64\regsvr32.exe /u /s KhiinPJH.dll
```

In the KhiinInstaller package, `Registry.wxs` enables proper Windows installation
and uninstallation support. To build or re-build the registry information:

1. Prepare builds of both 64-bit and 32-bit DLLs.
2. Download the [`RegistryChangesView`](https://www.nirsoft.net/utils/registry_changes_view.html) tool.
3. Open the tool and take a registry snapshot.
4. Use both 64- and 32-bit `regsvr32.exe` to install both DLLs.
5. Press OK to begin comparison.
6. Select all keys and values that were added as a result of `KhiinPJH::Registrar.cpp` (namely,
   all the related CLSID and CTF/TIP elements).
7. Export the selection as a `.reg` file (e.g. `khiin-windows.reg`)
8. Use the WiX `heat.exe` tool to build a `.wxs` file: `heat.exe reg .\khiin-windows.reg -dr INSTALLDIR64 -srd -gg -sfrag -suid -out tmp.wxs`
9. Modify the contents of `tmp.wxs` according to the existing `Registry.wxs` formatting,
   and change any path names (to the DLL file) using the existing variables.
