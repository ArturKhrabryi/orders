# Description

Orders is a cross-platform application built with C++ and Qt6, featuring optional Python utilities.  
The application provides a convenient way to manage product lists for orders — an organized alternative to using raw Excel tables.  
It can export the internal database to a formatted .ods spreadsheet using the toOds Python utility, eliminating the need for manual table styling.  
An additional barcodeGenerator tool creates EAN-13 barcode images directly from text codes.  
The project is built with CMake, runs on Windows and Linux, and includes Polish localization support.  

## Prerequisites
- CMake ≥ 3.28
- C++20 compiler
- Qt6 with components: Widgets, Sql, Svg, LinguistTools
- Python3 (only needed if you build the optional Python tools)  
Note: PyInstaller and Python dependencies are installed automatically into a build-local virtual env.

## Configure Options
- `-DBUILD\_TO\_ODS=ON|OFF (default ON)`
- `-DBUILD\_BARCODE\_GENERATOR=ON|OFF (default ON)`

## Building
From the project root:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cd build
cmake --build .
cpack -G ZIP # you can also use TGZ as a generator
```

The result will be an archive with the *orders* application and two Python utilities: toOds and barcodeGenerator
