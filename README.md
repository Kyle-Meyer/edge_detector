# Coin Counter - Computer Vision Object Detection

A C++ application that uses OpenCV to detect, count, and classify coins in images using computer vision techniques.

## Features

- **Binary Mask Generation**: Automatically creates binary masks from input images using adaptive thresholding and morphological operations
- **Object Detection**: Detects circular objects (coins) in images with configurable filtering
- **Coin Classification**: Identifies US coin types (penny, nickel, dime, quarter) based on size
- **Value Calculation**: Automatically calculates total monetary value of detected coins
- **Multiple Calibration Options**: Support for manual calibration, presets, and interactive calibration
- **Visualization**: Displays results with annotated images showing detected coins
- **Export Results**: Saves annotated images and binary masks

## Requirements

- OpenCV 3.4+ (recommended: OpenCV 4.x)
- CMake 3.10+
- C++11 compatible compiler
- pkg-config (for manual builds)

## Installation

### Prerequisites

**Install OpenCV**:
```bash
# Ubuntu/Debian
sudo apt-get install libopencv-dev

# Rocky Linux/RHEL/CentOS
sudo dnf install opencv-devel opencv-contrib-devel cmake gcc-c++

# macOS with Homebrew
brew install opencv

# Windows - download from opencv.org
```

### Build with CMake (Recommended)

1. **Clone and build**:
   ```bash
   git clone <repository-url>
   cd coin-counter
   mkdir build && cd build
   cmake ..
   make
   ```

2. **If OpenCV is not found automatically**:
   ```bash
   cmake -DOpenCV_DIR=/usr/local/lib64/cmake/opencv4 ..
   make
   ```

3. **Run the executable**:
   ```bash
   ./build/bin/BinaryMaskEstimator -i resources/<some_image> -o output_images/<some_image>.png -c 2 -b 11 -k 1 -iter 1 -coins -coinsum -summary
   ```

### Manual Build (Alternative)

```bash
g++ -std=c++11 src/main.cpp src/binaryMaskEstimator.cpp src/objectCounter.cpp -o coin_counter -Ilib `pkg-config --cflags --libs opencv4`
```

## Usage

### Basic Coin Detection
```bash
./bin/BinaryMaskEstimator -i coins.jpg -coins -preset phone -coinsum -display
```

### Manual Calibration
```bash
./bin/BinaryMaskEstimator -i coins.jpg -coins -ppmm 15.7 -coinsum -display
```

### Interactive Calibration
```bash
./bin/BinaryMaskEstimator -i coins.jpg -coins -interactive -display
```

### Custom Object Detection
```bash
./bin/BinaryMaskEstimator -i objects.png -minarea 100 -maxarea 5000 -shape -display
```
### Recommended settings for images provided
```bash
./build/bin/BinaryMaskEstimator -i resources/<some_image> -o output_images/<some_image>.png -c 2 -b 11 -k 1 -iter 1 -coins -coinsum -summary
```

## Command Line Options

### Input/Output
- `-i <path>`: Input image file path
- `-o <path>`: Output base path for results (optional)
- `-display`: Display the results in a window
- `-summary`: Print detailed object summary

### Object Detection Parameters
- `-minarea <value>`: Minimum object area (default: 200)
- `-maxarea <value>`: Maximum object area (default: 50000)
- `-mincirc <value>`: Minimum circularity for shape filtering (default: 0.3)
- `-maxaspect <value>`: Maximum aspect ratio for shape filtering (default: 2.0)
- `-noarea`: Disable area filtering
- `-shape`: Enable shape filtering

### Image Processing Parameters
- `-b <size>`: Block size for adaptive threshold (default: 11)
- `-c <value>`: C parameter for adaptive threshold (default: 2.0)
- `-k <size>`: Morphological kernel size (default: 3)
- `-iter <count>`: Morphological iterations (default: 1)

### Coin Detection Options
- `-coins`: Enable coin classification
- `-coinsum`: Print coin summary with total value
- `-ppmm <value>`: Set pixels per millimeter for size calibration
- `-preset <name>`: Use calibration preset (phone, camera, scanner, macro, webcam, tablet)
- `-calibrate <x> <y> <type>`: Calibrate using known coin at position
- `-interactive`: Interactive calibration mode

## Calibration Presets

| Preset | Pixels/mm | Description |
|--------|-----------|-------------|
| phone | 12.0 | Typical smartphone camera at 12 inches |
| camera | 15.0 | Digital camera at moderate distance |
| scanner | 11.8 | Flatbed scanner at 300 DPI |
| macro | 25.0 | Close-up macro photography |
| webcam | 8.0 | Standard webcam at arm's length |
| tablet | 10.0 | Tablet camera at typical distance |

## Supported Coin Types

- **Penny**: 8.0mm diameter
- **Nickel**: 9.0mm diameter  
- **Dime**: 7.56mm diameter
- **Quarter**: 10.25mm diameter

## Example Output

```
=== COIN DETECTION RESULTS ===
Found 8 coins worth $1.41

Coin breakdown:
  Penny: 1 ($0.01)
  Nickel: 2 ($0.10)
  Dime: 1 ($0.10)
  Quarter: 4 ($1.00)
  Unknown: 0 ($0.00)

Total coins: 8
Total value: $1.41
```

## Files Generated

- `*_annotated.png`: Original image with detected coins highlighted
- `*_mask.png`: Binary mask showing detected objects

## Tips for Best Results

1. **Good Lighting**: Ensure even lighting across the image
2. **Contrasting Background**: Use a plain, contrasting background
3. **Proper Distance**: Maintain consistent distance for better size calibration
4. **Clean Coins**: Clean coins are easier to detect
5. **Calibration**: Use interactive calibration or a known coin for best accuracy

## Project Structure

```
├── CMakeLists.txt            # CMake build configuration
├── src/
│   ├── main.cpp              # Main application with command-line interface
│   ├── binaryMaskEstimator.cpp # Implementation of mask estimation
│   └── objectCounter.cpp     # Implementation of object counting
├── lib/
│   ├── binaryMaskEstimator.hh # Header for binary mask generation
│   └── objectCounter.hh      # Header for object detection and coin classification
├── build/                    # Build directory (created during build)
├── bin/                      # Executable output directory
└── README.md                 # This file
```

## Algorithm Overview

1. **Preprocessing**: Apply Gaussian blur and contrast enhancement
2. **Thresholding**: Use adaptive thresholding to create binary mask
3. **Morphological Operations**: Clean up mask with opening/closing operations
4. **Contour Detection**: Find object contours in the binary mask
5. **Filtering**: Apply area and shape filters to remove noise
6. **Classification**: Classify coins based on diameter measurements
7. **Visualization**: Annotate and display results

## Troubleshooting

**No coins detected?**
- Try adjusting the area filter parameters (`-minarea`, `-maxarea`)
- Check if `-shape` filtering is too restrictive
- Verify image quality and lighting

**Poor classification accuracy?**
- Use interactive calibration (`-interactive`)
- Try different calibration presets (`-preset`)
- Ensure coins are well-separated and clearly visible

**Build errors?**
- Verify OpenCV installation: `pkg-config --libs opencv4`
- Check C++ compiler version (C++11+ required)
