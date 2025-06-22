#ifndef OBJECT_COUNTER_HH
#define OBJECT_COUNTER_HH

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <map>

enum class CoinType {
    UNKNOWN = 0,
    PENNY = 1,
    NICKEL = 2,
    DIME = 3,
    QUARTER = 4,
    HALF_DOLLAR = 5,
    DOLLAR = 6
};

struct CoinInfo {
    CoinType type;
    std::string name;
    double diameter_mm;
    cv::Scalar color;  // BGR color for visualization
};

struct ObjectInfo {
    int id;
    double area;
    cv::Point2f center;
    cv::Rect boundingBox;
    std::vector<cv::Point> contour;
    double circularity;
    double aspectRatio;
    
    // New coin-specific fields
    CoinType coinType;
    double diameter_pixels;
    double estimated_diameter_mm;
    double confidence;  // 0.0 to 1.0
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
    
    bool enableCoinClassification;
    double pixelsPerMM;  // Calibration factor for size-based classification
    std::map<CoinType, CoinInfo> coinDatabase;
    std::string configFilePath;

    // Internal methods
    void findContours();
    void analyzeObjects();
    double calculateCircularity(const std::vector<cv::Point>& contour, double area);
    double calculateAspectRatio(const cv::Rect& boundingBox);
    bool isValidObject(const ObjectInfo& obj);
    void drawObjectAnnotations(cv::Mat& image);
   
    //coin config loading 
    void initializeCoinDatabase();
    bool loadCoinConfigFromFile(const std::string& configPath);
    void loadDefaultCoinConfig();
    void classifyCoins();
    CoinType classifyBySize(double diameter_mm, double& confidence);
    double calculateDiameter(const std::vector<cv::Point>& contour);
    std::string coinTypeToString(CoinType type) const;
    cv::Scalar getCoinColor(CoinType type) const;
    CoinType stringToCoinType(const std::string& coinStr) const;
    cv::Scalar parseColor(const std::string& colorStr) const;
    


    // Static helper methods
    static void showImageInfo(const cv::Mat& image, const std::string& imageName);
    
public:
    // Constructor and Destructor
    ObjectCounter(std::string configPath);
    ~ObjectCounter();
    
    // Image loading methods
    bool loadImage(const std::string& imagePath);
    bool loadImage(const cv::Mat& image);
    
    // Binary mask loading method
    bool loadBinaryMask(const cv::Mat& mask);
    
    // Main processing method
    int countObjects();

    // Configuration methods for coin size and type
    bool loadCoinConfig(const std::string& configPath);
    void reloadCoinConfig();
    std::string getConfigPath() const;

    // Parameter setting methods
    void setAreaFilter(double minArea, double maxArea);
    void setShapeFilter(double minCircularity, double maxAspectRatio);
    void enableAreaFiltering(bool enable);
    void enableShapeFiltering(bool enable);
    
    // New coin classification methods
    void setCoinClassification(bool enable);
    void setPixelsPerMM(double pixelsPerMM);
    void calibrateWithKnownCoin(const cv::Point& coinCenter, CoinType knownType);
    std::map<CoinType, int> getCoinCounts() const;
    double getTotalValue() const;
    
    // Results and display methods
    std::vector<ObjectInfo> getObjectInfo() const;
    void printObjectSummary() const;
    void printCoinSummary() const;
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
    static std::string generateCoinSummaryText(const std::map<CoinType, int>& coinCounts, double totalValue);
};

#endif // OBJECT_COUNTER_HH
