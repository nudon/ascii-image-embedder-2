#ifndef FILE_SCORES_SEEN
#define FILE_SCORES_SEEN

namespace scores {
  class ScoreGenerator;
  class ScoreMatcher;
  class Embedder;
}

#include "opencv2/opencv.hpp"
#include "options.hpp"
#include "util.hpp"

namespace scores {
  /*
    Color scores are fairly standard, just go over pixels in region
    and calculate averages
    Optionall second pass over scores to produce relative albedo value
    where 0 would be darkest value found
    
    Edge scores generally lots of values to desribe dominant directions 
    of change in image.
    
    Both of these seem to be covered by opencv's concept of Histrograms
    which compresses down an image into a distribution of values
    Edges might be better off with something that preserves some spatial information...which would just be taking histrogram of sub-regions. 
    also a way of taking multi-dimentional histograms which might be useful for edges...doesn't account for spatial distribution of edges but sums up general trends. 
   */


  //simple difference of guassian filter
  void edge_filter_dog(bool keep_neg_edges,float sigma, int x_dim, int y_dim, cv::Mat &input, cv::Mat &output);
  //computes and extracts individual x/y gradients into channels 0(x) and 1(y)
  void gradient_extraction(options::AllOptions* opt, cv::Mat &input, cv::Mat &output);
  
  class Scores {
  public:
    Scores();

  private:
    float color_scores;
    float edge_scores;
  };


  //given font/image, generate scores for each cell/glyph
  class ScoreGenerator {
  public:
    ScoreGenerator(options::AllOptions* options) { opt = options; }
    void generate_image_scores();
    void generate_font_scores();
    void generate_score_for_region(cv::Rect reg, cv::Mat& img);
  private:
    options::AllOptions* opt = nullptr; 
  };

  //given scores, match themp based on weights
  class ScoreMatcher {
    ScoreMatcher(options::AllOptions* opt);
    void match_scores();
  };


  //given matches, output to image
  class Embedder {
    Embedder(options::AllOptions* opt);
    void embed();
  private:
    options::AllOptions* opt = nullptr;
    ScoreGenerator* score_gen = nullptr;
    ScoreMatcher* score_match = nullptr;
  };

}
  
#endif
