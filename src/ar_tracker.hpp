#pragma once
#include <opencv2/aruco.hpp>
#include <opencv2/highgui.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class ARTracker
{
public:
  ARTracker(int camId = 0,
            float markerLength = 0.08f); // metres
  bool grabFrame();                      // capture + detect
  bool markerVisible() const { return markerVisible_; }
  GLuint backgroundTex() const { return bgTex_; }
  glm::mat4 view() const { return V_; }
  glm::mat4 proj() const { return P_; }

private:
  cv::VideoCapture cap_;
  cv::Mat frame_;
  GLuint bgTex_{};
  cv::Mat camMat_, dist_;
  glm::mat4 V_{1.0f}, P_{1.0f};

  cv::aruco::ArucoDetector detector_;
  float markerLen_;
  bool markerVisible_{false};
  void uploadBackground();
  glm::mat4 cvToGlm(const cv::Vec3d &rvec, const cv::Vec3d &tvec);
};
