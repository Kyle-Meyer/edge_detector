#include "objectCounter.hh"
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
    std::cout << "  -b <block_size>      Block size for adaptive threshold (default: 11)" << std::endl;
    std::cout << "  -c <C_value>         C parameter for adaptive threshold (default: 2.0)" << std::endl;
    std::cout << "  -k <kernel_size>     Morphological kernel size (default: 5)" << std::endl;
    std::cout << "  -iter <iterations>   Morphological iterations (default: 2)" << std::endl;
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
        
        ObjectCounter counter;
        
        // Set filtering parameters
        counter.setAreaFilter(minArea, maxArea);
        counter.setShapeFilter(minCircularity, maxAspectRatio);
        counter.enableAreaFiltering(enableAreaFilter);
        counter.enableShapeFiltering(enableShapeFilter);
        
        // Set mask estimator parameters
        counter.setMaskEstimatorParams(blockSize, C, kernelSize, iterations);
        
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
        
        // Load and process the image
        if (counter.loadImage(inputPath)) {
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
                } else {
                    // Generate default output filename
                    size_t lastDot = inputPath.find_last_of(".");
                    std::string defaultOutput = inputPath.substr(0, lastDot) + "_object_count";
                    counter.saveResults(defaultOutput);
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
            std::cerr << "Failed to load input image: " << inputPath << std::endl;
            return 1;
        }
    }
    
    return 0;
}
