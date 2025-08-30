# Veilbit

Veilbit is a privacy-focused data management tool that enables secure storage, sharing, and retrieval of sensitive information.  
It provides a command-line interface for hiding messages inside image files using steganography.

## Features

- Command-line tool for hiding messages in images
- Supports multiple image formats:
  - BMP
  - PNG
  - JPEG
- Text file input/output support

## Installation

### Clone the repository

```bash
git clone https://github.com/tkalum/veilbit.git
cd veilbit
```



### Build the CLI tool (using CMake)

#### On Linux

```bash
mkdir build
cd build
cmake ..
make
```

#### On Windows (using CMake and a compatible compiler)

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The resulting executable will be in the `build` directory.

## Usage

### Hide a message in an image

```bash
veilbit hide -i <input_image> -m "Secret message" [-o <output_image>]
veilbit hide -i <input_image> -f <message_file> [-o <output_image>]
```

- `-i` specifies the input image file (supports BMP, PNG, and JPEG)
- `-m` specifies the message to hide directly
- `-f` specifies a text file containing the message to hide
- `-o` specifies the output image file.

### Extract a hidden message

```bash
veilbit extract -i <input_image> [-o <output_file>]
```

- `-i` specifies the image file containing the hidden message
- `-o` (optional) specifies the output text file. If not provided, displays message in console

## License

This project is licensed under the [MIT License](LICENSE).
