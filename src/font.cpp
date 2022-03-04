#include "font.hpp"
#include <stdexcept>
#include <iostream>
#include <stack>

using namespace std;
using namespace Magick;
using namespace options;
namespace font {

  void font_init() {
    Magick::InitializeMagick(nullptr);
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

  int UTF8Manager::bytes_needed_for_utf8_encoding(code_point code) {
    int bits = 0;
    int bytes = 0;
    do {
      code = code >> 1;
      bits++;
    } while (code != 0);

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
  
  auto UTF8Manager::utf8_to_code_point(utf8_val code) -> code_point {
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

  auto UTF8Manager::code_point_to_utf8(code_point code) -> utf8_val {
    u_int8_t byte_mask = 0b00111111;
    int byte_mask_active_bits = 6;
    
    int bytes_in_utf8 = bytes_needed_for_utf8_encoding(code);
    vector<u_int8_t> bytes(bytes_in_utf8, 0);

    //grab bits for every byte but first
    for (int i = bytes_in_utf8 - 1; i > 0; i--) {
      //bytes[i] = (code & byte_mask) + (1 << 7);
      bytes[i] = (code & byte_mask) | 0b01000000;
      code = code >> byte_mask_active_bits;
    }
    //first byte, set correct prefix then grab what's left in temp_val
    switch(bytes_in_utf8) {
    case 1:
      byte_mask = 0b00000000; // no mask, just assign raw code
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
    bytes[0] = code | byte_mask;
      
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
  
  FontManager::FontManager(AllOptions* opt) {
    FontOptions* font_opt = opt->font_opt;
    string font_dir = font_opt->font_dir;;
    string font_name = font_opt->font_name;
    string font_path = ".." + font_dir + font_name;
    int error = FT_Init_FreeType( &ft_lib );
    if (error) {
      throw std::runtime_error("Failed to initialize freetype library");
    }
    //index of -1 loads font and finds the total faces in font
    ft_face_count = load_face(font_path, -1);
    for (int i = 0; i < ft_face_count->num_faces; i++) {
      ft_faces.push_back(load_face(font_path, i));
    }
  }

  FontManager::~FontManager() {
    for (FT_Face &face : ft_faces) {
      FT_Done_Face(face);
    }
    FT_Done_Face(ft_face_count);
    int error = FT_Done_FreeType( ft_lib );
    if (error) {
      cerr << "Failed to cleanup freetype library" << endl;
    }
    ft_lib = nullptr;
  }

  FT_Face FontManager::load_face(string &font_path, int i) {
    FT_Face face = nullptr;
    int error = FT_New_Face(ft_lib, font_path.c_str(), i, &face);
    if (error) {
      cerr << "Opening " << font_path << " with face index " << i << " failed." << endl;
      throw runtime_error("Unable to open font with given index");
    }
    return face;
  }
  
  bool FontManager::code_point_in_font(code_point code) {
    for (FT_Face &face : ft_faces) {
      if (FT_Get_Char_Index(face, code) != 0) {
	return true;
      }
    }
    return false;
  }

  CharacterEncoder::CharacterEncoder(AllOptions* opt) {
    font_manager = make_unique<FontManager>(opt);    
  }

  auto CharacterEncoder::code_list(code_point start, code_point end) -> list<utf8_val> {
    FontManager *font = font_manager.get();
    list<utf8_val> codes;
    /* 
    //pretty sure looping over utf8_vals is uncessary work
    for (utf8_val code = start; code <= end; code = UTF8Manager::next_code(code)) {
      u_int code_point = UTF8Manager::utf8_to_code_point(code);
      if (font->code_point_in_font(code_point)) {
	codes.push_back(code);
      }
    }
    */
    for (code_point code = start; code <= end; code++) {
      if (font->code_point_in_font(code)) {
	  codes.push_back(UTF8Manager::code_point_to_utf8(code));
      }
    }
    return codes;
  }

  auto CharacterEncoder::code_list(list<code_point> &codes) -> list<utf8_val> {
    FontManager *font = font_manager.get();
    list<utf8_val> utf8_codes;
    for (code_point &code : codes) {
      if (font->code_point_in_font(code)) {
	utf8_codes.push_back(UTF8Manager::code_point_to_utf8(code));
      }
    }
    return utf8_codes;
  }

  string CharacterEncoder::utf8_string(utf8_val code) {
    stack<u_int8_t> byte_encoding;
    byte_stack(code,byte_encoding);
    return byte_string(byte_encoding);
  }

  string CharacterEncoder::utf8_string(list<utf8_val> &codes) {
    stack<u_int8_t> byte_encoding;
    for (auto rev_itr = codes.rbegin(); rev_itr != codes.rend(); rev_itr++) {
      utf8_val temp = *rev_itr;
      byte_stack(temp, byte_encoding);
    }
    return byte_string(byte_encoding);
  }

  void CharacterEncoder::byte_stack(utf8_val code, stack<u_int8_t> &bytes) {
    int byte_count = UTF8Manager::bytes_in_code(code);
    for (int i = 0; i < byte_count; i++) {
      u_int8_t byte = code & 0xff;
      bytes.push(byte);
      code = code >> 8;
    }
  }

  string CharacterEncoder::byte_string(stack<u_int8_t> &bytes) {
    string build;
    while(!bytes.empty()) {
      build.push_back(bytes.top());
      bytes.pop();
    }
    build.push_back('\0');
    return build;
  }
  

  FontRenderer::FontRenderer(AllOptions* opt) {
    FontOptions* font_opt = opt->font_opt;
    string dir = font_opt->font_dir;
    string name = font_opt->font_name;
    font_path = ".." + dir + name;
    font_name = name;
  }

  void FontRenderer::render_utf8_string(string &text,
			       int fontsize,
			       cv::Mat &render_out) {
    Magick::Image gm_img = blank_image(fontsize);
    Magick::Color white("white");
    Magick::Color red("red");
    Magick::Color black("black");
    Magick::TypeMetric metric;
    gm_img.fontTypeMetrics(text,&metric);
    gm_img.fontTypeMetrics(text,&metric);
    Magick::Geometry size(metric.textWidth(), metric.textHeight());
    gm_img.size(size);
    gm_img.floodFillColor(0,0,white);
    gm_img.floodFillColor(0,0,white); //for some reason need 2 fills to have any affect...
    
    Magick::Geometry position(0,size.height()); //might need to set height to size.height()
    gm_img.fillColor(black);
    gm_img.annotate(text, position);
    gm_img.write("test.png");
    //iterate over image and convert to a cv_mat
    
    render_out = gm_to_cv(gm_img);
  }

  list<TypeMetric> FontRenderer::gather_character_dims(list<utf8_val> &codes, int fontsize) {
    Magick::Image gm_img = blank_image(fontsize);

    list<TypeMetric> dims;

    for (utf8_val &code : codes) {
      TypeMetric metric;
      string utf8_text = CharacterEncoder::utf8_string(code);
      gm_img.fontTypeMetrics(utf8_text, &metric);
      dims.push_back(metric);
    }
    return dims;
  }

  Magick::Image FontRenderer::blank_image(int font_size) {
    Magick::Image gm_img;
    int i = font_name.find(".");
    string font = font_name.substr(0,i);
    gm_img.textEncoding("UTF-8");
    gm_img.font(font_path);
    gm_img.fontPointsize(font_size);
    return gm_img;
  }

  cv::Mat FontRenderer::gm_to_cv(Magick::Image & gm_img) {
    Geometry size = gm_img.size();
    cv::Mat cv_img(size.height(), size.width(), CV_8UC1);
    for (size_t row = 0; row < size.height(); row++) {
      for (size_t col = 0; col < size.width(); col++) {
	Magick::Color pix = gm_img.pixelColor(col,row);
	Magick::ColorGray gray(pix);
	cv_img.at<uchar>(row,col) = floor(gray.shade() * 255);
      }
    }
    return cv_img;
  }

  /*
    So, to get this running
    need calling code to have a CharacterEncoder build the character list
    and have options contain font name/path
   */
  CharacterSet::CharacterSet(AllOptions* opts, list<utf8_val> &characters) {
    FontOptions* font_opts = opts->font_opt;
    int font_size = font_opts->font_size;
    FontRenderer font_rend(opts);
    list<TypeMetric> dims;
    string font_codes = CharacterEncoder::utf8_string(characters);
  
    font_rend.render_utf8_string(font_codes, font_size, character_rendering);
    dims = font_rend.gather_character_dims(characters, font_size);
    /*
    cout << font_codes << endl;
    for (TypeMetric &dim : dims) {
      cout << "Character has width of " << dim.textWidth() <<
	" and height of " << dim.textHeight() << endl;
      
    }
    */
    //then, need to go over character rendering and make processing regions
    //and store vector of characters
  }

  const cv::Mat CharacterSet::get_rendering() {
    return character_rendering;
  }
  
}
