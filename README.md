# Khíín Tâigí Phah Jī Hoat



## Installation



## Usage



## Help

---

# Development

Khíín consists of separate projects in the following folders:

- `engine`: The cross-platform IME engine that plugs in to each app (`libkhiin`). (c++17)
- `windows`: A win32 Text Services Framework [TSF](https://learn.microsoft.com/en-us/windows/win32/tsf/text-services-framework-reference) application. (c++17)
- `android`: A Jetpack Compose based Android Input Method Service application. (kotlin)

## Engine (`khiin.lib` / `libkhiin.a`)

### Dependencies

The following dependencies are included in the source tree (`third_party`) and do not need to be installed separately:

- utf8cpp v3.2.1 (BSL-1.0)
    - Header only, included in `third_party/utf8cpp`
    - [nemtrif/utfcpp](https://github.com/nemtrif/utfcpp)
- unilib v3.2.0 (MPL-2.0)
    - Modified to be header only, included in `third_party/unilib`
    - [ufal/unilib](https://github.com/ufal/unilib)
- simpleini v4.19 (MIT)
    - Header only, included in `third_party/simpleini`
    - [brofield/simpleini](https://github.com/brofield/simpleini/)
- spdlog v1.9.2 (MIT)
    - Header only, included in `third_party/spdlog`
    - [gabime/spdlog](https://github.com/gabime/spdlog)

The following dependencies require additional installation:

- [SQLiteCpp 3.40.1#1 (MIT)](https://github.com/SRombauts/SQLiteCpp)
    - sqlite3 (included with SQLiteCpp)
- [protobuf v3.21.12](https://github.com/protocolbuffers/protobuf/releases/tag/v3.21.12)
- [GTest v1.13.0](https://github.com/google/googletest)

These packages can be installed with [`vcpkg`](https://github.com/microsoft/vcpkg/#getting-started):

```
git clone --depth 1 --branch 2023.02.24 https://github.com/microsoft/vcpkg
cd vcpkg
bootstrap-vcpkg.bat -disableMetrics
./vcpkg install sqlitecpp protobuf gtest --triplet=x64-windows-static-md
```

For release you will also need the x86 packages:

```
./vcpkg install sqlitecpp protobuf gtest --triplet=x86-windows-static-md
```

For Visual Studio integration:

```
./vcpkg integrate install
```

You must also set the environment variable `VCPKG_ROOT=/path/to/vcpkg`.

### Protobuf

- [protobuf v3.21.12](https://github.com/protocolbuffers/protobuf/releases/tag/v3.21.12). 

Supports protobuf-lite. Full protobuf is not needed.

Generated files are checked in to the `proto` folder, and do not need to be
regenerated unless you make any changes. For building, you only need to
link to the `libprotobuf-lite[d].lib` (`d` for Debug version).

To re-build protobuf generated files after modification, install `protoc`
and use the following command from the `proto` folder:

```
/path/to/protoc.exe --cpp_out=proto proto/*.proto
```

If you are using `vcpkg` on Windows, you will find this tool at:

```
/path/to/vcpkg/installed/x64-windows/tools/protobuf/protoc.exe
```

### Cmake

The `engine` folder contains a `cmake` project. For building in
Visual Studio, update `CmakeSettings.json::cmakeToolchain`
to point to your `vcpkg.cmake` file.

The engine has been built and tested to work on Linux (Ubuntu) with GCC 9,
see `engine/linux_build.md` for details.

Protobuf is used for all app-to-engine communication, in order to simplify
communication across boundaries like JNI or over the internet via gRPC.

At present, the engine only supports one client app at a time, but it
should be possible to extend this to multiple clients to make at least
the basic features available in an online demo.

### Database

The `khiin.db` SQLite database contains all of the information required
for default input conversion and character selection (ordering). This
database is generated from the data provided by Tâijī Siā, and may be
found in the [khiin-data](https://github.com/aiongg/khiin-data) repository.

The database is continually updated with user data during use, to
improve candidate prediction based on a simple N-gram model that
currently uses 1-gram and 2-gram frequencies. In the future this may be
extended to other precition algorithms for better results.

Users may provide an additional custom dictionary file, which
is simply a text file listing rows of space-delimited `input output`
options to display as candidates. (Everything after the first space
is taken as the output.) These candidates are displayed in addition
to the default database.

At present, data is not shared at all, and is strictly used within
the application itself. In future we would like to add an option
to sync user's data across devices, and an option to allow users
to share their (anonymized) data with us for improving our corpus.

---

## Windows IME

The Windows TSF `TextInputProcessor` DLL application is found in the
`windows/KhiinWindows.sln` Visual Studio solution. Built using
Visual Studio Community 2022.

### DLL Registration

Prior to deveopment, you must uninstall any previously installed
version of the IME from the system. After building both `libkhiin` and
the `KhiinWindows.sln` solution, you can manually register the DLL file
in an elevated PowerShell as follows:

```
cd windows\out\build\x64-Debug
regsvr32.exe /s KhiinPJH.dll    # /s for silent install
```

To register the x86 (32-bit) DLL, use an elevated 32-bit `cmd.exe`
prompt (`C:\Windows\SysWOW64\cmd.exe`, not PowerShell), then run:

```
cd windows\out\build\x86-Debug
C:\Windows\SysWOW64\regsvr32.exe /s KhiinPJH.dll
```

You should unregister these DLLs when you are not actively developing:

```
regsvr32.exe /s /u KhiinPJH.dll     # /u to unregister

# In a 32-bit cmd.exe
C:\Windows\SysWOW64\regsvr32.exe /u /s KhiinPJH.dll
```

The registration command will run the DLL using the entry point
`tip/DllModule.cpp#DllRegisterServer`, which delegates all
of the registration commands to `Registrar.cpp`. The main requirement
is simply to write the registry entries needed for the IME show up in
the Windows taskbar and language settings pages.

### TIP (Text Input Processor)

The Windows TSF (Text Services Framework) is an expansive and highly
over-engineered tool, at least for our use case. However, we need good
TSF integration to ensure that the IME works with as many applications
as possible.

The entire application has been written from scratch in modern C++17, using
some tools from `C++/WinRT`(https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)
which help make COM programming easier and less error-prone.
(In particular `com_ptr` and `implements` are very useful.)

We referred to at least half a dozen different open source TSF IMEs
throughout development, since Microsoft documentation is imprecise
or out of date in many areas.

- [mozc/tip](https://chromium.googlesource.com/external/mozc/+/master/src/win32/tip)
- [microsoft/Windows-classic-samples](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/input/tsf/textservice)
- [dinhngtu/VietType](https://github.com/dinhngtu/VietType)
- [chewing/windows-chewing-tsf](https://github.com/chewing/windows-chewing-tsf)
- [EasyIME/libIME2](https://github.com/EasyIME/libIME2)
- [rime/weasel](https://github.com/rime/weasel/)
- [keymanapp/keyman](https://github.com/keymanapp/keyman/tree/master/windows/src/engine/kmtip)

Hopefully this application will also serve as a good reference point
for others who wish to build Windows TSF IMEs.

The main DLL module code is found in the `windows/tip` folder. The following
are important classes to be aware of:

- `TextService`: implements the main TSF interface `ITfTextInputProcessorEx`,
  among others. Also the main interface used to pass messages between
  different parts of the program.
- `EngineController`: connects to the actual processing `Engine` from libkhiin.
- `CompositionMgr`: manipulates the in-line "pre-edit" text shown at the
  caret position in an application, including decorations like underlines
  for different states of input
- `CandidateListUI`: prepares data for and controls display of the `CandidateWindow`
- `KeyEventSink`: collect key events from the system, including regular keys and
  "preserved keys" (a.k.a. keyboard shortcuts registered with TSF)
- `EditSession`: obtains the `TfEditCookie` (session token). A new session token
  is required for every interaction with the composition. (Namely: setting,
  clearing, or measuring text, etc.)
- `KhiinClassFactory`: creates a `TextService` when the DLL is initialized
  by TSF

There are many other classes, most of which provide some minor but required
function, some of which don't seem to be required but are found in most
example IMEs, including Microsoft's sample IME. In any event, those classes
listed above are where most of the actual work happens.

The Windows app is only intended to support Windows 10 and above,
and has not been tested on Windows 7 or 8. There are almost definitely
some API calls or libraries used which are not available prior
to Windows 10. If you want to develop Windows 7/8 support, feel free
to work on it but it might be a big project for a small (and shrinking)
user base. The Windows app includes a 32-bit DLL only to support
32-bit applications on 64-bit Windows 10.

### Settings App

The settings app is a basic [property sheet](https://docs.microsoft.com/en-us/windows/win32/controls/property-sheets)
application, with a few dialog boxes and standard Win32 controls.

Most settings are saved in `khiin_config.ini`, which is then read
by the DLL to load the configuration options.

Some settings are saved in the registry, but we will migrate
these so that everything is saved in the `ini`.

### WiX Installer

In the `KhiinInstaller` package, `Registry.wxs` enables proper Windows installation
and uninstallation support. Many other IMEs use the DLL's own self-registration,
but this may leave behind unneeded / unwated registry data upon uninstallation.

The recommended solution is to provide all necessary registration information directly
in the `.msi` installation package. In order to obtain this information, you may
use the self-registering DLL and take a diff of the registry before and after
registration. Export the diff to a `.reg` file, which can then be consumed by WiX.
Below is a step-by-step guide to recreating `Registry.wxs`:

1. Prepare builds of both 64-bit and 32-bit DLLs.
2. Download the [RegistryChangesView](https://www.nirsoft.net/utils/registry_changes_view.html) tool.
3. Open the tool and take a registry snapshot.
4. Use both 64- and 32-bit `regsvr32.exe` to install both DLLs.
5. Press OK to begin comparison.
6. Select all keys and values that were added as a result of `KhiinPJH::Registrar.cpp` (namely,
   all the related CLSID and CTF/TIP elements).
7. Export the selection as a `.reg` file (e.g. `khiin-windows.reg`)
8. Use the WiX `heat.exe` tool to build a `.wxs` file: `heat.exe reg .\khiin-windows.reg -dr INSTALLDIR64 -srd -gg -sfrag -suid -out tmp.wxs`
9. Modify the contents of `tmp.wxs` according to the existing `Registry.wxs` formatting,
   and change any path names (to the DLL file) using the existing variables.

If any registry entries need to be added or changed (see `tip/Registrar.[h/cpp]`), you
must re-do the above steps and update `Registry.wxs` accordingly.

The installer comes in two language varieties, Taiwanese (HanLo) and
English. They install the exact same application binaries, and the application
itself supports Taiwanese (HanLo), Taiwanese (Lomaji), and English. The
application language is user-configurable from the Settings app.

---

## Android App

The Android IME is currently in progress. It is a modern Jetpack Compose app written
in Kotlin, and uses the Android NDK / JNI to load and communicate with `libkhiin`.

### Setting up the NDK toolchain

`libkhiin` needs to be cross-compiled for the four Android platforms:

- arm
- arm64
- x86
- x86-64

You may install the platform specific `libkhiin` dependencies again using `vcpkg`:

```
./vcpkg install sqlitecpp:x86-android sqlitecpp:x64-android sqlitecpp:arm-neon-android sqlitecpp:arm64-android
./vcpkg install protobuf:x86-android protobuf:x64-android protobuf:arm-neon-android protobuf:arm64-android
```

You must also set these environment variables:

- `ANDROID_NDK_HOME=/path/to/Android/Sdk/ndk/25.2.9519653`
- `VCPKG_ROOT=/path/to/vcpkg`

For build details, refer to `android/app/build.gradle.kts` sections `externalNativeBuild`
(there are two), and the associated `CMakeLists.txt` in the same folder.

Currently, the libkhiin and android cmake workflows are not very well structured.
They should probably be improved to use `find_package` in the future
(or a different build system altogether, like Bazel).
