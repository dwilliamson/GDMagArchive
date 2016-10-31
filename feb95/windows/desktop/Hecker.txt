Listing 1:
#include<windows.h>
#include<assert.h>


void TransparentBlt( BITMAPINFOHEADER *pDestHeader, BYTE *pDestBits,
			int XDest, int YDest, BITMAPINFOHEADER *pSourceHeader,
			BYTE *pSourceBits, BYTE TransparentColor ){
	int DestDeltaScan, DestWidthBytes, DestRealHeight;
	int SourceDeltaScan, XSource = 0, YSource = 0;
	int Width, Height;

	DestWidthBytes = (pDestHeader->biWidth + 3) & ~3; 	// dword align

	assert(pDestHeader->biSizeImage);		// insure biSizeImage is set

	if(pDestHeader->biHeight < 0){
		// dest is top-down
		DestRealHeight = -pDestHeader->biHeight;	// get positive height
		DestDeltaScan = DestWidthBytes;			// travel down dest
	}else{
		// dest is bottom-up
		DestRealHeight = pDestHeader->biHeight;
		DestDeltaScan = -DestWidthBytes;			// travel down dest
		// point to top scanline
		pDestBits += pDestHeader->biSizeImage - DestWidthBytes;
	}

	// pDestBits -> top scanline of dest
	// DestDeltaScan -> distance from scan to scan in dest

	// clip source to dest

	assert(pSourceHeader->biHeight < 0);	// assume top-down source DIB
	Width = pSourceHeader->biWidth;
	Height = -pSourceHeader->biHeight;

	if(XDest < 0){
		// left clipped
		Width += XDest;
		XSource = -XDest;
		XDest = 0;
	}

	if((XDest + Width) > pDestHeader->biWidth){
		//right clipped
		Width = pDestHeader->biWidth - XDest;
	}

	if(YDest < 0){
		// top clipped
		Height += YDest;
		YSource = -YDest;
		YDest = 0;
	}

	if((YDest + Height) > DestRealHeight){
		// bottom clipped
		Height = DestRealHeight - YDest;
	}

	SourceDeltaScan = (pSourceHeader->biWidth + 3) & ~3;	// dword align

	// step to starting source pixel
	pSourceBits += (YSource * SourceDeltaScan) + XSource;

	// step to starting dest pixel
	pDestBits += (YDest * DestDeltaScan) + XDest;
		
	// account for processed span in delta scans
	SourceDeltaScan -= Width;
	DestDeltaScan -= Width;

	if((Height > 0) && (Width > 0))	{
		// we have something to BLT
		int X, Y;
		
		for(Y = 0;Y < Height;Y++) {
			for(X = 0;X < Width;X++) {
				if(*pSourceBits != TransparentColor) {	// not transparent?
					*pDestBits = *pSourceBits;	// copy the pixel
				}
				pDestBits++;					// advance to next pixels
				pSourceBits++;
			}
			pDestBits += DestDeltaScan;			// advance to next dest
			pSourceBits += SourceDeltaScan;		// and source pixels
		}
	}
}

Listing 2:
#include<windows.h>
#include<windowsx.h>
#include<string.h>
#include<assert.h>



#define ISSKIPRUN( Record ) (int)((((DWORD)(Record)) & 0xFFFF0000) == 0x00010000)
#define ISCOPYRUN( Record ) (int)((((DWORD)(Record)) & 0xFFFF0000) == 0x00020000)

#define RUNLENGTH( Record ) (int)(((DWORD)(Record)) & 0xFFFF)


void TransparentBltRLE( BITMAPINFOHEADER *pDestHeader, BYTE *pDestBits,
			int XDest, int YDest, BITMAPINFOHEADER *pSourceHeader,
			BYTE *pSourceBits, BYTE TransparentColor ){
	int DestDeltaScan, DestWidthBytes, DestRealHeight;
 	int XSource = 0, YSource = 0;
 	int Width, Height;

	DestWidthBytes = (pDestHeader->biWidth + 3) & ~3; 	// dword align

	assert(pDestHeader->biSizeImage);		// insure biSizeImage is set

	if(pDestHeader->biHeight < 0){
		// dest is top-down
		DestRealHeight = -pDestHeader->biHeight;	// get positive height
		DestDeltaScan = DestWidthBytes;			// travel down dest
	}else{
		// dest is bottom-up
		DestRealHeight = pDestHeader->biHeight;
		DestDeltaScan = -DestWidthBytes;			// travel down dest
		// point to top scanline
		pDestBits += pDestHeader->biSizeImage - DestWidthBytes;
	}

	// pDestBits -> top scanline of dest
	// DestDeltaScan -> distance from scan to scan in dest

	// clip source to dest

	assert(pSourceHeader->biHeight < 0);		// assume top-down source DIB
	Width = pSourceHeader->biWidth;
	Height = -pSourceHeader->biHeight;

	if(XDest < 0){
		// left clipped
		Width += XDest;
		XSource = -XDest;
		XDest = 0;
	}

	if((XDest + Width) > pDestHeader->biWidth){
		//right clipped
		Width = pDestHeader->biWidth - XDest;
	}

	if(YDest < 0){
		// top clipped
		Height += YDest;
		YSource = -YDest;
		YDest = 0;
	}

	if((YDest + Height) > DestRealHeight){
		// bottom clipped
		Height = DestRealHeight - YDest;
	}

	// step to starting dest pixel
	pDestBits += (YDest * DestDeltaScan) + XDest;
		
	// account for span in delta scans
	DestDeltaScan -= Width;

	if((Height > 0) && (Width > 0)){
		// we have something to BLT
		int X, Y;
		DWORD *pCurrentSourceScan = (DWORD *)pSourceBits;

		// prestep to starting source Y

		for(Y = 0;Y < YSource;Y++){
			pCurrentSourceScan = (DWORD *)((BYTE *)pCurrentSourceScan +
									RUNLENGTH(*pCurrentSourceScan));
		}
		
		for(Y = 0;Y < Height;Y++){
			DWORD *pCurrentSourceRecord = pCurrentSourceScan + 1;
			
			// prestep to starting source X

			X = 0;

			while(X < XSource){
				X += RUNLENGTH(*pCurrentSourceRecord);

				if(X > XSource){
					// we need to partially process the current record

					int Overlap = X - XSource;
					int ActiveOverlap = (Overlap > Width) ? Width : Overlap;

					if(ISCOPYRUN(*pCurrentSourceRecord)){
						// copy overlap pixels to destination

						// get pointer to data
						BYTE *pCopyRun = (BYTE *)pCurrentSourceRecord + 4;

						// prestep to desired pixels
						pCopyRun += RUNLENGTH(*pCurrentSourceRecord) - Overlap;

						memcpy(pDestBits,pCopyRun,ActiveOverlap);
					}

					// skip to next dest pixel
					pDestBits += ActiveOverlap;
				}

				// skip to next record

				if(ISCOPYRUN(*pCurrentSourceRecord)){
					// skip any data bytes
					pCurrentSourceRecord =
						(DWORD *)((BYTE *)pCurrentSourceRecord +
							RUNLENGTH(*pCurrentSourceRecord));
				}

				pCurrentSourceRecord++;				// skip record itself
			}

			X = X - XSource;
			
			while(X < Width){
				int RunLength = RUNLENGTH(*pCurrentSourceRecord);
				int RemainingWidth = Width - X;
				int ActivePixels = (RunLength > RemainingWidth) ?
										RemainingWidth : RunLength;

				if(ISCOPYRUN(*pCurrentSourceRecord)){
					// copy pixels to destination

					// get pointer to data
					BYTE *pCopyRun = (BYTE *)pCurrentSourceRecord + 4;

					memcpy(pDestBits,pCopyRun,ActivePixels);
				}

				// skip to next dest pixel
				pDestBits += ActivePixels;
			
				// skip to next record

				if(ISCOPYRUN(*pCurrentSourceRecord)){
					// skip any data bytes
					pCurrentSourceRecord =
						(DWORD *)((BYTE *)pCurrentSourceRecord + RunLength);
				}

				pCurrentSourceRecord++;				// skip record itself

				X += RunLength;
			}

			pDestBits += DestDeltaScan;

			pCurrentSourceScan = (DWORD *)((BYTE *)pCurrentSourceScan +
									RUNLENGTH(*pCurrentSourceScan));
		}
	}
}
	


Listing 3:
#include<windows.h>
#include<windowsx.h>
#include<string.h>
#include<assert.h>

#define NEWLINE( Length ) ((DWORD)(0x00000000 | (short unsigned)(Length)))
#define SKIPRUN( Length ) ((DWORD)(0x00010000 | (short unsigned)(Length)))
#define COPYRUN( Length ) ((DWORD)(0x00020000 | (short unsigned)(Length)))


BYTE *CompressSprite( BITMAPINFOHEADER *pSourceHeader, BYTE *pSourceBits,
			BYTE TransparentColor ){
	int SourceWidthBytes = (pSourceHeader->biWidth + 3) & ~3;
	void *pOutputBuffer = GlobalAllocPtr(GHND,pSourceHeader->biSizeImage);
	DWORD *pOutputRecord = (DWORD *)pOutputBuffer;
	BYTE *pOutputByte;
	int X, Y;

	assert(pOutputBuffer);

	for(Y = 0;Y < pSourceHeader->biHeight;Y++){
		int Width = pSourceHeader->biWidth;
		enum state { InSkipRun, InCopyRun } State;
		BYTE *pSourceByte = pSourceBits;
		DWORD *pNewlineRecord = pOutputRecord++;
		int LineLength = 4;
		int CurrentRunLength = 1;

		pOutputByte = (BYTE *)(pOutputRecord + 1);

		if(*pSourceByte == TransparentColor){
			// we're starting a skip run
			State = InSkipRun;
			LineLength += 4;
		}else{	// source is data
			// we're starting a copy run
			State = InCopyRun;
			*pOutputByte++ = *pSourceByte;
			LineLength += 5;
		}

		pSourceByte++;

		for(X = 1;X < Width;X++){
			if(*pSourceByte == TransparentColor){
				if(State == InSkipRun){				// still in skip run
					CurrentRunLength++;
				}else{								// changing to skip run
					// write out copy record
					*pOutputRecord = COPYRUN(CurrentRunLength);
					pOutputRecord = (DWORD *)pOutputByte;

					CurrentRunLength = 1;
					State = InSkipRun;
					LineLength += 4;
				}
			}else{		// source is data
				if(State == InCopyRun){				// still in copy run
					CurrentRunLength++;
					*pOutputByte++ = *pSourceByte;
					LineLength++;
				}else{								// changing to copy run
					// write out skip record
					*pOutputRecord = SKIPRUN(CurrentRunLength);
					pOutputRecord++;
					pOutputByte = (BYTE *)(pOutputRecord + 1);

					CurrentRunLength = 1;
					State = InCopyRun;
					*pOutputByte++ = *pSourceByte;
					LineLength += 5;
				}
			}

			pSourceByte++;
		}

		// finish off current record

		if(State == InSkipRun){
			*pOutputRecord = SKIPRUN(CurrentRunLength);
			pOutputRecord++;
		}else{	// InCopyRun
			*pOutputRecord = COPYRUN(CurrentRunLength);
			pOutputRecord = (DWORD *)pOutputByte;
		}

		*pNewlineRecord = NEWLINE(LineLength);

		pSourceBits += SourceWidthBytes;
	}

	return (BYTE *)pOutputBuffer;
}




