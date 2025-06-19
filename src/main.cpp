#include "objectCounter.hh"
#include "binaryMaskEstimator.hh"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -i <image_path>      Input image file path" << std::endl;
    std::cout << "  -o <output_path>     Output base path for results (optional)" << std::endl;
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
    std::cout << "  -help                Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " -i coins.jpg -display -summary" << std::endl;
    std::cout << "  " << programName << " -i objects.png -o results -shape -mincirc 0.5" << std::endl;
    std::cout << "  " << programName << " -i image.jpg -minarea 100 -maxarea 10000 -display" << std::endl;
}


int main(int argc, char* argv[]) {
    std::cout << "Object Counter Test Program" << std::endl;
    std::cout << "==========================" << std::endl;
    
    // Parse command line arguments
    std::string inputPath = "";
    std::string outputPath = "";
    double minArea = 50.0;
    double maxArea = 50000.0;
    double minCircularity = 0.3;
    double maxAspectRatio = 3.0;
    bool enableAreaFilter = true;
    bool enableShapeFilter = false;
    int blockSize = 21;
    double C = 10.0;
    int kernelSize = 7;
    int iterations = 3;
    bool display = false;
    bool showSummary = false;
    bool showHelp = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-help") {
            showHelp = true;
        } else if (arg == "-i" && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
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
    }
    
    if (showHelp || argc == 1) {
        printUsage(argv[0]);
        
        if (argc == 1) {
            std::cout << "\nNo arguments provided. Running sample tests..." << std::endl;
            
            // Run sample tests if no arguments provided
            //runTestWithSampleImages();
            //runParameterComparison();
        }
        
        return 0;
    }
    
    // Main processing with user-provided image
    if (!inputPath.empty()) {
        std::cout << "\n=== Processing user image ===" << std::endl;
        std::cout << "Input: " << inputPath << std::endl;
        
        // Create binary mask estimator and object counter instances
        BinaryMaskEstimator maskEstimator;
        ObjectCounter counter;
        
        // Configure mask estimator
        maskEstimator.setAdaptiveThresholdParams(blockSize, C);
        maskEstimator.setMorphologicalParams(kernelSize, iterations);
        
        // Configure object counter filtering parameters
        counter.setAreaFilter(minArea, maxArea);
        counter.setShapeFilter(minCircularity, maxAspectRatio);
        counter.enableAreaFiltering(enableAreaFilter);
        counter.enableShapeFiltering(enableShapeFilter);
        
        // Print configuration
        std::cout << "Configuration:" << std::endl;
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
        
        std::cout << "  Threshold params: blockSize=" << blockSize << ", C=" << C << std::endl;
        std::cout << "  Morphology params: kernelSize=" << kernelSize 
                  << ", iterations=" << iterations << std::endl;
        
        // Step 1: Load image into mask estimator and generate binary mask
        std::cout << "\n=== Step 1: Generating Binary Mask ===" << std::endl;
        if (!maskEstimator.loadImage(inputPath)) {
            std::cerr << "Failed to load input image into mask estimator: " << inputPath << std::endl;
            return 1;
        }
        
        cv::Mat binaryMask = maskEstimator.estimateBinaryMask();
        if (binaryMask.empty()) {
            std::cerr << "Failed to generate binary mask!" << std::endl;
            return 1;
        }
        
        // Step 2: Load image and binary mask into object counter
        std::cout << "\n=== Step 2: Loading Image and Mask into Counter ===" << std::endl;
        if (!counter.loadImage(inputPath)) {
            std::cerr << "Failed to load input image into object counter: " << inputPath << std::endl;
            return 1;
        }
        
        if (!counter.loadBinaryMask(binaryMask)) {
            std::cerr << "Failed to load binary mask into object counter!" << std::endl;
            return 1;
        }
        
        // Step 3: Count objects
        std::cout << "\n=== Step 3: Counting Objects ===" << std::endl;
        int objectCount = counter.countObjects();
        
        if (objectCount >= 0) {
            // Generate and display the main result
            std::string imageName = inputPath.substr(inputPath.find_last_of("/\\") + 1);
            std::cout << "\n" << std::string(50, '=') << std::endl;
            std::cout << "RESULT: " << ObjectCounter::generateSummaryText(objectCount, imageName) << std::endl;
            std::cout << std::string(50, '=') << std::endl;
            
            // Print detailed summary if requested
            if (showSummary) {
                counter.printObjectSummary();
            }
            
            // Save results if output path specified
            if (!outputPath.empty()) {
                counter.saveResults(outputPath);
                // Also save the original binary mask from the estimator
                size_t lastDot = outputPath.find_last_of(".");
                std::string basePathNoExt = (lastDot != std::string::npos) ? outputPath.substr(0, lastDot) : outputPath;
                maskEstimator.saveImage(basePathNoExt + "_original_mask.png", binaryMask);
            } else {
                // Generate default output filename
                size_t lastDot = inputPath.find_last_of(".");
                std::string defaultOutput = inputPath.substr(0, lastDot) + "_object_count";
                counter.saveResults(defaultOutput);
                maskEstimator.saveImage(defaultOutput + "_original_mask.png", binaryMask);
            }
            
            // Display if requested
            if (display) {
                counter.displayResults("Object Count Results");
            }
            
            std::cout << "\nProcessing completed successfully!" << std::endl;
        } else {
            std::cerr << "Failed to count objects!" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "No input image specified. Use -i <image_path> to specify an input image." << std::endl;
        std::cerr << "Use -help to see all available options." << std::endl;
        return 1;
    }
    
    return 0;
}
