# Install vcpkg
- git clone https://github.com/Microsoft/vcpkg.git
- cd vcpkg
- ./bootstrap-vcpkg.sh 
- sudo ln -s [YOUR_VCPKG_PATH] /usr/bin
  - Ex: sudo ln -s /home/bylin/projects/vcpkg/vcpkg/vcpkg /ur/bin
- vcpkg integrate install

# Run cmake
- Create a `out` folder
- cd out
- cmake .. [YOUR vcpkg.cmake file]
  - Ex: cmake . "-DCMAKE_TOOLCHAIN_FILE=/home/bylin/projects/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake"
  - The vcpkg.cmake will show when you run `vcpkg integrate install`

# Build protof
- [YOUR_PROTOC_EXEC_FILE] --cpp_out=proto proto/*.proto
  - Ex: /home/bylin/projects/vcpkg/vcpkg/packages/protobuf_x64-linux/tools/protobuf/protoc --cpp_out=proto proto/*.proto

# Run make
  - make all