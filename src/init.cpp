#include <opencv2/opencv.hpp>

#include "CLI11.hpp"
#include "scores.hpp"


using namespace options;
int main (int argc, char** argv) {
  std::cout << "Hi" << std::endl;
  WeightOptions weight_opts;
  ScoreGeneratorOptions score_gen_opts;
  EmbedOptions embed_opts;
  embed_opts.image_name = std::string(argv[1]);
  AllOptions all_opts {.weight_opt = &weight_opts, .embed_opt = &embed_opts, .score_gen_opt = &score_gen_opts };
  scores::ScoreGenerator gen(&all_opts);
  gen.generate_image_scores();
}
