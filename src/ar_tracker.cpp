#include <glad/glad.h>
#include "ar_tracker.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

static glm::mat4 makeProj(const cv::Mat &K, int w, int h, float near, float far)
{
  float fx = K.at<double>(0, 0);
  float fy = K.at<double>(1, 1);
  float cx = K.at<double>(0, 2);
  float cy = K.at<double>(1, 2);
  glm::mat4 P(0.0f);
  P[0][0] = 2 * fx / w;
  P[1][1] = 2 * fy / h;
  P[2][0] = 1 - 2 * cx / w;
  P[2][1] = 2 * cy / h - 1;
  P[2][2] = -(far + near) / (far - near);
  P[2][3] = -1;
  P[3][2] = -(2 * far * near) / (far - near);
  return P;
}

ARTracker::ARTracker(int camId, float len)
    : markerLen_(len),
      detector_(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250))
{
  cap_.open(camId);
  if (!cap_.isOpened())
    throw std::runtime_error("cam failed");
  // ----- quick dummy intrinsics (better: load from calibration.yml)
  int w = (int)cap_.get(cv::CAP_PROP_FRAME_WIDTH);
  int h = (int)cap_.get(cv::CAP_PROP_FRAME_HEIGHT);
  double f = 0.9 * w;
  camMat_ = (cv::Mat_<double>(3, 3) << f, 0, w / 2, 0, f, h / 2, 0, 0, 1);
  dist_ = cv::Mat::zeros(1, 5, CV_64F);
  P_ = makeProj(camMat_, w, h, 0.05f, 100.f);

  glGenTextures(1, &bgTex_);
  glBindTexture(GL_TEXTURE_2D, bgTex_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

glm::mat4 ARTracker::cvToGlm(const cv::Vec3d &rvec, const cv::Vec3d &tvec)
{
  cv::Mat R;
  cv::Rodrigues(rvec, R);
  glm::mat4 M(1.0f);
  for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c)
      M[c][r] = (float)R.at<double>(r, c);
  M[3][0] = (float)tvec[0];
  M[3][1] = (float)tvec[1];
  M[3][2] = (float)tvec[2];
  return glm::inverse(M); // camera-to-world → view matrix
}

void ARTracker::uploadBackground()
{
  cv::cvtColor(frame_, frame_, cv::COLOR_BGR2RGB);
  glBindTexture(GL_TEXTURE_2D, bgTex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_.cols, frame_.rows,
               0, GL_RGB, GL_UNSIGNED_BYTE, frame_.data);
}

bool ARTracker::grabFrame()
{
  if (!cap_.read(frame_))
    return false;

  std::vector<int> ids;
  std::vector<std::vector<cv::Point2f>> corners, reject;
  detector_.detectMarkers(frame_, corners, ids, reject);
  
  markerVisible_ = !ids.empty();             // remember state

  if (markerVisible_)
  {
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::aruco::estimatePoseSingleMarkers(corners, markerLen_, camMat_, dist_,
                                         rvecs, tvecs);
    V_ = cvToGlm(rvecs[0], tvecs[0]);
  }
  uploadBackground();                         // always upload feed
  return true;
}
