#ifndef OBJECT_COUNTER_HH
#define OBJECT_COUNTER_HH

#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
 
//handy struct to track image data easier
struct ObjectInfo {

  int id;
  double area;
  cv::Point2f centroid;
  cv::Rect boundingBox;
  double perimeter;
  double circularity; //4*pi*area/permiter^2
  std::vector<cv::Point> contour;
};

class ObjectCounter{
private:
  cv::Mat inputImage;
  cv::Mat binaryMask;
  std::vector<ObjectInfo> detectedObjects;

  //parameters for object detection
  int minObjArea;
  int maxObjArea;
  double minCircularity;
  double maxCircularity;
  bool useAreaFilter;
  bool useCircularity; //may not be needed?

  void findContours(std::vector<std::vector<cv::Point>>& controus,
                    std::vector<cv::Vec4i>& hierarchy);

  void filterContours(const std::vector<std::vector<cv::Point>>& controus);

  void calculateObjectProperties(const std::vector<cv::Point>& contour,
                                 ObjectInfo& info,
                                 int id);

  double calculateCircularity(double area, double perimeter);

  cv::Point2f calculateCentroid(const std::vector<cv::Point>& contour);

public:
  //defualt constructor
  ObjectCounter();

  //default destructor
  ~ObjectCounter();

  //fun time
  
  bool loadBinaryMask(const cv::Mat& mask);
  bool loadInputImage(const cv::Mat& image);

  int countObjects();

  void setAreaFilter(int minArea, int maxArea);
  void setCircularityFilter(int minArea, int maxArea);
  void enableAreaFilter(bool enable);
  void enableCircularityFilter(bool enable);

  //getters
  int getObjCount() const;
  std::vector<ObjectInfo> getDetectedObjects() const;
  ObjectInfo getObjInfo(int index) const;
  cv::Mat getBinaryMask() const;

  //vis methods
  cv::Mat drawObjectBoundaries(const cv::Mat& baseImg = cv::Mat(),
                               const cv::Scalar& color = cv::Scalar(0, 255, 0), //default to green
                               int thickness = 2);

  cv::Mat drawObjectCentroids(const cv::Mat& baseImg = cv::Mat(),
                              const cv::Scalar& color = cv::Scalar(255, 0, 0), //default to red
                              int radius = 2);

  cv::Mat drawObjectLabels(const cv::Mat& baseImg = cv::Mat(),
                           const cv::Scalar& color = cv::Scalar(255, 255, 255)); //default white 

  void displayResults(const std::string& windowName = "Obj Detection results");

  //util
  void saveResults(const std::string& outputPath, const cv::Mat& resultImage);
  void printStats() const;
  void saveToCSV(const std::string& csvPath) const;

  //static util 
  static cv::Mat createColorMask(const cv::Mat& binaryMask);
  static void showDetectionInfo(int objectCounter, const std::vector<ObjectInfo>& objects);
};

#endif //end of OBJECT_COUNTER_HH
