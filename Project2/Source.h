

#ifndef IMAGE_DENOISING_H
#define IMAGE_DENOISING_H
typedef struct
{
	unsigned char x, y, z, w;
} uchar4;

extern "C" void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name);
extern "C" void SaveBMPFile(uchar4 *dst, int *width, int *height, const char *name, const char *names);
#endif
