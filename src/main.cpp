#include "binaryMaskEstimator.hh"
#include <iostream>
#include <string>

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " <input_image_path> [output_mask_path]" << std::endl;
    std::cout << "  input_image_path: Path to the input image" << std::endl;
    std::cout << "  output_mask_path: (Optional) Path to save the binary mask" << std::endl;
}
int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return -1;
    }
    
    std::string inputPath = argv[1];
    std::string outputPath = (argc > 2) ? argv[2] : "";
    
    std::cout << "=== Binary Mask Estimation Program ===" << std::endl;
    std::cout << "Input image: " << inputPath << std::endl;
    
    try {
        // Create BinaryMaskEstimator instance
        BinaryMaskEstimator estimator;
        
        // Load the input image
        if (!estimator.loadImage(inputPath)) {
            std::cerr << "Failed to load input image. Exiting..." << std::endl;
            return -1;
        }
        
        // Set custom parameters (optional)
        // You can adjust these parameters based on your specific needs
        estimator.setAdaptiveThresholdParams(11, 2.0);  // blockSize, C
        estimator.setMorphologicalParams(5, 2);         // kernelSize, iterations
        
        std::cout << "\nProcessing image..." << std::endl;
        
        // Estimate the binary mask
        cv::Mat binaryMask = estimator.estimateBinaryMask();
        
        if (binaryMask.empty()) {
            std::cerr << "Failed to estimate binary mask. Exiting..." << std::endl;
            return -1;
        }
        
        // Display the results
        std::cout << "\nDisplaying results (original image and binary mask)..." << std::endl;
        estimator.displayImages("Original Image and Binary Mask");
        
        // Save the binary mask if output path is provided
        if (!outputPath.empty()) {
            estimator.saveImage(outputPath, binaryMask);
        } else {
            // Save with default name
            std::string defaultOutput = "binary_mask_output.png";
            estimator.saveImage(defaultOutput, binaryMask);
        }
        
        // Optional: Show some statistics
        cv::Mat mask = estimator.getBinaryMask();
        int whitePixels = cv::countNonZero(mask);
        int totalPixels = mask.rows * mask.cols;
        double coverage = (double)whitePixels / totalPixels * 100.0;
        
        std::cout << "\n=== Results ===" << std::endl;
        std::cout << "Binary mask generated successfully!" << std::endl;
        std::cout << "Mask coverage: " << coverage << "% of the image" << std::endl;
        std::cout << "White pixels: " << whitePixels << " / " << totalPixels << std::endl;
        
        // Demonstration of different parameter settings
        std::cout << "\n=== Testing Different Parameters ===" << std::endl;
        
        // Test with more aggressive thresholding
        estimator.setAdaptiveThresholdParams(15, 5.0);
        cv::Mat mask2 = estimator.estimateBinaryMask();
        estimator.saveImage("mask_aggressive.png", mask2);
        
        // Test with gentler thresholding
        estimator.setAdaptiveThresholdParams(7, 1.0);
        cv::Mat mask3 = estimator.estimateBinaryMask();
        estimator.saveImage("mask_gentle.png", mask3);
        
        std::cout << "Additional masks saved: mask_aggressive.png, mask_gentle.png" << std::endl;
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV Exception: " << e.what() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Standard Exception: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "\nProgram completed successfully!" << std::endl;
    return 0;
}
