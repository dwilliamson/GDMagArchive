#include "..\\include\\move.h"
#define __GLOBAL__
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"


void close_direct_draw(void) {
//Release direct draw
if(lpdd!=NULL)
	{if(lpddsprimary!=NULL)
		{lpddsprimary->lpVtbl->Release(lpddsprimary);
		 lpddsprimary=NULL;
		}
	 lpdd->lpVtbl->Release(lpdd);
	 lpdd=NULL;
	}
}


long init_direct_draw(HWND hwnd) {
//Initialize direct draw
//hwnd is handle of parent window
DDSURFACEDESC ddsd;DDSCAPS ddscaps;HRESULT ddrval;char s[256],fail=255;
ddrval=DirectDrawCreate(NULL,&lpdd,NULL);
if(ddrval==DD_OK)
	{// Get exclusive mode
	 ddrval=lpdd->lpVtbl->SetCooperativeLevel(lpdd,hwnd,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES);
	 fail=1;
	 if(ddrval==DD_OK)
		{//Switch to full screen
		 fail=2;
		 ddrval=lpdd->lpVtbl->SetDisplayMode(lpdd,SCREEN_X,SCREEN_Y,8);
		 if (ddrval==DD_OK)
			{// Create the primary surface
			 fail=3;
			 ddsd.dwSize=sizeof(ddsd);
			 ddsd.dwFlags=DDSD_CAPS;
			 ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
			 ddrval=lpdd->lpVtbl->CreateSurface(lpdd,&ddsd,&lpddsprimary,NULL);
			 if(ddrval==DD_OK)
				{
				 MoveWindow(hwnd,0,0,SCREEN_X,SCREEN_Y,TRUE);
				 return 0;
				 }
			 fail=4;
			 }
		 }
	}
return fail;
}

static BYTE *ddraw_video,*ddraw_source;long ddraw_pitch,ddraw_ysize,ddraw_xsize,ddraw_offset,ddraw_real_pitch;

extern void warpscreen(void);
#pragma aux warpscreen = \
		"cld"\
		"mov esi,ddraw_source"\
		"mov edi,ddraw_video"\
		"mov ecx,480"\
		"mov edx,ddraw_pitch"\
		"looppage2:"\
		"mov ebx,ecx"\
		"mov ecx,160"\
		"looppage:"\
		"mov eax,[esi]"\
		"mov [edi],eax"\
		"add edi,4"\
		"add esi,4"\
		"dec ecx"\
		"jnz looppage"\
		"add edi,edx"\
		"mov ecx,ebx"\
		"dec ecx"\
		"jnz looppage2"\
		modify [eax ecx edx esi ebx edi];

void warp_page(void) {
DDSURFACEDESC ddsd;HRESULT ddrval;long x,xr;
ddsd.dwSize=sizeof(ddsd);
while ((ddrval=lpddsprimary->lpVtbl->Lock(lpddsprimary,NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING);
if( ddrval==DDERR_SURFACELOST)
		{ddrval=lpddsprimary->lpVtbl->Restore(lpddsprimary);
		 while ((ddrval=lpddsprimary->lpVtbl->Lock(lpddsprimary,NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING);
		 if (ddrval==DDERR_SURFACELOST)
			 return;
		 }
if (ddrval!=DD_OK)
	 return;
ddraw_video=(BYTE *)ddsd.lpSurface;
ddraw_pitch=ddsd.lPitch-SCREEN_X;
ddraw_source=videobuffer;
warpscreen();
lpddsprimary->lpVtbl->Unlock(lpddsprimary,NULL);
}

BOOL copy_system_pal(void) {
//puts the system palette in the lights array
long i,amount;HDC hdc;PALETTEENTRY pal[256];
hdc=GetDC(mainwindow);
amount=GetSystemPaletteEntries(hdc,0,256,&pal);
ReleaseDC(mainwindow,hdc);
if (!amount) return FALSE;
for(i=0;i<10;i++)
	{
	 lights[i*3]=pal[i].peRed/4;
	 lights[i*3+1]=pal[i].peGreen/4;
	 lights[i*3+2]=pal[i].peBlue/4;
	 }
for(i=246;i<256;i++)
	{
	 lights[i*3]=pal[i].peRed/4;
	 lights[i*3+1]=pal[i].peGreen/4;
	 lights[i*3+2]=pal[i].peBlue/4;
	 }
return TRUE;
}

BOOL load_palette_for_windows(unsigned char s[]) {
//Load in the palette and copy the system colors in it
FILE *fi;
if ((fi=fopen(s,"rb"))!=NULL)
	{fread(lights,768,1,fi);
	 fclose(fi);
	 if (copy_system_pal())
		return TRUE;
	 }
return FALSE;
}

IDirectDrawPalette *make_ddraw_palette(IDirectDraw *pdd)
{
//Convert the game's palette that's in lights to ddraw format
IDirectDrawPalette* ddpal;PALETTEENTRY ape[256];long i;
for(i=0;i<256;i++)
		{ape[i].peRed=lights[i*3]*4;
		 ape[i].peGreen=lights[i*3+1]*4;
		 ape[i].peBlue=lights[i*3+2]*4;
		 ape[i].peFlags=0;
		}
if (pdd->lpVtbl->CreatePalette(pdd,DDPCAPS_8BIT | DDPCAPS_ALLOW256 | DDPCAPS_INITIALIZE, ape, &ddpal, NULL)>DD_OK)
	return NULL;
return ddpal;
}


BOOL set_ddraw_palette(void) {
//Set the ddraw palette
long ret;
if (lpddpal)
	{if ((ret=lpddsprimary->lpVtbl->SetPalette(lpddsprimary,lpddpal))>DD_OK)
		{if(ret==DDERR_SURFACELOST)
			{lpddsprimary->lpVtbl->Restore(lpddsprimary);
			 if ((ret=lpddsprimary->lpVtbl->SetPalette(lpddsprimary,lpddpal))>DD_OK)
				return FALSE;
			 return TRUE;
			 }
		}
	 else
		return TRUE;
	}
else
	{
	 lpddpal=make_ddraw_palette(lpdd);
	 if (lpddpal)
		{if ((ret=lpddsprimary->lpVtbl->SetPalette(lpddsprimary,lpddpal))>DD_OK)
			{if(ret==DDERR_SURFACELOST)
				{lpddsprimary->lpVtbl->Restore(lpddsprimary);
				  if ((ret=lpddsprimary->lpVtbl->SetPalette(lpddsprimary,lpddpal))>DD_OK)
					return FALSE;
				  return TRUE;
				  }
			}
		 else
			return TRUE;
		 }
	 }
return FALSE;
}

BOOL init_game_palette(void) {
//Initialize the palette
if ((lights=malloc(768))==NULL)
	return FALSE;
if (!load_palette_for_windows("static\\gamepal.pal"))
	{MessageBeep(0xffffffff);
	 return FALSE;
	 }
if (!set_ddraw_palette())
	{
	 return FALSE;
	 }
return TRUE;
}


BOOL read_color_tables(void){
long i=0;FILE *fp=fopen("static\\trans.000","rb");
if (fp==NULL)
	return FALSE;
if ((look_up_pal=malloc(NR_OF_COLOR_TABLES*sizeof(unsigned char*)))==NULL)
	return FALSE;
for (i=0;i<NR_OF_COLOR_TABLES;i++)
	{
	if ((look_up_pal[i]=malloc(256*sizeof(unsigned char)))==NULL)
		return FALSE;
	fread(look_up_pal[i],256,1,fp);
	}
fclose(fp);
return TRUE;
}




