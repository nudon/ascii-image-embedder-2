#ifndef FILE_IMG_SAMPLE_SEEN
#define FILE_IMG_SAMPLE_SEEN

#include <opencv2/imgproc.hpp>

namespace image_data {
  
  class HistData {
  public:
    HistData(cv::Mat &edge_hist);
    static double compare(HistData *a, HistData *b);
    cv::Mat Data();
  private:
    cv::Mat hist;
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
