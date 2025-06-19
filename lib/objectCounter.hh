#ifndef OBJECT_COUNTER_HH
#define OBJECT_COUNTER_HH

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct ObjectInfo {
    int id;
    double area;
    cv::Point2f center;
    cv::Rect boundingBox;
    std::vector<cv::Point> contour;
    double circularity;
    double aspectRatio;
};

class ObjectCounter {
private:
    cv::Mat inputImage;
    cv::Mat binaryMask;
    std::vector<ObjectInfo> detectedObjects;
    
    // Parameters for object detection
    double minObjectArea;
    double maxObjectArea;
    double minCircularity;
    double maxAspectRatio;
    bool useAreaFiltering;
    bool useShapeFiltering;
    
    // Internal methods
    void findContours();
    void analyzeObjects();
    double calculateCircularity(const std::vector<cv::Point>& contour, double area);
    double calculateAspectRatio(const cv::Rect& boundingBox);
    bool isValidObject(const ObjectInfo& obj);
    void drawObjectAnnotations(cv::Mat& image);
    
    // Static helper methods
    static void showImageInfo(const cv::Mat& image, const std::string& imageName);
    
public:
    // Constructor and Destructor
    ObjectCounter();
    ~ObjectCounter();
    
    // Image loading methods
    bool loadImage(const std::string& imagePath);
    bool loadImage(const cv::Mat& image);
    
    // Binary mask loading method
    bool loadBinaryMask(const cv::Mat& mask);
    
    // Main processing method
    int countObjects();
    
    // Parameter setting methods
    void setAreaFilter(double minArea, double maxArea);
    void setShapeFilter(double minCircularity, double maxAspectRatio);
    void enableAreaFiltering(bool enable);
    void enableShapeFiltering(bool enable);
    
    // Results and display methods
    std::vector<ObjectInfo> getObjectInfo() const;
    void printObjectSummary() const;
    void displayResults(const std::string& windowName = "Object Detection Results");
    cv::Mat getAnnotatedImage();
    
    // Save methods
    void saveAnnotatedImage(const std::string& outputPath);
    void saveBinaryMask(const std::string& outputPath);
    void saveResults(const std::string& basePath);
    
    // Getter methods
    cv::Mat getInputImage() const;
    cv::Mat getBinaryMask() const;
    int getObjectCount() const;
    
    // Static utility methods
    static cv::Mat combineImages(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& img3);
    static std::string generateSummaryText(int objectCount, const std::string& imageName = "");
};

#endif // OBJECT_COUNTER_HH
