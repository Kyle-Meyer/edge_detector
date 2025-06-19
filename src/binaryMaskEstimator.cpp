#include "binaryMaskEstimator.hh"
#include <iostream>
#include <opencv2/opencv.hpp>

// Constructor
BinaryMaskEstimator::BinaryMaskEstimator() 
    : blockSize(11), C(2.0), morphKernelSize(5), morphIterations(2) 
{
    //magical values that I just found by playing with the program
    setAdaptiveThresholdParams(21, 10.0);
    setMorphologicalParams(7, 3);
}

// Destructor
BinaryMaskEstimator::~BinaryMaskEstimator() {
    cv::destroyAllWindows();
}

// Load image from file path
bool BinaryMaskEstimator::loadImage(const std::string& imagePath) {
    inputImage = cv::imread(imagePath, cv::IMREAD_COLOR);
    
    if (inputImage.empty()) {
        std::cerr << "Error: Could not load image from " << imagePath << std::endl;
        return false;
    }
    
    std::cout << "Image loaded successfully: " << imagePath << std::endl;
    showImageInfo(inputImage, "Input Image");
    return true;
}

// Load image from cv::Mat
bool BinaryMaskEstimator::loadImage(const cv::Mat& image) {
    if (image.empty()) {
        std::cerr << "Error: Input image is empty" << std::endl;
        return false;
    }
    
    inputImage = image.clone();
    std::cout << "Image loaded successfully from cv::Mat" << std::endl;
    showImageInfo(inputImage, "Input Image");
    return true;
}

// Main method to estimate binary mask
cv::Mat BinaryMaskEstimator::estimateBinaryMask() {
    if (inputImage.empty()) {
        std::cerr << "Error: No input image loaded" << std::endl;
        return cv::Mat();
    }
    
    cv::Mat processedImage = inputImage.clone();
    
    // Step 1: Preprocess the image
    preprocessImage(processedImage);
    
    // Step 2: Convert to grayscale if needed
    cv::Mat grayImage;
    if (processedImage.channels() == 3) {
        cv::cvtColor(processedImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = processedImage.clone();
    }
    
    // Step 3: Apply adaptive thresholding
    applyAdaptiveThreshold(grayImage, binaryMask);
    
    // Step 4: Apply morphological operations
    applyMorphologicalOperations(binaryMask);
    
    // Step 5: Remove small components
    removeSmallComponents(binaryMask, 100);
    
    std::cout << "Binary mask estimation completed" << std::endl;
    return binaryMask.clone();
}

// Preprocess the input image
void BinaryMaskEstimator::preprocessImage(cv::Mat& image) {
    // Apply Gaussian blur to reduce noise
    cv::GaussianBlur(image, image, cv::Size(5, 5), 0);
    
    // Enhance contrast using CLAHE if it's a color image
    if (image.channels() == 3) {
        cv::Mat lab;
        cv::cvtColor(image, lab, cv::COLOR_BGR2Lab);
        
        std::vector<cv::Mat> labChannels;
        cv::split(lab, labChannels);
        
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        clahe->apply(labChannels[0], labChannels[0]);
        
        cv::merge(labChannels, lab);
        cv::cvtColor(lab, image, cv::COLOR_Lab2BGR);
    }
}

// Apply adaptive thresholding
void BinaryMaskEstimator::applyAdaptiveThreshold(const cv::Mat& grayImage, cv::Mat& mask) {
    cv::adaptiveThreshold(grayImage, mask, 255, 
                         cv::ADAPTIVE_THRESH_GAUSSIAN_C, 
                         cv::THRESH_BINARY_INV, blockSize, C);
}

// Apply morphological operations to clean up the mask
void BinaryMaskEstimator::applyMorphologicalOperations(cv::Mat& mask) {
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
                                               cv::Size(morphKernelSize, morphKernelSize));
    
    // Close small gaps
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), morphIterations);
    
    // Open to remove small noise
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), morphIterations);
}

// Remove small connected components
void BinaryMaskEstimator::removeSmallComponents(cv::Mat& mask, int minArea) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    
    cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Create a new mask with only large components
    cv::Mat cleanMask = cv::Mat::zeros(mask.size(), mask.type());
    
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area >= minArea) {
            cv::fillPoly(cleanMask, std::vector<std::vector<cv::Point>>{contours[i]}, cv::Scalar(255));
        }
    }
    
    mask = cleanMask;
}

// Set adaptive threshold parameters
void BinaryMaskEstimator::setAdaptiveThresholdParams(int blockSize, double C) {
    this->blockSize = (blockSize % 2 == 0) ? blockSize + 1 : blockSize; // Ensure odd number
    this->C = C;
}

// Set morphological operation parameters
void BinaryMaskEstimator::setMorphologicalParams(int kernelSize, int iterations) {
    this->morphKernelSize = kernelSize;
    this->morphIterations = iterations;
}

// Save image to file
void BinaryMaskEstimator::saveImage(const std::string& outputPath, const cv::Mat& image) {
    if (image.empty()) {
        std::cerr << "Error: Cannot save empty image" << std::endl;
        return;
    }
    
    bool success = cv::imwrite(outputPath, image);
    if (success) {
        std::cout << "Image saved successfully: " << outputPath << std::endl;
    } else {
        std::cerr << "Error: Could not save image to " << outputPath << std::endl;
    }
}

// Display original and binary mask images
void BinaryMaskEstimator::displayImages(const std::string& windowName) {
    if (inputImage.empty() || binaryMask.empty()) {
        std::cerr << "Error: Images not ready for display" << std::endl;
        return;
    }
    
    cv::Mat combined = combineImages(inputImage, binaryMask);
    
    cv::imshow(windowName, combined);
    std::cout << "Press any key to close the display window..." << std::endl;
    cv::waitKey(0);
    cv::destroyWindow(windowName);
}

// Getter methods
cv::Mat BinaryMaskEstimator::getInputImage() const {
    return inputImage.clone();
}

cv::Mat BinaryMaskEstimator::getBinaryMask() const {
    return binaryMask.clone();
}

// Static method to combine two images side by side
cv::Mat BinaryMaskEstimator::combineImages(const cv::Mat& img1, const cv::Mat& img2) {
    cv::Mat combined;
    cv::Mat img2_color;
    
    // Convert binary mask to 3-channel for display
    if (img2.channels() == 1) {
        cv::cvtColor(img2, img2_color, cv::COLOR_GRAY2BGR);
    } else {
        img2_color = img2.clone();
    }
    
    // Resize images to same height if needed
    cv::Mat img1_resized = img1.clone();
    cv::Mat img2_resized = img2_color.clone();
    
    if (img1.rows != img2_color.rows) {
        int targetHeight = std::min(img1.rows, img2_color.rows);
        double ratio1 = (double)targetHeight / img1.rows;
        double ratio2 = (double)targetHeight / img2_color.rows;
        
        cv::resize(img1, img1_resized, cv::Size(), ratio1, ratio1);
        cv::resize(img2_color, img2_resized, cv::Size(), ratio2, ratio2);
    }
    
    cv::hconcat(img1_resized, img2_resized, combined);
    return combined;
}

// Static method to show image information
void BinaryMaskEstimator::showImageInfo(const cv::Mat& image, const std::string& imageName) {
    std::cout << imageName << " Info:" << std::endl;
    std::cout << "  Size: " << image.cols << "x" << image.rows << std::endl;
    std::cout << "  Channels: " << image.channels() << std::endl;
    std::cout << "  Type: " << image.type() << std::endl << std::endl;
}
