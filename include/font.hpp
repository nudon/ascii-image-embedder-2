#ifndef FILE_FONT_SEEN
#define FILE_FONT_SEEN

//#include <MagickWand/MagickWand.h> //ehh I'll try GraphicsMatick instead
#include <vector>
#include <Magick++.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "options.hpp"

namespace font {
  /*
    Manages bitshifts and other calculations on utf8-encoded values

    see rfc3629 for specification
   */
  class UTF8Manager {
  public:
    using utf8_val = u_int32_t;
    static int bytes_in_code(utf8_val code);
    static int bytes_needed_for_utf8_encoding(u_int val);
    static int utf8_to_code_point(utf8_val code);
    static utf8_val code_point_to_utf8(int val);
    static utf8_val next_code(utf8_val code);
    

  };

  /*
    Handles font rendering and deals with freetype and GraphicsMagick  
   */
  class FontManager {
  public:
    using utf8_val = UTF8Manager::utf8_val;
    FontManager(options::AllOptions*);
    ~FontManager();
  private:
    FT_Library ft_lib = nullptr;
    FT_Face ft_face;
    cv::Mat FontAlphabet;
    std::vector<u_int32_t> CodePoints;
    bool code_point_in_font(u_int val);
    void generate_characters(utf8_val start, utf8_val end,
			     int fontsize, std::string font_fn);
  };
}
#endif
