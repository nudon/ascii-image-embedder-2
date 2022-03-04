#ifndef FILE_FONT_SEEN
#define FILE_FONT_SEEN

#include <vector>
#include <stack>
#include <Magick++.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "options.hpp"

namespace font {
  using utf8_val = u_int32_t;
  using code_point = u_int;
  void font_init();
  /*
    Manages conversion of UTF8 encodings to/from codepoints
    see rfc3629 for specification and help with understanding bitshifts used

    Maybe also rework so code-points have their own aliased type
    did that, though assigning utf8_val's to/from code_point's gives no warning 
    ideally make the compiler complain about that instead
   */
  class UTF8Manager {
  public:
    using utf8_val = u_int32_t;
    using code_point = u_int;
    static int bytes_in_code(utf8_val code);
    static int bytes_needed_for_utf8_encoding(code_point code);
    static code_point utf8_to_code_point(utf8_val code);
    static utf8_val code_point_to_utf8(code_point code);
    static utf8_val next_code(utf8_val code);
    

  };
  
  /*
    Manages loading and querying of font to see if it contains a given code point
  */
  class FontManager {
  public:
    using utf8_val = UTF8Manager::utf8_val;
    using code_point = UTF8Manager::code_point;
    FontManager(options::AllOptions*);
    ~FontManager();
    bool code_point_in_font(code_point code);
  private:
    FT_Library ft_lib = nullptr;
    FT_Face ft_face_count = nullptr;
    std::list<FT_Face> ft_faces;
    FT_Face load_face(std::string &font_path, int index);
  };

  /*
    Managers generation of utf8-embeddings of utf8 values, 
    and utilities for turning code points into utf8-values (with checks that font has each code point)
   */
  class CharacterEncoder {
  public:
    using utf8_val = UTF8Manager::utf8_val;
    using code_point = UTF8Manager::code_point;
    CharacterEncoder(options::AllOptions*);
    std::list<utf8_val> code_list(code_point start, code_point end);
    std::list<utf8_val> code_list(std::list<code_point> &codes); //allows for non-contiguous codepoint sets
    static std::string utf8_string(utf8_val);
    static std::string utf8_string(std::list<utf8_val> &codes);
  private:
    std::unique_ptr<FontManager> font_manager = nullptr;
    static void byte_stack(utf8_val code, std::stack<u_int8_t> &bytes);
    static std::string byte_string(std::stack<u_int8_t> &bytes);
  };
  
  /*
    Handles font rendering and deals with GraphicsMagick  
  */
  class FontRenderer {
  public:
    using utf8_val = UTF8Manager::utf8_val;
    FontRenderer(options::AllOptions*);
    
    void render_utf8_string(std::string &text, int fontsize, cv::Mat &render_out);
    std::list<Magick::TypeMetric> gather_character_dims(std::list<utf8_val> &codes,
							int fontsize);
    static cv::Mat gm_to_cv(Magick::Image & img);
  private:
    std::string font_path;
    std::string font_name;
    Magick::Image blank_image(int font_size);
  };

  /*
    Responsible for building a rendering of given characters
    and keep track of characters and their respective processing regions in rendering
    
    to provide some way of association processing regions to characters
    can either do with fancy indexing math (if uniform width) 
    or binary search of regions (assuming it is built in some known order)

   */
  class CharacterSet {
  public:
    using utf8_val = UTF8Manager::utf8_val;
    using Regions = int;
    CharacterSet(options::AllOptions*, std::list<utf8_val>&);
    const cv::Mat get_rendering();
  private:
    //std::unique_ptr<CharacterEncoder> encoder = nullptr;
    std::vector<utf8_val> characters;
    std::vector<Regions> regions;
    bool uniform_width;
    cv::Mat character_rendering;
  };
}
#endif
