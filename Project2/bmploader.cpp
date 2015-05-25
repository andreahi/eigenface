/*
* Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/


#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#   pragma warning( disable : 4996 ) // disable deprecated warning 
#endif

#pragma pack(1)

typedef struct
{
	short type;
	int size;
	short reserved1;
	short reserved2;
	int offset;
} BMPHeader;

typedef struct
{
	int size;
	int width;
	int height;
	short planes;
	short bitsPerPixel;
	unsigned compression;
	unsigned imageSize;
	int xPelsPerMeter;
	int yPelsPerMeter;
	int clrUsed;
	int clrImportant;
} BMPInfoHeader;



//Isolated definition
typedef struct
{
	unsigned char x, y, z, w;
} uchar4;

extern "C" void SaveBMPFile(uchar4 *dst, int *width, int *height, const char *name, const char *names)
{
	BMPHeader hdr;
	BMPInfoHeader infoHdr;
	int x, y;

	FILE *fd, *fdw;


	printf("Loading %s...\n", name);

	if (sizeof(uchar4) != 4)
	{
		printf("***Bad uchar4 size***\n");
		exit(EXIT_SUCCESS);
	}

	if (!(fd = fopen(name, "rb")))
	{
		printf("***BMP load error: file access denied***\n");
		exit(EXIT_SUCCESS);
	}
	if (!(fdw = fopen(names, "wb")))
	{
		printf("***BMP load error: file access denied***\n");
		exit(EXIT_SUCCESS);
	}



	fread(&hdr, sizeof(hdr), 1, fd);
	fwrite(&hdr, sizeof(hdr), 1, fdw);

	if (hdr.type != 0x4D42)
	{
		printf("***BMP load error: bad file format***\n");
		exit(EXIT_SUCCESS);
	}

	fread(&infoHdr, sizeof(infoHdr), 1, fd);
	fwrite(&infoHdr, sizeof(infoHdr), 1, fdw);

	if (infoHdr.bitsPerPixel != 24)
	{
		printf("***BMP load error: invalid color depth***\n");
		exit(EXIT_SUCCESS);
	}

	if (infoHdr.compression)
	{
		printf("***BMP load error: compressed image***\n");
		exit(EXIT_SUCCESS);
	}

	*width = infoHdr.width;
	*height = infoHdr.height;

	printf("BMP width: %u\n", infoHdr.width);
	printf("BMP height: %u\n", infoHdr.height);

	fseek(fd, hdr.offset - sizeof(hdr) - sizeof(infoHdr), SEEK_CUR);
	fseek(fdw, hdr.offset - sizeof(hdr) - sizeof(infoHdr), SEEK_CUR);

	for (y = 0; y < infoHdr.height; y++)
	{
		for (x = 0; x < infoHdr.width; x++)
		{
			//(*dst)[(y * infoHdr.width + x)].z = fgetc(fd);
			//(*dst)[(y * infoHdr.width + x)].y = fgetc(fd);
			//(*dst)[(y * infoHdr.width + x)].x = fgetc(fd);
			fgetc(fd);
				fgetc(fd);
				fgetc(fd);
	//		fputc((fgetc(fd)), fdw);
	//		fputc((fgetc(fd)), fdw);
	//		fputc((fgetc(fd)), fdw);
			fputc(dst[(y * infoHdr.width + x)].z, fdw);
			fputc(dst[(y * infoHdr.width + x)].y, fdw);
			fputc(dst[(y * infoHdr.width + x)].x, fdw);
		//	;
		//	(fgetc(fd));
		//	(fgetc(fd));
		}

		for (x = 0; x < (4 - (3 * infoHdr.width) % 4) % 4; x++)
			fputc(fgetc(fd), fdw);
	}


	if (ferror(fd))
	{
		printf("***Unknown BMP load error.***\n");
//		free(*dst);
		exit(EXIT_SUCCESS);
	}
	else
		printf("BMP file loaded successfully!\n");

	fclose(fd);
	fclose(fdw);
}

extern "C" void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name)
{
	BMPHeader hdr;
	BMPInfoHeader infoHdr;
	int x, y;

	FILE *fd;


	printf("Loading %s...\n", name);

	if (sizeof(uchar4) != 4)
	{
		printf("***Bad uchar4 size***\n");
		exit(EXIT_SUCCESS);
	}

	if (!(fd = fopen(name, "rb")))
	{
		printf("***BMP load error: file access denied***\n");
		exit(EXIT_SUCCESS);
	}

	fread(&hdr, sizeof(hdr), 1, fd);

	if (hdr.type != 0x4D42)
	{
		printf("***BMP load error: bad file format***\n");
		exit(EXIT_SUCCESS);
	}

	fread(&infoHdr, sizeof(infoHdr), 1, fd);

	if (infoHdr.bitsPerPixel != 24)
	{
		printf("***BMP load error: invalid color depth***\n");
		exit(EXIT_SUCCESS);
	}

	if (infoHdr.compression)
	{
		printf("***BMP load error: compressed image***\n");
		exit(EXIT_SUCCESS);
	}

	*width = infoHdr.width;
	*height = infoHdr.height;
	*dst = (uchar4 *)malloc(*width * *height * 4);

	printf("BMP width: %u\n", infoHdr.width);
	printf("BMP height: %u\n", infoHdr.height);

	fseek(fd, hdr.offset - sizeof(hdr) - sizeof(infoHdr), SEEK_CUR);

	for (y = 0; y < infoHdr.height; y++)
	{
		for (x = 0; x < infoHdr.width; x++)
		{
			(*dst)[(y * infoHdr.width + x)].z = fgetc(fd);
			(*dst)[(y * infoHdr.width + x)].y = fgetc(fd);
			(*dst)[(y * infoHdr.width + x)].x = fgetc(fd);
		}

		for (x = 0; x < (4 - (3 * infoHdr.width) % 4) % 4; x++)
			fgetc(fd);
	}


	if (ferror(fd))
	{
		printf("***Unknown BMP load error.***\n");
		free(*dst);
		exit(EXIT_SUCCESS);
	}
	else
		printf("BMP file loaded successfully!\n");

	fclose(fd);
}
