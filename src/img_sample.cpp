#include "img_sample.hpp"
#include <opencv2/imgproc.hpp>

using namespace cv;
namespace image_data {
  
  HistData::HistData(Mat &hist) {
    this->hist = hist.clone();
  }

  double HistData::compare(HistData *a, HistData *b) {
    return compareHist(a->Data(), b->Data(), HISTCMP_CHISQR);
  }

  Mat HistData::Data() {
    return hist;
  }

  ProcessingRegion::ProcessingRegion(Rect rect, Mat mat) {
    processing_region = rect;
    src_img = mat;
  }

  Rect ProcessingRegion::get_region() {
    return processing_region;
  }

  Mat ProcessingRegion::get_full_img() {
    return src_img;
  }

  Mat ProcessingRegion::get_region_img() {
    Mat region(src_img, processing_region);
    return region;
  }
}
