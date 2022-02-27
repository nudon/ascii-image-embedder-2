#ifndef FILE_SCORES_SEEN
#define FILE_SCORES_SEEN

namespace scores {
  class ScoreGenerator;
  //class ScoreMatcher;
  class Embedder;
}

#include "opencv2/opencv.hpp"
#include "options.hpp"
#include "img_sample.hpp"
#include "match.hpp"
namespace scores {
  
  /*
    Temp spot for most of the driving code
   */
  void rework(options::AllOptions* opt);
  
  /*
    Generates regions of interest in image
    grid_regions covers whole image, except any excess that doesn't fit in cell dimensions
   */
  class RegionGenerator {
  public:
    static std::list<image_data::ProcessingRegion> grid_regions(cv::Mat &img, int width, int height);
  };

  /*
    Generates data/statistics from regions to be used as matching criteria
    also holds some minor image processing functions
   */
  class DataGenerator {
  public:
    static std::list<image_data::HistData> edge_histograms(cv::Mat &img, std::list<image_data::ProcessingRegion> &regions, options::AllOptions* opt);
  private:
    static void edge_filter_dog(bool keep_neg_edges, float sigma, int x_dim, int y_dim, cv::Mat &input, cv::Mat &output);
    static void gradient_extraction(cv::Mat &input, cv::Mat &output, options::AllOptions* opt);
    static void hist_2D(cv::Mat &x_in, cv::Mat &y_in, cv::Mat &hist_out, int x_size, int y_size);
  };

  /*
    Holds information used for matching, and ideally a sort of factory for handling various options
   */
  class ImageData {
  public:
    ImageData(cv::Mat &img, options::AllOptions* opt);
    std::list<image_data::ProcessingRegion>* get_regions();
    std::list<image_data::HistData>* get_data();
  private:
    cv::Mat src_img;
    std::list<image_data::ProcessingRegion> region_list;
    std::list<image_data::HistData> data_list;
  };
  
}
  
#endif
