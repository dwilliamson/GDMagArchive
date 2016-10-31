// INCLUDES //////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <conio.h>

#include "iff.hpp"
#include "pixbuf.hpp"

// MEMBER FUNCTIONS //////////////////////////////////////////////////////////

lbm::lbm()
{
  
  width = 320;
  height = 240;
  bufSize = 0;
  
  lbmBuf = NULL;
  palette = NULL;
  printf("Default Constructor lbm\n");
  
  printf("CLOSING: Default Constructor lbm\n");
}

//////////////////////////////////////////////////////////////////////////////

long lbm::intelByteOrderLong( long hostBytesLong )
{
  return( (( hostBytesLong & 0xff000000L) >> 24) 
        + (( hostBytesLong & 0x00ff0000L) >> 8 )
        + (( hostBytesLong & 0x0000ff00L) << 8 )
        + (( hostBytesLong & 0x000000ffL) << 24) );
} 

//////////////////////////////////////////////////////////////////////////////

int lbm::intelByteOrder( int hostBytes )
{
  return( (( hostBytes & 0xff00) >> 8) 
        | (( hostBytes & 0x00ff) << 8) );
}

//////////////////////////////////////////////////////////////////////////////

void lbm::unPacker( char *filename)
{
  short ilbmByteCount;
  unsigned char *scanlineBuf;
  FILE *fp;
  
  form_chunk fchunk;
  ChunkHeader chunk;
  BitMapHeader ilbmHeader;
  signed char ilbmByte;
  int ilbmByteIndex;
  int ilbmHeight, ilbmRow;    
  unsigned int offset;
  
  offset = 0;
  scanlineBuf = NULL;
  
  if ((fp = fopen(filename,"r+b"))==NULL)
  {
    printf("File [%s] NOT FOUND!\n", filename);
     exit(1);
  }
    
  printf("Reading ILBM Chunks\n");
 
  fread(&fchunk,1,sizeof(form_chunk),fp);
  
  // loop through the chunks  
  while(1)
  {
    
    fread(&chunk,1,sizeof(ChunkHeader),fp);
    chunk.ckSize = intelByteOrderLong( chunk.ckSize );
    if (chunk.ckSize & 1)
      ++chunk.ckSize;
      
    if (!memcmp(chunk.ckID,CMAP, BYTE_LEN) )
    {
      printf("CMAP Chunk\n");
      freePalette();
      palette = new char[MAX_TRIPLET_SIZE];
      assert(palette != 0);
      fread(palette, 1, 768, fp);
      
      continue;
    } // if ID_CMAP
    
    if(!memcmp(chunk.ckID, BMHD, BYTE_LEN) )
    {
      printf("BMHD Chunk\n");
      fread(&ilbmHeader,1,sizeof(BitMapHeader), fp);
    
      ilbmHeader.w = intelByteOrder(ilbmHeader.w);
      ilbmHeader.h = intelByteOrder(ilbmHeader.h);

      if (ilbmHeader.w > width)
        ilbmByteCount = width;
      else
        ilbmByteCount = ilbmHeader.w;
        
      if (ilbmHeader.h > height)
        ilbmHeight = height;
      else
        ilbmHeight = ilbmHeader.h;
         
      if (scanlineBuf != NULL)
        delete[] scanlineBuf;
  
      printf("new scanlineBuf\n");      
      scanlineBuf = new unsigned char[ilbmHeader.w+1];
      assert( scanlineBuf != 0);
  
      if (lbmBuf != NULL)
        delete [] lbmBuf;
    
      lbmBuf = new unsigned char[ilbmHeader.w * ilbmHeader.h];
      assert(lbmBuf != 0);
      continue;
    }
    
    if (!memcmp(chunk.ckID, BODY, BYTE_LEN) )
    {
      unsigned char *scanlinePtr;
      printf("BODY Chunk\n");
      for (ilbmRow=0; ilbmRow<ilbmHeight; ilbmRow++)
      {
            scanlinePtr = scanlineBuf;
            ilbmByteIndex = ilbmHeader.w;
            while(ilbmByteIndex)
            {
              if (ilbmHeader.compression)
              {
                ilbmByte = fgetc(fp);
                if (ilbmByte == 128)
                  continue;
                if (ilbmByte > 0)
                {
                  int len;
                  len = ilbmByte + 1;
                  ilbmByteIndex -= len;
                  if (!(fread(scanlinePtr,len,1,fp)))
                  {
                        fclose(fp);
                        return;
                  }
                  scanlinePtr += len;
                }
                else
                {
                  int count;
                  count = -ilbmByte;
                  count++;
                  ilbmByteIndex -= count;
                  ilbmByte = fgetc(fp);
                  while (--count >= 0)
                        *scanlinePtr++ = ilbmByte;
                }
              } // if compression
              else
              {
                fread(scanlinePtr, ilbmByteIndex, 1, fp);  // throw on the planes
                ilbmByteIndex = 0;
              }
            } // while ilbmByteIndex
            memcpy( lbmBuf + offset, scanlineBuf, ilbmByteCount);
           offset += ilbmByteCount;
      }// for ilbmRow
      break;
    }  // if ID_BODY
    fseek(fp, chunk.ckSize, SEEK_CUR);
  }
  fclose(fp);
  delete[] scanlineBuf;
  printf("Finished reading IFF\n");
  return;
}

//////////////////////////////////////////////////////////////////////////////

void lbm::unPacker( char *filename, pixelBuffer& p)
{
  short ilbmByteCount;
  unsigned char *scanlineBuf;
  FILE *fp;
  
  form_chunk fchunk;
  ChunkHeader chunk;
  BitMapHeader ilbmHeader;
  signed char ilbmByte;
  int ilbmByteIndex;
  int ilbmHeight, ilbmRow;    
  unsigned int offset;
  
  offset = 0;
  scanlineBuf = NULL;
  
  if ((fp = fopen(filename,"r+b"))==NULL)
  {
    printf("File [%s] NOT FOUND!\n", filename);
     exit(1);
  }
  
  
  
  printf("Reading ILBM Chunks\n");
 
  fread(&fchunk,1,sizeof(form_chunk),fp);
  
  // loop through the chunks  
  while(1)
  {
    fread(&chunk,1,sizeof(ChunkHeader),fp);
    chunk.ckSize = intelByteOrderLong( chunk.ckSize );
    if (chunk.ckSize & 1)
      ++chunk.ckSize;
      
    if (!memcmp(chunk.ckID,CMAP, BYTE_LEN) )
    {
      
      printf("CMAP Chunk\n");
          freePalette();
      palette = new char[MAX_TRIPLET_SIZE];
      assert(palette != 0);
      fread(palette, 1, 768, fp);
      
      continue;
    } // if ID_CMAP
    
    if(!memcmp(chunk.ckID, BMHD, BYTE_LEN) )
    {
      printf("BMHD Chunk\n");
      fread(&ilbmHeader,1,sizeof(BitMapHeader), fp);
    
      ilbmHeader.w = intelByteOrder(ilbmHeader.w);
      ilbmHeader.h = intelByteOrder(ilbmHeader.h);

      if (ilbmHeader.w > width)
        ilbmByteCount = width;
      else
        ilbmByteCount = ilbmHeader.w;
        
      if (ilbmHeader.h > height)
        ilbmHeight = height;
      else
        ilbmHeight = ilbmHeader.h;
         
      if (scanlineBuf != NULL)
        delete[] scanlineBuf;
  
      printf("new scanlineBuf\n");      
      scanlineBuf = new unsigned char[ilbmHeader.w+1];
      assert( scanlineBuf != 0);
      continue;
    }
    
    if (!memcmp(chunk.ckID, BODY, BYTE_LEN) )
    {
      unsigned char *scanlinePtr;
      printf("BODY Chunk\n");
      for (ilbmRow=0; ilbmRow<ilbmHeight; ilbmRow++)
      {
            scanlinePtr = scanlineBuf;
            ilbmByteIndex = ilbmHeader.w;
            while(ilbmByteIndex)
            {
              if (ilbmHeader.compression)
              {
                ilbmByte = fgetc(fp);
                if (ilbmByte == 128)
                  continue;
                if (ilbmByte > 0)
                {
                  int len;
                  len = ilbmByte + 1;
                  ilbmByteIndex -= len;
                  if (!(fread(scanlinePtr,len,1,fp)))
                  {
                        fclose(fp);
                        return;
                  }
                  scanlinePtr += len;
                }
                else
                {
                  int count;
                  count = -ilbmByte;
                  count++;
                  ilbmByteIndex -= count;
                  ilbmByte = fgetc(fp);
                  while (--count >= 0)
                        *scanlinePtr++ = ilbmByte;
                }
              } // if compression
              else
              {
                fread(scanlinePtr, ilbmByteIndex, 1, fp);  // throw on the planes
                ilbmByteIndex = 0;
              }
            } // while ilbmByteIndex
            memcpy( p.pixBuf + offset, scanlineBuf, ilbmByteCount);
           offset += ilbmByteCount;
      }// for ilbmRow
      break;
    }  // if ID_BODY
    fseek(fp, chunk.ckSize, SEEK_CUR);
  }
  fclose(fp);
  delete[] scanlineBuf;
  printf("Finished reading IFF\n");
  return;
}

//////////////////////////////////////////////////////////////////////////////

void lbm::freePalette()
{
  if (palette != NULL)
    delete[] palette;
}   

//////////////////////////////////////////////////////////////////////////////

void lbm::freeBuf()
{
  if (lbmBuf != NULL)
    delete[] lbmBuf;
}

//////////////////////////////////////////////////////////////////////////////

void lbm::convertPal()
{
    unsigned int i;

    
    for(i=0;i<768;i++)
    {
        palette[i] = palette[i] >> 2;
    }
}   


//////////////////////////////////////////////////////////////////////////////

lbm::~lbm()
{  
  freeBuf();
  freePalette();
  printf("lbm default destructor completetion\n");
}            
        
//////////////////////////////////////////////////////////////////////////////
