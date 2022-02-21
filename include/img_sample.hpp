#ifndef FILE_IMG_SAMPLE_SEEN
#define FILE_IMG_SAMPLE_SEEN

#include <opencv2/imgproc.hpp>

namespace image_sample {


  class EdgeHistData {
  public:
    EdgeHistData(cv::Mat &edge_hist);
    static double compare(EdgeHistData* a, EdgeHistData* b);
    cv::Mat Data();
  private:
    cv::Mat edge_hist;
  };

  class ProcessingRegion {
  public:
    ProcessingRegion(cv::Rect pr, cv::Mat mat);
    cv::Rect get_region();
    cv::Mat get_full_img();
    cv::Mat get_region_img();
  private:
    cv::Rect processing_region;
    cv::Mat src_img;
  };
}
#endif
