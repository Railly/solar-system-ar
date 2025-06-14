#include <glad/glad.h>
#include "ar_tracker.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include "logger.hpp"

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
  if (!cap_.isOpened()) {
    LOG_ERR("Camera %d failed to open", camId);
    throw std::runtime_error("cam failed");
  }
  
  // ----- quick dummy intrinsics (better: load from calibration.yml)
  int w = (int)cap_.get(cv::CAP_PROP_FRAME_WIDTH);
  int h = (int)cap_.get(cv::CAP_PROP_FRAME_HEIGHT);
  double f = 0.9 * w;
  camMat_ = (cv::Mat_<double>(3, 3) << f, 0, w / 2, 0, f, h / 2, 0, 0, 1);
  dist_ = cv::Mat::zeros(1, 5, CV_64F);
  P_ = makeProj(camMat_, w, h, 0.01f, 100.f);  // closer near plane
  
  LOG_INF("Camera initialized: %dx%d, marker_len=%.3fm", w, h, markerLen_);

  glGenTextures(1, &bgTex_);
  glBindTexture(GL_TEXTURE_2D, bgTex_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

glm::mat4 ARTracker::cvToGlm(const cv::Vec3d &rvec, const cv::Vec3d &tvec)
{
  // OpenCV rotation → 3x3 matrix
  cv::Mat Rcv;
  cv::Rodrigues(rvec, Rcv);
  
  glm::mat4 T(1.0f);
  // Copy rotation (column-major for GLM)
  for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c)
      T[c][r] = static_cast<float>(Rcv.at<double>(r, c));
  
  // Translation
  T[3][0] = static_cast<float>(tvec[0]);
  T[3][1] = static_cast<float>(tvec[1]);
  T[3][2] = static_cast<float>(tvec[2]);

  // ---- Convert OpenCV(+Z) → OpenGL(-Z) coordinate system ----
  const glm::mat4 cvToGl = glm::mat4( 1, 0, 0, 0,
                                      0,-1, 0, 0,
                                      0, 0,-1, 0,
                                      0, 0, 0, 1 );

  glm::mat4 markerToCamera = cvToGl * T; // already is view matrix

  return markerToCamera;                 // NO glm::inverse() needed!
}

void ARTracker::uploadBackground()
{
  if (frame_.empty()) {
    LOG_DBG("Frame empty, skipping upload");
    return;            // avoid first empty frame
  }
  cv::cvtColor(frame_, frame_, cv::COLOR_BGR2RGB);
  glBindTexture(GL_TEXTURE_2D, bgTex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_.cols, frame_.rows,
               0, GL_RGB, GL_UNSIGNED_BYTE, frame_.data);
  LOG_DBG("Background uploaded: %dx%d", frame_.cols, frame_.rows);
}

bool ARTracker::grabFrame()
{
  if (!cap_.read(frame_) || frame_.empty()) {
    LOG_ERR("Camera read failed or empty frame");
    return false;
  }

  std::vector<int> ids;
  std::vector<std::vector<cv::Point2f>> corners, reject;
  detector_.detectMarkers(frame_, corners, ids, reject);
  
  markerVisible_ = !ids.empty();             // remember state
  LOG_DBG("Marker visible: %d (found %zu markers)", markerVisible_, ids.size());

  if (markerVisible_)
  {
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::aruco::estimatePoseSingleMarkers(corners, markerLen_, camMat_, dist_,
                                         rvecs, tvecs);
    V_ = cvToGlm(rvecs[0], tvecs[0]);
    LOG_DBG("Pose: rvec=(%.2f,%.2f,%.2f) tvec=(%.2f,%.2f,%.2f)",
            rvecs[0][0], rvecs[0][1], rvecs[0][2],
            tvecs[0][0], tvecs[0][1], tvecs[0][2]);
  }
  uploadBackground();                         // always upload feed
  return true;
}
