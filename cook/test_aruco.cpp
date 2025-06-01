#include <opencv2/aruco.hpp>
#include <iostream>

int main()
{
  auto dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
  std::cout << "Aruco is available" << std::endl;
  return 0;
}