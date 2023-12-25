
# Setup

### Update vcpkg
`.\bootstrap-vcpkg.bat`

### Install Dependencies

```sh
.\vcpkg install boost:x64-windows
.\vcpkg install curl:x64-windows
.\vcpkg install liblzma:x64-windows
.\vcpkg install fmt:x64-windows
```

# Build

```sh
cmake -S . -B build
```

```sh
cmake --build build ; ./build/client.exe
```
