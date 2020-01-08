#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <png.h>

int main(int argc, char *argv[])
{
  if(argc != 5) {
    std::cout << "Usage: " << argv[0] << " size width [no]fill filename"
	      << std::endl;
    return 1;
  }

  bool fill;
  if(strcmp(argv[3], "fill") == 0)
    fill = true;
  else if(strcmp(argv[3], "nofill") == 0)
    fill = false;
  else {
    std::cout << "Invalid fill `" << argv[3] << "'. Should be fill or nofill."
	      << std::endl;
    return 1;
  }

  int const w = atoi(argv[1]);
  if(w < 10) {
    std::cout << "Invalid size: " << w << "." << std::endl;
    return 1;
  }
  int const h = w;

  double const isophote_width = atof(argv[2]);
  if(isophote_width <= 0.0) {
    std::cout << "Invalid width: " << isophote_width << "." << std::endl;
    return 1;
  }

  unsigned char *data = new unsigned char[w * h * 3];

  for(int i = 0, index = 0; i < w; ++i) {
    double xm = (double)i / (double)(w - 1) - 0.5;
    if(fill) xm /= sqrt(2.0);
    for(int j = 0; j < h; ++j) {
      double ym = (double)j / (double)(h - 1) - 0.5;
      if(fill) ym /= sqrt(2.0);
      double const length2 = xm * xm + ym * ym;
      if(length2 <= 0.25) {
	double const z = 8.0 * (0.25 - length2) - 1.0;
	double const angle = (std::acos(z) * 180.0 / M_PI) / 2.0;
	bool color =
	  static_cast<int>(std::floor(angle / isophote_width)) % 2 == 0;
	data[index++] = 255;
	data[index++] = color ? 0 : 255;
	data[index++] = color ? 0 : 255;
      } else { // outside the interesting region, just fill with zeroes
	data[index++] = 0;
	data[index++] = 0;
	data[index++] = 0;
      }
    }
  }

  FILE *fp = fopen(argv[4], "wb");
  if(!fp) {
    std::cerr << "Unable to open `" << argv[4] << "'." << std::endl;
    return 1;
  }
  
  png_structp png_ptr =
    png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    std::cerr << "Unable to create PNG write structure." << std::endl;
    fclose(fp);
    return 1;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    std::cerr << "Unable to create PNG info structure." << std::endl;
    fclose(fp);
    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    std::cerr << "Unable to write PNG file." << std::endl;
    fclose(fp);
    return 1;
  }

  png_init_io(png_ptr, fp);
  png_set_compression_level(png_ptr, 9);
  png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
	       PNG_FILTER_TYPE_DEFAULT);

  png_byte **row_pointers = new png_byte*[h];
  for(int i = 0, index = (h-1)*w*3; i < h; ++i, index -= w*3)
    row_pointers[i] = (png_byte *)&data[index];
  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  delete[] row_pointers;

  fclose(fp);

  delete[] data;

  return 0;
}
