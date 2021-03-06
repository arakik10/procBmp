/*
  procBmp.c
  Windowsの24bitフルカラービットマップ操作用C言語ライブラリ
*/
#ifndef _PROCBMP_C
#define _PROCBMP_C
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "procBmp.h"

// bitmapファイル用のデータ領域をメモリ上に作成する
// 使い方:
//   bmp = allocBmp ( xMax, yMax )
//   |                |     |
//   |                | int型, y-方向の画素数
//   |                int型, x-方向の画素数
//   BMPData* 型, bitmapファイル用のデータ領域
//
BMPData *allocBmp ( int xMax ,int yMax )
{
  BMPData *bmp;
  bmp= (BMPData*) calloc(1,sizeof(BMPData));
  if ( bmp )
  {
    int padding= setPadding(xMax);
    int numTotalPixelPadding=
      ( xMax*3 + padding )*yMax;

    // x, y 方向のピクセルサイズのセット
    bmp->xMax= xMax;
    bmp->yMax= yMax;

    // BMPデータのパディングサイズのセット
    bmp->padding= padding;

    // 画素データの保存領域の確保
    bmp= allocPixelPointer(bmp);
    if ( !bmp ) goto case_allocError;

    // 画像情報(BitmapFileHeader構造体)の格納
    bmp->f.Type=      0x4D42;
    bmp->f.Size=      54 + numTotalPixelPadding;
    bmp->f.Reserved1= 0;
    bmp->f.Reserved2= 0;
    bmp->f.OffBits=   54;

    // 画像情報(BitmapInfoHeader構造体)の格納
    bmp->i.Size=         40;
    bmp->i.Width=        bmp->xMax;
    bmp->i.Height=       bmp->yMax;
    bmp->i.Planes=       1;
    bmp->i.BitCount=     24;
    bmp->i.Compression=  0;
    bmp->i.SizeImage=    numTotalPixelPadding;
    bmp->i.XPixPerMeter= 7560;
    bmp->i.YPixPerMeter= 7560;
    bmp->i.ClrUsed=      0;
    bmp->i.ClrImportant= 0;

    return bmp;
  }
  case_allocError:
    printf("allocBmp():メモリが取れないお\n");
    return 0;
}

// allocBmp(), readBmp() の補助のための bitmap の
// データ保存領域と画素データアクセス用変数のメモリ
// アロケーション用関数。
// 使い方:
//   bmp = allocPixelPointer ( bmp )
//   |                         |
//   |               BMPData* 型, 画像データ
//   BMPData* 型, メモリ確保に失敗すると 0 を返す
//        メモリ確保に成功したら元のアドレスを返す
//
BMPData *allocPixelPointer ( BMPData *bmp )
{
  // 画素データアクセス用変数のメモリアロケーション
  size_t sizePtrYMax= sizeof(int*)*(bmp->yMax);
  bmp->r= (int**) malloc(sizePtrYMax);
  bmp->g= (int**) malloc(sizePtrYMax);
  bmp->b= (int**) malloc(sizePtrYMax);

  // 画素データの保存領域のメモリアロケーション
  bmp->pxl= (int*) malloc(
    sizePtrYMax*(bmp->xMax)*3);

  // メモリアロケーションに失敗したときの処理
  if(!(bmp->pxl && bmp->r && bmp->g && bmp->b))
    goto case_allocError;

  //画素データの保存領域とアクセス用変数の対応付け
  //注意：データの格納の順番について
  //bmp->pxlは3×xMax×yMax個のメモリがあり
  //[r][r]...[r][g][g]...[g][b][b]...[b]
  // |           |           |
  // |           |           bmp->bで読み書き
  // |           bmp->gで読み書き
  // bmp->rで読み書き
  //
  { int i ,offset= (bmp->xMax)*(bmp->yMax);
    for ( i=0; i < bmp->yMax; i++ ) {
      int *addr_i_0= bmp->pxl +i*(bmp->xMax);
      bmp->r[i]= addr_i_0;
      bmp->g[i]= addr_i_0 + offset;
      bmp->b[i]= addr_i_0 + offset*2;
    }
  }

  return bmp;

  case_allocError:
    freeBmp( bmp );
    printf("allocPixelPointer():"
      " メモリが取れなかったお\n");
    return 0;
}

// BMPData*構造体のメモリ解放関数
void freeBmp ( BMPData *bmp )
{
  if ( bmp ) {
    if ( bmp->pxl ) free( bmp->pxl );
    if ( bmp->r ) free( bmp->r );
    if ( bmp->g ) free( bmp->g );
    if ( bmp->b ) free( bmp->b );
    free( bmp );
  }
}

// bitmap ファイルデータを読み込む関数
// 使い方:
//   bmp = readBmp ( fileBmp )
//   |               |
//   |               char* 型, ファイル名
//   BMPData* 型, fileBmp の画像やサイズ等のデータ
//     読み込みエラー等があると 0 を返す
//
BMPData *readBmp ( char *fileBmp )
{
  FILE *fp= fopen(fileBmp,"rb");
  if ( fp )
  {
    BMPData *bmp;
    printf("\nreadBmp(): %sを読むお\n",fileBmp);

    bmp= (BMPData*) calloc(1,sizeof(BMPData));
    if ( !bmp ) goto case_allocError;

    bmp->filename= fileBmp;

    // 画像情報の読み出し
    if ( readBmpHeader(fp,bmp) )
      goto case_readError;

    // x, y 方向のピクセルサイズのセット
    bmp->xMax= bmp->i.Width;
    bmp->yMax= bmp->i.Height;

    // BMPデータのパディングサイズのセット
    bmp->padding= setPadding(bmp->xMax);

    // 画素データの保存領域の確保
    bmp= allocPixelPointer(bmp);
    if ( !bmp ) goto case_allocError;

    // 画素データのファイルからの読み込み
    { int y,x;
      for ( y=0; y < bmp->yMax; y++ ) {
        // 画素の横の列のRGBの読み出し
        for ( x=0; x < bmp->xMax; x++ ) {
          if( (bmp->b[y][x]= fgetc(fp)) != EOF )
          if( (bmp->g[y][x]= fgetc(fp)) != EOF )
          if( (bmp->r[y][x]= fgetc(fp)) != EOF )
            continue;
          goto case_readError;
        }
        // Bitmap特有のデータの余白の読み飛ばし
        for ( x=0; x < bmp->padding; x++ ) {
          if ( fgetc(fp) == EOF )
            goto case_readError;
        }
      }
    }
    printf("readBmp(): (x,y)=(%d,%d) ピクセル\n"
      ,bmp->xMax ,bmp->yMax);
    fclose(fp);
    return bmp;

    case_allocError:
      printf("readBmp(): メモリが取れないお\n");
      fclose(fp);
      return 0;

    case_readError:
      printf("readBmp(): ファイル'%s'が"
        "読めないお\n",fileBmp);
      freeBmp( bmp );
      fclose(fp);
      return 0;
  }
  case_openError:
    printf("readBmp():"
      " ファイル'%s'が無いお\n",fileBmp);
    return 0;
}

// bitmap のヘッダデータをファイルから読み込む関数
// 使い方:
//   err = readBmpHeader ( fp, bmp )
//   |                     |   |
//   |                     |   BMPData*型
//   |         bitmapデータファイルのポインタ
//   int型 読み込み成功すると 0 を返す
//         読み込みエラーがあると 1 を返す
//
int readBmpHeader ( FILE *fp ,BMPData *bmp )
{
  if(fread((void*)&bmp->f.Type     ,2,1,fp))
  if(fread((void*)&bmp->f.Size     ,4,1,fp))
  if(fread((void*)&bmp->f.Reserved1,2,1,fp))
  if(fread((void*)&bmp->f.Reserved2,2,1,fp))
  if(fread((void*)&bmp->f.OffBits  ,4,1,fp))
  if(fread((void*)&bmp->i.Size        ,4,1,fp))
  if(fread((void*)&bmp->i.Width       ,4,1,fp))
  if(fread((void*)&bmp->i.Height      ,4,1,fp))
  if(fread((void*)&bmp->i.Planes      ,2,1,fp))
  if(fread((void*)&bmp->i.BitCount    ,2,1,fp))
  if(fread((void*)&bmp->i.Compression ,4,1,fp))
  if(fread((void*)&bmp->i.SizeImage   ,4,1,fp))
  if(fread((void*)&bmp->i.XPixPerMeter,4,1,fp))
  if(fread((void*)&bmp->i.YPixPerMeter,4,1,fp))
  if(fread((void*)&bmp->i.ClrUsed     ,4,1,fp))
  if(fread((void*)&bmp->i.ClrImportant,4,1,fp))
    return 0;
  printf("readBmpHeader(): データが読めないお");
  return 1;
}

// bitmap特有の行末の空白サイズの計算
int setPadding ( int xMax )
{
  return xMax % 4;
}

// bitmap データをファイルに書き込む関数
// 使い方:
//   err = writeBmp ( fileBmp , bmp )
//   |                |         |
//   |                | BMPData*型, 画像データ
//   |            char* 型, 書き込みファイル名
//   int 型, 書き込み成功で 0, 失敗は 0 以外.
//
int writeBmp ( char *fileBmp ,BMPData *bmp )
{
  FILE *fp= fopen(fileBmp,"wb");
  if ( !fp ) goto case_openError;
  if ( !bmp ) goto case_bmpError;

  // 画像情報の書き込み
  if ( writeBmpHeader(fp,bmp) )
    goto case_writeError;

  // 画素データのファイルの書き込み
  { int y,x;
    for ( y=0; y < bmp->yMax; y++ ) {
      for ( x=0; x < bmp->xMax; x++ ) {
        if ( fputc( bmp->b[y][x] ,fp ) != EOF )
        if ( fputc( bmp->g[y][x] ,fp ) != EOF )
        if ( fputc( bmp->r[y][x] ,fp ) != EOF )
          continue;
        goto case_writeError;
      }
      for ( x=0; x < bmp->padding; x++ ) {
        if ( fputc(0,fp) == EOF )
          goto case_writeError;
      }
    }
  }
  fclose(fp);
  printf("writeBmp(): %sに書き込んだお\n"
    ,fileBmp);
  printf("writeBmp():  (x,y)=(%d,%d)"
    "ピクセルだお\n",bmp->xMax,bmp->yMax);
  return 0;

  case_bmpError:
    printf("writeBmp(): データがないお\n");
    return 1;

  case_openError:
    printf("writeBmp():"
      " ファイルがopenできないお\n");
    return 2;

  case_writeError:
    printf("writeBmp():"
      " データが出力できないお\n");
    return 3;
}

// bitmap のヘッダデータをファイルに書き込む関数
// 使い方:
//   err = writeBmpHeader ( fp, bmp )
//   |                      |   |
//   |                      |   BMPData*型
//   |          bitmapデータファイルのポインタ
//   int型 書き込み成功すると 0 を返す
//         書き込みエラーがあると 1 を返す
//
int writeBmpHeader ( FILE *fp ,BMPData *bmp )
{
  if(fwrite((void*)&bmp->f.Type     ,2,1,fp))
  if(fwrite((void*)&bmp->f.Size     ,4,1,fp))
  if(fwrite((void*)&bmp->f.Reserved1,2,1,fp))
  if(fwrite((void*)&bmp->f.Reserved2,2,1,fp))
  if(fwrite((void*)&bmp->f.OffBits  ,4,1,fp))
  if(fwrite((void*)&bmp->i.Size        ,4,1,fp))
  if(fwrite((void*)&bmp->i.Width       ,4,1,fp))
  if(fwrite((void*)&bmp->i.Height      ,4,1,fp))
  if(fwrite((void*)&bmp->i.Planes      ,2,1,fp))
  if(fwrite((void*)&bmp->i.BitCount    ,2,1,fp))
  if(fwrite((void*)&bmp->i.Compression ,4,1,fp))
  if(fwrite((void*)&bmp->i.SizeImage   ,4,1,fp))
  if(fwrite((void*)&bmp->i.XPixPerMeter,4,1,fp))
  if(fwrite((void*)&bmp->i.YPixPerMeter,4,1,fp))
  if(fwrite((void*)&bmp->i.ClrUsed     ,4,1,fp))
  if(fwrite((void*)&bmp->i.ClrImportant,4,1,fp))
    return 0;
  printf("writeBmpHeader(): ヘッダ情報書けないお");
  return 1;
}

#endif // _PROCBMP_C
