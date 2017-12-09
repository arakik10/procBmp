/*
  runProcBmp.c
  procBmp.c のAPIを用いてbitmapデータを処理するコード
*/
#include <stdio.h>
#include "procBmp.h"
#include "procBmp.c"

BMPData *copyBmp( BMPData *bmp0 )
{
  BMPData *bmp1;
  bmp1= allocBmp( bmp0->xMax, bmp0->yMax );
  if ( bmp1 ) {
    int x, y;
    for ( y= 0; y < bmp1->yMax; y++ ) {
    for ( x= 0; x < bmp1->xMax; x++ ) {
      bmp1->b[y][x]= bmp0->b[y][x];
      bmp1->g[y][x]= bmp0->g[y][x];
      bmp1->r[y][x]= bmp0->r[y][x];
    }
    }
    return bmp1;
  }
  return 0;
}

//---------------------------------------------
// エントリポイント
//---------------------------------------------
char *fileInput=  (char*) "lena_std.bmp";
char *fileOutput= (char*) "lena_out.bmp";

int main ( void )
{
  BMPData *bmp0= readBmp( fileInput );
  if ( bmp0 )
  {
    BMPData *bmp1= copyBmp( bmp0 );
    if ( bmp1 )
    {
      writeBmp( fileOutput, bmp1 );
      if ( bmp1 != bmp0 ) freeBmp( bmp1 );
    }
    else {
      printf("main(): bmp1ができてないお！！！");
    }
    freeBmp( bmp0 );
  }
  return 0;
}
