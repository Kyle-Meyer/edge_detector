#include "objectCounter.hh"
#include <iostream>
#include <algorithm>
#include <iomanip>

// Constructor
ObjectCounter::ObjectCounter() 
    : minObjectArea(50.0), maxObjectArea(50000.0), minCircularity(0.3), 
      maxAspectRatio(3.0), useAreaFiltering(true), useShapeFiltering(false)
{
    // Default parameters work well for coins and similar circular objects
}

// Destructor
ObjectCounter::~ObjectCounter() {
    cv::destroyAllWindows();
}

// Load image from file path
bool ObjectCounter::loadImage(const std::string& imagePath) {
    inputImage = cv::imread(imagePath, cv::IMREAD_COLOR);
    
    if (inputImage.empty()) {
        std::cerr << "Error: Could not load image from " << imagePath << std::endl;
        return false;
    }
    
    std::cout << "Image loaded successfully: " << imagePath << std::endl;
    showImageInfo(inputImage, "Input Image");
    
    // Clear previous results
    detectedObjects.clear();
    binaryMask = cv::Mat();
    
    return true;
}

// Load image from cv::Mat
bool ObjectCounter::loadImage(const cv::Mat& image) {
    if (image.empty()) {
        std::cerr << "Error: Input image is empty" << std::endl;
        return false;
    }
    
    inputImage = image.clone();
    std::cout << "Image loaded successfully from cv::Mat" << std::endl;
    showImageInfo(inputImage, "Input Image");
    
    // Clear previous results
    detectedObjects.clear();
    binaryMask = cv::Mat();
    
    return true;
}

// Main method to count objects
int ObjectCounter::countObjects() {
    if (inputImage.empty()) {
        std::cerr << "Error: No input image loaded" << std::endl;
        return -1;
    }
    
    std::cout << "Starting object counting process..." << std::endl;
    
    // Step 1: Generate binary mask using the mask estimator
    if (!maskEstimator.loadImage(inputImage)) {
        std::cerr << "Error: Failed to load image into mask estimator" << std::endl;
        return -1;
    }
    
    binaryMask = maskEstimator.estimateBinaryMask();
    if (binaryMask.empty()) {
        std::cerr << "Error: Failed to generate binary mask" << std::endl;
        return -1;
    }
    
    std::cout << "Binary mask generated successfully" << std::endl;
    
    // Step 2: Find contours in the binary mask
    findContours();
    
    // Step 3: Analyze objects and filter based on criteria
    analyzeObjects();
    
    int objectCount = static_cast<int>(detectedObjects.size());
    std::cout << "Object counting completed. Found " << objectCount << " objects." << std::endl;
    
    return objectCount;
}

// Find contours in the binary mask
void ObjectCounter::findContours() {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    
    cv::findContours(binaryMask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    std::cout << "Found " << contours.size() << " contours" << std::endl;
    
    // Convert contours to ObjectInfo structures
    detectedObjects.clear();
    detectedObjects.reserve(contours.size());
    
    for (size_t i = 0; i < contours.size(); i++) {
        ObjectInfo obj;
        obj.id = static_cast<int>(i);
        obj.contour = contours[i];
        obj.area = cv::contourArea(contours[i]);
        obj.boundingBox = cv::boundingRect(contours[i]);
        
        // Calculate center
        cv::Moments moments = cv::moments(contours[i]);
        if (moments.m00 != 0) {
            obj.center.x = static_cast<float>(moments.m10 / moments.m00);
            obj.center.y = static_cast<float>(moments.m01 / moments.m00);
        } else {
            obj.center.x = static_cast<float>(obj.boundingBox.x + obj.boundingBox.width / 2);
            obj.center.y = static_cast<float>(obj.boundingBox.y + obj.boundingBox.height / 2);
        }
        
        // Calculate shape properties
        obj.circularity = calculateCircularity(contours[i], obj.area);
        obj.aspectRatio = calculateAspectRatio(obj.boundingBox);
        
        detectedObjects.push_back(obj);
    }
}

// Analyze objects and filter based on criteria
void ObjectCounter::analyzeObjects() {
    std::vector<ObjectInfo> validObjects;
    
    for (const auto& obj : detectedObjects) {
        if (isValidObject(obj)) {
            validObjects.push_back(obj);
        }
    }
    
    // Update detected objects with only valid ones
    detectedObjects = validObjects;
    
    // Reassign IDs
    for (size_t i = 0; i < detectedObjects.size(); i++) {
        detectedObjects[i].id = static_cast<int>(i);
    }
    
    std::cout << "After filtering: " << detectedObjects.size() << " valid objects" << std::endl;
}

// Calculate circularity (4π * area / perimeter²)
double ObjectCounter::calculateCircularity(const std::vector<cv::Point>& contour, double area) {
    double perimeter = cv::arcLength(contour, true);
    if (perimeter == 0) return 0.0;
    
    return (4.0 * CV_PI * area) / (perimeter * perimeter);
}

// Calculate aspect ratio (width / height)
double ObjectCounter::calculateAspectRatio(const cv::Rect& boundingBox) {
    if (boundingBox.height == 0) return 0.0;
    return static_cast<double>(boundingBox.width) / static_cast<double>(boundingBox.height);
}

// Check if object meets filtering criteria
bool ObjectCounter::isValidObject(const ObjectInfo& obj) {
    // Area filtering
    if (useAreaFiltering) {
        if (obj.area < minObjectArea || obj.area > maxObjectArea) {
            return false;
        }
    }
    
    // Shape filtering
    if (useShapeFiltering) {
        if (obj.circularity < minCircularity || obj.aspectRatio > maxAspectRatio) {
            return false;
        }
    }
    
    return true;
}

// Draw annotations on the image
void ObjectCounter::drawObjectAnnotations(cv::Mat& image) {
    for (size_t i = 0; i < detectedObjects.size(); i++) {
        const ObjectInfo& obj = detectedObjects[i];
        
        // Draw contour
        std::vector<std::vector<cv::Point>> contours = {obj.contour};
        cv::drawContours(image, contours, -1, cv::Scalar(0, 255, 0), 2);
        
        // Draw bounding box
        cv::rectangle(image, obj.boundingBox, cv::Scalar(255, 0, 0), 1);
        
        // Draw center point
        cv::circle(image, obj.center, 3, cv::Scalar(0, 0, 255), -1);
        
        // Draw object number
        std::string label = std::to_string(i + 1);
        cv::putText(image, label, cv::Point(obj.center.x - 10, obj.center.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);
    }
    
    // Draw summary text
    std::string summary = "Objects detected: " + std::to_string(detectedObjects.size());
    cv::putText(image, summary, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 
                1.0, cv::Scalar(255, 255, 255), 2);
}

// Set area filter parameters
void ObjectCounter::setAreaFilter(double minArea, double maxArea) {
    this->minObjectArea = minArea;
    this->maxObjectArea = maxArea;
}

// Set shape filter parameters
void ObjectCounter::setShapeFilter(double minCircularity, double maxAspectRatio) {
    this->minCircularity = minCircularity;
    this->maxAspectRatio = maxAspectRatio;
}

// Set mask estimator parameters
void ObjectCounter::setMaskEstimatorParams(int blockSize, double C, int kernelSize, int iterations) {
    maskEstimator.setAdaptiveThresholdParams(blockSize, C);
    maskEstimator.setMorphologicalParams(kernelSize, iterations);
}

// Enable/disable area filtering
void ObjectCounter::enableAreaFiltering(bool enable) {
    this->useAreaFiltering = enable;
}

// Enable/disable shape filtering
void ObjectCounter::enableShapeFiltering(bool enable) {
    this->useShapeFiltering = enable;
}

// Get object information
std::vector<ObjectInfo> ObjectCounter::getObjectInfo() const {
    return detectedObjects;
}

// Print object summary
void ObjectCounter::printObjectSummary() const {
    std::cout << "\n=== Object Detection Summary ===" << std::endl;
    std::cout << "Total coins detected: " << detectedObjects.size() << std::endl;
    
    if (!detectedObjects.empty()) {
        std::cout << "\nObject Details:" << std::endl;
        std::cout << std::setw(4) << "ID" << std::setw(10) << "Area" 
                  << std::setw(12) << "Center X" << std::setw(12) << "Center Y"
                  << std::setw(12) << "Circularity" << std::setw(12) << "Aspect Ratio" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        for (const auto& obj : detectedObjects) {
            std::cout << std::setw(4) << (obj.id + 1)
                      << std::setw(10) << std::fixed << std::setprecision(1) << obj.area
                      << std::setw(12) << std::fixed << std::setprecision(1) << obj.center.x
                      << std::setw(12) << std::fixed << std::setprecision(1) << obj.center.y
                      << std::setw(12) << std::fixed << std::setprecision(3) << obj.circularity
                      << std::setw(12) << std::fixed << std::setprecision(2) << obj.aspectRatio
                      << std::endl;
        }
        
        // Calculate statistics
        double totalArea = 0.0;
        double avgCircularity = 0.0;
        for (const auto& obj : detectedObjects) {
            totalArea += obj.area;
            avgCircularity += obj.circularity;
        }
        
        std::cout << "\nStatistics:" << std::endl;
        std::cout << "  Total area: " << std::fixed << std::setprecision(1) << totalArea << std::endl;
        std::cout << "  Average area: " << std::fixed << std::setprecision(1) 
                  << totalArea / detectedObjects.size() << std::endl;
        std::cout << "  Average circularity: " << std::fixed << std::setprecision(3) 
                  << avgCircularity / detectedObjects.size() << std::endl;
    }
    
    std::cout << "=================================" << std::endl;
}

// Display results
void ObjectCounter::displayResults(const std::string& windowName) {
    if (inputImage.empty()) {
        std::cerr << "Error: No input image to display" << std::endl;
        return;
    }
    
    cv::Mat annotatedImage = getAnnotatedImage();
    
    if (binaryMask.empty()) {
        cv::imshow(windowName, annotatedImage);
    } else {
        cv::Mat combined = combineImages(inputImage, binaryMask, annotatedImage);
        cv::imshow(windowName, combined);
    }
    
    std::cout << "Press any key to close the display window..." << std::endl;
    cv::waitKey(0);
    cv::destroyWindow(windowName);
}

// Get annotated image
cv::Mat ObjectCounter::getAnnotatedImage() {
    cv::Mat annotatedImage = inputImage.clone();
    drawObjectAnnotations(annotatedImage);
    return annotatedImage;
}

// Save annotated image
void ObjectCounter::saveAnnotatedImage(const std::string& outputPath) {
    cv::Mat annotatedImage = getAnnotatedImage();
    
    bool success = cv::imwrite(outputPath, annotatedImage);
    if (success) {
        std::cout << "Annotated image saved: " << outputPath << std::endl;
    } else {
        std::cerr << "Error: Could not save annotated image to " << outputPath << std::endl;
    }
}

// Save binary mask
void ObjectCounter::saveBinaryMask(const std::string& outputPath) {
    if (binaryMask.empty()) {
        std::cerr << "Error: No binary mask to save" << std::endl;
        return;
    }
    
    bool success = cv::imwrite(outputPath, binaryMask);
    if (success) {
        std::cout << "Binary mask saved: " << outputPath << std::endl;
    } else {
        std::cerr << "Error: Could not save binary mask to " << outputPath << std::endl;
    }
}

// Save all results
void ObjectCounter::saveResults(const std::string& basePath) {
    size_t lastDot = basePath.find_last_of(".");
    std::string basePathNoExt = (lastDot != std::string::npos) ? basePath.substr(0, lastDot) : basePath;
    
    saveAnnotatedImage(basePathNoExt + "_annotated.png");
    saveBinaryMask(basePathNoExt + "_mask.png");
}

// Getter methods
cv::Mat ObjectCounter::getInputImage() const {
    return inputImage.clone();
}

cv::Mat ObjectCounter::getBinaryMask() const {
    return binaryMask.clone();
}

int ObjectCounter::getObjectCount() const {
    return static_cast<int>(detectedObjects.size());
}

// Static method to combine three images
cv::Mat ObjectCounter::combineImages(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& img3) {
    cv::Mat combined;
    cv::Mat img2_color, img3_resized;
    
    // Convert binary mask to 3-channel for display
    if (img2.channels() == 1) {
        cv::cvtColor(img2, img2_color, cv::COLOR_GRAY2BGR);
    } else {
        img2_color = img2.clone();
    }
    
    img3_resized = img3.clone();
    
    // Resize all images to same height
    int targetHeight = std::min({img1.rows, img2_color.rows, img3_resized.rows});
    
    cv::Mat img1_resized, img2_resized;
    double ratio1 = (double)targetHeight / img1.rows;
    double ratio2 = (double)targetHeight / img2_color.rows;
    double ratio3 = (double)targetHeight / img3_resized.rows;
    
    cv::resize(img1, img1_resized, cv::Size(), ratio1, ratio1);
    cv::resize(img2_color, img2_resized, cv::Size(), ratio2, ratio2);
    cv::resize(img3_resized, img3_resized, cv::Size(), ratio3, ratio3);
    
    // Combine horizontally
    cv::Mat temp;
    cv::hconcat(img1_resized, img2_resized, temp);
    cv::hconcat(temp, img3_resized, combined);
    
    return combined;
}

// Static method to generate summary text
std::string ObjectCounter::generateSummaryText(int objectCount, const std::string& imageName) {
    std::string summary;
    
    if (!imageName.empty()) {
        summary = imageName + " has ";
    } else {
        summary = "Image has ";
    }
    
    summary += std::to_string(objectCount);
    
    if (objectCount == 0) {
        summary += " objects.";
    } else if (objectCount == 1) {
        summary += " object.";
    } else {
        summary += " objects.";
    }
    
    return summary;
}

// Static method to show image information
void ObjectCounter::showImageInfo(const cv::Mat& image, const std::string& imageName) {
    std::cout << imageName << " Info:" << std::endl;
    std::cout << "  Size: " << image.cols << "x" << image.rows << std::endl;
    std::cout << "  Channels: " << image.channels() << std::endl;
    std::cout << "  Type: " << image.type() << std::endl << std::endl;
}
