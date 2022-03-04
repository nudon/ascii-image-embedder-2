#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <memory>
#include "scores.hpp"
#include "img_sample.hpp"
#include "match.hpp"

using namespace std;
using namespace cv;
using namespace options;
using namespace image_data;
using namespace match;
namespace scores {

  std::list<ProcessingRegion> RegionGenerator::grid_regions(cv::Mat &img, int width, int height) {
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

  void DataGenerator::edge_filter_dog(bool keep_neg_edges, float sigma, int x_dim, int y_dim, Mat &input, Mat &output) {
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

  
  void DataGenerator::gradient_extraction(Mat &input, Mat &output, AllOptions* opt) {
    ImageDataOptions* gen_opt = opt->image_data_opt;
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

  void DataGenerator::hist_2D(Mat &x_in, Mat &y_in, Mat &hist_out, int x_size, int y_size) {
    Mat hist_in[] = {x_in, y_in};
    int hist_in_size = 2;
    int channels[] = {0,1};
    int hist_out_dim = 2;
    int hist_out_dims[] = {x_size, y_size};
    float range[] = {0, 256};
    const float* ranges[] = {range, range};
    bool uniform = true;
    bool accumulate = false;
    calcHist(hist_in, hist_in_size, channels, Mat(),
	     hist_out, hist_out_dim, hist_out_dims, ranges,
	     uniform, accumulate);
  }

  
  list<HistData> DataGenerator::edge_histograms(Mat &img, list<ProcessingRegion> &region_list, AllOptions* opt) {
    Mat grad;
    Mat x_edges, y_edges;
    list<HistData> data_list;
    //TODO: get size from options
    int x_size = 4;
    int y_size = 4;
    DataGenerator::gradient_extraction(img, grad, opt);

    extractChannel(grad, x_edges, 0);
    extractChannel(grad, y_edges, 1);

    for (ProcessingRegion &region : region_list) {
      Rect region_rect = region.get_region();
      Mat x_region = Mat(x_edges, region_rect);
      Mat y_region = Mat(y_edges, region_rect);
      Mat region_edge_hist;
      hist_2D(x_region, y_region, region_edge_hist, x_size, y_size);
      data_list.emplace_back(region_edge_hist);
    }
    return data_list;
  }

  ImageData::ImageData(Mat &img, AllOptions* opt) {
    ImageDataOptions *data_opt = opt->image_data_opt;
    Size cell_dim (data_opt->grid_width, data_opt->grid_height);

    region_list = RegionGenerator::grid_regions(img, cell_dim.width, cell_dim.height);
    if (data_opt->edge_metric == EdgeMetric::Histogram) {
      data_list = DataGenerator::edge_histograms(img, region_list, opt);
    }
  }

  list<ProcessingRegion>* ImageData::get_regions() {
    return &region_list;
  }
  list<HistData>* ImageData::get_data() {
    return &data_list;
  }
}
