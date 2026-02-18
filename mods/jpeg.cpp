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

static bool save_jpeg(std::string filename, unsigned char *data, util::Size<int> size, bool _save_rgba)
{
	/* This struct contains the JPEG compression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 * It is possible to have several such structures, representing multiple
	 * compression/decompression processes, in existence at once.  We refer
	 * to any one struct (and its associated working data) as a "JPEG object".
	 */
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.  It is declared separately
	 * because applications often want to supply a specialized error handler
	 * (see the second half of this file for an example).  But here we just
	 * take the easy way out and use the standard error handler, which will
	 * print a message on stderr and call exit() if compression fails.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	 * step fails.  (Unlikely, but it could happen if you are out of memory.)
	 * This routine fills in the contents of struct jerr, and returns jerr's
	 * address which we place into the link field in cinfo.
	 */
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	 * stdio stream.  You can also write your own code to do something else.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to write binary files.
	 */
	if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	 * Four fields of the cinfo struct must be filled in:
	 */
	cinfo.image_width = size.w; 	/* image width and height, in pixels */
	cinfo.image_height = size.h;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	 * (You must set at least cinfo.in_color_space before calling this,
	 * since the defaults depend on the source color space.)
	 */
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	 * Here we just illustrate the use of quality (quantization table) scaling:
	 */
	const int quality = 100;
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	 * Pass TRUE unless you are very sure of what you're doing.
	 */
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 * To keep things simple, we pass one scanline per call; you can pass
	 * more if you wish, though.
	 */
	row_stride = size.w * 3;	/* JSAMPLEs per row in image_buffer */

	unsigned char *buffer = new unsigned char[size.w*3];
	int y = 0;

	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		for (int i = 0; i < size.w; i++) {
			unsigned char *p = buffer + i * 3;
			unsigned char *d = data + (size.h - y - 1) * size.w * 4 + i * 4;
			*p++ = *d++;
			*p++ = *d++;
			*p++ = *d++;
		}
		row_pointer[0] = buffer;
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
		y++;
	}

	delete[] buffer;

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	/* And we're done! */

	return true;
}

extern "C" {

	void EXPORT booboo_start(void)
	{
		gfx::Image::register_image_loader("jpg", read_jpeg);
		gfx::Image::register_image_loader("jpeg", read_jpeg);
		gfx::Image::register_image_saver("jpg", save_jpeg);
		gfx::Image::register_image_saver("jpeg", save_jpeg);
	}

}
