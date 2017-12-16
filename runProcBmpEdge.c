/*
  runProcBmp.c
  procBmp.c のAPIを用いてbitmapデータを処理するコード
*/
#include <stdio.h>
#include "procBmp.h"
#include "procBmp.c"

BMPData *edgeBmp( BMPData *bmpIn )
{
  BMPData *bmpOut= allocBmp( bmpIn->xMax, bmpIn->yMax );
  if ( bmpOut ) {
    int x,y;
    for ( y=0; y < bmpOut->yMax; y++ ) {
    for ( x=0; x < bmpOut->xMax; x++ ) {
      bmpIn->b[y][x]= bmpIn->b[y][x]*.114
                    + bmpIn->g[y][x]*.587
                    + bmpIn->r[y][x]*.299;
    }
    }
    for ( y=1; y < bmpOut->yMax-1; y++ ) {
    for ( x=1; x < bmpOut->xMax-1; x++ ) {
      int i, j, data= -9*bmpIn->b[y][x];
      for ( j= -1; j <= 1; j++ ) {
      for ( i= -1; i <= 1; i++ ) {
        data+= bmpIn->b[y+j][x+i];
      }
      }
      if ( data > 255 ) data= 255;
      if ( data <   0 ) data=   0;
      bmpOut->r[y][x]= bmpOut->g[y][x]= bmpOut->b[y][x]= 255-data;
    }
    }
    return bmpOut;
  }
  return 0;
}

//---------------------------------------------
// エントリポイント
//---------------------------------------------
char *fileInput=  (char*) "lena_std.bmp";
char *fileOutput= (char*) "lena_out.bmp";

int main ( int argc, char *argv[] )
{
  if ( argc > 1 ) fileInput= argv[1];
  BMPData *bmpIn= readBmp( fileInput );
  if ( bmpIn )
  {
    BMPData *bmpOut= edgeBmp( bmpIn );
    if ( bmpOut )
    {
      if ( argc > 2 ) fileOutput= argv[2];
      writeBmp( fileOutput, bmpOut );
      if ( bmpOut != bmpIn ) freeBmp( bmpOut );
    }
    else {
      printf("main(): bmpOutができてないお！！！");
    }
    freeBmp( bmpIn );
  }
  return 0;
}
