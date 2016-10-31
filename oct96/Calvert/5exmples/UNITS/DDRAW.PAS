
{*****************************************************************************}
{                                                                             }
{    DirectDraw Import Unit for Delphi32                                      }
{                                                                             }
{    Copyright (c) 1995 Mike Scott All Rights Reserved                        }
{    I've moved the constants into DDRAW.INC - Charlie                  }   
{                                                                             }
{    For support or information, contact Mike Scott at:-                      }
{      Internet   : 100140.2420@compuserve.com                                }
{      CompuServe : 100140,2420                                               }
{      Telephone  : +44 131 467 3267                                          }
{      Snail-mail : 3 East End, West Calder, West Lothian EH55 8AB, UK.       }
{                                                                             }
{    THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,          }
{    EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    }
{    MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE. MICHAEL SCOTT   }
{    CANNOT BE HELD RESPONSIBLE FOR ANY LOSSES, EITHER DIRECT OR INDIRECT,    }
{    OF ANY PARTY MAKING USE OF THIS SOFTWARE. IN MAKING USE OF THIS          }
{    SOFTWARE, YOU AGREE TO BE BOUND BY THE TERMS AND CONDITIONS FOUND IN     }
{    THE ACCOMPANYING WINDOWS HELP FILE AND TO INDEMNIFY MICHAEL SCOTT        }
{    AGAINST ANY ACTION TAKEN AGAINST HIM WITH RESPECT TO YOUR OR YOUR        }
{    CLIENTS USE OF THIS SOFTWARE.                                            }
{                                                                             }
{*****************************************************************************}

unit DDraw;

interface

uses
  Windows, Ole2;

const
  {$INCLUDE DDRAW.INC}

type
  PDDColorKey = ^TDDColorKey ;
  TDDColorKey = record
    dwColorSpaceLowValue  : integer ;
    dwColorSpaceHighValue : integer ;
  end ;

  PDDPixelFormat = ^TDDPixelFormat ;
  TDDPixelFormat = record
    dwSize   : integer ;
    dwFlags  : integer ;
    dwFourCC : integer ;
    case integer of
      0 : ( dwRGBBitCount     : integer ;
            dwRBitMask        : integer ;
            dwGBitMask        : integer ;
            dwBBitMask        : integer ;
            dwRGBAlphaBitMask : integer ) ;
      1 : ( dwYUVBitCount     : integer ;
            dwYBitMask        : integer ;
            dwUBitMask        : integer ;
            dwVBitMask        : integer ;
            dwYUVAlphaBitMask : integer ) ;
      2 : ( dwZBufferBitDepth : integer ) ;
      3 : ( dwAlphaBitDepth   : integer ) ;
  end ;

  PDDSCaps = ^TDDSCaps ;
  TDDSCaps = record
    dwCaps : integer ;
  end ;

  PDDCaps = ^TDDCaps ;
  TDDCaps = record
    dwSize                         : integer ;
    dwCaps                         : integer ;
    dwCaps2                        : integer ;
    dwCKeyCaps                     : integer ;
    dwFXCaps                       : integer ;
    dwFXAlphaCaps                  : integer ;
    dwPalCaps                      : integer ;
    dwSVCaps                       : integer ;
    dwAlphaBltConstBitDepths       : integer ;
    dwAlphaBltPixelBitDepths       : integer ;
    dwAlphaBltSurfaceBitDepths     : integer ;
    dwAlphaOverlayConstBitDepths   : integer ;
    dwAlphaOverlayPixelBitDepths   : integer ;
    dwAlphaOverlaySurfaceBitDepths : integer ;
    dwZBufferBitDepths             : integer ;
    dwVidMemTotal                  : integer ;
    dwVidMemFree                   : integer ;
    dwMaxVisibleOverlays           : integer ;
    dwCurrVisibleOverlays          : integer ;
    dwNumFourCCCodes               : integer ;
    dwAlignBoundarySrc             : integer ;
    dwAlignSizeSrc                 : integer ;
    dwAlignBoundaryDest            : integer ;
    dwAlignSizeDest                : integer ;
    dwAlignStrideAlign             : integer ;
    dwRops                         : array[ 0..DD_ROP_SPACE - 1 ] of integer ;
    DDSCaps	                   : TDDSCaps;
    dwMinOverlayStretch            : integer ;
    dwMaxOverlayStretch            : integer ;
    dwMinLiveVideoStretch          : integer ;
    dwMaxLiveVideoStretch          : integer ;
    dwMinHwCodecStretch            : integer ;
    dwMaxHwCodecStretch            : integer ;
    dwReserved1                    : integer ;
    dwReserved2                    : integer ;
    dwReserved3                    : integer ;
  end ;

  PDDSurfaceDesc = ^TDDSurfaceDesc ;
  TDDSurfaceDesc = record
    dwSize              : DWORD;
    dwFlags             : DWORD;
    dwHeight            : DWORD;
    dwWidth             : DWORD;
    lPitch              : integer;
    dwBackBufferCount   : DWORD;
    dwZBufferBitDepth   : DWORD;
    dwAlphaBitDepth     : DWORD;
    Reserved            : DWORD;
    lpSurface           : pointer;
    ddckCKDestOverlay   : TDDColorKey;
    ddckCKDestBlt       : TDDColorKey;
    ddckCKSrcOverlay    : TDDColorKey;
    ddckCKSrcBlt        : TDDColorKey;
    ddpfPIXELFORMAT     : TDDPixelFormat;
    DDSCaps	        : TDDSCaps;
  end ;

  PDDModeDesc = ^TDDModeDesc ;
  TDDModeDesc = record
    dwSize             : integer ;         { size of structure }
    dwFlags            : integer ;         { enumeration flags }
    dwMonitorFrequency : integer ;         { frequency of monitor in mode }
    dsdSurfaceDesc     : TDDSurfaceDesc ;  { surface being enumerated }
  end ;

  PRgnDataHeader = ^TRgnDataHeader ;
  TRgnDataHeader = record
    dwSize   : integer ;
    iType    : integer ;
    nCount   : integer ;
    nRgnSize : integer ;
    rcBound  : TRect ;
  end ;

  PRgnData = ^TRgnData ;
  TRgnData = record
    rdh    : TRgnDataHeader ;
    Buffer : array[ 0..0 ] of byte ;
  end ;

  IDirectDrawSurface = class ;

  PDDBltFx = ^TDDBltFx ;
  TDDBltFx = record
    dwSize               : integer ;
    dwDDFX               : integer ;
    dwROP                : integer ;
    dwDDROP              : integer ;
    dwRotationAngle      : integer ;
    dwZBufferOpCode      : integer ;
    dwZBufferLow         : integer ;
    dwZBufferHigh        : integer ;
    dwZBufferBaseDest    : integer ;
    dwZDestConstBitDepth : integer ;
    case integer of
      0 : ( dwZDestConst     : integer ;
            dwZSrcConstBitDepth : integer ;
            dwZSrcConst      : integer ;
            dwAlphaEdgeBlendBitDepth : integer ;
            dwAlphaEdgeBlend         : integer ;
            dwReserved               : integer ;
            dwAlphaDestConstBitDepth : integer ;
            wAlphaDestConst          : integer ;
            dwAlphaSrcConstBitDepth  : integer ;
            dwAlphaSrcConst          : integer ;
            dwFillColor              : integer ;
            dckDestColorkey          : TDDColorKey ;
            dckSrcColorkey           : TDDColorKey ) ;
      1 : ( lpDDSZBufferDest : IDirectDrawSurface ;
            Filler1          : integer ;
            lpDDSZBufferSrc  : IDirectDrawSurface ;
            Filler2          : array[ 1..4 ] of integer ;
            lpDDSAlphaDest   : IDirectDrawSurface ;
            Filler3          : integer ;
            lpDDSAlphaSrc    : IDirectDrawSurface ;
            lpDDSPattern     : IDirectDrawSurface ) ;
  end ;

  PDDBltBatch = ^TDDBltBatch ;
  TDDBltBatch = record
    lprDest   : PRect ;
    lpDDSSrc  : IDirectDrawSurface ;
    lprSrc    : PRect ;
    dwFlags   : integer ;
    lpDDBltFx : PDDBltFx ;
  end ;

  PDDOverlayFx = ^TDDOverlayFx ;
  TDDOverlayFx = record
    dwSize : integer ;
    dwAlphaEdgeBlendBitDepth : integer ;
    dwAlphaEdgeBlend         : integer ;
    dwReserved               : integer ;
    dwAlphaDestConstBitDepth : integer ;
    case integer of
      0 : ( dwAlphaDestConst        : integer ;
            dwAlphaSrcConstBitDepth : integer ;
            dwAlphaSrcConst         : integer ;
            dckDestColorkey         : TDDColorKey ;
            dckSrcColorkey          : TDDColorKey ;
            dwDDFX                  : integer ;
            dwFlags                 : integer ) ;
      1 : ( lpDDSAlphaDest   : IDirectDrawSurface ;
            Filler1          : integer ;
            lpDDSAlphaSrc    : IDirectDrawSurface ) ;
  end ;

  IDirectDraw = class ;

  IDirectDrawPalette = class( IUnknown )
    function  GetCaps( var Caps : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetEntries( Flags : integer ;
                          Base  : integer ;
                          NumEntries : integer ;
                          var Entries ) : HResult ; virtual ; stdcall ; abstract ;
    function  Initialize( DirectDraw : IDirectDraw ;
                          Flags      : integer ;
                          var ColorTable ) : HResult ; virtual ; stdcall ; abstract ;
    function  SetEntries( Flags         : integer ;
                          StartingEntry : integer ;
                          Count         : integer ;
                          var Entries ) : HResult ; virtual ; stdcall ; abstract ;
  end ;

  IDirectDrawClipper = class( IUnknown )
    function  GetClipList( const Rect : TRect ;
                           ClipList : PRGNData ;
                           var Size : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetHWnd( var Wnd : HWnd ) : HResult ; virtual ; stdcall ; abstract ;
    function  Initialize( DirectDraw : IDirectDraw ;
                          Flags      : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  IsClipListChanged( var Changed : PBOOL ) : HResult ; virtual ; stdcall ; abstract ;
    function  SetClipList( ClipList : PRGNData ;
                           Flags    : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  SetHWnd(Flags: DWord; Wnd : HWnd ) : HResult ; virtual ; stdcall ; abstract ;
  end ;

  IDirectDrawSurface = class( IUnknown )
    function  AddAttachedSurface( AttachedSurface : IDirectDrawSurface ) : HResult ; virtual ; stdcall ; abstract ;
    function  AddOverlayDirtyRect( const Rect : TRect ) : HResult ; virtual ; stdcall ; abstract ;
    function  Blt( const DestRect : TRect ;
                   SrcSurface   : IDirectDrawSurface ;
                   const SrcRect  : TRect ;
                   Flags        : integer ;
                   DDBltFx      : PDDBltFx ) : HResult ; virtual ; stdcall ; abstract ;
    function  BltBatch( BltBatch : PDDBltBatch ;
                        Count    : integer ;
                        Flags    : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  BltFast( x             : integer ;
                       y             : integer ;
                       SrcSurface    : IDirectDrawSurface ;
                       const SrcRect : TRect ;
                       Trans         : integer) : HResult ; virtual ; stdcall ; abstract ;
    function  DeleteAttachedSurface( Flags           : integer ;
                                     AttachedSurface : IDirectDrawSurface ) : HResult ; virtual ; stdcall ; abstract ;
    function  EnumAttachedSurfaces( Context              : pointer ;
                                    EnumSurfacesCallback : TFarProc ) : HResult ; virtual ; stdcall ; abstract ;
    function  EnumOverlayZOrders( Flags : integer ;
                                  Context : pointer ;
                                  EnumSurfacesCallback : TFarProc ) : HResult ; virtual ; stdcall ; abstract ;
    function  Flip( SurfaceTargetOverride : IDirectDrawSurface ;
                    Flags                 : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetAttachedSurface( var DDSCaps         : TDDSCaps ;
                                  var AttachedSurface : IDirectDrawSurface ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetBltStatus( Flags : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetCaps( var DDSCaps    : TDDSCaps ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetClipper( var Clipper : IDirectDrawClipper ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetColorKey( Flags          : integer ;
                           var ColorKey   : TDDColorKey ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetDC( var DC : HDC ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetFlipStatus( Flags : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetOverlayPosition( var X : integer ;
                                  var Y : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetPalette( var DirectDrawPalette : IDirectDrawPalette ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetPixelFormat( PixelFormat : PDDPixelFormat ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetSurfaceDesc(var SurfaceDesc : TDDSurfaceDesc) : HResult ; virtual ; stdcall ; abstract ;
    function  Initialize( DirectDraw      : IDirectDraw ;
                          var SurfaceDesc : PDDSurfaceDesc ) : HResult ; virtual ; stdcall ; abstract ;
    function  IsLost : HResult ; virtual ; stdcall ; abstract ;
    function  Lock( DestRect     : PRect ;
                    SurfaceDesc  : PDDSurfaceDesc ;
                    Flags        : integer ;
                    Event        : THandle ) : HResult ; virtual ; stdcall ; abstract ;
    function  ReleaseDC( DC : HDC ) : HResult ; virtual ; stdcall ; abstract ;
    function  Restore : HResult ; virtual ; stdcall ; abstract ;
    function  SetClipper( DirectDrawClipper : IDirectDrawClipper ) : HResult ; virtual ; stdcall ; abstract ;
    function  SetColorKey( Flags          : integer ;
                           var ColorKey   : TDDColorKey ) : HResult ; virtual ; stdcall ; abstract ;
    function  SetOverlayPosition( x : integer ;
                                  y : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  SetPalette( DirectDrawPalette : IDirectDrawPalette ) : HResult ; virtual ; stdcall ; abstract ;
    function  Unlock( SurfaceData : pointer ) : HResult ; virtual ; stdcall ; abstract ;
    function  UpdateOverlay( const SrcRect  : TRect ;
                             DestSurface    : IDirectDrawSurface ;
                             const DestRect : TRect ;
                             Flags          : integer ;
                             OverlayFx      : PDDOverlayFx ) : HResult ; virtual ; stdcall ; abstract ;
    function  UpdateOverlayDisplay( Flags : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  UpdateOverlayZOrder( Flags            : integer ;
                                   ReferenceSurface : IDirectDrawSurface ) : HResult ; virtual ; stdcall ; abstract ;
  end ;

  IDirectDraw = class( IUnknown )
    function  Compact : HResult ; virtual ; stdcall ; abstract ;
    function  CreateClipper( Flags: DWord;
                             var DirectDrawClipper : IDirectDrawClipper ;
                             unk                   : IUnknown ) : HResult ; virtual ; stdcall ; abstract ;
    function  CreatePalette( Flags                 : integer ;
                             PaletteEntry          : PPaletteEntry ;
                             var DirectDrawPalette : IDirectDrawPalette ;
                             unk                   : IUnknown ) : HResult ; virtual ; stdcall ; abstract ;
    function  CreateSurface( var SurfaceDesc : TDDSurfaceDesc ;
                             var Surface     : IDirectDrawSurface ;
                             unk             : IUnknown ) : HResult ; virtual ; stdcall ; abstract ;
    function  DuplicateSurface( var DDSurface : IDirectDrawSurface ) : HResult ; virtual ; stdcall ; abstract ;
    function  EnumDisplayModes( Flags         : integer ;
                                DDSurfaceDesc : PDDSurfaceDesc ;
                                Context       : pointer ;
                                CallbackProc  : TFarProc ) : HResult ; virtual ; stdcall ; abstract ;
    function  EnumSurfaces( Flags         : integer ;
                            DDSurfaceDesc : PDDSurfaceDesc ;
                            Context       : pointer ;
                            CallbackProc  : TFarProc ) : HResult ; virtual ; stdcall ; abstract ;
    function  FlipToGDISurface : HResult ; virtual ; stdcall ; abstract ;
    function  GetCaps( var DDDriverCaps : TDDCaps ;
                       var DDHELCaps    : TDDCaps ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetDisplayMode( DDSurfaceDesc : PDDSurfaceDesc ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetFourCCCodes( var NumCodes : integer ;
                              Codes        : pointer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetGDISurface( var DDSurface : IDirectDrawSurface ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetMonitorFrequency( var Frequency : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetScanLine( var ScanLine : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  GetVerticalBlankStatus( var IsInVerticalBlank : WordBool ) : HResult ; virtual ; stdcall ; abstract ;
    function  Initialize( var GUID : TGuid ) : HResult ; virtual ; stdcall ; abstract ;
    function  RestoreDisplayMode : HResult ; virtual ; stdcall ; abstract ;
    function  SetCooperativeLevel( Wnd   : HWnd ;
                                   Flags : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  SetDisplayMode( Width        : integer ;
                              Height       : integer ;
                              BitsPerPixel : integer ) : HResult ; virtual ; stdcall ; abstract ;
    function  WaitForVerticalBlank( Flags  : integer ;
                                    Handle : THandle ) : HResult ; virtual ; stdcall ; abstract ;
  end ;

function  DirectDrawCreate( Guid           : PGuid ;
                            var DirectDraw : IDirectDraw ;
                            UnkOuter       : IUnknown ) : HResult ; stdcall ; 

implementation

function  DirectDrawCreate; external 'ddraw.dll' name 'DirectDrawCreate';

end.
