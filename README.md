# Veilbit

Veilbit is a privacy-focused data management tool that enables secure storage, sharing, and retrieval of sensitive information.  
It provides a command-line interface for hiding messages inside image files using steganography.

## Features

- Command-line tool for hiding messages in images
- Only bmp format support (PNG adding soon...)

## Installation

### Clone the repository

```bash
git clone https://github.com/yourusername/veilbit.git
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
./veilbit hide -i input.bmp -m "Secret message" [-o output.bmp]
```

- `-i` specifies the input image file (BMP format).
- `-m` specifies the message to hide.
- `-o` (optional) specifies the output image file. Defaults to `output.bmp` if not provided.

## License

This project is licensed under the [MIT License](LICENSE).

## Contact

For questions or feedback, please contact [veilbit.support@example.com](mailto:veilbit.support@example.com).