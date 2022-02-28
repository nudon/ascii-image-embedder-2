#include "font.hpp"
#include <stdexcept>
#include <iostream>
#include <stack>

using namespace options;
using namespace std;
namespace font {

  /*
    So ideal interaction is, given some font, font-size, (probably some code-block), start by setting up libs, then rendering all good glyphs to a straight line in GM, gathering font dim statistics. Then, convert IM into some cvMat
    Also want some buffer of chars/u_int32, and some way of converting ProcessingRegion/rects to which char is at that point by doing some maths
  */
  FontManager::FontManager(AllOptions*) {
    int error = FT_Init_FreeType( &ft_lib );
    if (error) {
      throw std::runtime_error("Failed to initialize freetype library");
    }
    bool use_ascii = true;
    int start=-1, end=-1;
    int size = 12;
    string font = "comicsans.ttf";
    if (use_ascii) {
      start = 32;
      end = 126; 
    }
    
    generate_characters(start,end, size, font); //font size, font name in options
  }

  FontManager::~FontManager() {
    int error = FT_Done_FreeType( ft_lib );
    if (error) {
      cerr << "Failed to cleanup freetype library" << endl;
    }
    ft_lib = nullptr;
  }

  auto UTF8Manager::bytes_in_code(utf8_val code) -> int {
    int byte_shift = 8;
    int bytes = 0;
    do {
      bytes++;
      code = code >> byte_shift;
    } while (code != 0);
    return bytes;
  }

  int bytes_needed_for_utf8_encoding(u_int val) {
    int bits = 0;
    int bytes = 0;
    while (val != 0) {
      val = val >> 1;
      bits++;
    }
    if (bits <= 7) {
      bytes = 1;
    }
    else if (bits <= 6 * 1 + 5) {
      bytes = 2;
    }
    else if (bits <= 6 * 2 + 4) {
      bytes = 3;
    }
    else if (bits <= 6 * 3 + 3) {
      bytes = 4;
    }
    else {
      cerr << "Value would need more than 4 bytes to store in utf8" << endl;
      throw invalid_argument("Value can not be encoded in utf8");
    }
    return bytes;
  }
  
  int UTF8Manager::utf8_to_code_point(utf8_val code) {
    u_int8_t byte_mask = 0b00111111;
    int shift_amount = 6;
    utf8_val temp_code = code;
    int byte_count = bytes_in_code(code);
    int code_point = 0;
    vector<u_int8_t> bytes(byte_count, 0);
    for (int i = byte_count - 1; i >= 0; i--) {
      bytes[i] = temp_code & 0xff;
      temp_code = temp_code >> 8;
    }
    temp_code = code;

    //iterate over every byte but the first, which have same byte mask
    for (int i = 1; i <= byte_count - 1; i++) {
      code_point = code_point << shift_amount;
      code_point += bytes[i] & byte_mask;
    }
    //for first byte, mask depends.
    switch(byte_count) {
    case 1: // ascii value
      byte_mask = 0b01111111;
      break;
    case 2: //grab 5 bits
      byte_mask = 0b00011111;
      break;
    case 3: //grab 4 bits
      byte_mask = 0b00001111;
      break;
    case 4: //grab 3 bits;
      byte_mask = 0b00000111;
      break;
    default:
      //since u_int32_t are used something spooky would need to happen to trigger this
      cerr << "Found some utf8 codepoint that needs " << byte_count << " bytes, check if code block range is correct" << endl;
      throw invalid_argument("Error in advancing to next code point");
      break;      
    }
    code_point += (bytes[0] & byte_mask) << ((byte_count - 1) * 6);
    
    return code_point;
  }

  auto UTF8Manager::code_point_to_utf8(int val) -> utf8_val {
    u_int8_t byte_mask = 0b00111111;
    int byte_mask_active_bits = 6;
    int temp_val = val;

    int bytes_in_utf8 = bytes_needed_for_utf8_encoding(val);
    vector<u_int8_t> bytes(bytes_in_utf8, 0);

    //grab bits for every byte but first
    for (int i = bytes_in_utf8 - 1; i > 0; i--) {
      //bytes[i] = (temp_val & byte_mask) + (1 << 7);
      bytes[i] = (temp_val & byte_mask) | 0b01000000;
      temp_val = temp_val >> byte_mask_active_bits;
    }
    //first byte, set correct prefix then grab what's left in temp_val
    switch(bytes_in_utf8) {
    case 1:
      byte_mask = 0b01111111;
      break;
    case 2:
      byte_mask = 0b11000000;
      break;
    case 3:
      byte_mask = 0b11100000;
      break;
    case 4:
      byte_mask = 0b11110000;
      break;
    default:
      break;
    }
    //bytes[0] = byte_mask + temp;
    bytes[0] = temp_val | byte_mask;
      
    utf8_val ret = 0;
    for (int i = 0; i < bytes_in_utf8; i++) {
      ret = ret << 8;
      ret += bytes[i];
    }
    return ret;
  }

  auto UTF8Manager::next_code(utf8_val code) -> utf8_val {
    int seq_num = utf8_to_code_point(code);
    seq_num++;
    utf8_val ret = code_point_to_utf8(seq_num);
    return ret;    
  }
  
  void FontManager::generate_characters(utf8_val start, utf8_val end, int fontsize, string font_file_name) {
    //iterate from start to endcode, copy codes that exist in font to buffer
    vector<utf8_val> codes;
    for (utf8_val code = start; code <= end; code = UTF8Manager::next_code(code)) {
      //have some font loaded in free_type,  check if intcode exists in font;
      u_int code_point = UTF8Manager::utf8_to_code_point(code);
      if (code_point_in_font(code_point)) {
	//copy to some buffer, ideally in whatever GraphicsMagick expects it to be
	//seems to take char*, so just cut utf-8 into bytes?
	codes.push_back(code);
      }
    }
    //then have IM render buffer, copy rendering to cvMat
    //set encoding to  utf-8
    //hopefully you can just do that by setting encoding to  utf8
    //and handing in bytes in char* field in Drawinfo;
    stack<u_int8_t> byte_encoding;
    for (int i = codes.size() - 1; i >= 0; i--) {
      utf8_val temp = codes[i];
      for (int j = 0; j < UTF8Manager::bytes_in_code(temp); j++) {
	u_int8_t byte = temp & 0xff;
	byte_encoding.push(byte);
	temp = temp >> 8;
      }
    }
    
    string fun;
    while(!byte_encoding.empty()) {
      fun.push_back(byte_encoding.top());
      byte_encoding.pop();
    }
    fun.push_back('\0');

    Magick::Image gm_img;
    Magick::Color white("white");
    Magick::Color black("black");
    gm_img.textEncoding("UTF-8");
    gm_img.font(font_file_name);
    gm_img.fontPointsize(fontsize);

    Magick::TypeMetric metric;
    gm_img.fontTypeMetrics(fun,&metric);
    Magick::Geometry size(metric.textWidth(), metric.textHeight());
    gm_img.size(size);
    gm_img.floodFillColor(0,0,white);
    //can't seem to set annotation color?
    Magick::Geometry position(0,0); //might need to set height to size.height()
    gm_img.annotate(fun, position);

    //iterate over image and convert to a cv_mat
    cv::Mat cv_img(size.height(), size.width(), CV_8UC1);
    for (int row = 0; row < size.height(); row++) {
      for (int col = 0; col < size.width(); col++) {
	Magick::Color pix = gm_img.pixelColor(col,row);
	Magick::ColorGray gray(pix);
	cv_img.at<uchar>(row,col) = floor(gray.shade() * 255);
      }
    }
    
  }
  
}
