#include <stdlib.h>
#include <assert.h>
#include <windows.h>

typedef signed char int8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef unsigned int uint;

// PSD file header, everything is bigendian
#pragma pack(1)
typedef struct {
    uint32 Signature;
    uint16 Version;
    uint8  Reserved[6];
    uint16 Channels;
    uint32 Rows;
    uint32 Columns;
    uint16 Depth;
    uint16 Mode;
} PSDFile;

#define SWAP16(w) ((uint16)((((w)>>8)&0xFF)|(((w)<<8)&0xFF00)))
#define SWAP32(w) ((uint32)((((w)>>24)&0xFF)|(((w)<<24)&0xFF000000)|(((w)<<8)&0xFF0000)|(((w)>>8)&0xFF00)))

// followed by chunks (uint32 Length as first field):
// Color mode data section
// Image resources section
// Layer and Mask information section
// uint16 Compression = 0 for raw, = 1 for RLE
// Image data section, one channel at a time
// then bits, scanline order, no pad

typedef struct
{
   int w;
   int h;
   uint32 data[1];
} PSDbitmap;

PSDbitmap *loadPSD( char *FileName)
{
    PSDbitmap *res = NULL;
    HANDLE File;
    HANDLE FileMap;
    void *FileData;
    uint8 *FileBytes;

    assert(FileName);
    File = CreateFile(FileName,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if (File == INVALID_HANDLE_VALUE) return res;
    FileMap = CreateFileMapping(File,0,PAGE_READONLY,0,0,0);
    if (FileMap == INVALID_HANDLE_VALUE) { CloseHandle(File); return NULL; }
    FileData = MapViewOfFile(FileMap,FILE_MAP_READ,0,0,0);
    if (FileData == NULL) { CloseHandle(FileMap); CloseHandle(File); return NULL; }
    FileBytes = (uint8 *)FileData;
    if(FileBytes[0] == '8' && FileBytes[1] == 'B' &&
        FileBytes[2] == 'P' && FileBytes[3] == 'S')
     {
         // it's a Photoshop PSD file
         PSDFile *Psd = (PSDFile *)FileBytes;
         uint32 Width = SWAP32(Psd->Columns);
         uint32 Height = SWAP32(Psd->Rows);
         int channels = SWAP16(Psd->Channels);

         if((channels == 1 || channels == 3 || channels >= 4) && 
            (SWAP16(Psd->Version) == 1) && 
            (SWAP16(Psd->Depth) == 8))
         {
             uint16 Compression;
             int LineStride;
             int Channel;
             uint32 *data;
             unsigned int i,j;

             res = malloc(sizeof(*res) + sizeof(res->data[0]) * Width * Height);
             res->w = Width;
             res->h = Height;
             data = res->data;
             memset(data, 0, sizeof(*data) * Width * Height);

             // we want the result to be bottom-up because it matches the terrain
             LineStride = (Width);   // walk up the bitmap

             FileBytes += sizeof(PSDFile);
             // skip Color mode
             FileBytes += SWAP32(*(uint32 *)(FileBytes)) + 4;
             // skip Image resources
             FileBytes += SWAP32(*(uint32 *)(FileBytes)) + 4;
             // skip Layers
             FileBytes += SWAP32(*(uint32 *)(FileBytes)) + 4;
             Compression = SWAP16(*(uint16*)FileBytes);
             FileBytes += 2;
             if (channels > 4) channels = 4;
             for (Channel = 0; Channel < channels; ++Channel) {
                int shift = 8 * Channel;
                if(Compression == 0)
                {
                    // uncompressed data

                    // skip to the specified channel
                    uint32 ChannelSize = Width * Height;
                    uint8 *p = FileBytes + ChannelSize * Channel;

                    // rip out the specified channel
                    for(i = 0;i < Height;++i)
                    {
                        for(j = 0;j < Width;++j)
                        {
                            data[j+i*LineStride] |= *p++ << shift;
                        }
                    }
                }
                else
                {
                    uint8 *p;
                    uint32 *d = data;

                    // RLE compressed data
                    // header of uint16 line lengths
                    // n b+
                    // n=[00,7F]: copy n+1 b's
                    // n=[81,FF]: dupe b -n+1 times
                    // n=80: finished/skip

                    // figure out where this channel's scanlines start
                    uint BytesToChannel = 2*Height*Channel;
                    uint LineOffset = 2*Height*SWAP16(Psd->Channels);
                    int BytesToWrite = Width*Height;
                    int LineCount = 0;
                    uint i = 0;
                    while(i < BytesToChannel) {
                        LineOffset += SWAP16(*(uint16*)(FileBytes+i));
                        i += 2;
                    }
                    p = FileBytes + LineOffset;

                    // now we're at Channel
                    while(BytesToWrite > 0) {
                        // decompress run
                        int Count = *(int8*)p;
                        ++p;
                        if(Count >= 0) {
                            // copy
                            Count += 1;
                            BytesToWrite -= Count;
                            LineCount += Count;
                            while(Count--) {
                                *d++ |= *p++ << shift;
                            }
                        } else
                        if(Count > -128) {
                            // run
                            int i;
                            Count = -Count + 1;
                            BytesToWrite -= Count;
                            for (i=0; i < Count; ++i)
                               *d++ |= *p << shift;
                            ++p;
                            LineCount += Count;
                        }
                        // line end?
                        assert((uint32)(LineCount) <= Width);
                        if((uint32)(LineCount) >= Width) {
                            // go back to start of this line and then to next
                            LineCount = 0;
                            d += LineStride - Width;
                        }
                        assert(BytesToWrite >= 0);
                    }
                }
            }
        }
    }

    UnmapViewOfFile(FileData);
    CloseHandle(FileMap);
    CloseHandle(File);

    return res;
} 
