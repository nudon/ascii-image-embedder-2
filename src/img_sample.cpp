#include "img_sample.hpp"
#include <opencv2/imgproc.hpp>

using namespace cv;
namespace image_sample {
  
  EdgeHistData::EdgeHistData(Mat &hist) {
    edge_hist = hist.clone();
  }

  double EdgeHistData::compare(EdgeHistData* a, EdgeHistData* b) {
    return compareHist(a->Data(), b->Data(), HISTCMP_CHISQR);
  }

  Mat EdgeHistData::Data() {
    return edge_hist;
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
