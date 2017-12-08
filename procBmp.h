/*
  procBmp.h
  Windowsの24bitフルカラービットマップ操作用C言語ライブラリ
*/
#ifndef _PROCBMP_H
#define _PROCBMP_H
#include <stdio.h>

struct BitmapFileHeader {
  unsigned short Type;
  unsigned long  Size;
  unsigned short Reserved1;
  unsigned short Reserved2;
  unsigned long  OffBits;
};

struct BitmapInfoHeader {
  unsigned long  Size;
  long           Width;
  long           Height;
  unsigned short Planes;
  unsigned short BitCount;
  unsigned long  Compression;
  unsigned long  SizeImage;
  long           XPixPerMeter;
  long           YPixPerMeter;
  unsigned long  ClrUsed;
  unsigned long  ClrImportant;
};

struct bitmap_data {
  char *filename; // 画像ファイル名
  int xMax // x-方向の画素数
     ,yMax // y-方向の画素数
     ,*pxl // R/G/Bデータの保存領域
     ,**r  // Rデータにアクセスするためのポインタ
     ,**g  // Gデータにアクセスするためのポインタ
     ,**b  // Bデータにアクセスするためのポインタ
     ,padding // Bitmap 特有のメモリスキップ数
  ;
  struct BitmapFileHeader f;
  struct BitmapInfoHeader i;
};

typedef struct bitmap_data BMPData;

BMPData* allocBmp(int xMax, int yMax);
BMPData* allocPixelPointer(BMPData *bmp);
void     freeBmp(BMPData *bmp);
BMPData* readBmp(char *fileBmp);
int      readBmpHeader(FILE *fp, BMPData *bmp);
int      setPadding(int xMax);
int      writeBmp(char *fileBmp, BMPData *bmp);
int      writeBmpHeader(FILE *fp, BMPData *bmp);

#endif // _PROCBMP_H
