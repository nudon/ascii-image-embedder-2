#ifndef FILE_OPTIONS_SEEN
#define FILE_OPTIONS_SEEN

namespace options {
  class WeightOptions;
  class ImageDataOptions;
  class EmbedOptions;
  class AllOptions;
}

#include <string>
#include <opencv2/imgproc.hpp>

namespace options {
  class WeightOptions {
  public:
    float edge_weight = 1;
    float color_weight = 1;
  };


  enum class EdgeMetric{Histogram};
  class ImageDataOptions {
  public:
    EdgeMetric edge_metric = EdgeMetric::Histogram;
    float gaussian_blur_sigma = 1;
    int gaussian_blur_size = 7;
    int grid_width = 100;
    int grid_height = 200;
    bool keep_negative_edges = true;
    
    enum cv::HistCompMethods hist_compare_method = cv::HISTCMP_CHISQR;
  };

  class EmbedOptions {
  public:
    std::string other_image_name; //debug option specifying another image
    std::string image_name;
    std::string image_dir = "/img_src/";

    std::string font_name;
    std::string font_dir;
    std::string output_name;
    std::string output_dir;

    int font_size;
  };

  class AllOptions {
  public:
    WeightOptions* weight_opt;
    EmbedOptions* embed_opt;
    ImageDataOptions* image_data_opt;
  };

}

#endif
