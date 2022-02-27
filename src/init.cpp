#include <opencv2/opencv.hpp>

#include "CLI11.hpp"
#include "scores.hpp"


using namespace options;
int main (int argc, char** argv) {
  std::cout << "Hi" << std::endl;
  WeightOptions weight_opts;
  ImageDataOptions image_data_opts;
  EmbedOptions embed_opts;
  if (argc >= 2) {
    embed_opts.image_name = std::string(argv[1]);
    embed_opts.other_image_name = embed_opts.image_name;
  }
  if (argc >= 3) {
    embed_opts.other_image_name = std::string(argv[2]);
  }
  AllOptions all_opts {.weight_opt = &weight_opts, .embed_opt = &embed_opts, .image_data_opt = &image_data_opts };
  scores::rework(&all_opts);
}
