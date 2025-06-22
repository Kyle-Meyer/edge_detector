#include "objectCounter.hh"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>

// Constructor
ObjectCounter::ObjectCounter(std::string aConfigPath) 
    : minObjectArea(50.0), maxObjectArea(50000.0), minCircularity(0.3), 
      maxAspectRatio(3.0), useAreaFiltering(true), useShapeFiltering(false),
      enableCoinClassification(false), pixelsPerMM(0.0),
      configFilePath(aConfigPath)
{
    // Default parameters work well for coins and similar circular objects
    initializeCoinDatabase();
}

// Destructor
ObjectCounter::~ObjectCounter() {
    cv::destroyAllWindows();
}

// Initialize coin database with standard US coin specifications
void ObjectCounter::initializeCoinDatabase() {
    //all magic values that I just had to sit here and try ovr and over to work with
    /*
    coinDatabase[CoinType::PENNY] = {CoinType::PENNY, "Penny", 8.0, cv::Scalar(139, 69, 19)};      // Brown
    coinDatabase[CoinType::NICKEL] = {CoinType::NICKEL, "Nickel", 9.0, cv::Scalar(192, 192, 192)};  // Silver
    coinDatabase[CoinType::DIME] = {CoinType::DIME, "Dime", 7.56, cv::Scalar(211, 211, 211)};        // Light Silver
    coinDatabase[CoinType::QUARTER] = {CoinType::QUARTER, "Quarter", 10.25, cv::Scalar(169, 169, 169)}; // Gray
    */
    if (!loadCoinConfigFromFile(configFilePath)) 
    {
        std::cout << "Config file not found or invalid, using default coin specifications." << std::endl;
        loadDefaultCoinConfig();
    }
}

bool ObjectCounter::loadCoinConfigFromFile(const std::string& configPath) 
{
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open coin config file: " << configPath << std::endl;
        return false;
    }
    
    std::cout << "Loading coin configuration from: " << configPath << std::endl;
    
    coinDatabase.clear();
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) 
    {
        lineNumber++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') 
        {
            continue;
        }
        
        // Parse line: TYPE,NAME,DIAMETER_MM,COLOR_BGR
        // Example: PENNY,Penny,19.05,139:69:19
        std::stringstream ss(line);
        std::string typeStr, name, diameterStr, colorStr;
        
        if (!std::getline(ss, typeStr, ',') ||
            !std::getline(ss, name, ',') ||
            !std::getline(ss, diameterStr, ',') ||
            !std::getline(ss, colorStr)) 
        {
            std::cerr << "Warning: Invalid format at line " << lineNumber 
                      << " in config file: " << configPath << std::endl;
            continue;
        }
        
        // Convert type string to enum
        CoinType coinType = stringToCoinType(typeStr);
        if (coinType == CoinType::UNKNOWN) 
        {
            std::cerr << "Warning: Unknown coin type '" << typeStr 
                      << "' at line " << lineNumber << std::endl;
            continue;
        }
        
        // Parse diameter
        double diameter;
        try 
        {
            diameter = std::stod(diameterStr);
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Warning: Invalid diameter '" << diameterStr 
                      << "' at line " << lineNumber << std::endl;
            continue;
        }
        
        // Parse color
        cv::Scalar color = parseColor(colorStr);
        
        // Add to database
        coinDatabase[coinType] = {coinType, name, diameter, color};
        std::cout << "  Loaded: " << name << " (diameter: " << diameter 
                  << "mm, color: " << colorStr << ")" << std::endl;
    }
    
    file.close();
    
    if (coinDatabase.empty()) 
    {
        std::cerr << "Error: No valid coin configurations loaded from file." << std::endl;
        return false;
    }
    
    std::cout << "Successfully loaded " << coinDatabase.size() 
              << " coin configurations." << std::endl;
    return true;
}

CoinType ObjectCounter::stringToCoinType(const std::string& coinStr) const 
{
    std::string lower = coinStr;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "penny") return CoinType::PENNY;
    if (lower == "nickel") return CoinType::NICKEL;
    if (lower == "dime") return CoinType::DIME;
    if (lower == "quarter") return CoinType::QUARTER;
    
    return CoinType::UNKNOWN;
}

cv::Scalar ObjectCounter::parseColor(const std::string& colorStr) const 
{
    cv::Scalar defaultColor(128, 128, 128); // Gray default
    
    // Expected format: "B:G:R" or "B,G,R"
    std::string str = colorStr;
    char delimiter = ':';
    if (str.find(',') != std::string::npos) {
        delimiter = ',';
    }
    
    std::stringstream ss(str);
    std::string bStr, gStr, rStr;
    
    if (std::getline(ss, bStr, delimiter) &&
        std::getline(ss, gStr, delimiter) &&
        std::getline(ss, rStr)) {
        
        try {
            int b = std::stoi(bStr);
            int g = std::stoi(gStr);
            int r = std::stoi(rStr);
            
            // Clamp values to valid range
            b = std::max(0, std::min(255, b));
            g = std::max(0, std::min(255, g));
            r = std::max(0, std::min(255, r));
            
            return cv::Scalar(b, g, r);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Invalid color format '" << colorStr 
                      << "', using default gray." << std::endl;
        }
    } else {
        std::cerr << "Warning: Invalid color format '" << colorStr 
                  << "', expected 'B:G:R' or 'B,G,R'." << std::endl;
    }
    
    return defaultColor;
}

// Public method to load/reload coin configuration
bool ObjectCounter::loadCoinConfig(const std::string& configPath) {
    configFilePath = configPath;
    return loadCoinConfigFromFile(configPath);
}

// Reload current configuration
void ObjectCounter::reloadCoinConfig() {
    initializeCoinDatabase();
}

// Get current config file path
std::string ObjectCounter::getConfigPath() const {
    return configFilePath;
}

// Load default coin configuration (fallback)
void ObjectCounter::loadDefaultCoinConfig() 
{
    std::cout << "Loading default US coin specifications..." << std::endl;
    
    coinDatabase.clear();
    
    // Default US coin specifications (diameter in mm)
    coinDatabase[CoinType::PENNY] = {CoinType::PENNY, "Penny", 19.05, cv::Scalar(139, 69, 19)};
    coinDatabase[CoinType::NICKEL] = {CoinType::NICKEL, "Nickel", 21.21, cv::Scalar(192, 192, 192)};
    coinDatabase[CoinType::DIME] = {CoinType::DIME, "Dime", 17.91, cv::Scalar(211, 211, 211)};
    coinDatabase[CoinType::QUARTER] = {CoinType::QUARTER, "Quarter", 24.26, cv::Scalar(169, 169, 169)};
    coinDatabase[CoinType::HALF_DOLLAR] = {CoinType::HALF_DOLLAR, "Half Dollar", 30.61, cv::Scalar(190, 190, 190)};
    coinDatabase[CoinType::DOLLAR] = {CoinType::DOLLAR, "Dollar", 26.50, cv::Scalar(200, 200, 150)};
    
    std::cout << "Default coin database initialized with " << coinDatabase.size() 
              << " coin types." << std::endl;
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

// Load binary mask from cv::Mat
bool ObjectCounter::loadBinaryMask(const cv::Mat& mask) {
    if (mask.empty()) {
        std::cerr << "Error: Binary mask is empty" << std::endl;
        return false;
    }
    
    // Ensure the mask is single channel
    if (mask.channels() == 1) {
        binaryMask = mask.clone();
    } else {
        cv::cvtColor(mask, binaryMask, cv::COLOR_BGR2GRAY);
    }
    
    // Ensure the mask is binary (0 or 255)
    cv::threshold(binaryMask, binaryMask, 127, 255, cv::THRESH_BINARY);
    
    std::cout << "Binary mask loaded successfully" << std::endl;
    showImageInfo(binaryMask, "Binary Mask");
    
    // Clear previous object detection results
    detectedObjects.clear();
    
    return true;
}

// Main method to count objects
int ObjectCounter::countObjects() {
    if (inputImage.empty()) {
        std::cerr << "Error: No input image loaded" << std::endl;
        return -1;
    }
    
    if (binaryMask.empty()) {
        std::cerr << "Error: No binary mask loaded. Please load a binary mask first." << std::endl;
        return -1;
    }
    
    // Verify that image and mask have compatible dimensions
    if (inputImage.rows != binaryMask.rows || inputImage.cols != binaryMask.cols) {
        std::cerr << "Error: Input image and binary mask have different dimensions" << std::endl;
        std::cerr << "Image size: " << inputImage.cols << "x" << inputImage.rows << std::endl;
        std::cerr << "Mask size: " << binaryMask.cols << "x" << binaryMask.rows << std::endl;
        return -1;
    }
    
    std::cout << "Starting object counting process..." << std::endl;
    
    // Step 1: Find contours in the binary mask
    findContours();
    
    // Step 2: Analyze objects and filter based on criteria
    analyzeObjects();
    
    // Step 3: Classify coins if enabled
    if (enableCoinClassification) {
        classifyCoins();
    }
    
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
        
        // Initialize coin-specific fields
        obj.coinType = CoinType::UNKNOWN;
        obj.diameter_pixels = calculateDiameter(contours[i]);
        obj.estimated_diameter_mm = 0.0;
        obj.confidence = 0.0;
        
        detectedObjects.push_back(obj);
    }
}

// Calculate diameter of a contour
double ObjectCounter::calculateDiameter(const std::vector<cv::Point>& contour) {
    // Use minimum enclosing circle for diameter estimation
    cv::Point2f center;
    float radius;
    cv::minEnclosingCircle(contour, center, radius);
    return 2.0 * radius;
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

// Classify coins based on size
void ObjectCounter::classifyCoins() {
    if (pixelsPerMM <= 0.0) {
        std::cout << "Warning: No calibration set. Cannot classify coins by size." << std::endl;
        std::cout << "Use setPixelsPerMM() or calibrateWithKnownCoin() first." << std::endl;
        return;
    }
    
    std::cout << "Classifying coins using calibration: " << pixelsPerMM << " pixels per mm" << std::endl;
    
    for (auto& obj : detectedObjects) {
        // Convert pixel diameter to millimeters
        obj.estimated_diameter_mm = obj.diameter_pixels / pixelsPerMM;
        
        // Classify based on size
        obj.coinType = classifyBySize(obj.estimated_diameter_mm, obj.confidence);
    }
}

// Classify coin by size with confidence score
CoinType ObjectCounter::classifyBySize(double diameter_mm, double& confidence) {
    CoinType bestMatch = CoinType::UNKNOWN;
    double smallestDifference = std::numeric_limits<double>::max();
    
    for (const auto& pair : coinDatabase) {
        const CoinInfo& coinInfo = pair.second;
        double difference = std::abs(diameter_mm - coinInfo.diameter_mm);
        
        if (difference < smallestDifference) {
            smallestDifference = difference;
            bestMatch = coinInfo.type;
        }
    }
    
    // Calculate confidence based on how close the size match is
    // Tolerance of ±2mm gives good confidence, beyond that confidence drops
    double tolerance = 2.0; // mm
    confidence = std::max(0.0, 1.0 - (smallestDifference / tolerance));
    
    // Only classify if confidence is above threshold
    if (confidence < 0.3) {
        return CoinType::UNKNOWN;
    }
    
    return bestMatch;
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
        
        // Choose color based on coin type if coin classification is enabled
        cv::Scalar color = cv::Scalar(0, 255, 0); // Default green
        if (enableCoinClassification && obj.coinType != CoinType::UNKNOWN) {
            color = getCoinColor(obj.coinType);
        }
        
        // Draw contour
        std::vector<std::vector<cv::Point>> contours = {obj.contour};
        cv::drawContours(image, contours, -1, color, 2);
        
        // Draw bounding box
        cv::rectangle(image, obj.boundingBox, cv::Scalar(255, 0, 0), 1);
        
        // Draw center point
        cv::circle(image, obj.center, 3, cv::Scalar(0, 0, 255), -1);
        
        // Draw label
        std::string label;
        if (enableCoinClassification && obj.coinType != CoinType::UNKNOWN) {
            label = coinTypeToString(obj.coinType);
            if (obj.confidence > 0) {
                label += " (" + std::to_string(static_cast<int>(obj.confidence * 100)) + "%)";
            }
        } else {
            label = std::to_string(i + 1);
        }
        
        cv::putText(image, label, cv::Point(obj.center.x - 10, obj.center.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
    }
    
    // Draw summary text
    std::string summary;
    if (enableCoinClassification) {
        auto coinCounts = getCoinCounts();
        double totalValue = getTotalValue();
        summary = "Coins: " + std::to_string(detectedObjects.size()) + 
                 ", Value: $" + std::to_string(totalValue).substr(0, std::to_string(totalValue).find('.') + 3);
    } else {
        summary = "Objects detected: " + std::to_string(detectedObjects.size());
    }
    
    cv::putText(image, summary, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 
                0.8, cv::Scalar(255, 255, 255), 2);
}

// Convert coin type to string
std::string ObjectCounter::coinTypeToString(CoinType type) const {
    auto it = coinDatabase.find(type);
    if (it != coinDatabase.end()) {
        return it->second.name;
    }
    return "Unknown";
}

// Get color for coin type
cv::Scalar ObjectCounter::getCoinColor(CoinType type) const {
    auto it = coinDatabase.find(type);
    if (it != coinDatabase.end()) {
        return it->second.color;
    }
    return cv::Scalar(128, 128, 128); // Gray for unknown
}

// Enable/disable coin classification
void ObjectCounter::setCoinClassification(bool enable) {
    this->enableCoinClassification = enable;
}

// Set pixels per millimeter for calibration
void ObjectCounter::setPixelsPerMM(double pixelsPerMM) {
    this->pixelsPerMM = pixelsPerMM;
    std::cout << "Calibration set: " << pixelsPerMM << " pixels per millimeter" << std::endl;
}

// Calibrate using a known coin at specified position
void ObjectCounter::calibrateWithKnownCoin(const cv::Point& coinCenter, CoinType knownType) {
    if (detectedObjects.empty()) {
        std::cerr << "Error: No objects detected. Run countObjects() first." << std::endl;
        return;
    }
    
    auto it = coinDatabase.find(knownType);
    if (it == coinDatabase.end()) {
        std::cerr << "Error: Unknown coin type for calibration" << std::endl;
        return;
    }
    
    // Find the closest object to the specified point
    double minDistance = std::numeric_limits<double>::max();
    const ObjectInfo* closestObject = nullptr;
    
    for (const auto& obj : detectedObjects) {
        double distance = cv::norm(cv::Point2f(coinCenter) - obj.center);
        if (distance < minDistance) {
            minDistance = distance;
            closestObject = &obj;
        }
    }
    
    if (closestObject == nullptr) {
        std::cerr << "Error: No object found near calibration point" << std::endl;
        return;
    }
    
    // Calculate pixels per mm based on known coin
    double knownDiameterMM = it->second.diameter_mm;
    pixelsPerMM = closestObject->diameter_pixels / knownDiameterMM;
    
    std::cout << "Calibration completed using " << coinTypeToString(knownType) << std::endl;
    std::cout << "Measured diameter: " << closestObject->diameter_pixels << " pixels" << std::endl;
    std::cout << "Known diameter: " << knownDiameterMM << " mm" << std::endl;
    std::cout << "Calibration: " << pixelsPerMM << " pixels per mm" << std::endl;
}

// Get count of each coin type
std::map<CoinType, int> ObjectCounter::getCoinCounts() const {
    std::map<CoinType, int> counts;
    
    // Initialize all coin types to 0
    for (const auto& pair : coinDatabase) {
        counts[pair.first] = 0;
    }
    counts[CoinType::UNKNOWN] = 0;
    
    // Count detected coins
    for (const auto& obj : detectedObjects) {
        counts[obj.coinType]++;
    }
    
    return counts;
}

// Calculate total monetary value
double ObjectCounter::getTotalValue() const {
    double total = 0.0;
    
    for (const auto& obj : detectedObjects) {
        switch (obj.coinType) {
            case CoinType::PENNY: total += 0.01; break;
            case CoinType::NICKEL: total += 0.05; break;
            case CoinType::DIME: total += 0.10; break;
            case CoinType::QUARTER: total += 0.25; break;
            case CoinType::HALF_DOLLAR: total += 0.50; break;
            case CoinType::DOLLAR: total += 1.00; break;
            default: break; // Unknown coins don't add value
        }
    }
    
    return total;
}

// Print coin summary
void ObjectCounter::printCoinSummary() const {
    std::cout << "\n=== Coin Detection Summary ===" << std::endl;
    
    auto coinCounts = getCoinCounts();
    double totalValue = getTotalValue();
    
    std::cout << "Coin breakdown:" << std::endl;
    for (const auto& pair : coinDatabase) {
        CoinType type = pair.first;
        const CoinInfo& info = pair.second;
        int count = coinCounts[type];
        
        if (count > 0) {
            double value = 0.0;
            switch (type) {
                case CoinType::PENNY: value = count * 0.01; break;
                case CoinType::NICKEL: value = count * 0.05; break;
                case CoinType::DIME: value = count * 0.10; break;
                case CoinType::QUARTER: value = count * 0.25; break;
                case CoinType::HALF_DOLLAR: value = count * 0.50; break;
                case CoinType::DOLLAR: value = count * 1.00; break;
            }
            
            std::cout << "  " << info.name << ": " << count 
                      << " ($" << std::fixed << std::setprecision(2) << value << ")" << std::endl;
        }
    }
    
    if (coinCounts[CoinType::UNKNOWN] > 0) {
        std::cout << "  Unknown: " << coinCounts[CoinType::UNKNOWN] << " ($0.00)" << std::endl;
    }
    
    std::cout << "Total coins: " << detectedObjects.size() << std::endl;
    std::cout << "Total value: $" << std::fixed << std::setprecision(2) << totalValue << std::endl;
    std::cout << "===============================" << std::endl;
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
    std::cout << "Total objects detected: " << detectedObjects.size() << std::endl;
    
    if (!detectedObjects.empty()) {
        std::cout << "\nObject Details:" << std::endl;
        std::cout << std::setw(4) << "ID" << std::setw(10) << "Area" 
                  << std::setw(12) << "Center X" << std::setw(12) << "Center Y"
                  << std::setw(12) << "Circularity" << std::setw(12) << "Aspect Ratio";
        
        if (enableCoinClassification) {
            std::cout << std::setw(12) << "Coin Type" << std::setw(12) << "Diameter(mm)" << std::setw(10) << "Confidence";
        }
        std::cout << std::endl;
        
        int lineWidth = enableCoinClassification ? 104 : 70;
        std::cout << std::string(lineWidth, '-') << std::endl;
        
        for (const auto& obj : detectedObjects) {
            std::cout << std::setw(4) << (obj.id + 1)
                      << std::setw(10) << std::fixed << std::setprecision(1) << obj.area
                      << std::setw(12) << std::fixed << std::setprecision(1) << obj.center.x
                      << std::setw(12) << std::fixed << std::setprecision(1) << obj.center.y
                      << std::setw(12) << std::fixed << std::setprecision(3) << obj.circularity
                      << std::setw(12) << std::fixed << std::setprecision(2) << obj.aspectRatio;
            
            if (enableCoinClassification) {
                std::cout << std::setw(12) << coinTypeToString(obj.coinType)
                          << std::setw(12) << std::fixed << std::setprecision(2) << obj.estimated_diameter_mm
                          << std::setw(10) << std::fixed << std::setprecision(1) << (obj.confidence * 100) << "%";
            }
            std::cout << std::endl;
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
        
        if (enableCoinClassification && pixelsPerMM > 0) {
            std::cout << "  Calibration: " << std::fixed << std::setprecision(2) 
                      << pixelsPerMM << " pixels per mm" << std::endl;
        }
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

// Static method to generate coin summary text
std::string ObjectCounter::generateCoinSummaryText(const std::map<CoinType, int>& coinCounts, double totalValue) {
    std::string summary = "Found ";
    
    int totalCoins = 0;
    for (const auto& pair : coinCounts) {
        totalCoins += pair.second;
    }
    
    summary += std::to_string(totalCoins) + " coins";
    
    if (totalValue > 0) {
        summary += " worth $" + std::to_string(totalValue).substr(0, std::to_string(totalValue).find('.') + 3);
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
