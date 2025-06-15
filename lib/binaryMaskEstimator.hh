#ifndef BINARY_MASK_ESTIMATOR_H
#define BINARY_MASK_ESTIMATOR_H

#include <opencv2/opencv.hpp>
#include <string>

class BinaryMaskEstimator {
private:
    cv::Mat inputImage;
    cv::Mat binaryMask;
    
    // Parameters for mask estimation
    int blockSize;
    double C;
    int morphKernelSize;
    int morphIterations;
    
    // Helper methods
    void preprocessImage(cv::Mat& image);
    void applyAdaptiveThreshold(const cv::Mat& grayImage, cv::Mat& mask);
    void applyMorphologicalOperations(cv::Mat& mask);
    void removeSmallComponents(cv::Mat& mask, int minArea);

public:
    // Constructor
    BinaryMaskEstimator();
    
    // Destructor
    ~BinaryMaskEstimator();
    
    // Main functionality
    bool loadImage(const std::string& imagePath);
    bool loadImage(const cv::Mat& image);
    cv::Mat estimateBinaryMask();
    
    // Parameter setters
    void setAdaptiveThresholdParams(int blockSize, double C);
    void setMorphologicalParams(int kernelSize, int iterations);
    
    // Utility methods
    void saveImage(const std::string& outputPath, const cv::Mat& image);
    void displayImages(const std::string& windowName = "Binary Mask Estimation");
    cv::Mat getInputImage() const;
    cv::Mat getBinaryMask() const;
    
    // Static utility methods
    static cv::Mat combineImages(const cv::Mat& img1, const cv::Mat& img2);
    static void showImageInfo(const cv::Mat& image, const std::string& imageName);
};

#endif // BINARY_MASK_ESTIMATOR_H
