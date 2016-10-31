/*==========================================================================
 *
 *  Copyright (C) 1995 Microsoft Corporation. All Rights Reserved.
 *
 *  File:       dsutil.cpp
 *  Content:    Routines for dealing with sounds from resources
 *
 *
 ***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <dsound.h>


typedef struct
{
    BYTE *pbWaveData;               // pointer into wave resource (for restore)
    DWORD cbWaveSize;               // size of wave data (for restore)
    int iAlloc;                     // number of buffers.
    int iCurrent;                   // current buffer
    IDirectSoundBuffer* Buffers[1]; // list of buffers

} SNDOBJ, *HSNDOBJ;

#define _HSNDOBJ_DEFINED
#include "dsutil.h"

static const char c_szWAV[] = "WAVE";

///////////////////////////////////////////////////////////////////////////////
//
// DSLoadSoundBuffer
//
///////////////////////////////////////////////////////////////////////////////

IDirectSoundBuffer *DSLoadSoundBuffer(IDirectSound *pDS, LPCTSTR lpName)
{
    IDirectSoundBuffer *pDSB = NULL;
    DSBUFFERDESC dsBD = {0};
    BYTE *pbWaveData;

    if (DSGetWaveResource(NULL, lpName, &dsBD.lpwfxFormat, &pbWaveData, &dsBD.dwBufferBytes))
    {
        dsBD.dwSize = sizeof(dsBD);
        dsBD.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLDEFAULT | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRL3D;

        if (SUCCEEDED(IDirectSound_CreateSoundBuffer(pDS, &dsBD, &pDSB, NULL)))
        {
            if (!DSFillSoundBuffer(pDSB, pbWaveData, dsBD.dwBufferBytes))
            {
                IDirectSoundBuffer_Release(pDSB);
                pDSB = NULL;
            }
        }
        else
        {
            pDSB = NULL;
        }
    }

    return pDSB;
}

///////////////////////////////////////////////////////////////////////////////
//
// DSReloadSoundBuffer
//
///////////////////////////////////////////////////////////////////////////////

BOOL DSReloadSoundBuffer(IDirectSoundBuffer *pDSB, LPCTSTR lpName)
{
    BOOL result=FALSE;
    BYTE *pbWaveData;
    DWORD cbWaveSize;

    if (DSGetWaveResource(NULL, lpName, NULL, &pbWaveData, &cbWaveSize))
    {
        if (SUCCEEDED(IDirectSoundBuffer_Restore(pDSB)) &&
            DSFillSoundBuffer(pDSB, pbWaveData, cbWaveSize))
        {
            result = TRUE;
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
//
// DSGetWaveResource
//
///////////////////////////////////////////////////////////////////////////////

BOOL DSGetWaveResource(HMODULE hModule, LPCTSTR lpName,
    WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pcbWaveSize)
{
    HRSRC hResInfo;
    HGLOBAL hResData;
    void *pvRes;

    if (((hResInfo = FindResource(hModule, lpName, c_szWAV)) != NULL) &&
        ((hResData = LoadResource(hModule, hResInfo)) != NULL) &&
        ((pvRes = LockResource(hResData)) != NULL) &&
        DSParseWaveResource(pvRes, ppWaveHeader, ppbWaveData, pcbWaveSize))
    {
        return TRUE;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
// DSGetWaveFile
//
///////////////////////////////////////////////////////////////////////////////
/*
BOOL DSGetWaveFile(LPCTSTR lpName,
    WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pcbWaveSize)
{
    void *pvRes;
	byte *buffer;

	// load data from file
	CFILE file(FileName, CFile::modeRead);
	buffer = new byte[file.GetLength()];

	if(DSParseWaveResource(pvRes, ppWaveHeader, ppbWaveData, pcbWaveSize))
    {
		file.close();
		delete buffer[];
        return TRUE;
    }
	file.close();
	delete buffer[];
    return FALSE;
}
*/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL DSFillSoundBuffer(IDirectSoundBuffer *pDSB, BYTE *pbWaveData, DWORD cbWaveSize)
{
    if (pDSB && pbWaveData && cbWaveSize)
    {
        LPVOID pMem1, pMem2;
        DWORD dwSize1, dwSize2;

        if (SUCCEEDED(IDirectSoundBuffer_Lock(pDSB, 0, cbWaveSize,
            &pMem1, &dwSize1, &pMem2, &dwSize2, 0)))
        {
            CopyMemory(pMem1, pbWaveData, dwSize1);

            if ( 0 != dwSize2 )
                CopyMemory(pMem2, pbWaveData+dwSize1, dwSize2);

            IDirectSoundBuffer_Unlock(pDSB, pMem1, dwSize1, pMem2, dwSize2);
            return TRUE;
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL DSParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData,DWORD *pcbWaveSize)
{
    DWORD *pdw;
    DWORD *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;

    if (ppWaveHeader)
        *ppWaveHeader = NULL;

    if (ppbWaveData)
        *ppbWaveData = NULL;

    if (pcbWaveSize)
        *pcbWaveSize = 0;

    pdw = (DWORD *)pvRes;
    dwRiff = *pdw++;
    dwLength = *pdw++;
    dwType = *pdw++;

    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
        goto exit;      // not even RIFF

    if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
        goto exit;      // not a WAV

    pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

    while (pdw < pdwEnd)
    {
        dwType = *pdw++;
        dwLength = *pdw++;

        switch (dwType)
        {
        case mmioFOURCC('f', 'm', 't', ' '):
            if (ppWaveHeader && !*ppWaveHeader)
            {
                if (dwLength < sizeof(WAVEFORMAT))
                    goto exit;      // not a WAV

                *ppWaveHeader = (WAVEFORMATEX *)pdw;

                if ((!ppbWaveData || *ppbWaveData) &&
                    (!pcbWaveSize || *pcbWaveSize))
                {
                    return TRUE;
                }
            }
            break;

        case mmioFOURCC('d', 'a', 't', 'a'):
            if ((ppbWaveData && !*ppbWaveData) ||
                (pcbWaveSize && !*pcbWaveSize))
            {
                if (ppbWaveData)
                    *ppbWaveData = (LPBYTE)pdw;

                if (pcbWaveSize)
                    *pcbWaveSize = dwLength;

                if (!ppWaveHeader || *ppWaveHeader)
                    return TRUE;
            }
            break;
        }

        pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
    }

exit:
    return FALSE;
}
