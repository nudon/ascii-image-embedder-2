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
    WeightOptions();
    float edge_weight;
    float color_weight;
  };

  enum class EdgeMetric{Histogram};
  class ImageDataOptions {
  public:
    ImageDataOptions();
    EdgeMetric edge_metric;
    float gaussian_blur_sigma;
    int gaussian_blur_size;
    int grid_width;
    int grid_height;
    bool keep_negative_edges;
    
    enum cv::HistCompMethods hist_compare_method = cv::HISTCMP_CHISQR;
  };

  class EmbedOptions {
  public:
    EmbedOptions();
    std::string other_image_name; //debug option specifying another image
    std::string image_name;
    std::string image_dir;
    std::string output_name;
    std::string output_dir;
  };

  class FontOptions {
  public:
    FontOptions();
    std::string font_name;
    std::string font_dir;
    int font_size;
  };

  class AllOptions {
  public:
    //AllOptions();
    WeightOptions* weight_opt = nullptr;
    EmbedOptions* embed_opt = nullptr;
    ImageDataOptions* image_data_opt = nullptr;
    FontOptions* font_opt = nullptr;
  };

}

#endif
