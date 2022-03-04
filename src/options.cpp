#include "options.hpp"

namespace options {
  WeightOptions::WeightOptions() {
    float edge_weight = 1;
    float color_weight = 1;
  }

  ImageDataOptions::ImageDataOptions() {
    edge_metric = EdgeMetric::Histogram;
    gaussian_blur_sigma = 1;
    gaussian_blur_size =  7;
    grid_width = 100;
    grid_height = 100;
    keep_negative_edges = true;
    hist_compare_method = cv::HISTCMP_CHISQR;
  }

  EmbedOptions::EmbedOptions() {
    image_dir = "/img_src/";
    output_name = "out.png";
    output_dir = "/img_dst/";
  }
  
  FontOptions::FontOptions() {
    font_name = "DejaVuSansMono.ttf";
    font_dir = "/fonts/";
    font_size = 12;
  }
}
