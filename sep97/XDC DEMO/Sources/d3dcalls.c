/*
 *  Copyright (C) 1995, 1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File: d3dcalls.c
 *
 *  Calls to Direct3D objects needed for rendering.  Part of D3DApp.
 *
 *  D3DApp is a collection of helper functions for Direct3D applications.
 *  D3DApp consists of the following files:
 *	d3dapp.h    Main D3DApp header to be included by application
 *      d3dappi.h   Internal header
 *      d3dapp.c    D3DApp functions seen by application.
 *      ddcalls.c   All calls to DirectDraw objects except textures
 *      d3dcalls.c  All calls to Direct3D objects except textures
 *      texture.c   Texture loading and managing texture list
 *      misc.c	    Miscellaneous calls
 */

#define INITGUID
#include "d3dappi.h"

/***************************************************************************/
/*                            Creation of D3D                              */
/***************************************************************************/
BOOL
D3DAppICreateD3D(void)
{
    LastError = d3dappi.lpDD->lpVtbl->QueryInterface(d3dappi.lpDD,
				    &IID_IDirect3D, (LPVOID*)&d3dappi.lpD3D);
    if (LastError != DD_OK) {
        D3DAppISetErrorString("Creation of IDirect3D failed.\n%s",
			      D3DAppErrorToString(LastError));
	goto exit_with_error;
    }
    return TRUE;
exit_with_error:
    return FALSE;
}

/***************************************************************************/
/*                           D3D Device Enumeration                        */
/***************************************************************************/
/*
 * enumDeviceFunc
 * Device enumeration callback.  Record information about the D3D device
 * reported by D3D.
 */
static HRESULT
WINAPI enumDeviceFunc(LPGUID lpGuid, LPSTR lpDeviceDescription,
		      LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc,
		      LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{
    lpContext = lpContext;
    /*
     * Don't accept any hardware D3D devices if emulation only option is set
     */
    if (lpHWDesc->dcmColorModel && d3dappi.bOnlyEmulation)
	return D3DENUMRET_OK;
    /*
     * Record the D3D driver's inforamation
     */
    memcpy(&d3dappi.Driver[d3dappi.NumDrivers].Guid, lpGuid, sizeof(GUID));
    lstrcpy(d3dappi.Driver[d3dappi.NumDrivers].About, lpDeviceDescription);
    lstrcpy(d3dappi.Driver[d3dappi.NumDrivers].Name, lpDeviceName);
    /*
     * Is this a hardware device or software emulation?  Checking the color
     * model for a valid model works.
     */
    if (lpHWDesc->dcmColorModel) {
	d3dappi.Driver[d3dappi.NumDrivers].bIsHardware = TRUE;
	memcpy(&d3dappi.Driver[d3dappi.NumDrivers].Desc, lpHWDesc,
	       sizeof(D3DDEVICEDESC));
    } else {
	d3dappi.Driver[d3dappi.NumDrivers].bIsHardware = FALSE;
	memcpy(&d3dappi.Driver[d3dappi.NumDrivers].Desc, lpHELDesc,
	       sizeof(D3DDEVICEDESC));
    }
    /*
     * Does this driver do texture mapping?
     */
    d3dappi.Driver[d3dappi.NumDrivers].bDoesTextures =
	(d3dappi.Driver[d3dappi.NumDrivers].Desc.dpcTriCaps.dwTextureCaps &
	 D3DPTEXTURECAPS_PERSPECTIVE) ? TRUE : FALSE;
    /*
     * Can this driver use a z-buffer?
     */
    d3dappi.Driver[d3dappi.NumDrivers].bDoesZBuffer =
	d3dappi.Driver[d3dappi.NumDrivers].Desc.dwDeviceZBufferBitDepth
		? TRUE : FALSE;
    /*
     * Can this driver render to the Windows display depth
     */
    d3dappi.Driver[d3dappi.NumDrivers].bCanDoWindow =
	(d3dappi.Driver[d3dappi.NumDrivers].Desc.dwDeviceRenderBitDepth &
	 D3DAppIBPPToDDBD(d3dappi.WindowsDisplay.bpp)) ? TRUE : FALSE;
    if (!d3dappi.bIsPrimary)
	d3dappi.Driver[d3dappi.NumDrivers].bCanDoWindow = FALSE;

    d3dappi.NumDrivers++;
    if (d3dappi.NumDrivers == D3DAPP_MAXD3DDRIVERS)
        return (D3DENUMRET_CANCEL);
    return (D3DENUMRET_OK);
}

/*
 * D3DAppIEnumDevices
 * Get the available drivers from Direct3D by enumeration.
 */
BOOL
D3DAppIEnumDevices(void)
{
    d3dappi.NumDrivers = 0;
    LastError = d3dappi.lpD3D->lpVtbl->EnumDevices(d3dappi.lpD3D,
						   enumDeviceFunc, NULL);
    if (LastError != DD_OK) {
        D3DAppISetErrorString("Enumeration of drivers failed.\n%s",
			      D3DAppErrorToString(LastError));
	return FALSE;
    }
    d3dappi.CurrDriver = 0;
    return TRUE;
}

/***************************************************************************/
/*                    Enumeration of texure format                         */
/***************************************************************************/
/*
 * EnumTextureFormatsCallback
 * Record information about each texture format the current D3D driver can
 * support. Choose one as the default format (paletted formats are prefered)
 * and return it through lpContext.
 */
static HRESULT
CALLBACK EnumTextureFormatsCallback(LPDDSURFACEDESC lpDDSD, LPVOID lpContext)
{
    unsigned long m;
    int r, g, b;
    int *lpStartFormat = (int *)lpContext;
    /*
     * Record the DDSURFACEDESC of this texture format
     */
    memset(&d3dappi.TextureFormat[d3dappi.NumTextureFormats], 0,
	   sizeof(D3DAppTextureFormat));
    memcpy(&d3dappi.TextureFormat[d3dappi.NumTextureFormats].ddsd, lpDDSD,
	   sizeof(DDSURFACEDESC));
    /*
     * Is this format palettized?  How many bits?  Otherwise, how many RGB
     * bits?
     */
    if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized = TRUE;
	d3dappi.TextureFormat[d3dappi.NumTextureFormats].IndexBPP = 8;
    } else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4) {
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized = TRUE;
	d3dappi.TextureFormat[d3dappi.NumTextureFormats].IndexBPP = 4;
    } else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {
		/*
		 * The sample apps don't currently understand
		 * the alpha bit - just filter this format
		 * away for now.
		 */

		return DDENUMRET_OK;
    } else
    {
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized = FALSE;
	d3dappi.TextureFormat[d3dappi.NumTextureFormats].IndexBPP = 0;
        for (r = 0, m = lpDDSD->ddpfPixelFormat.dwRBitMask; !(m & 1);
							       r++, m >>= 1);
        for (r = 0; m & 1; r++, m >>= 1);
        for (g = 0, m = lpDDSD->ddpfPixelFormat.dwGBitMask; !(m & 1);
							       g++, m >>= 1);
        for (g = 0; m & 1; g++, m >>= 1);
        for (b = 0, m = lpDDSD->ddpfPixelFormat.dwBBitMask; !(m & 1);
							       b++, m >>= 1);
        for (b = 0; m & 1; b++, m >>= 1);
	d3dappi.TextureFormat[d3dappi.NumTextureFormats].RedBPP = r;
	d3dappi.TextureFormat[d3dappi.NumTextureFormats].GreenBPP = g;
	d3dappi.TextureFormat[d3dappi.NumTextureFormats].BlueBPP = b;
    }
    /*
     * If lpStarFormat is -1, this is the first format.  Select it.
     */
    if (*lpStartFormat == -1)
        *lpStartFormat = d3dappi.NumTextureFormats;
    /* 
     * If this format is NOT paletted, select it.  (H. Barad)
	 * I prefer 565.
     */
    if (!d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized) {
        *lpStartFormat = d3dappi.NumTextureFormats;
    }
    d3dappi.NumTextureFormats++;
    return DDENUMRET_OK;
}

/*
 * D3DAppIEnumTextureFormats
 * Get a list of available texture map formats from the Direct3D driver by
 * enumeration.  Choose a default format (paletted is prefered).
 */
BOOL
D3DAppIEnumTextureFormats(void)
{
    int StartFormat;
    /*
     * Set the default format to -1 to let the callback know it's being 
     * called for the first time.
     */
    StartFormat = -1;
    d3dappi.NumTextureFormats = 0;
    LastError =
	 d3dappi.lpD3DDevice->lpVtbl->EnumTextureFormats(d3dappi.lpD3DDevice,
						  EnumTextureFormatsCallback,
                                                  (LPVOID)&StartFormat);
    if (LastError != DD_OK) {
	D3DAppISetErrorString("Enumeration of texture formats failed.\n%s",
			      D3DAppErrorToString(LastError));
	return FALSE;
    }
    memcpy(&d3dappi.ThisTextureFormat, &d3dappi.TextureFormat[StartFormat],
	   sizeof(D3DAppTextureFormat));
    d3dappi.CurrTextureFormat = StartFormat;
    return TRUE;
}

/***************************************************************************/
/*                               Device creation                           */
/***************************************************************************/
/*
 * D3DAppICreateDevice
 * Create the D3D device and enumerate the texture formats
 */
BOOL
D3DAppICreateDevice(int driver)
{

    RELEASE(d3dappi.lpD3DDevice);

    if (d3dappi.Driver[driver].bIsHardware && !d3dappi.bBackBufferInVideo) {
	D3DAppISetErrorString("Could not fit the rendering surfaces in video memory for this hardware device.\n");
	goto exit_with_error;
    }

    d3dappi.CurrDriver = driver;
    memcpy(&d3dappi.ThisDriver, &d3dappi.Driver[driver], sizeof(D3DAppD3DDriver));
    LastError =
	   d3dappi.lpBackBuffer->lpVtbl->QueryInterface(d3dappi.lpBackBuffer,
						&d3dappi.Driver[driver].Guid,
					      (LPVOID*)&d3dappi.lpD3DDevice);
    if (LastError != DD_OK) {
    	D3DAppISetErrorString("Create D3D device failed.\n%s",
			      D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    d3dappi.CurrDriver = driver;
    d3dappi.NumTextureFormats = 0;
    if (d3dappi.Driver[driver].bDoesTextures) {
	if (!D3DAppIEnumTextureFormats())
	    goto exit_with_error;
    }

    return TRUE;
exit_with_error:
    RELEASE(d3dappi.lpD3DDevice);
    return FALSE;
}

/***************************************************************************/
/*                      Setting the render state                           */
/***************************************************************************/
/*
 * D3DAppISetRenderState
 * Create and execute an execute buffer which will set the render state and
 * light state for the current viewport.
 */
BOOL
D3DAppISetRenderState()
{
    D3DEXECUTEBUFFERDESC debDesc;
    D3DEXECUTEDATA d3dExData;
    LPDIRECT3DEXECUTEBUFFER lpD3DExCmdBuf = NULL;
    LPVOID lpBuffer, lpInsStart;
    size_t size;

    /*
     * If there is no D3D Viewport, we must return true because it is not
     * required.
     */
    if (!d3dappi.lpD3DViewport)
	return TRUE;
    /*
     * Create an execute buffer of the required size
     */
    size = 0;
    size += sizeof(D3DINSTRUCTION) * 3;
    size += sizeof(D3DSTATE) * 17;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    LastError =
	d3dappi.lpD3DDevice->lpVtbl->CreateExecuteBuffer(d3dappi.lpD3DDevice,
					     &debDesc, &lpD3DExCmdBuf, NULL);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("CreateExecuteBuffer failed in SetRenderState.\n%s",
			      D3DAppErrorToString(LastError));
	goto exit_with_error;
    }
    /*
     * Lock the execute buffer so it can be filled
     */
    LastError = lpD3DExCmdBuf->lpVtbl->Lock(lpD3DExCmdBuf, &debDesc);
    if (LastError != D3D_OK) {
	D3DAppISetErrorString("Lock failed on execute buffer in SetRenderState.\n%s",
			      D3DAppErrorToString(LastError));
	goto exit_with_error;
    }
    memset(debDesc.lpData, 0, size);

    lpInsStart = debDesc.lpData;
    lpBuffer = lpInsStart;
    /*
     * Set render state
     */
    OP_STATE_RENDER(14, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_SHADEMODE, d3dapprs.ShadeMode, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, d3dapprs.bPerspCorrect,
		 lpBuffer);
      STATE_DATA(D3DRENDERSTATE_ZENABLE, d3dapprs.bZBufferOn &&
				  d3dappi.ThisDriver.bDoesZBuffer, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, d3dapprs.bZBufferOn, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, d3dapprs.TextureFilter,lpBuffer);
      STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, d3dapprs.TextureFilter,lpBuffer);
      STATE_DATA(D3DRENDERSTATE_TEXTUREMAPBLEND, d3dapprs.TextureBlend,
		 lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FILLMODE, d3dapprs.FillMode, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_DITHERENABLE, d3dapprs.bDithering, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_SPECULARENABLE, d3dapprs.bSpecular,lpBuffer);
      STATE_DATA(D3DRENDERSTATE_ANTIALIAS, d3dapprs.bAntialiasing, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FOGENABLE, d3dapprs.bFogEnabled, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FOGCOLOR, d3dapprs.FogColor, lpBuffer);
    /*
     * Set light state
     */
    OP_STATE_LIGHT(3, lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGMODE, d3dapprs.bFogEnabled ?
                 d3dapprs.FogMode : D3DFOG_NONE, lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGSTART, *(unsigned long*)&d3dapprs.FogStart,
		 lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGEND, *(unsigned long*)&d3dapprs.FogEnd,
		 lpBuffer);
    OP_EXIT(lpBuffer);

    LastError = lpD3DExCmdBuf->lpVtbl->Unlock(lpD3DExCmdBuf);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Unlock failed on execute buffer in SetRenderState.\n%s",
			      D3DAppErrorToString(LastError));
	goto exit_with_error;
    }
    /*
     * Set the execute data and exectue the buffer
     */
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwInstructionOffset = (ULONG) 0;
    d3dExData.dwInstructionLength = (ULONG) ((char*)lpBuffer -
							  (char*)lpInsStart);
    lpD3DExCmdBuf->lpVtbl->SetExecuteData(lpD3DExCmdBuf, &d3dExData);
    LastError = d3dappi.lpD3DDevice->lpVtbl->BeginScene(d3dappi.lpD3DDevice);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("BeginScene failed in SetRenderState.\n%s",
			      D3DAppErrorToString(LastError));
	goto exit_with_error;
    }
    LastError = d3dappi.lpD3DDevice->lpVtbl->Execute(d3dappi.lpD3DDevice,
						     lpD3DExCmdBuf,
						     d3dappi.lpD3DViewport,
						     D3DEXECUTE_UNCLIPPED);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Execute failed in SetRenderState.\n%s",
			      D3DAppErrorToString(LastError));
	goto exit_with_error;
    }
    LastError = d3dappi.lpD3DDevice->lpVtbl->EndScene(d3dappi.lpD3DDevice);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("EndScene failed in SetRenderState.\n%s",
			      D3DAppErrorToString(LastError));
	goto exit_with_error;
    }
    /*
     * We are done with the execute buffer, so release it.
     */
    lpD3DExCmdBuf->lpVtbl->Release(lpD3DExCmdBuf);
    return TRUE;

exit_with_error:
    RELEASE(lpD3DExCmdBuf);
    return FALSE;
}

