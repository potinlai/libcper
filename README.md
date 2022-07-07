# CPER JSON Representation & Conversion Library
This repository specifies a structure for representing UEFI CPER records (as described in UEFI Specification Appendix N) in a human-readable JSON format, in addition to a library which can readily convert back and forth between the standard CPER binary format and the specified structured JSON.

## Building
This project uses CMake (>=3.10). To build, simply run:
```
cmake .
make
```
A static library file for the parsing library will be written to `lib/`, and test executables will be written to `bin/`.

## Specification
The specification for this project can be found in `specification/`.
Specification for the CPER binary format can be found in [UEFI Specification Appendix N](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_9_2021_03_18.pdf) (2021/03/18).