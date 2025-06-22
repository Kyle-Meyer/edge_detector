#include "objectCounter.hh"
#include "binaryMaskEstimator.hh"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -i <image_path>      Input image file path" << std::endl;
    std::cout << "  -o <output_path>     Output base path for results (optional)" << std::endl;
    std::cout << "  -config <config_path> Path to coin configuration file (default: coins.cfg)" << std::endl;
    std::cout << "  -minarea <value>     Minimum object area (default: 50)" << std::endl;
    std::cout << "  -maxarea <value>     Maximum object area (default: 50000)" << std::endl;
    std::cout << "  -mincirc <value>     Minimum circularity for shape filtering (default: 0.3)" << std::endl;
    std::cout << "  -maxaspect <value>   Maximum aspect ratio for shape filtering (default: 3.0)" << std::endl;
    std::cout << "  -noarea              Disable area filtering" << std::endl;
    std::cout << "  -shape               Enable shape filtering" << std::endl;
    std::cout << "  -b <block_size>      Block size for adaptive threshold (default: 21)" << std::endl;
    std::cout << "  -c <C_value>         C parameter for adaptive threshold (default: 10.0)" << std::endl;
    std::cout << "  -k <kernel_size>     Morphological kernel size (default: 7)" << std::endl;
    std::cout << "  -iter <iterations>   Morphological iterations (default: 3)" << std::endl;
    std::cout << "  -display             Display the results" << std::endl;
    std::cout << "  -summary             Print detailed object summary" << std::endl;
    
    // Coin detection options
    std::cout << std::endl << "Coin Detection Options:" << std::endl;
    std::cout << "  -coins               Enable coin classification" << std::endl;
    std::cout << "  -ppmm <value>        Pixels per millimeter for size calibration" << std::endl;
    std::cout << "  -calibrate <x> <y> <type>  Calibrate using known coin at position (x,y)" << std::endl;
    std::cout << "                       Types: penny, nickel, dime, quarter, half, dollar" << std::endl;
    std::cout << "  -preset <name>       Use preset calibration (phone, camera, scanner)" << std::endl;
    std::cout << "  -coinsum             Print coin summary with total value" << std::endl;
    std::cout << "  -interactive         Interactive calibration mode" << std::endl;
    
    std::cout << "  -help                Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " -i coins.jpg -coins -preset phone -coinsum -display" << std::endl;
    std::cout << "  " << programName << " -i coins.jpg -coins -interactive -display" << std::endl;
    std::cout << "  " << programName << " -i coins.jpg -coins -coinsum -display" << std::endl;
    std::cout << "  " << programName << " -i coins.jpg -coins -calibrate 100 150 quarter -coinsum" << std::endl;
    std::cout << "  " << programName << " -i coins.jpg -coins -ppmm 15.7 -coinsum -display" << std::endl;
    std::cout << "  " << programName << " -i objects.png -o results -shape -mincirc 0.5" << std::endl;
}

struct CalibrationPreset {
    std::string name;
    double pixelsPerMM;
    std::string description;
};

std::vector<CalibrationPreset> getCalibrationPresets() {
    return {
        {"phone", 12.0, "Typical smartphone camera at 12 inches"},
        {"camera", 15.0, "Digital camera at moderate distance"},
        {"scanner", 11.8, "Flatbed scanner at 300 DPI"},
        {"macro", 25.0, "Close-up macro photography"},
        {"webcam", 8.0, "Standard webcam at arm's length"},
        {"tablet", 10.0, "Tablet camera at typical distance"}
    };
}

void printPresets() {
    auto presets = getCalibrationPresets();
    std::cout << "\nAvailable calibration presets:" << std::endl;
    for (const auto& preset : presets) {
        std::cout << "  " << preset.name << ": " << preset.pixelsPerMM 
                  << " pixels/mm (" << preset.description << ")" << std::endl;
    }
}

// Function to get preset calibration value
double getPresetCalibration(const std::string& presetName) {
    auto presets = getCalibrationPresets();
    for (const auto& preset : presets) {
        if (preset.name == presetName) {
            return preset.pixelsPerMM;
        }
    }
    return -1.0;  // Not found
}

// Function to convert string to CoinType
CoinType stringToCoinType(const std::string& coinStr) {
    std::string lower = coinStr;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "penny") return CoinType::PENNY;
    if (lower == "nickel") return CoinType::NICKEL;
    if (lower == "dime") return CoinType::DIME;
    if (lower == "quarter") return CoinType::QUARTER;
    return CoinType::UNKNOWN;
}




int main(int argc, char* argv[]) {
    std::cout << "Coin Counter Test Program" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // Parse command line arguments
    std::string inputPath = "";
    std::string outputPath = "";
    std::string configPath = "coins.cfg";
    double minArea = 200.0;
    double maxArea = 50000.0;
    double minCircularity = 0.3;
    double maxAspectRatio = 2.0;
    bool enableAreaFilter = true;
    bool enableShapeFilter = true;
    int blockSize = 11;
    double C = 2.0;
    int kernelSize = 2;
    int iterations = 1;
    bool display = false;
    bool showSummary = false;
    bool showHelp = false;
    
    // Coin detection parameters
    bool enableCoins = false;
    bool showCoinSummary = false;
    double pixelsPerMM = 12.0;  // defaulting to phone 
    std::string presetName = "";
    bool doCalibration = false;
    cv::Point calibrationPoint;
    CoinType calibrationCoinType = CoinType::UNKNOWN;
    bool interactiveMode = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-help") {
            showHelp = true;
        } else if (arg == "-i" && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
        } else if (arg == "-config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg == "-minarea" && i + 1 < argc) {
            minArea = std::stod(argv[++i]);
        } else if (arg == "-maxarea" && i + 1 < argc) {
            maxArea = std::stod(argv[++i]);
        } else if (arg == "-mincirc" && i + 1 < argc) {
            minCircularity = std::stod(argv[++i]);
        } else if (arg == "-maxaspect" && i + 1 < argc) {
            maxAspectRatio = std::stod(argv[++i]);
        } else if (arg == "-noarea") {
            enableAreaFilter = false;
        } else if (arg == "-shape") {
            enableShapeFilter = true;
        } else if (arg == "-b" && i + 1 < argc) {
            blockSize = std::stoi(argv[++i]);
        } else if (arg == "-c" && i + 1 < argc) {
            C = std::stod(argv[++i]);
        } else if (arg == "-k" && i + 1 < argc) {
            kernelSize = std::stoi(argv[++i]);
        } else if (arg == "-iter" && i + 1 < argc) {
            iterations = std::stoi(argv[++i]);
        } else if (arg == "-display") {
            display = true;
        } else if (arg == "-summary") {
            showSummary = true;
        }
        // Coin detection arguments
        else if (arg == "-coins") {
            enableCoins = true;
        } else if (arg == "-coinsum") {
            showCoinSummary = true;
        } else if (arg == "-ppmm" && i + 1 < argc) {
            pixelsPerMM = std::stod(argv[++i]);
        } else if (arg == "-preset" && i + 1 < argc) {
            presetName = argv[++i];
            enableCoins = true;  // Automatically enable coin detection
        } else if (arg == "-calibrate" && i + 3 < argc) {
            doCalibration = true;
            calibrationPoint.x = std::stoi(argv[++i]);
            calibrationPoint.y = std::stoi(argv[++i]);
            calibrationCoinType = stringToCoinType(argv[++i]);
            if (calibrationCoinType == CoinType::UNKNOWN) {
                std::cerr << "Error: Unknown coin type for calibration: " << argv[i] << std::endl;
                return 1;
            }
            enableCoins = true;  // Automatically enable coin detection
        } else if (arg == "-interactive") {
            interactiveMode = true;
            enableCoins = true;  // Automatically enable coin detection
        }
    }
    
    if (showHelp || argc == 1) {
        printUsage(argv[0]);
        printPresets();
        
        if (argc == 1) {
            std::cout << "\nNo arguments provided." << std::endl;
        }
        
        return 0;
    }
    
    // Main processing
    if (!inputPath.empty()) {
        std::cout << "\n=== Processing Image ===" << std::endl;
        std::cout << "Input: " << inputPath << std::endl;
        std::cout << "Config for coins : " << configPath << std::endl;
        
        //print the stats being used for the binary mask
        std::cout << "=======================================================" << std::endl;
        std::cout << "initializing binary mask with the following parameters " << std::endl;
        std::cout << "=======================================================" << std::endl;
        std::cout << "block size: " << blockSize << std::endl;
        std::cout << "Adaptive threshold C: " << C << std::endl;
        std::cout << "kernel size: " << kernelSize << std::endl;
        std::cout << "# of iterations: " << iterations << std::endl;
        // Create instances
        BinaryMaskEstimator maskEstimator;
        ObjectCounter counter(configPath);
         
        
        // Configure mask estimator
        maskEstimator.setAdaptiveThresholdParams(blockSize, C);
        maskEstimator.setMorphologicalParams(kernelSize, iterations);
        
        // Configure object counter
        counter.setAreaFilter(minArea, maxArea);
        counter.setShapeFilter(minCircularity, maxAspectRatio);

        std::cout << "\nConfiguration (Permissive for all coins):" << std::endl;
        std::cout << "  Area filter: " << (enableAreaFilter ? "enabled" : "disabled");
        if (enableAreaFilter) {
            std::cout << " (min: " << minArea << ", max: " << maxArea << ")";
        }
        std::cout << std::endl;

        counter.enableAreaFiltering(enableAreaFilter);
        counter.enableShapeFiltering(enableShapeFilter);
        counter.setCoinClassification(enableCoins);  // Fixed: use setCoinClassification instead
        
        // Handle preset calibration
        if (!presetName.empty()) {
            double presetValue = getPresetCalibration(presetName);
            if (presetValue > 0) {
                pixelsPerMM = presetValue;
                std::cout << "Using preset calibration '" << presetName << "': " 
                          << pixelsPerMM << " pixels/mm" << std::endl;
            } else {
                std::cerr << "Error: Unknown preset '" << presetName << "'" << std::endl;
                printPresets();
                return 1;
            }
        }
        
        if (pixelsPerMM > 0) {
            std::cout << "\n\nsetting to " << pixelsPerMM << std::endl;
            counter.setPixelsPerMM(pixelsPerMM);
        }
        
        // Print configuration
        std::cout << "\nConfiguration:" << std::endl;
        std::cout << "  Area filter: " << (enableAreaFilter ? "enabled" : "disabled");
        if (enableAreaFilter) {
            std::cout << " (min: " << minArea << ", max: " << maxArea << ")";
        }
        std::cout << std::endl;
        
        std::cout << "  Shape filter: " << (enableShapeFilter ? "enabled" : "disabled");
        if (enableShapeFilter) {
            std::cout << " (min circularity: " << minCircularity 
                      << ", max aspect ratio: " << maxAspectRatio << ")";
        }
        std::cout << std::endl;
        
        std::cout << "  Coin detection: " << (enableCoins ? "enabled" : "disabled");
        if (enableCoins && pixelsPerMM > 0) {
            std::cout << " (calibration: " << pixelsPerMM << " pixels/mm)";
        }
        std::cout << std::endl;
        
        // Step 1: Generate binary mask
        std::cout << "\n=== Step 1: Generating Binary Mask ===" << std::endl;
        if (!maskEstimator.loadImage(inputPath)) {
            std::cerr << "Failed to load image: " << inputPath << std::endl;
            return 1;
        }
        
        cv::Mat binaryMask = maskEstimator.estimateBinaryMask();
        if (binaryMask.empty()) {
            std::cerr << "Failed to generate binary mask!" << std::endl;
            return 1;
        }
        
        // Step 2: Load into object counter
        std::cout << "\n=== Step 2: Loading Image and Mask ===" << std::endl;
        if (!counter.loadImage(inputPath)) {
            std::cerr << "Failed to load image into counter!" << std::endl;
            return 1;
        }
        
        if (!counter.loadBinaryMask(binaryMask)) {
            std::cerr << "Failed to load binary mask!" << std::endl;
            return 1;
        }
        
        // Step 3: Count objects
        std::cout << "\n=== Step 3: Counting Objects ===" << std::endl;
        int objectCount = counter.countObjects();
        
        if (objectCount < 0) {
            std::cerr << "Failed to count objects!" << std::endl;
            return 1;
        }
        
        // Step 4: Handle calibration
        if (interactiveMode && enableCoins) {
            // Re-run classification after calibration
            counter.countObjects();
        } else if (doCalibration && enableCoins) {
            std::cout << "\n=== Step 4: Calibration ===" << std::endl;
            counter.calibrateWithKnownCoin(calibrationPoint, calibrationCoinType);
            // Re-run classification after calibration
            counter.countObjects();
        }
        
        // Step 5: Display results
        std::cout << "\n=== Results ===" << std::endl;
        std::string imageName = inputPath.substr(inputPath.find_last_of("/\\") + 1);
        
        if (enableCoins) {
            auto coinCounts = counter.getCoinCounts();
            double totalValue = counter.getTotalValue();
            std::cout << std::string(60, '=') << std::endl;
            std::cout << "COIN DETECTION RESULTS" << std::endl;
            std::cout << std::string(60, '=') << std::endl;
            std::cout << ObjectCounter::generateCoinSummaryText(coinCounts, totalValue) << std::endl;
            std::cout << std::string(60, '=') << std::endl;
            
            if (showCoinSummary) {
                counter.printCoinSummary();
            }
        } else {
            std::cout << std::string(50, '=') << std::endl;
            std::cout << "OBJECT DETECTION RESULTS" << std::endl;
            std::cout << std::string(50, '=') << std::endl;
            std::cout << ObjectCounter::generateSummaryText(objectCount, imageName) << std::endl;
            std::cout << std::string(50, '=') << std::endl;
        }
        
        // Print detailed summary if requested
        if (showSummary) {
            counter.printObjectSummary();
        }
        
        // Save results
        if (!outputPath.empty()) {
            counter.saveResults(outputPath);
            size_t lastDot = outputPath.find_last_of(".");
            std::string basePathNoExt = (lastDot != std::string::npos) ? outputPath.substr(0, lastDot) : outputPath;
            //maskEstimator.saveImage(basePathNoExt + "_original_mask.png", binaryMask);
        } else {
            // Generate default output filename
            size_t lastDot = inputPath.find_last_of(".");
            std::string defaultOutput = inputPath.substr(0, lastDot) + "_results";
            counter.saveResults(defaultOutput);
            //maskEstimator.saveImage(defaultOutput + "_original_mask.png", binaryMask);
        }
        
        // Display if requested
        if (display) {
            counter.displayResults("Coin Detection Results");
        }
        
        std::cout << "\nProcessing completed successfully!" << std::endl;
        
    } else {
        std::cerr << "No input image specified. Use -i <image_path>" << std::endl;
        std::cerr << "Use -help to see all available options." << std::endl;
        return 1;
    }
    
    return 0;
}
