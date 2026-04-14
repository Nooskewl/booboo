#include "booboo/booboo.h"
#include "shim5/shim5.h"

using namespace noo;

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__ ((visibility("default"))) 
#endif

/*
	mpglib: test program for libmpg123, in the style of the legacy mpglib test program

	This is example code only sensible to be considered in the public domain.
	Initially written by Thomas Orgis.
*/

#include <mpg123.h>

/* unistd.h is not available under MSVC, 
 io.h defines the read and write functions */
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

#ifdef _WIN32
#include <fcntl.h>
#endif

#include <stdio.h>

#define INBUFF  (1 << 16) /**< input buffer size */
#define OUTBUFF (1 << 17) /**< output buffer size */

Uint8 *decode_mp3(SDL_IOStream *file, char *errmsg, SDL_AudioSpec *spec, Uint32 *sz)
{
	size_t size;
	unsigned char buf[INBUFF];  /* input buffer  */
	unsigned char out[OUTBUFF]; /* output buffer */
	ssize_t len;
	int ret;
	size_t in = 0, outc = 0;
	mpg123_handle *m;

	Uint8 *data = nullptr;
	Uint32 data_sz = 0;

#if MPG123_API_VERSION < 46
	// Newer versions of the library don't need that anymore, but it is safe
	// to have the no-op call present for compatibility with old versions.
	mpg123_init();
#endif
	m = mpg123_new(NULL, &ret);
	if(m == NULL)
	{
		snprintf(errmsg,1000,"Unable to create mpg123 handle: %s\n", mpg123_plain_strerror(ret));
		return nullptr;
	}
	mpg123_param(m, MPG123_VERBOSE, 2, 0); /* Brabble a bit about the parsing/decoding. */

	/* Now mpg123 is being prepared for feeding. The main loop will read chunks from stdin and feed them to mpg123;
	   then take decoded data as available to write to stdout. */
	mpg123_open_feed(m);
	if(m == NULL) return nullptr;

	fprintf(stderr, "Feed me some MPEG audio to stdin, I will decode to stdout.\n");
	while(1) /* Read and write until everything is through. */
	{
		len = SDL_ReadIO(file,buf,INBUFF);
		if(len <= 0)
		{
			fprintf(stderr, "input data end\n");
			break;
		}
		in += len;
		/* Feed input chunk and get first chunk of decoded audio. */
		ret = mpg123_decode(m,buf,len,out,OUTBUFF,&size);
		if(ret == MPG123_NEW_FORMAT)
		{
			long rate;
			int channels, enc;
			mpg123_getformat(m, &rate, &channels, &enc);
			spec->format = SDL_AUDIO_S16LE;
			spec->channels = channels;
			spec->freq = rate;
			fprintf(stderr, "New format: %li Hz, %i channels, encoding value %i\n", rate, channels, enc);
			data = new Uint8[mpg123_length(m)*spec->channels*2];
		}

		memcpy(data+data_sz, out, size);
		data_sz += size;

		outc += size;
		while(ret != MPG123_ERR && ret != MPG123_NEED_MORE)
		{ /* Get all decoded audio that is available now before feeding more input. */
			ret = mpg123_decode(m,NULL,0,out,OUTBUFF,&size);
			memcpy(data+data_sz, out, size);
			data_sz += size;
			outc += size;
		}
		if(ret == MPG123_ERR){ fprintf(stderr, "some error: %s", mpg123_strerror(m)); break; }
	}
	fprintf(stderr, "%lu bytes in, %lu bytes out\n", (unsigned long)in, (unsigned long)outc);

	/* Done decoding, now just clean up and leave. */
	mpg123_delete(m);

	*sz = outc;

	return data;
}

extern "C" {

void EXPORT booboo_start()
{
	audio::Sample::register_sample_loader("mp3", decode_mp3);
}

}
