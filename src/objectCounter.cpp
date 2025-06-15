#include "objectCounter.hh"
#include <climits>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <opencv2/highgui.hpp>

//constructor
ObjectCounter::ObjectCounter() : 
  minObjArea(50), maxObjArea(INT_MAX),
  minCircularity(0.0), maxCircularity(1.0),
  useAreaFilter(true), useCircularity(false)
{

}

//destructor
ObjectCounter::~ObjectCounter()
{
  cv::destroyAllWindows();
}

bool ObjectCounter::loadBinaryMask(const cv::Mat& mask)
{
  bool tRet = true;
  if(mask.empty())
  {
    std::cerr << "Input binary mask is empty!" << std::endl;
    tRet = false;
  }
  if(mask.channels() != 1)
  {
    std::cerr << "Error, input must be a single channel binary mask" << std::endl;
    tRet = false;
  }
  binaryMask = mask.clone();
  std::cout << "Binary Mask loaded" << std::endl;
  return tRet;
}

bool ObjectCounter::loadInputImage(const cv::Mat& image)
{
  bool tRet = true;
  if(image.empty())
  {
    std::cerr << "empty image passed as input!" << std::endl;
    tRet = false;
  }
  inputImage = image.clone();
  std::cout << "loaded input image successfully!" << std::endl;
  return tRet;
}

int ObjectCounter::countObjects()
{
  int tRet = -1;
  if(binaryMask.empty())
  {
    std::cerr << "Error no binary mask loaded" << std::endl;
  }

  detectedObjects.clear();

  //find contours
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;

  //pain
  filterContours(contours);

  std::cout << "Object counting complete, found " << detectedObjects.size() << " significant objects" << std::endl;
  tRet = static_cast<int>(detectedObjects.size()); // bad juju
  return tRet;
}


