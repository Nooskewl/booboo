#include "booboo/booboo.h"
#include "shim5/shim5.h"
#include <jpeglib.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__ ((visibility("default"))) 
#endif

using namespace booboo;
using namespace noo;

// This is based on the FILE source manager in libjpeg

const static JOCTET EOI_BUFFER[1] = { JPEG_EOI };
struct my_source_mgr {
  struct jpeg_source_mgr pub;
  const JOCTET *data;
  size_t       len;
};

static void my_init_source(j_decompress_ptr cinfo) {}

static boolean my_fill_input_buffer(j_decompress_ptr cinfo) {
  my_source_mgr* src = (my_source_mgr*)cinfo->src;
  // No more data.  Probably an incomplete image;  just output EOI.
  src->pub.next_input_byte = EOI_BUFFER;
  src->pub.bytes_in_buffer = 1;
  return TRUE;
}
static void my_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
  my_source_mgr* src = (my_source_mgr*)cinfo->src;
  if (src->pub.bytes_in_buffer < num_bytes) {
    // Skipping over all of remaining data;  output EOI.
    src->pub.next_input_byte = EOI_BUFFER;
    src->pub.bytes_in_buffer = 1;
  } else {
    // Skipping over only some of the remaining data.
    src->pub.next_input_byte += num_bytes;
    src->pub.bytes_in_buffer -= num_bytes;
  }
}
static void my_term_source(j_decompress_ptr cinfo) {}

static void my_set_source_mgr(j_decompress_ptr cinfo, const char* data, size_t len) {
  my_source_mgr* src;
  if (cinfo->src == 0) { // if this is first time;  allocate memory
    cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)
      ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(my_source_mgr));
  }
  src = (my_source_mgr*) cinfo->src;
  src->pub.init_source = my_init_source;
  src->pub.fill_input_buffer = my_fill_input_buffer;
  src->pub.skip_input_data = my_skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; // default
  src->pub.term_source = my_term_source;
  // fill the buffers
  src->data = (const JOCTET *)data;
  src->len = len;
  src->pub.bytes_in_buffer = len;
  src->pub.next_input_byte = src->data;
}

static unsigned char *read_jpeg(std::string filename, util::Size<int> &out_size, SDL_Color *out_palette, util::Point<int> *opaque_topleft, util::Point<int> *opaque_bottomright, bool *has_alpha, bool load_from_filesystem)
{
	printf("HERE!\n");
  struct jpeg_decompress_struct cinfo; 
  struct jpeg_error_mgr jerr;

  int len;
  char *data;
  if (load_from_filesystem) {
    data = util::slurp_file_from_filesystem(filename, &len);
  }
  else {
    data = util::slurp_file(filename, &len);
  }
  printf("data=%p len=%d\n", data, len);

  // Setup decompression structure
  cinfo.err = jpeg_std_error(&jerr); 
  jpeg_create_decompress(&cinfo); 
  my_set_source_mgr(&cinfo, data,len);

  // read info from header.
  int r = jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  int row_stride = cinfo.output_width * cinfo.output_components;
  JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  char *bytes = new char[4*cinfo.output_width*cinfo.output_height];

  printf("components=%d\n", cinfo.output_components);

  int y = 0;
 
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    for (int i = 0; i < cinfo.output_width; i++) {
	    char *p = bytes + (cinfo.output_height - y - 1) * cinfo.output_width * 4 + 4 * i;
	    if (cinfo.output_components == 1) {
		    int c = buffer[0][i];
		    *p++ = c;
		    *p++ = c;
		    *p++ = c;
		    *p++ = 255;
	    }
	    else {
		    int r = buffer[0][i*3+0];
		    int g = buffer[0][i*3+1];
		    int b = buffer[0][i*3+2];
		    *p++ = r;
		    *p++ = g;
		    *p++ = b;
		    *p++ = 255;
	    }
    }
    y++;
  } 

  (void) jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo); 

  if (out_palette != nullptr) {
	  memset(out_palette, 0, 256*sizeof(SDL_Color));
  }
  *opaque_topleft = util::Point<int>(0, 0);
  *opaque_bottomright = util::Point<int>(cinfo.output_width-1, cinfo.output_height-1);
  *has_alpha = false;

  out_size.w = cinfo.output_width;
  out_size.h = cinfo.output_height;

  return (unsigned char *)bytes;
}

extern "C" {

void EXPORT booboo_start(void)
{
	printf("reging\n");
	gfx::Image::register_image_loader("jpg", read_jpeg);
	gfx::Image::register_image_loader("jpeg", read_jpeg);
}

}
