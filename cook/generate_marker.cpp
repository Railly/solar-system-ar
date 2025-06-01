#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp> // drawFrameAxes
#include <opencv2/aruco.hpp>   // main aruco API

int main()
{
  cv::VideoCapture cam(0);
  if (!cam.isOpened())
    return -1;

  auto dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
  cv::aruco::DetectorParameters params; // tweak later
  cv::aruco::ArucoDetector detector(dict, params);

  while (true)
  {
    cv::Mat frame;
    cam >> frame;
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejected;
    detector.detectMarkers(frame, corners, ids, rejected); // 🔑 call :contentReference[oaicite:1]{index=1}
    if (!ids.empty())
    {
      cv::aruco::drawDetectedMarkers(frame, corners, ids);
      // Optional pose if you already have camera matrix/dist coeffs
      // cv::aruco::estimatePoseSingleMarkers(corners, markerLength,
      //                                      camMat, distCoeffs, rvecs, tvecs);
      // cv::drawFrameAxes(frame, camMat, distCoeffs, rvec, tvec, 0.05f);
    }
    cv::imshow("ArUco feed", frame);
    if (cv::waitKey(10) == 27)
      break; // Esc to quit
  }
}
