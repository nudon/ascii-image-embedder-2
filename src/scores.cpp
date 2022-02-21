#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <memory>
#include "scores.hpp"
#include "img_sample.hpp"
#include "match.hpp"

using namespace std;
using namespace cv;
using namespace options;
using namespace image_sample;
using namespace match;
namespace scores {

  void edge_filter_dog(bool keep_neg_edges, float sigma, int x_dim, int y_dim, cv::Mat &input, cv::Mat &output) {
    Mat blur;
    if (x_dim < 0 || x_dim % 2 == 0 ||
	y_dim < 0 || y_dim % 2 == 0) {
      cerr << "Kernel dims: x=" << x_dim << " y=" << y_dim << endl;
      throw invalid_argument("Invalid kernel dimensions.");
    }
    else if (sigma < 0) {
      cerr << "Sigma=" << sigma << endl;
      throw invalid_argument("Invalid sigma for blurring.");
    }

    GaussianBlur(input, blur, Size(x_dim, y_dim), sigma, sigma);
    if (keep_neg_edges) {
      Mat temp_input, temp_blur, temp_output;
      input.convertTo(temp_input, CV_8S, 0.5, 0);
      blur.convertTo(temp_blur, CV_8S, 0.5, 0);
      temp_output = temp_input - temp_blur;
      temp_output.convertTo(output, CV_8U, 1, 255/2);
    }
    else {
      output = input - blur;
    }
  }

  void gradient_extraction(AllOptions* opt, Mat &input, Mat &output) {
    ScoreGeneratorOptions* gen_opt = opt->score_gen_opt;
    float sigma = gen_opt->gaussian_blur_sigma;
    int kernel_size = gen_opt->gaussian_blur_size;
    bool neg_edges = gen_opt->keep_negative_edges;
    Mat x_edges, y_edges;
    edge_filter_dog(neg_edges, sigma, 1, kernel_size, input, x_edges);
    edge_filter_dog(neg_edges, sigma, kernel_size, 1, input, y_edges);
    if (input.channels() != 1) {
      cout << "Converting edge mats to grayscale" << endl;
      cvtColor(x_edges, x_edges, COLOR_BGR2GRAY);
      cvtColor(y_edges, y_edges, COLOR_BGR2GRAY);
    }
    normalize(x_edges, x_edges, 0, 255, NORM_MINMAX, -1, Mat());
    normalize(y_edges, y_edges, 0, 255, NORM_MINMAX, -1, Mat());

    output.create(input.size().height, input.size().width, CV_8UC2);
    Mat channels[] = {x_edges, y_edges};
    merge(channels, 2, output);
  }

  void edge_hist(Mat &x_in, Mat &y_in, Mat &hist_out) {
    Mat hist_in[] = {x_in, y_in};
    int hist_in_size = 2;
    int channels[] = {0,1};
    int hist_out_dim = 2;
    int x_bin_size = 4;
    int y_bin_size = 4;
    int hist_out_dims[] = {x_bin_size, y_bin_size};
    float range[] = {0, 256};
    const float* ranges[] = {range, range};
    bool uniform = true;
    bool accumulate = false;
    calcHist(hist_in, hist_in_size, channels, Mat(),
	     hist_out, hist_out_dim, hist_out_dims, ranges,
	     uniform, accumulate);
  }

  list<ProcessingRegion> generate_grid_regions(Mat img,int width, int height) {
    int region_rows = floor((float)img.size().height / height);
    int region_cols = floor((float)img.size().width / width);
    list<ProcessingRegion> region_list;
    for (int rows = 0; rows < region_rows; rows++) {
      int p_y = rows * height;
      for (int cols = 0; cols < region_cols; cols++) {
	int p_x = cols * width;
	Rect rect(p_x, p_y, width, height);
	
	ProcessingRegion region(rect, img);
	region_list.emplace_back(rect,img);
      }
    }
    return region_list;
  }

  list<EdgeHistData> generate_edge_histograms(Mat img, list<ProcessingRegion> &region_list, AllOptions* opt) {
    Mat grad;
    gradient_extraction(opt, img, grad);
    Mat x_edges, y_edges;
    extractChannel(grad, x_edges, 0);
    extractChannel(grad, y_edges, 1);
    /*
      imshow("image", img);
      waitKey();
      imshow("image", x_edges);
      waitKey();
      imshow("image", y_edges);
      waitKey();
    */
    list<EdgeHistData> data_list;
    for (ProcessingRegion &region : region_list) {
      Rect region_rect = region.get_region();
      Mat x_region = Mat(x_edges, region_rect);
      Mat y_region = Mat(y_edges, region_rect);
      Mat region_edge_hist;
      edge_hist(x_region, y_region, region_edge_hist);
      data_list.emplace_back(region_edge_hist);
    }
    return data_list;
  }
  template<class D,class I>
  list<DataMatchEntry<D*,I*>> pair_lists(list<I> &i_list, list<D> &d_list) {
    bool done = false;
    auto i_itr = i_list.begin();
    auto d_itr = d_list.begin();
    if (d_list.size() != i_list.size()) {
      throw runtime_error("Lists are not of the same size");
    }
    list<DataMatchEntry<D*,I*>> pair_list;
    while(!done) {
      if (i_itr == i_list.end()) {
	done = true;
      }
      else {
	I &i = *i_itr;
	D &d = *d_itr;
	pair_list.emplace_back(&d, &i);
	i_itr++;
	d_itr++;
      }
    }
    return pair_list;
  }

  void rework(AllOptions* opt) {
    EmbedOptions* emb = opt->embed_opt;
    string img_path = ".." + emb->image_dir + emb->image_name;
    cv::Mat img = cv::imread(img_path, IMREAD_COLOR);

    //somehow based on font_size and font get cell size
    Size img_dim = img.size();
    Size cell_dim(img_dim.width / 4, img_dim.height / 4);

    using EntryType = DataMatchEntry<EdgeHistData*, ProcessingRegion*>;
    using MatchType = DataMatch<EdgeHistData*, ProcessingRegion*>;
    list<ProcessingRegion> index_list = generate_grid_regions(img, cell_dim.width, cell_dim.height);
    list<EdgeHistData> data_list = generate_edge_histograms(img, index_list, opt);
    list<EntryType> work_list = pair_lists(index_list, data_list);
      
    list<MatchType> match_list;

    auto hist_compare_lambda([](EdgeHistData* a, EdgeHistData* b) {
      return EdgeHistData::compare(a,b);
    });
    Comparer<EdgeHistData*> comp (hist_compare_lambda, true);
      
    for (EntryType &base : work_list) {
      match_list.emplace_back(&base, &comp);
    }
    MatchType::find_best_matches(match_list, work_list);
      
    for (MatchType &match : match_list) {
      break;
      EntryType* a = match.get_base();
      EntryType* b = match.get_match();

      Mat base_img = a->get_index()->get_region_img();
      Mat match_img = b->get_index()->get_region_img();
      imshow("base image", base_img);
      waitKey();
      imshow("matched image", match_img);
      waitKey();
    }
    cout << "Rework done!" << endl;
  }
  
  //well design is spaghetti so I'll just work on score generator
  void ScoreGenerator::generate_image_scores() {
    //somehow have path + filename
    EmbedOptions* emb = opt->embed_opt;
    string img_path = ".." + emb->image_dir + emb->image_name;
    cv::Mat img = cv::imread(img_path, IMREAD_COLOR);

    //somehow based on font_size and font get cell size
    Size img_dim = img.size();
    Size cell_dim(img_dim.width / 4, img_dim.height / 4);
    //do wacky spacing/trimming for any excess regions
    //currently ignoring excess

    //generate list of regions based on img and cell dim
    int region_rows = floor((float)img_dim.height / cell_dim.height);
    int region_cols = floor((float)img_dim.width / cell_dim.width);
    Mat grad;
    gradient_extraction(opt, img, grad);

    Mat x_edges, y_edges;
    extractChannel(grad, x_edges, 0);
    extractChannel(grad, y_edges, 1);

    /*
    imshow("image", img);
    waitKey();
    imshow("image", x_edges);
    waitKey();
    imshow("image", y_edges);
    waitKey();
    */
    {
      //alternate where pointers everywehre
      using EntryType = DataMatchEntry<EdgeHistData*, ProcessingRegion*>;
      using MatchType = DataMatch<EdgeHistData*, ProcessingRegion*>;
      list<EntryType> work_list;
      list<unique_ptr<ProcessingRegion>> index_list;
      list<unique_ptr<EdgeHistData>> data_list;
      for (int rows = 0; rows < region_rows; rows++) {
	int p_y = rows * cell_dim.height;
	for (int cols = 0; cols < region_cols; cols++) {
	  int p_x = cols * cell_dim.width;
	  Rect rect(p_x, p_y, cell_dim.width, cell_dim.height);
	
	  ProcessingRegion region(rect, img);
	  Mat x_region = Mat(x_edges, rect);
	  Mat y_region = Mat(y_edges, rect);
	  Mat region_edge_hist;
	  edge_hist(x_region, y_region, region_edge_hist);

	  
	  index_list.push_back(make_unique<ProcessingRegion>(rect, img));
	  data_list.push_back(make_unique<EdgeHistData>(region_edge_hist));
	  ProcessingRegion* index_ptr = index_list.back().get();
	  EdgeHistData* data_ptr = data_list.back().get();
	  
	  work_list.emplace_back(data_ptr, index_ptr);
	}
      
      }
      list<MatchType> match_list;

      auto hist_compare_lambda([](EdgeHistData* a, EdgeHistData* b) {
	return compareHist(a->Data(), b->Data(), HISTCMP_CHISQR);
      });
      Comparer<EdgeHistData*> comp (hist_compare_lambda, true);
      
      for (EntryType &base : work_list) {
	match_list.emplace_back(&base, &comp);
      }
      MatchType::find_best_matches(match_list, work_list);
      for (MatchType &match : match_list) {
	break;
	EntryType* a = match.get_base();
	EntryType* b = match.get_match();

	Mat base_img = a->get_index()->get_region_img();
	Mat match_img = b->get_index()->get_region_img();
	imshow("base image", base_img);
	waitKey();
	imshow("matched image", match_img);
	waitKey();
      }
      cout << "Done!" << endl;
      rework(opt);
    }

    //print out ascii text and maybe internal rendering of ascii art
    string out_path;
    string out_text;
    cv::Mat out_img;
    //cv::imwrite(out_path, text_out);
  }

  void ScoreGenerator::generate_score_for_region(Rect reg, Mat& img) {
    //extract region from img;
    //or see if functions can operate on regions
    Mat sub_img;

    //blur/sobel or diff of gauss img along 1 d
    opt->score_gen_opt->gaussian_blur_sigma;
    Mat horz_edges;
    Mat vert_edges;

    //generate scores?
    float arr[2] = {1, 0};
    Mat base_line (1, 2, CV_32FC1, arr);
    for (int y = 0; y < reg.height; ++y) {
      for (int x = 0; x < reg.width; ++x) {
	//trig to get andle out of x/y comps.
	float edge_vals[2] = {vert_edges.at<float>(y,x) , horz_edges.at<float>(y,x)};
	Mat edge_line (1, 2, CV_32FC1, edge_vals);
	float dotprod = edge_line.dot(base_line);
	float theta = acos(dotprod / ( norm(edge_line))); //would do norm(edge) * norm(base) if norm(base) != 1 
	//or maybe one of the cross/dot products deals with angles and clamping and wraparound better I always forget those
	
	//nah will not work, at least anything with acos. atan2, than if y value is negative add 2PI to get [0,2PI] value range
      }
    }
  }  
}
