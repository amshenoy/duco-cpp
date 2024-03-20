
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

`vcpkg install boost --triplet x64-windows`



### Update Packages

1) vcpkg update
2) vcpkg upgrade
3) vcpkg upgrade --no-dry-run --triplet x64-windows




# Build

```sh
cmake -S . -B build
```

```sh
cmake --build build
```


Specify CMake executable and run in Powershell:
`& "C:\Program Files\CMake\bin\cmake.exe" -S . -B build`
`& "C:\Program Files\CMake\bin\cmake.ese" --build build`


# Run

## Download CLI

```sh
./build/download-cli.exe --symbol GBPUSD --start 2023-11-01 --end 2023-11-21
```

