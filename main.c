#include <spng.h>
#include <stdio.h>

int main(int argc, char **argv) {
	FILE *png;
	FILE *png2;
	int ret = 0;
	spng_ctx *ctx = NULL;
	spng_ctx *ctx2 = NULL;
	unsigned char *image = NULL;
	unsigned char *image2 = NULL;

	if(argc < 3)
	{
		printf("no input files\n");
		goto error;
	}

	png = fopen(argv[1], "rb");

	if(png == NULL)
	{
		printf("error opening input file %s\n", argv[1]);
		goto error;
	}
	png2 = fopen(argv[2], "rb");

	if(png2 == NULL)
	{
		printf("error opening input file %s\n", argv[1]);
		goto error;
	}

	ctx = spng_ctx_new(0);

	if(ctx == NULL)
	{
		printf("spng_ctx_new() failed\n");
		goto error;
	}

	/* Ignore and don't calculate chunk CRC's */
	spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);

	/* Set memory usage limits for storing standard and unknown chunks,
	   this is important when reading untrusted files! */
	size_t limit = 1024 * 1024 * 64;
	spng_set_chunk_limits(ctx, limit, limit);

	/* Set source PNG */
	spng_set_png_file(ctx, png); /* or _buffer(), _stream() */

	ctx2 = spng_ctx_new(0);

	if(ctx2 == NULL)
	{
		printf("spng_ctx_new() failed\n");
		goto error;
	}

	/* Ignore and don't calculate chunk CRC's */
	spng_set_crc_action(ctx2, SPNG_CRC_USE, SPNG_CRC_USE);

	/* Set memory usage limits for storing standard and unknown chunks,
	   this is important when reading untrusted files! */
	spng_set_chunk_limits(ctx2, limit, limit);

	/* Set source PNG */
	spng_set_png_file(ctx2, png2); /* or _buffer(), _stream() */

	struct spng_ihdr ihdr;
	ret = spng_get_ihdr(ctx, &ihdr);

	if(ret)
	{
		printf("spng_get_ihdr() error: %s\n", spng_strerror(ret));
		goto error;
	}

	size_t image_size;
	/* Output format, does not depend on source PNG format except for
	   SPNG_FMT_PNG, which is the PNG's format in host-endian or
	   big-endian for SPNG_FMT_RAW.
	   Note that for these two formats <8-bit images are left byte-packed */
	int fmt = SPNG_FMT_PNG;

	/* With SPNG_FMT_PNG indexed color images are output as palette indices,
	   pick another format to expand them. */
	if(ihdr.color_type == SPNG_COLOR_TYPE_INDEXED) fmt = SPNG_FMT_RGB8;

	ret = spng_decoded_image_size(ctx, fmt, &image_size);

	if(ret) goto error;

	int fmt2 = SPNG_FMT_PNG;

	/* With SPNG_FMT_PNG indexed color images are output as palette indices,
	   pick another format to expand them. */
	if(ihdr.color_type == SPNG_COLOR_TYPE_INDEXED) fmt2 = SPNG_FMT_RGB8;

	image = malloc(image_size);
	unsigned char* diff = malloc(image_size);

	if(image == NULL) goto error;
	if (image2 == NULL) goto error;
	if (diff == NULL) goto error;

	/* Decode the image in one go */
	ret = spng_decode_image(ctx, image, image_size, SPNG_FMT_RGBA8, 0);
	if(ret)
	{
		printf("spng_decode_image() error: %s\n", spng_strerror(ret));
		goto error;
	}

	ret = spng_decode_image(ctx2, image2, image_size, SPNG_FMT_RGBA8, 0);
	if(ret)
	{
		printf("image2 spng_decode_image() error: %s\n", spng_strerror(ret));
		goto error;
	}
	for (int i = 0; i < image_size; i += 4) {
		diff[i] = image[i] - image2[i];
		diff[i+1] = image[i+1] - image2[i+1];
		diff[i+2] = image[i+2] - image2[i+2];
		diff[i+3] = image[i+3];
	}
	spng_ctx *en_ctx = NULL;
	en_ctx = spng_ctx_new(SPNG_CTX_ENCODER);
	FILE* out = fopen("out.png", "w");
	spng_set_png_file(en_ctx, out);
	spng_set_ihdr(en_ctx, &ihdr);


	ret = spng_encode_image(en_ctx, diff, image_size, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
	if(ret)
	{
		printf("encode spng_decode_image() error: %s\n", spng_strerror(ret));
		goto error;
	}

error:

	spng_ctx_free(ctx);
	free(image);

	return ret;
}
