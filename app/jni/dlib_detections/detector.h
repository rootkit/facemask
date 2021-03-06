
#pragma once

//Uncomment if you have to build for abiFilters 'armeabi-v7a'
/*
#include <cmath>

namespace std {
    inline int round(float x) {
        return ::round(x);
    }
}
*/

#include <jni_common/jni_fileutils.h>
#include <dlib/image_loader/load_image.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/opencv/cv_image.h>
#include <dlib/image_loader/load_image.h>
#include <glog/logging.h>
#include <jni.h>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

class DLibHOGDetector {
private:
    typedef dlib::scan_fhog_pyramid<dlib::pyramid_down<6>> image_scanner_type;
    dlib::object_detector<image_scanner_type> mObjectDetector;

    inline void init() {
        LOG(INFO) << "Model Path: " << mModelPath;
        if (jniutils::fileExists(mModelPath)) {
            dlib::deserialize(mModelPath) >> mObjectDetector;
        } else {
            LOG(INFO) << "Not exist " << mModelPath;
        }
    }

public:
    DLibHOGDetector(const std::string &modelPath = "/sdcard/person.svm")
            : mModelPath(modelPath) {
        init();
    }

    virtual inline int det(const std::string &path) {
        using namespace jniutils;
        if (!fileExists(mModelPath) || !fileExists(path)) {
            LOG(WARNING) << "No module path or input file path";
            return 0;
        }
        cv::Mat src_img = cv::imread(path, CV_LOAD_IMAGE_COLOR);
        if (src_img.empty())
            return 0;
        int img_width = src_img.cols;
        int img_height = src_img.rows;
        int im_size_min = MIN(img_width, img_height);
        int im_size_max = MAX(img_width, img_height);

        float scale = float(INPUT_IMG_MIN_SIZE) / float(im_size_min);
        if (scale * im_size_max > INPUT_IMG_MAX_SIZE) {
            scale = (float) INPUT_IMG_MAX_SIZE / (float) im_size_max;
        }

        if (scale != 1.0) {
            cv::Mat outputMat;
            cv::resize(src_img, outputMat,
                       cv::Size(img_width * scale, img_height * scale));
            src_img = outputMat;
        }

        // cv::resize(src_img, src_img, cv::Size(320, 240));
        dlib::cv_image<dlib::bgr_pixel> cimg(src_img);

        double thresh = 0.5;
        mRets = mObjectDetector(cimg, thresh);
        return mRets.size();
    }

    inline std::vector<dlib::rectangle> getResult() { return mRets; }

    virtual ~DLibHOGDetector() {}

protected:
    std::vector<dlib::rectangle> mRets;
    std::string mModelPath;
    const int INPUT_IMG_MAX_SIZE = 800;
    const int INPUT_IMG_MIN_SIZE = 600;
};

/*
 * DLib face detect and face feature extractor
 */
class DLibHOGFaceDetector : public DLibHOGDetector {
private:
    std::string mLandMarkModel;
    dlib::shape_predictor msp;
    std::unordered_map<int, dlib::full_object_detection> mFaceShapeMap;
    dlib::frontal_face_detector mFrontalFaceDetector;
    bool mLandmarksEnabled = true;

    inline void init() {
        LOG(INFO) << "Init mFrontalFaceDetector";
        mFrontalFaceDetector = dlib::get_frontal_face_detector();
    }

public:
    DLibHOGFaceDetector() { init(); }

    DLibHOGFaceDetector(const std::string &landmarkmodel)
            : mLandMarkModel(landmarkmodel) {
        init();
        if (!mLandMarkModel.empty() && jniutils::fileExists(mLandMarkModel)) {
            dlib::deserialize(mLandMarkModel) >> msp;
            LOG(INFO) << "Load landmarkmodel from " << mLandMarkModel;
        }
    }

    void setLandmarksFlag(bool enabled) {
        mLandmarksEnabled = enabled;
    }

    virtual inline int det(const std::string &path) {
        LOG(INFO) << "Read path from " << path;
        cv::Mat src_img = cv::imread(path, CV_LOAD_IMAGE_COLOR);
        return det(src_img);
    }

    // The format of mat should be BGR or Gray
    // If converting 4 channels to 3 channls because the format could be BGRA or
    // ARGB
    virtual inline int det(const cv::Mat &image) {
        if (image.empty())
            return 0;
        if (image.channels() == 1) {
            cv::cvtColor(image, image, CV_GRAY2BGR);
        }
        CHECK(image.channels() == 3);
        // TODO : Convert to gray image to speed up detection
        // It's unnecessary to use color image for face/landmark detection
        dlib::cv_image<dlib::bgr_pixel> img(image);
        mRets = mFrontalFaceDetector(img);
        //LOG(INFO) << "Dlib HOG face det size : " << mRets.size();
        mFaceShapeMap.clear();
        if (mLandmarksEnabled) {
            // Process shape
            if (mRets.size() != 0 && mLandMarkModel.empty() == false) {
                for (unsigned long j = 0; j < mRets.size(); ++j) {
                    dlib::full_object_detection shape = msp(img, mRets[j]);
                    //LOG(INFO) << "face index:" << j << "number of parts: " << shape.num_parts();
                    mFaceShapeMap[j] = shape;
                }
            }
        }
        return mRets.size();
    }

    std::unordered_map<int, dlib::full_object_detection> &getFaceShapeMap() {
        return mFaceShapeMap;
    }
};
