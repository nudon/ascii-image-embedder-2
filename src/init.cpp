#include <opencv2/opencv.hpp>

#include "CLI11.hpp"
#include "scores.hpp"
#include "font.hpp"


using namespace std;
using namespace cv;
using namespace options;
using namespace scores;
using namespace font;
using namespace image_data;
using namespace match;

void rework(AllOptions* opt);

int main (int argc, char** argv) {
  std::cout << "Hi" << std::endl;
  WeightOptions weight_opts;
  ImageDataOptions image_data_opts;
  EmbedOptions embed_opts;
  FontOptions font_opts;
  if (argc >= 2) {
    embed_opts.image_name = std::string(argv[1]);
    embed_opts.other_image_name = embed_opts.image_name;
  }
  if (argc >= 3) {
    embed_opts.other_image_name = std::string(argv[2]);
  }
  AllOptions all_opts {
    .weight_opt = &weight_opts,
    .embed_opt = &embed_opts,
    .image_data_opt = &image_data_opts,
    .font_opt = &font_opts
  };
  
  font::font_init();

  rework(&all_opts);
}

  void rework(AllOptions* opt) {
    bool test_match = false;
    bool test_font = true;
    if (test_match) {
      EmbedOptions* emb = opt->embed_opt;
      string img_path = ".." + emb->image_dir + emb->image_name;
      string other_img_path = ".." + emb->image_dir + emb->other_image_name;
      Mat img = imread(img_path, IMREAD_COLOR);
      Mat other_img = imread(other_img_path, IMREAD_COLOR);

      ImageData img_data(img, opt);
      ImageData other_img_data(other_img, opt);

      using MatcherType = DataMatch<HistData*, ProcessingRegion*>;
      using ScorerType = ScoreMatcher<HistData, ProcessingRegion>;
      using EntryType = MatcherType::Entry;
      using MatchType = MatcherType::Match;

      Comparer<HistData*> comp ( [](HistData* a, HistData* b) {
	return HistData::compare(a,b);
      }, true);
    

      //TODO: Have more of this managed by settings in options
      list<EntryType> match_entries = ScorerType::build_entry_list(img_data.get_regions(), img_data.get_data());
      list<EntryType> search_entries = ScorerType::build_entry_list(other_img_data.get_regions(), other_img_data.get_data());
      list<MatchType> match_list = ScorerType::build_match_list(&match_entries, &comp);

      for (MatchType &match : match_list) {
	match.find_best_match(search_entries);
	//break;
	EntryType* a = match.get_base();
	EntryType* b = match.get_match();
      
	Mat base_img = a->get_index()->get_region_img();
	Mat match_img = b->get_index()->get_region_img();
	imshow("base image", base_img);
	waitKey();
	imshow("matched image", match_img);
	waitKey();
      }
    }
    if (test_font) {
      int start = 32;
      int end = 126;
      font::CharacterEncoder encoder(opt);
      std::cout << "building utf8 list" << std::endl;
      std::list<font::utf8_val> codes = encoder.code_list(start,end);
      std::cout << "building character list" << std::endl;
      font::CharacterSet characters(opt, codes);
      cv::imshow("characters", characters.get_rendering());
      cv::waitKey();
    }
    cout << "Rework done!" << endl;

    
  }
