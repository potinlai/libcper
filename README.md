# CPER JSON Representation & Conversion Library
This repository specifies a structure for representing UEFI CPER records (as described in UEFI Specification Appendix N) in a human-readable JSON format, in addition to a library which can readily convert back and forth between the standard CPER binary format and the specified structured JSON.

## Prerequisites
Before building this library and its associated tools, you must have CMake (>=3.10) and SWIG installed, with the following CMake modules available:
- `FetchContent`
- `GoogleTest`
- `FindSWIG`
- `UseSWIG`

You should also have Python3 and associated libraries installed for the purposes of building the Python bindings.

## Building
This project uses CMake (>=3.10). To build for native architecture, simply run:
```
cmake .
make
```
A static library file for the parsing library will be written to `lib/`, and test executables will be written to `bin/`.

### Cross Compilation
To cross compile for ARM/AArch64 architecture from x86, instead use the below commands (ensure `cmake clean .` beforehand).
You will need either the `arm-linux-gnueabi` or `aarch64-linux-gnu` toolchain installed.
```bash
cmake -DCMAKE_TOOLCHAIN_FILE="toolchains/arm-toolchain.cmake" -S. -Bbin # arm-linux-gnueabi
cmake -DCMAKE_TOOLCHAIN_FILE="toolchains/aarch64-toolchain.cmake" -S. -Bbin # aarch64-linux-gnu
make
```

## Usage
This project comes with several binaries to help you deal with CPER binary and CPER-JSON. The first of these is `cper-convert`, which is a command line tool that can be found in `bin/`. With this, you can convert to and from CPER and CPER-JSON through the command line. An example usage scenario is below:
```
cper-convert to-cper samples/cper-json-test-arm.json --out cper.dump
cper-convert to-json cper.generated.dump
```
Another tool bundled with this repository is `cper-generate`, found in `bin/`. This allows you to generate pseudo-random valid CPER records with sections of specified types for testing purposes. An example use of the program is below:
```
cper-generate --out cper.generated.dump --sections generic ia32x64
```
Help for both of these tools can be accessed through using the `--help` flag in isolation.

Finally, a static library containing symbols for converting CPER and CPER-JSON between an intermediate JSON format can be found generated at `lib/libcper-parse.a`. This contains the following useful library symbols:
```
json_object* cper_to_ir(FILE* cper_file);
void ir_to_cper(json_object* ir, FILE* out);
```

This library also has Python bindings generated on build, which are placed at `lib/cperparse.py`. The static library `_cperparse_pylib.a` (as well as the C file `cper-parsePYTHON_wrap.c`) are generated specifically for the purpose of wrapping types for the Python library, and should not be used as a standard static C library.

## Specification
The specification for this project's CPER-JSON format can be found in `specification/`, defined in both JSON Schema format and also as a LaTeX document.
Specification for the CPER binary format can be found in [UEFI Specification Appendix N](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_9_2021_03_18.pdf) (2021/03/18).