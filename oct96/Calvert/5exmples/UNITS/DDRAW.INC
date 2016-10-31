  (*
   * ddsCaps field is valid.
   *)
  DDSD_CAPS = $00000000 ;     { default }

  (*
   * dwHeight field is valid.
   *)
  DDSD_HEIGHT = $00000002 ;

  (*
   * dwWidth field is valid.
   *)
  DDSD_WIDTH = $00000004 ;

  (*
   * lPitch is valid.
   *)
  DDSD_PITCH = $00000008 ;

  (*
   * dwBackBufferCount is valid.
   *)
  DDSD_BACKBUFFERCOUNT = $00000020 ;

  (*
   * dwZBufferBitDepth is valid.
   *)
  DDSD_ZBUFFERBITDEPTH = $00000040 ;

  (*
   * dwAlphaBitDepth is valid.
   *)
  DDSD_ALPHABITDEPTH = $00000080 ;


  (*
   * lpSurface is valid.
   *)
  DDSD_LPSURFACE = $00000800 ;

  (*
   * ddpfPixelFormat is valid.
   *)
  DDSD_PIXELFORMAT = $00001000 ;

  (*
   * ddckCKDestOverlay is valid.
   *)
  DDSD_CKDESTOVERLAY = $00002000 ;

  (*
   * ddckCKDestBlt is valid.
   *)
  DDSD_CKDESTBLT = $00004000 ;

  (*
   * ddckCKSrcOverlay is valid.
   *)
  DDSD_CKSRCOVERLAY = $00008000 ;

  (*
   * ddckCKSrcBlt is valid.
   *)
  DDSD_CKSRCBLT = $00010000 ;

  (*
   * All input fields are valid.
   *)
  DDSD_ALL = $0001f9ee ;

  (*============================================================================
   *
   * Direct Draw Capability Flags
   *
   * These flags are used to describe the capabilities of a given Surface.
   * All flags are bit flags.
   *
   *==========================================================================*)

  (****************************************************************************
   *
   * DIRECTDRAWSURFACE CAPABILITY FLAGS
   *
   ****************************************************************************)
  (*
   * Indicates that this surface is a front buffer, back buffer, or
   * texture map that is being used in conjunction with a 3DDDI or
   * Direct3D HAL.
   *)
  DDSCAPS_3D = $00000001 ;

  (*
   * Indicates that this surface contains alpha information.  The pixe ;
   * format must be interrogated to determine whether this surface
   * contains only alpha information or alpha information interlaced
   * with pixel color data (e.g. RGBA or YUVA).
   *)
  DDSCAPS_ALPHA = $00000002 ;

  (*
   * Indicates that this surface is a backbuffer.  It is generally
   * set by CreateSurface when the DDSCAPS_FLIP capability bit is set.f
   * It indicates that this surface is THE back buffer of a surface
   * flipping structure.  DirectDraw supports N surfaces in a
   * surface flipping structure.  Only the surface that immediately
   * precedeces the DDSCAPS_FRONTBUFFER has this capability bit set.
   * The other surfaces are identified as back buffers by the presence
   * of the DDSCAPS_FLIP capability, their attachment order, and the
   * absence of the DDSCAPS_FRONTBUFFER and DDSCAPS_BACKBUFFER
   * capabilities.  The bit is sent to CreateSurface when a standalone
   * back buffer is being created.  This surface could be attached to
   * a front buffer and/or back buffers to form a flipping surface
   * structure after the CreateSurface call.  See AddAttachments for
   * a detailed description of the behaviors in this case.
   *)
  DDSCAPS_BACKBUFFER = $00000004 ;

  (*
   * Indicates a complex surface structure is being described.  A
   * complex surface structure results in the creation of more than
   * one surface.  The additional surfaces are attached to the root
   * surface.  The complex structure can only be destroyed by
   * destroying the root.
   *)
  DDSCAPS_COMPLEX = $00000008 ;

  (*
   * Indicates that this surface is a part of a surface flipping structure.
   * When it is passed to CreateSurface the DDSCAPS_FRONTBUFFER and
   * DDSCAP_BACKBUFFER bits are not set.  They are set by CreateSurface
   * on the resulting creations.  The dwBackBufferCount field in the
   * DDSURFACEDESC structure must be set to at least 1 in order for
   * the CreateSurface call to succeed.  The DDSCAPS_COMPLEX capability
   * must always be set with creating multiple surfaces through CreateSurface.
   *)
  DDSCAPS_FLIP = $00000010 ;

  (*
   * Indicates that this surface is THE front buffer of a surface flipping
   * structure.  It is generally set by CreateSurface when the DDSCAPS_FLIP
   * capability bit is set.
   * If this capability is sent to CreateSurface then a standalonw front buffer
   * is created.  This surface will not have the DDSCAPS_FLIP capability.
   * It can be attached to other back buffers to form a flipping structure.
   * See AddAttachments for a detailed description of the behaviors in this
   * case.
   *)
  DDSCAPS_FRONTBUFFER = $00000020 ;

  (*
   * Indicates that this surface is any offscreen surface that is not an overlay,
   * texture, zbuffer, front buffer, back buffer, or alpha surface.  It is used
   * to identify plain vanilla surfaces.
   *)
  DDSCAPS_OFFSCREENPLAIN = $00000040 ;

  (*
   * Indicates that this surface is an overlay.  It may or may not be directly visible
   * depending on whether or not it is currently being overlayed onto the primary
   * surface.  DDSCAPS_VISIBLE can be used to determine whether or not it is being
   * overlayed at the moment.
   *)
  DDSCAPS_OVERLAY = $00000080 ;

  (*
   * Indicates that unique DirectDrawPalette objects can be created and
   * attached to this surface.
   *)
  DDSCAPS_PALETTE = $00000100 ;

  (*
   * Indicates that this surface is the primary surface.  The primary
   * surface represents what the user is seeing at the moment.
   *)
  DDSCAPS_PRIMARYSURFACE = $00000200 ;

  (*
   * Indicates that this surface is the primary surface for the left eye.
   * The primary surface for the left eye represents what the user is seeing
   * at the moment with the users left eye.  When this surface is created the
   * DDSCAPS_PRIMARYSURFACE represents what the user is seeing with the users
   * right eye.
   *)
  DDSCAPS_PRIMARYSURFACELEFT = $00000400 ;

  (*
   * Indicates that this surface memory was allocated in system memory
   *)
  DDSCAPS_SYSTEMMEMORY = $00000800 ;

  (*
   * Indicates that this surface can be used as a 3D texture.  It does not
   * indicate whether or not the surface is being used for that purpose.
   *)
  DDSCAPS_TEXTUREMAP = $00001000 ;

  (*
   * Indicates that this surface exists in video memory.
   *)
  DDSCAPS_VIDEOMEMORY = $00004000 ;

  (*
   * Indicates that changes made to this surface are immediately visible.
   * It is always set for the primary surface and is set for overlays while
   * they are being overlayed and texture maps while they are being textured.
   *)
  DDSCAPS_VISIBLE = $00008000 ;

  (*
   * Indicates that only writes are permitted to the surface.  Read accesses
   * from the surface may or may not generate a protection fault, but the
   * results of a read from this surface will not be meaningful.  READ ONLY.
   *)
  DDSCAPS_WRITEONLY = $00010000 ;

  (*
   * Indicates that this surface is the z buffer.  The z buffer does not contain
   * displayable information.  Instead it contains bit depth information that is
   * used to determine which pixels are visible and which are obscured.
   *)
  DDSCAPS_ZBUFFER = $00020000 ;

  (*
   * Indicates surface will have a DC associated long term
   *)
  DDSCAPS_OWNDC = $00040000 ;

  (*
   * Indicates surface should be able to receive live video
   *)
  DDSCAPS_LIVEVIDEO = $00080000 ;

  (*
   * Indicates surface should be able to have a stream decompressed
   * to it by the hardware.
   *)
  DDSCAPS_HWCODEC = $00100000 ;

  (*
   * Surface is a 32 = $200 or 32 = $240 ModeX surface
   *)
  DDSCAPS_MODEX = $00200000 ;

  (****************************************************************************
   *
   * DIRECTDRAW DRIVER CAPABILITY FLAGS
   *
   ****************************************************************************)

  (*
   * Display hardware has 3D acceleration.
   *)
  DDCAPS_3D = $00000001 ;

  (*
   * Indicates that DirectDraw will support only dest rectangles that are aligned
   * on DIRECTDRAWCAPS.dwAlignBoundaryDest boundaries of the surface, respectively.
   * READ ONLY.
   *)
  DDCAPS_ALIGNBOUNDARYDEST = $00000002 ;

  (*
   * Indicates that DirectDraw will support only source rectangles  whose sizes in
   * BYTEs are DIRECTDRAWCAPS.dwAlignSizeDest multiples, respectively.  READ ONLY.
   *)
  DDCAPS_ALIGNSIZEDEST = $00000004 ;
  (*
   * Indicates that DirectDraw will support only source rectangles that are aligned
   * on DIRECTDRAWCAPS.dwAlignBoundarySrc boundaries of the surface, respectively.
   * READ ONLY.
   *)
  DDCAPS_ALIGNBOUNDARYSRC = $00000008 ;

  (*
   * Indicates that DirectDraw will support only source rectangles  whose sizes in
   * BYTEs are DIRECTDRAWCAPS.dwAlignSizeSrc multiples, respectively.  READ ONLY.
   *)
  DDCAPS_ALIGNSIZESRC = $00000010 ;

  (*
   * Indicates that DirectDraw will create video memory surfaces that have a stride 
   * alignment equal to DIRECTDRAWCAPS.dwAlignStride.  READ ONLY.
   *)
  DDCAPS_ALIGNSTRIDE = $00000020 ;

  (*
   * Display hardware is capable of blt operations.
   *)
  DDCAPS_BLT = $00000040 ;

  (*
   * Display hardware is capable of asynchronous blt operations.
   *)
  DDCAPS_BLTQUEUE = $00000080 ;

  (*
   * Display hardware is capable of color space conversions during the blt operation.
   *)
  DDCAPS_BLTFOURCC = $00000100 ;

  (*
   * Display hardware is capable of stretching during blt operations.
   *)
  DDCAPS_BLTSTRETCH = $00000200 ;

  (*
   * Display hardware is shared with GDI.
   *)
  DDCAPS_GDI = $00000400 ;

  (*
   * Display hardware can overlay.
   *)
  DDCAPS_OVERLAY = $00000800 ;

  (*
   * Set if display hardware supports overlays but can not clip them.
   *)
  DDCAPS_OVERLAYCANTCLIP = $00001000 ;

  (*
   * Indicates that overlay hardware is capable of color space conversions during
   * the overlay operation.
   *)
  DDCAPS_OVERLAYFOURCC = $00002000 ;

  (*
   * Indicates that stretching can be done by the overlay hardware.
   *)
  DDCAPS_OVERLAYSTRETCH = $00004000 ;

  (*
   * Indicates that unique DirectDrawPalettes can be created for DirectDrawSurfaces
   * other than the primary surface.
   *)
  DDCAPS_PALETTE = $00008000 ;

  (*
   * Indicates that palette changes can be syncd with the veritcal refresh.
   *)
  DDCAPS_PALETTEVSYNC = $00010000 ;

  (*
   * Display hardware can return the current scan line.
   *)
  DDCAPS_READSCANLINE = $00020000 ;

  (*
   * Display hardware has stereo vision capabilities.  DDSCAPS_PRIMARYSURFACELEFT 
   * can be created.
   *)
  DDCAPS_STEREOVIEW = $00040000 ;

  (*
   * Display hardware is capable of generating a vertical blank interrupt.
   *)
  DDCAPS_VBI = $00080000 ;

  (*
   * Supports the use of z buffers with blt operations.
   *)
  DDCAPS_ZBLTS = $00100000 ;

  (*
   * Supports Z Ordering of overlays.
   *)
  DDCAPS_ZOVERLAYS = $00200000 ;

  (*
   * Supports color key
   *)
  DDCAPS_COLORKEY = $00400000 ;

  (*
   * Supports alpha surfaces
   *)
  DDCAPS_ALPHA = $00800000 ;

  (*
   * colorkey is hardware assisted(DDCAPS_COLORKEY will also be set)
   *)
  DDCAPS_COLORKEYHWASSIST = $01000000 ;

  (*
   * no hardware support at al ;
   *)
  DDCAPS_NOHARDWARE = $02000000 ;

  (*
   * Display hardware is capable of color fill with bltter
   *)
  DDCAPS_BLTCOLORFILL = $04000000 ;

  (*
   * Display hardware is bank switched, and potentially very slow at
   * random access to VRAM.
   *)
  DDCAPS_BANKSWITCHED = $08000000 ;

  (****************************************************************************
   *
   * MORE DIRECTDRAW DRIVER CAPABILITY FLAGS (dwCaps2)
   *
   ****************************************************************************)

  (*
   * Display hardware is certified
   *)
  DDCAPS2_CERTIFIED = $00000001 ;

  (****************************************************************************
   *
   * DIRECTDRAW FX ALPHA CAPABILITY FLAGS
   *
   ****************************************************************************)

  (*
   * Supports alpha blending around the edge of a source color keyed surface.
   * For Blt.
   *)
  DDFXALPHACAPS_BLTALPHAEDGEBLEND = $00000001 ;

  (*
   * Supports alpha information in the pixel format.  The bit depth of alpha
   * information in the pixel format can be 1,2,4, or 8.  The alpha value becomes
   * more opaque as the alpha value increases.  (0 is transparent.)
   * For Blt.
   *)
  DDFXALPHACAPS_BLTALPHAPIXELS = $00000002 ;

  (*
   * Supports alpha information in the pixel format.  The bit depth of alpha 
   * information in the pixel format can be 1,2,4, or 8.  The alpha value 
   * becomes more transparent as the alpha value increases.  (0 is opaque.) 
   * This flag can only be set if DDCAPS_ALPHA is set.
   * For Blt.
   *)
  DDFXALPHACAPS_BLTALPHAPIXELSNEG = $00000004 ;

  (*
   * Supports alpha only surfaces.  The bit depth of an alpha only surface can be
   * 1,2,4, or 8.  The alpha value becomes more opaque as the alpha value increases.
   * (0 is transparent.)
   * For Blt.
   *)
  DDFXALPHACAPS_BLTALPHASURFACES = $00000008 ;

  (*
   * The depth of the alpha channel data can range can be 1,2,4, or 8.  
   * The NEG suffix indicates that this alpha channel becomes more transparent
   * as the alpha value increases. (0 is opaque.)  This flag can only be set if
   * DDCAPS_ALPHA is set.
   * For Blt.
   *)
  DDFXALPHACAPS_BLTALPHASURFACESNEG = $00000010 ;

  (*
   * Supports alpha blending around the edge of a source color keyed surface.
   * For Overlays.
   *)
  DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND = $00000020 ;

  (*
   * Supports alpha information in the pixel format.  The bit depth of alpha
   * information in the pixel format can be 1,2,4, or 8.  The alpha value becomes
   * more opaque as the alpha value increases.  (0 is transparent.)
   * For Overlays.
   *)
  DDFXALPHACAPS_OVERLAYALPHAPIXELS = $00000040 ;

  (*
   * Supports alpha information in the pixel format.  The bit depth of alpha 
   * information in the pixel format can be 1,2,4, or 8.  The alpha value 
   * becomes more transparent as the alpha value increases.  (0 is opaque.)
   * This flag can only be set if DDCAPS_ALPHA is set.
   * For Overlays.
   *)
  DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG = $00000080 ;

  (*
   * Supports alpha only surfaces.  The bit depth of an alpha only surface can be
   * 1,2,4, or 8.  The alpha value becomes more opaque as the alpha value increases.
   * (0 is transparent.)
   * For Overlays.
   *)
  DDFXALPHACAPS_OVERLAYALPHASURFACES = $00000100 ;

  (*
   * The depth of the alpha channel data can range can be 1,2,4, or 8.  
   * The NEG suffix indicates that this alpha channel becomes more transparent
   * as the alpha value increases. (0 is opaque.)  This flag can only be set if
   * DDCAPS_ALPHA is set.
   * For Overlays.
   *)
  DDFXALPHACAPS_OVERLAYALPHASURFACESNEG = $00000200 ;

  (****************************************************************************
   *
   * DIRECTDRAW FX CAPABILITY FLAGS
   *
   ****************************************************************************)

  (*
   * Uses arithmetic operations to stretch and shrink surfaces during blt
   * rather than pixel doubling techniques.  Along the Y axis.
   *)
  DDFXCAPS_BLTARITHSTRETCHY = $00000020 ;

  (*
   * Uses arithmetic operations to stretch during blt
   * rather than pixel doubling techniques.  Along the Y axis. Only
   * works for x1, x2, etc.
   *)
  DDFXCAPS_BLTARITHSTRETCHYN = $00000010 ;

  (*
   * Supports mirroring left to right in blt.
   *)
  DDFXCAPS_BLTMIRRORLEFTRIGHT = $00000040 ;

  (*
   * Supports mirroring top to bottom in blt.
   *)
  DDFXCAPS_BLTMIRRORUPDOWN = $00000080 ;

  (*
   * Supports arbitrary rotation for blts.
   *)
  DDFXCAPS_BLTROTATION = $00000100 ;

  (*
   * Supports 90 degree rotations for blts.
   *)
  DDFXCAPS_BLTROTATION90 = $00000200 ;

  (*
   * DirectDraw supports arbitrary shrinking of a surface along the
   * x axis (horizontal direction) for blts.
   *)
  DDFXCAPS_BLTSHRINKX = $00000400 ;

  (*
   * DirectDraw supports integer shrinking (1x,2x,) of a surface
   * along the x axis (horizontal direction) for blts.
   *)
  DDFXCAPS_BLTSHRINKXN = $00000800 ;

  (*
   * DirectDraw supports arbitrary shrinking of a surface along the
   * y axis (horizontal direction) for blts.  
   *)
  DDFXCAPS_BLTSHRINKY = $00001000 ;

  (*
   * DirectDraw supports integer shrinking (1x,2x,) of a surface
   * along the y axis (vertical direction) for blts.  
   *)
  DDFXCAPS_BLTSHRINKYN = $00002000 ;

  (*
   * DirectDraw supports arbitrary stretching of a surface along the
   * x axis (horizontal direction) for blts.
   *)
  DDFXCAPS_BLTSTRETCHX = $00004000 ;

  (*
   * DirectDraw supports integer stretching (1x,2x,) of a surface
   * along the x axis (horizontal direction) for blts.
   *)
  DDFXCAPS_BLTSTRETCHXN = $00008000 ;

  (*
   * DirectDraw supports arbitrary stretching of a surface along the
   * y axis (horizontal direction) for blts.
   *)
  DDFXCAPS_BLTSTRETCHY = $00010000 ;

  (*
   * DirectDraw supports integer stretching (1x,2x,) of a surface
   * along the y axis (vertical direction) for blts.  
   *)
  DDFXCAPS_BLTSTRETCHYN = $00020000 ;

  (*
   * Uses arithmetic operations to stretch and shrink surfaces during
   * overlay rather than pixel doubling techniques.  Along the Y axis 
   * for overlays.
   *)
  DDFXCAPS_OVERLAYARITHSTRETCHY = $00040000 ;

  (*
   * Uses arithmetic operations to stretch surfaces during 
   * overlay rather than pixel doubling techniques.  Along the Y axis 
   * for overlays. Only works for x1, x2, etc.
   *)
  DDFXCAPS_OVERLAYARITHSTRETCHYN = $00000008 ;

  (*
   * DirectDraw supports arbitrary shrinking of a surface along the
   * x axis (horizontal direction) for overlays.
   *)
  DDFXCAPS_OVERLAYSHRINKX = $00080000 ;

  (*
   * DirectDraw supports integer shrinking (1x,2x,) of a surface
   * along the x axis (horizontal direction) for overlays.
   *)
  DDFXCAPS_OVERLAYSHRINKXN = $00100000 ;

  (*
   * DirectDraw supports arbitrary shrinking of a surface along the
   * y axis (horizontal direction) for overlays.
   *)
  DDFXCAPS_OVERLAYSHRINKY = $00200000 ;

  (*
   * DirectDraw supports integer shrinking (1x,2x,) of a surface
   * along the y axis (vertical direction) for overlays.  
   *)
  DDFXCAPS_OVERLAYSHRINKYN = $00400000 ;

  (*
   * DirectDraw supports arbitrary stretching of a surface along the
   * x axis (horizontal direction) for overlays.
   *)
  DDFXCAPS_OVERLAYSTRETCHX = $00800000 ;

  (*
   * DirectDraw supports integer stretching (1x,2x,) of a surface
   * along the x axis (horizontal direction) for overlays.
   *)
  DDFXCAPS_OVERLAYSTRETCHXN = $01000000 ;

  (*
   * DirectDraw supports arbitrary stretching of a surface along the
   * y axis (horizontal direction) for overlays.  
   *)
  DDFXCAPS_OVERLAYSTRETCHY = $02000000 ;

  (*
   * DirectDraw supports integer stretching (1x,2x,) of a surface
   * along the y axis (vertical direction) for overlays.  
   *)
  DDFXCAPS_OVERLAYSTRETCHYN = $04000000 ;

  (*
   * DirectDraw supports mirroring of overlays across the vertical axis
   *)
  DDFXCAPS_OVERLAYMIRRORLEFTRIGHT = $08000000 ;

  (*
   * DirectDraw supports mirroring of overlays across the horizontal axis
   *)
  DDFXCAPS_OVERLAYMIRRORUPDOWN = $10000000 ;

  (****************************************************************************
   *
   * DIRECTDRAW STEREO VIEW CAPABILITIES
   *
   ****************************************************************************)

  (*
   * The stereo view is accomplished via enigma encoding.
   *)
  DDSVCAPS_ENIGMA = $00000001 ;

  (*
   * The stereo view is accomplished via high frequency flickering.
   *)
  DDSVCAPS_FLICKER = $00000002 ;

  (*
   * The stereo view is accomplished via red and blue filters applied
   * to the left and right eyes.  All images must adapt their colorspaces
   * for this process.
   *)
  DDSVCAPS_REDBLUE = $00000004 ;

  (*
   * The stereo view is accomplished with split screen technology.
   *)
  DDSVCAPS_SPLIT = $00000008 ;

  (****************************************************************************
   *
   * DIRECTDRAWPALETTE CAPABILITIES
   *
   ****************************************************************************)

  (*
   * Index is 4 bits.  There are sixteen color entries in the palette table.
   *)
  DDPCAPS_4BIT = $00000001 ;

  (*
   * Index is onto a 8 bit color index.  This field is only valid with the
   * DDPCAPS_4BIT capability and the target surface is in 8bpp. Each color
   * entry is one byte long and is an index into destination surface's 8bpp
   * palette.
   *)
  DDPCAPS_8BITENTRIES = $00000002 ;

  (*
   * Index is 8 bits.  There are 256 color entries in the palette table.
   *)
  DDPCAPS_8BIT = $00000004 ;

  (*
   * Indicates that this DIRECTDRAWPALETTE should use the palette color array
   * passed into the lpDDColorArray parameter to initialize the DIRECTDRAWPALETTE
   * object.
   *)
  DDPCAPS_INITIALIZE = $00000008 ;

  (*
   * This palette is the one attached to the primary surface.  Changing this
   * table has immediate effect on the display unless DDPSETPAL_VSYNC is specified
   * and supported.
   *)
  DDPCAPS_PRIMARYSURFACE = $00000010 ;

  (*
   * This palette is the one attached to the primary surface left.  Changing
   * this table has immediate effect on the display for the left eye unless
   * DDPSETPAL_VSYNC is specified and supported.
   *)
  DDPCAPS_PRIMARYSURFACELEFT = $00000020 ;

  (*
   * This palette can have all 256 entries defined
   *)
  DDPCAPS_ALLOW256 = $00000040 ;

  (*
   * This palette can have modifications to it synced with the monitors
   * refresh rate.
   *)
  DDPCAPS_VSYNC = $00000080 ;


  (****************************************************************************
   *
   * DIRECTDRAWPALETTE SETENTRY CONSTANTS
   *
   ****************************************************************************)

  (*
   * Palette changes take effect immediately.
   *)
  DDPSETPAL_IMMEDIATE = $00000000 ; { default }

  (*
   * Palette changes should take effect during the vertical blank to avoid
   * palette tearing.
   *)
  DDPSETPAL_VSYNC = $00000002 ;

  (****************************************************************************
   *
   * DIRECTDRAWPALETTE GETENTRY CONSTANTS
   *
   ****************************************************************************)

  (* 0 is the only legal value *)

  (****************************************************************************
   *
   * DIRECTDRAWSURFACE SETPALETTE CONSTANTS
   *
   ****************************************************************************)

  (*
   * Palette change takes effect immediately.
   *)
  DDSETPAL_IMMEDIATE = $00000000 ;     { default }

  (*
   * Palette change should take effect during the vertical blank to avoid
   * palette tearing.
   *)
  DDSETPAL_VSYNC = $00000002 ;

  (****************************************************************************
   *
   * DIRECTDRAW BITDEPTH CONSTANTS
   *
   * NOTE:  These are only used to indicate supported bit depths.   These
   * are flags only, they are not to be used as an actual bit depth.   The
   * absolute numbers 1, 2, 4, 8, 16, 24 and 32 are used to indicate actua ;
   * bit depths in a surface or for changing the display mode.
   *
   ****************************************************************************)

  (*
   * 1 bit per pixel.
   *)
  DDBD_1 = $00004000 ;

  (*
   * 2 bits per pixel.
   *)
  DDBD_2 = $00002000 ;

  (*
   * 4 bits per pixel.
   *)
  DDBD_4 = $00001000 ;

  (*
   * 8 bits per pixel.
   *)
  DDBD_8 = $00000800 ;

  (*
   * 16 bits per pixel.
   *)
  DDBD_16 = $00000400 ;

  (*
   * 24 bits per pixel.
   *)
  DDBD_24 = $00000200 ;

  (*
   * 32 bits per pixel.
   *)
  DDBD_32 = $00000100 ;

  (****************************************************************************
   *
   * DIRECTDRAWSURFACE SET/GET COLOR KEY FLAGS
   *
   ****************************************************************************)

  (*
   * Set if the structure contains a color space.  Not set if the structure
   * contains a single color key.
   *)
  DDCKEY_COLORSPACE = $00000001 ;

  (*
   * Set if the structure specifies a color key or color space which is to be
   * used as a destination color key for blt operations.
   *)
  DDCKEY_DESTBLT = $00000002 ;

  (*
   * Set if the structure specifies a color key or color space which is to be
   * used as a destination color key for overlay operations.
   *)
  DDCKEY_DESTOVERLAY = $00000004 ;

  (*
   * Set if the structure specifies a color key or color space which is to be
   * used as a source color key for blt operations.
   *)
  DDCKEY_SRCBLT = $00000008 ;

  (*
   * Set if the structure specifies a color key or color space which is to be
   * used as a source color key for overlay operations.
   *)
  DDCKEY_SRCOVERLAY = $00000010 ;


  (****************************************************************************
   *
   * DIRECTDRAW COLOR KEY CAPABILITY FLAGS
   *
   ****************************************************************************)

  (*
   * Supports transparent blting using a color key to identify the replaceable
   * bits of the destination surface for RGB colors.
   *)
  DDCKEYCAPS_DESTBLT = $00000001 ;

  (*
   * Supports transparent blting using a color space to identify the replaceable
   * bits of the destination surface for RGB colors.
   *)
  DDCKEYCAPS_DESTBLTCLRSPACE = $00000002 ;

  (*
   * Supports transparent blting using a color space to identify the replaceable
   * bits of the destination surface for YUV colors.
   *)
  DDCKEYCAPS_DESTBLTCLRSPACEYUV = $00000004 ;

  (*
   * Supports transparent blting using a color key to identify the replaceable 
   * bits of the destination surface for YUV colors.
   *)
  DDCKEYCAPS_DESTBLTYUV = $00000008 ;

  (*
   * Supports overlaying using colorkeying of the replaceable bits of the surface
   * being overlayed for RGB colors.
   *)
  DDCKEYCAPS_DESTOVERLAY = $00000010 ;

  (*
   * Supports a color space as the color key for the destination for RGB colors.
   *)
  DDCKEYCAPS_DESTOVERLAYCLRSPACE = $00000020 ;

  (*
   * Supports a color space as the color key for the destination for YUV colors.
   *)
  DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV = $00000040 ;

  (*
   * Supports only one active destination color key value for visible overlay
   * surfaces.
   *)
  DDCKEYCAPS_DESTOVERLAYONEACTIVE = $00000080 ;

  (*
   * Supports overlaying using colorkeying of the replaceable bits of the
   * surface being overlayed for YUV colors.
   *)
  DDCKEYCAPS_DESTOVERLAYYUV = $00000100 ;

  (*
   * Supports transparent blting using the color key for the source with
   * this surface for RGB colors.
   *)
  DDCKEYCAPS_SRCBLT = $00000200 ;

  (*
   * Supports transparent blting using a color space for the source with
   * this surface for RGB colors.
   *)
  DDCKEYCAPS_SRCBLTCLRSPACE = $00000400 ;

  (*
   * Supports transparent blting using a color space for the source with
   * this surface for YUV colors.
   *)
  DDCKEYCAPS_SRCBLTCLRSPACEYUV = $00000800 ;

  (*
   * Supports transparent blting using the color key for the source with
   * this surface for YUV colors.
   *)
  DDCKEYCAPS_SRCBLTYUV = $00001000 ;

  (*
   * Supports overlays using the color key for the source with this
   * overlay surface for RGB colors.
   *)
  DDCKEYCAPS_SRCOVERLAY = $00002000 ;

  (*
   * Supports overlays using a color space as the source color key for
   * the overlay surface for RGB colors.
   *)
  DDCKEYCAPS_SRCOVERLAYCLRSPACE = $00004000 ;

  (*
   * Supports overlays using a color space as the source color key for
   * the overlay surface for YUV colors.
   *)
  DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV = $00008000 ;

  (*
   * Supports only one active source color key value for visible
   * overlay surfaces.
   *)
  DDCKEYCAPS_SRCOVERLAYONEACTIVE = $00010000 ;

  (*
   * Supports overlays using the color key for the source with this
   * overlay surface for YUV colors.
   *)
  DDCKEYCAPS_SRCOVERLAYYUV = $00020000 ;

  (*
   * there are no bandwidth trade-offs for using colorkey with an overlay
   *)
  DDCKEYCAPS_NOCOSTOVERLAY = $00040000 ;


  (****************************************************************************
   *
   * DIRECTDRAW PIXELFORMAT FLAGS
   *
   ****************************************************************************)

  (*
   * The surface has alpha channel information in the pixel format.
   *)
  DDPF_ALPHAPIXELS = $00000001 ;

  (*
   * The pixel format contains alpha only information
   *)
  DDPF_ALPHA = $00000002 ;

  (*
   * The FourCC code is valid.
   *)
  DDPF_FOURCC = $00000004 ;

  (*
   * The surface is 4-bit color indexed.
   *)
  DDPF_PALETTEINDEXED4 = $00000008 ;

  (*
   * The surface is 4-bit color indexed to an 8-bit palette.
   *)
  DDPF_PALETTEINDEXED4TO8 = $00000010 ;

  (*
   * The surface is 8-bit color indexed.
   *)
  DDPF_PALETTEINDEXED8 = $00000020 ;

  (*
   * The RGB data in the pixel format structure is valid.
   *)
  DDPF_RGB = $00000040 ;

  (*
   * The surface will accept pixel data in the format specified
   * and compress it during the write.
   *)
  DDPF_COMPRESSED = $00000080 ;

  (*
   * The surface will accept RGB data and translate it during
   * the write to YUV data.  The format of the data to be written
   * will be contained in the pixel format structure.  The DDPF_RGB
   * flag will be set. 
   *)
  DDPF_RGBTOYUV = $00000100 ;

  (*
   * pixel format is YUV - YUV data in pixel format struct is valid
   *)
  DDPF_YUV = $00000200 ;

  (*
   * pixel format is a z buffer only surface
   *)
  DDPF_ZBUFFER = $00000400 ;

  (*===========================================================================
   *
   *
   * DIRECTDRAW CALLBACK FLAGS
   *
   *
   *==========================================================================*)

  (****************************************************************************
   *
   * DIRECTDRAW ENUMSURFACES FLAGS
   *
   ****************************************************************************)

  (*
   * Enumerate all of the surfaces that meet the search criterion.
   *)
  DDENUMSURFACES_ALL = $00000001 ;

  (*
   * A search hit is a surface that matches the surface description.
   *)
  DDENUMSURFACES_MATCH = $00000002 ;

  (*
   * A search hit is a surface that does not match the surface description.
   *)
  DDENUMSURFACES_NOMATCH = $00000004 ;

  (*
   * Enumerate the first surface that can be created which meets the search criterion.
   *)
  DDENUMSURFACES_CANBECREATED = $00000008 ;

  (*
   * Enumerate the surfaces that already exist that meet the search criterion.
   *)
  DDENUMSURFACES_DOESEXIST = $00000010 ;


  (****************************************************************************
   *
   * DIRECTDRAW SETCOOPERATIVELEVEL FLAGS
   *
   ****************************************************************************)

  (*
   * Exclusive mode owner will be responsible for the entire primary surface.
   * GDI can be ignored. used with DD
   *)
  DDSCL_FULLSCREEN = $00000001 ;

  (*
   * allow CTRL_ALT_DEL to work while in fullscreen exclusive mode 
   *)
  DDSCL_ALLOWREBOOT = $00000002 ;

  (*
   * prevents DDRAW from modifying the application window.
   * prevents DDRAW from minimize/restore the application window on activation.
   *)
  DDSCL_NOWINDOWCHANGES = $00000004 ;

  (*
   * app wants to work as a regular Windows application
   *)
  DDSCL_NORMAL = $00000008 ;

  (*
   * app wants exclusive access
   *)
  DDSCL_EXCLUSIVE = $00000010 ;


  (*
   * app can deal with non-windows display modes
   *)
  DDSCL_ALLOWMODEX = $00000040 ;


  (****************************************************************************
   *
   * DIRECTDRAW BLT FLAGS
   *
   ****************************************************************************)

  (*
   * Use the alpha information in the pixel format or the alpha channel surface
   * attached to the destination surface as the alpha channel for this blt.
   *)
  DDBLT_ALPHADEST = $00000001 ;

  (*
   * Use the dwConstAlphaDest field in the DDBLTFX structure as the alpha channe ;
   * for the destination surface for this blt.
   *)
  DDBLT_ALPHADESTCONSTOVERRIDE = $00000002 ;

  (*
   * The NEG suffix indicates that the destination surface becomes more
   * transparent as the alpha value increases. (0 is opaque)
   *)
  DDBLT_ALPHADESTNEG = $00000004 ;

  (*
   * Use the lpDDSAlphaDest field in the DDBLTFX structure as the alpha
   * channel for the destination for this blt.
   *)
  DDBLT_ALPHADESTSURFACEOVERRIDE = $00000008 ;

  (*
   * Use the dwAlphaEdgeBlend field in the DDBLTFX structure as the alpha channe ;
   * for the edges of the image that border the color key colors.
   *)
  DDBLT_ALPHAEDGEBLEND = $00000010 ;

  (*
   * Use the alpha information in the pixel format or the alpha channel surface
   * attached to the source surface as the alpha channel for this blt.
   *)
  DDBLT_ALPHASRC = $00000020 ;

  (*
   * Use the dwConstAlphaSrc field in the DDBLTFX structure as the alpha channe ;
   * for the source for this blt.
   *)
  DDBLT_ALPHASRCCONSTOVERRIDE = $00000040 ;

  (*
   * The NEG suffix indicates that the source surface becomes more transparent
   * as the alpha value increases. (0 is opaque)
   *)
  DDBLT_ALPHASRCNEG = $00000080 ;

  (*
   * Use the lpDDSAlphaSrc field in the DDBLTFX structure as the alpha channe ;
   * for the source for this blt. 
   *)
  DDBLT_ALPHASRCSURFACEOVERRIDE = $00000100 ;

  (*
   * Do this blt asynchronously through the FIFO in the order received.  If
   * there is no room in the hardware FIFO fail the call.
   *)
  DDBLT_ASYNC = $00000200 ;

  (*
   * Uses the dwFillColor field in the DDBLTFX structure as the RGB color
   * to fill the destination rectangle on the destination surface with.
   *)
  DDBLT_COLORFILL = $00000400 ;

  (*
   * Uses the dwDDFX field in the DDBLTFX structure to specify the effects
   * to use for the blt.
   *)
  DDBLT_DDFX = $00000800 ;

  (*
   * Uses the dwDDROPS field in the DDBLTFX structure to specify the ROPS
   * that are not part of the Win32 API.
   *)
  DDBLT_DDROPS = $00001000 ;

  (*
   * Use the color key associated with the destination surface.
   *)
  DDBLT_KEYDEST = $00002000 ;

  (*
   * Use the dckDestColorkey field in the DDBLTFX structure as the color key
   * for the destination surface.
   *)
  DDBLT_KEYDESTOVERRIDE = $00004000 ;

  (*
   * Use the color key associated with the source surface.
   *)
  DDBLT_KEYSRC = $00008000 ;

  (*
   * Use the dckSrcColorkey field in the DDBLTFX structure as the color key
   * for the source surface.
   *)
  DDBLT_KEYSRCOVERRIDE = $00010000 ;

  (*
   * Use the dwROP field in the DDBLTFX structure for the raster operation
   * for this blt.  These ROPs are the same as the ones defined in the Win32 API.
   *)
  DDBLT_ROP = $00020000 ;

  (*
   * Use the dwRotationAngle field in the DDBLTFX structure as the angle
   * (specified in 1/100th of a degree) to rotate the surface.
   *)
  DDBLT_ROTATIONANGLE = $00040000 ;

  (*
   * Z-buffered blt using the z-buffers attached to the source and destination
   * surfaces and the dwZBufferOpCode field in the DDBLTFX structure as the
   * z-buffer opcode.
   *)
  DDBLT_ZBUFFER = $00080000 ;

  (*
   * Z-buffered blt using the dwConstDest Zfield and the dwZBufferOpCode field
   * in the DDBLTFX structure as the z-buffer and z-buffer opcode respectively
   * for the destination.
   *)
  DDBLT_ZBUFFERDESTCONSTOVERRIDE = $00100000 ;

  (*
   * Z-buffered blt using the lpDDSDestZBuffer field and the dwZBufferOpCode
   * field in the DDBLTFX structure as the z-buffer and z-buffer opcode
   * respectively for the destination.
   *)
  DDBLT_ZBUFFERDESTOVERRIDE = $00200000 ;

  (*
   * Z-buffered blt using the dwConstSrcZ field and the dwZBufferOpCode field
   * in the DDBLTFX structure as the z-buffer and z-buffer opcode respectively
   * for the source.
   *)
  DDBLT_ZBUFFERSRCCONSTOVERRIDE = $00400000 ;

  (*
   * Z-buffered blt using the lpDDSSrcZBuffer field and the dwZBufferOpCode
   * field in the DDBLTFX structure as the z-buffer and z-buffer opcode
   * respectively for the source.
   *)
  DDBLT_ZBUFFERSRCOVERRIDE = $00800000 ;

  (*
   * wait until the device is ready to handle the blt
   * this will cause blt to not return DDERR_WASSTILLDRAWING
   *)
  DDBLT_WAIT = $01000000 ;


  (****************************************************************************
   *
   * BLTFAST FLAGS
   *
   ****************************************************************************)

  DDBLTFAST_NOCOLORKEY   = $00000000 ;
  DDBLTFAST_SRCCOLORKEY  = $00000001 ;
  DDBLTFAST_DESTCOLORKEY = $00000002 ;
  DDBLTFAST_WAIT         = $00000010 ;

  (****************************************************************************
   *
   * LOCK FLAGS
   *
   ****************************************************************************)

  DDLOCK_WAIT      = $00000001 ;
  DDLOCK_READONLY  = $00000010 ;  { surface will only be read from }
  DDLOCK_WRITEONLY = $00000020 ;  { surface will only be written to }

  (****************************************************************************
   *
   * FLIP FLAGS
   *
   ****************************************************************************)

  DDFLIP_WAIT = $00000001 ;


  (****************************************************************************
   *
   * DIRECTDRAW SURFACE OVERLAY FLAGS
   *
   ****************************************************************************)

  (*
   * Use the alpha information in the pixel format or the alpha channel surface
   * attached to the destination surface as the alpha channel for the
   * destination overlay.
   *)
  DDOVER_ALPHADEST = $00000001 ;

  (*
   * Use the dwConstAlphaDest field in the DDOVERLAYFX structure as the
   * destination alpha channel for this overlay.
   *)
  DDOVER_ALPHADESTCONSTOVERRIDE = $00000002 ;

  (*
   * The NEG suffix indicates that the destination surface becomes more
   * transparent as the alpha value increases. 
   *)
  DDOVER_ALPHADESTNEG = $00000004 ;

  (*
   * Use the lpDDSAlphaDest field in the DDOVERLAYFX structure as the alpha
   * channel destination for this overlay.
   *)
  DDOVER_ALPHADESTSURFACEOVERRIDE = $00000008 ;

  (*
   * Use the dwAlphaEdgeBlend field in the DDOVERLAYFX structure as the alpha
   * channel for the edges of the image that border the color key colors.
   *)
  DDOVER_ALPHAEDGEBLEND = $00000010 ;

  (*
   * Use the alpha information in the pixel format or the alpha channel surface
   * attached to the source surface as the source alpha channel for this overlay.
   *)
  DDOVER_ALPHASRC = $00000020 ;

  (*
   * Use the dwConstAlphaSrc field in the DDOVERLAYFX structure as the source
   * alpha channel for this overlay.
   *)
  DDOVER_ALPHASRCCONSTOVERRIDE = $00000040 ;

  (*
   * The NEG suffix indicates that the source surface becomes more transparent
   * as the alpha value increases.
   *)
  DDOVER_ALPHASRCNEG = $00000080 ;

  (*
   * Use the lpDDSAlphaSrc field in the DDOVERLAYFX structure as the alpha channe ;
   * source for this overlay.
   *)
  DDOVER_ALPHASRCSURFACEOVERRIDE = $00000100 ;

  (*
   * Turn this overlay off.
   *)
  DDOVER_HIDE = $00000200 ;

  (*
   * Use the color key associated with the destination surface.
   *)
  DDOVER_KEYDEST = $00000400 ;

  (*
   * Use the dckDestColorkey field in the DDOVERLAYFX structure as the color key
   * for the destination surface
   *)
  DDOVER_KEYDESTOVERRIDE = $00000800 ;

  (*
   * Use the color key associated with the source surface.
   *)
  DDOVER_KEYSRC = $00001000 ;

  (*
   * Use the dckSrcColorkey field in the DDOVERLAYFX structure as the color key
   * for the source surface.
   *)
  DDOVER_KEYSRCOVERRIDE = $00002000 ;

  (*
   * Turn this overlay on.
   *)
  DDOVER_SHOW = $00004000 ;

  (*
   * Add a dirty rect to an emulated overlayed surface.
   *)
  DDOVER_ADDDIRTYRECT = $00008000 ;

  (*
   * Redraw all dirty rects on an emulated overlayed surface.
   *)
  DDOVER_REFRESHDIRTYRECTS = $00010000 ;

  (*
   * Redraw the entire surface on an emulated overlayed surface.
   *)
  DDOVER_REFRESHALL = $00020000 ;

  (*
   * Force redrawing onto the destination surface without regard for the background
   *)
  DDOVER_REFRESHPOUND = $00040000 ;

  (*
   * Use the overlay FX flags to define special overlay FX
   *)
  DDOVER_DDFX = $00080000 ;


  (****************************************************************************
   *
   * DIRECTDRAWSURFACE LOCK FLAGS
   *
   ****************************************************************************)

  (*
   * The default.  Set to indicate that Lock should return a valid memory pointer
   * to the top of the specified rectangle.  If no rectangle is specified then a
   * pointer to the top of the surface is returned.
   *)
  DDLOCK_SURFACEMEMORYPTR = $00000000 ;  { default }

  (*
   * Set if an event handle is being passed to Lock.  Lock will trigger the event
   * when it can return the surface memory pointer requested.
   *)
  DDLOCK_EVENT = $00000002 ;

  (****************************************************************************
   *
   * DIRECTDRAWSURFACE BLT FX FLAGS
   *
   ****************************************************************************)

  (*
   * If stretching, use arithmetic stretching along the Y axis for this blt.
   *)
  DDBLTFX_ARITHSTRETCHY = $00000001 ;

  (*
   * Do this blt mirroring the surface left to right.  Spin the
   * surface around its y-axis.
   *)
  DDBLTFX_MIRRORLEFTRIGHT = $00000002 ;

  (*
   * Do this blt mirroring the surface up and down.  Spin the surface
   * around its x-axis.
   *)
  DDBLTFX_MIRRORUPDOWN = $00000004 ;

  (*
   * Schedule this blt to avoid tearing.
   *)
  DDBLTFX_NOTEARING = $00000008 ;

  (*
   * Do this blt rotating the surface one hundred and eighty degrees.
   *)
  DDBLTFX_ROTATE180 = $00000010 ;

  (*
   * Do this blt rotating the surface two hundred and seventy degrees.
   *)
  DDBLTFX_ROTATE270 = $00000020 ;

  (*
   * Do this blt rotating the surface ninety degrees.
   *)
  DDBLTFX_ROTATE90 = $00000040 ;

  (*
   * Do this z blt using dwZBufferLow and dwZBufferHigh as  range values
   * specified to limit the bits copied from the source surface.
   *)
  DDBLTFX_ZBUFFERRANGE = $00000080 ;

  (*
   * Do this z blt adding the dwZBufferBaseDest to each of the sources z values
   * before comparing it with the desting z values.
   *)
  DDBLTFX_ZBUFFERBASEDEST = $00000100 ;

  (****************************************************************************
   *
   * DIRECTDRAWSURFACE OVERLAY FX FLAGS
   *
   ****************************************************************************)

  (*
   * If stretching, use arithmetic stretching along the Y axis for this overlay.
   *)
  DDOVERFX_ARITHSTRETCHY = $00000001 ;

  (*
   * Mirror the overlay across the vertical axis
   *)
  DDOVERFX_MIRRORLEFTRIGHT = $00000002 ;

  (*
   * Mirror the overlay across the horizontal axis
   *)
  DDOVERFX_MIRRORUPDOWN = $00000004 ;

  (****************************************************************************
   *
   * DIRECTDRAW WAITFORVERTICALBLANK FLAGS
   *
   ****************************************************************************)

  (*
   * return when the vertical blank interval begins
   *)
  DDWAITVB_BLOCKBEGIN = $00000001 ;

  (*
   * set up an event to trigger when the vertical blank begins
   *)
  DDWAITVB_BLOCKBEGINEVENT = $00000002 ;

  (*
   * return when the vertical blank interval ends and display begins
   *)
  DDWAITVB_BLOCKEND = $00000004 ;

  (****************************************************************************
   *
   * DIRECTDRAW GETFLIPSTATUS FLAGS
   *
   ****************************************************************************)

  (*
   * is it OK to flip now?
   *)
  DDGFS_CANFLIP = $00000001 ;

  (*
   * is the last flip finished?
   *)
  DDGFS_ISFLIPDONE = $00000002 ;

  (****************************************************************************
   *
   * DIRECTDRAW GETBLTSTATUS FLAGS
   *
   ****************************************************************************)

  (*
   * is it OK to blt now?
   *)
  DDGBS_CANBLT = $00000001 ;

  (*
   * is the blt to the surface finished?
   *)
  DDGBS_ISBLTDONE = $00000002 ;


  (****************************************************************************
   *
   * DIRECTDRAW ENUMOVERLAYZORDER FLAGS
   *
   ****************************************************************************)

  (*
   * Enumerate overlays back to front.
   *)
  DDENUMOVERLAYZ_BACKTOFRONT = $00000000 ;

  (*
   * Enumerate overlays front to back
   *)
  DDENUMOVERLAYZ_FRONTTOBACK = $00000001 ;

  (****************************************************************************
   *
   * DIRECTDRAW UPDATEOVERLAYZORDER FLAGS
   *
   ****************************************************************************)

  (*
   * Send overlay to front
   *)
  DDOVERZ_SENDTOFRONT = $00000000 ;

  (*
   * Send overlay to back
   *)
  DDOVERZ_SENDTOBACK = $00000001 ;

  (*
   * Move Overlay forward
   *)
  DDOVERZ_MOVEFORWARD = $00000002 ;

  (*
   * Move Overlay backward
   *)
  DDOVERZ_MOVEBACKWARD = $00000003 ;

  (*
   * Move Overlay in front of relative surface
   *)
  DDOVERZ_INSERTINFRONTOF = $00000004 ;

  (*
   * Move Overlay in back of relative surface
   *)
  DDOVERZ_INSERTINBACKOF = $00000005 ;
  
  (*===========================================================================
   *
   *
   * DIRECTDRAW RETURN CODES
   *
   * The return values from DirectDraw Commands and Surface that return an HRESULT
   * are codes from DirectDraw concerning the results of the action
   * requested by DirectDraw.
   *
   *==========================================================================*)

  (*
   * Status is OK
   *
   * Issued by: DirectDraw Commands and all callbacks
   *)

  DD_OK_HANDLED    = 1;
  DD_OK_NOTHANDLED = 0;
  DD_OK            = 0;

  (****************************************************************************
   *
   * DIRECTDRAW ENUMCALLBACK RETURN VALUES
   *
   * EnumCallback returns are used to control the flow of the DIRECTDRAW and
   * DIRECTDRAWSURFACE object enumerations.   They can only be returned by
   * enumeration callback routines.
   *
   ****************************************************************************)

  (*
   * stop the enumeration
   *)
  DDENUMRET_CANCEL = 0 ;

  (*
   * continue the enumeration
   *)
  DDENUMRET_OK = 1 ;

  (****************************************************************************
   *
   * DIRECTDRAW ERRORS
   *
   * Errors are represented by negative values and cannot be combined.
   *
   ****************************************************************************)

  DDErrorMask = $88760000 ;

  (*
   * This object is already initialized
   *)

  DDERR_ALREADYINITIALIZED = DDErrorMask or 5;

  (*
   * This surface can not be attached to the requested surface.
   *)
  DDERR_CANNOTATTACHSURFACE = DDErrorMask or 10 ;

  (*
   * This surface can not be detached from the requested surface.
   *)
  DDERR_CANNOTDETACHSURFACE = DDErrorMask or 20 ;

  (*
   * Support is currently not available.
   *)
  DDERR_CURRENTLYNOTAVAIL = DDErrorMask or 40 ;

  (*
   * An exception was encountered while performing the requested operation
   *)
  DDERR_EXCEPTION = DDErrorMask or 55 ;

  (*
   * Generic failure.
   *)
  DDERR_GENERIC = $80004005; (* E_FAIL *)

  (*
   * Height of rectangle provided is not a multiple of reqd alignment
   *)
  DDERR_HEIGHTALIGN = DDErrorMask or 90 ;

  (*
   * Unable to match primary surface creation request with existing 
   * primary surface.
   *)
  DDERR_INCOMPATIBLEPRIMARY = DDErrorMask or 95 ;

  (*
   * One or more of the caps bits passed to the callback are incorrect.
   *)
  DDERR_INVALIDCAPS = DDErrorMask or 100 ;

  (*
   * DirectDraw does not support provided Cliplist.
   *)
  DDERR_INVALIDCLIPLIST = DDErrorMask or 110 ;

  (*
   * DirectDraw does not support the requested mode
   *)
  DDERR_INVALIDMODE = DDErrorMask or 120 ;

  (*
   * DirectDraw received a pointer that was an invalid DIRECTDRAW object.
   *)
  DDERR_INVALIDOBJECT = DDErrorMask or 130 ;

  (*
   * One or more of the parameters passed to the callback function are
   * incorrect.
   *)
  DDERR_INVALIDPARAMS = $80070057; (* E_INVALIDARG *)

  (*
   * pixel format was invalid as specified
   *)
  DDERR_INVALIDPIXELFORMAT = DDErrorMask or 145 ;

  (*
   * Rectangle provided was invalid.
   *)
  DDERR_INVALIDRECT = DDErrorMask or 150 ;

  (*
   * Operation could not be carried out because one or more surfaces are locked
   *)
  DDERR_LOCKEDSURFACES = DDErrorMask or 160 ;

  (*
   * There is no 3D present.
   *)
  DDERR_NO3D = DDErrorMask or 170 ;

  (*
   * Operation could not be carried out because there is no alpha accleration
   * hardware present or available.
   *)
  DDERR_NOALPHAHW = DDErrorMask or 180 ;

  (*
   * no clip list available
   *)
  DDERR_NOCLIPLIST = DDErrorMask or 205 ;

  (*
   * Operation could not be carried out because there is no color conversion
   * hardware present or available.
   *)
  DDERR_NOCOLORCONVHW = DDErrorMask or 210 ;

  (*
   * Create function called without DirectDraw object method SetCooperativeLevel
   * being called.
   *)
  DDERR_NOCOOPERATIVELEVELSET = DDErrorMask or 212 ;

  (*
   * Surface doesn't currently have a color key
   *)
  DDERR_NOCOLORKEY = DDErrorMask or 215 ;

  (*
   * Operation could not be carried out because there is no hardware support
   * of the dest color key.
   *)
  DDERR_NOCOLORKEYHW = DDErrorMask or 220 ;

  (*
   * No DirectDraw support possible with current display driver
   *)
  DDERR_NODIRECTDRAWSUPPORT = DDErrorMask or 222 ;

  (*
   * Operation requires the application to have exclusive mode but the
   * application does not have exclusive mode.
   *)
  DDERR_NOEXCLUSIVEMODE = DDErrorMask or 225 ;

  (*
   * Flipping visible surfaces is not supported.
   *)
  DDERR_NOFLIPHW = DDErrorMask or 230 ;

  (*
   * There is no GDI present.
   *)
  DDERR_NOGDI = DDErrorMask or 240 ;

  (*
   * Operation could not be carried out because there is no hardware present
   * or available.
   *)
  DDERR_NOMIRRORHW = DDErrorMask or 250 ;

  (*
   * Requested item was not found
   *)
  DDERR_NOTFOUND = DDErrorMask or 255 ;

  (*
   * Operation could not be carried out because there is no overlay hardware
   * present or available.
   *)
  DDERR_NOOVERLAYHW = DDErrorMask or 260 ;

  (*
   * Operation could not be carried out because there is no appropriate raster
   * op hardware present or available.
   *)
  DDERR_NORASTEROPHW = DDErrorMask or 280 ;

  (*
   * Operation could not be carried out because there is no rotation hardware
   * present or available.
   *)
  DDERR_NOROTATIONHW = DDErrorMask or 290 ;

  (*
   * Operation could not be carried out because there is no hardware support
   * for stretching
   *)
  DDERR_NOSTRETCHHW = DDErrorMask or 310 ;

  (*
   * DirectDrawSurface is not in 4 bit color palette and the requested operation
   * requires 4 bit color palette.
   *)
  DDERR_NOT4BITCOLOR = DDErrorMask or 316 ;

  (*
   * DirectDrawSurface is not in 4 bit color index palette and the requested
   * operation requires 4 bit color index palette.
   *)
  DDERR_NOT4BITCOLORINDEX = DDErrorMask or 317 ;

  (*
   * DirectDraw Surface is not in 8 bit color mode and the requested operation
   * requires 8 bit color.
   *)
  DDERR_NOT8BITCOLOR = DDErrorMask or 320 ;

  (*
   * Operation could not be carried out because there is no texture mapping
   * hardware present or available.
   *)
  DDERR_NOTEXTUREHW = DDErrorMask or 330 ;

  (*
   * Operation could not be carried out because there is no hardware support
   * for vertical blank synchronized operations.
   *)
  DDERR_NOVSYNCHW = DDErrorMask or 335 ;

  (*
   * Operation could not be carried out because there is no hardware support
   * for zbuffer blting.
   *)
  DDERR_NOZBUFFERHW = DDErrorMask or 340 ;

  (*
   * Overlay surfaces could not be z layered based on their BltOrder because
   * the hardware does not support z layering of overlays.
   *)
  DDERR_NOZOVERLAYHW = DDErrorMask or 350 ;

  (*
   * The hardware needed for the requested operation has already been
   * allocated.
   *)
  DDERR_OUTOFCAPS = DDErrorMask or 360 ;

  (*
   * DirectDraw does not have enough memory to perform the operation.
   *)
  DDERR_OUTOFMEMORY = $8007000E; (* E_OUTOFMEMORY *)

  (*
   * DirectDraw does not have enough memory to perform the operation.
   *)
  DDERR_OUTOFVIDEOMEMORY = DDErrorMask or 380 ;

  (*
   * hardware does not support clipped overlays
   *)
  DDERR_OVERLAYCANTCLIP = DDErrorMask or 382 ;

  (*
   * Can only have ony color key active at one time for overlays
   *)
  DDERR_OVERLAYCOLORKEYONLYONEACTIVE = DDErrorMask or 384 ;

  (*
   * Access to this palette is being refused because the palette is already
   * locked by another thread.
   *)
  DDERR_PALETTEBUSY = DDErrorMask or 387 ;

  (*
   * No src color key specified for this operation.
   *)
  DDERR_COLORKEYNOTSET = DDErrorMask or 400 ;

  (*
   * This surface is already attached to the surface it is being attached to.
   *)
  DDERR_SURFACEALREADYATTACHED = DDErrorMask or 410 ;

  (*
   * This surface is already a dependency of the surface it is being made a
   * dependency of.
   *)
  DDERR_SURFACEALREADYDEPENDENT = DDErrorMask or 420 ;

  (*
   * Access to this surface is being refused because the surface is already
   * locked by another thread.
   *)
  DDERR_SURFACEBUSY = DDErrorMask or 430 ;

  (*
   * Access to Surface refused because Surface is obscured.
   *)
  DDERR_SURFACEISOBSCURED = DDErrorMask or 440 ;

  (*
   * Access to this surface is being refused because the surface is gone.
   * The DIRECTDRAWSURFACE object representing this surface should 
   * have Restore called on it.
   *)
  DDERR_SURFACELOST = DDErrorMask or 450 ;

  (*
   * The requested surface is not attached.
   *)
  DDERR_SURFACENOTATTACHED = DDErrorMask or 460 ;

  (*
   * Height requested by DirectDraw is too large.
   *)
  DDERR_TOOBIGHEIGHT = DDErrorMask or 470 ;

  (*
   * Size requested by DirectDraw is too large --  The individual height and
   * width are OK.
   *)
  DDERR_TOOBIGSIZE = DDErrorMask or 480 ;

  (*
   * Width requested by DirectDraw is too large.
   *)
  DDERR_TOOBIGWIDTH = DDErrorMask or 490 ;

  (*
   * Action not supported.
   *)
  DDERR_UNSUPPORTED = $80004001; (* E_NOTIMPL *)

  (*
   * FOURCC format requested is unsupported by DirectDraw
   *)
  DDERR_UNSUPPORTEDFORMAT = DDErrorMask or 510 ;

  (*
   * Bitmask in the pixel format requested is unsupported by DirectDraw
   *)
  DDERR_UNSUPPORTEDMASK = DDErrorMask or 520 ;

  (*
   * vertical blank is in progress
   *)
  DDERR_VERTICALBLANKINPROGRESS = DDErrorMask or 537 ;

  (*
   * Informs DirectDraw that the previous Blt which is transfering information
   * to or from this Surface is incomplete.
   *)
  DDERR_WASSTILLDRAWING = DDErrorMask or 540 ;

  (*
   * Rectangle provided was not horizontally aligned on reqd. boundary
   *)
  DDERR_XALIGN = DDErrorMask or 560 ;

  (*
   * The GUID passed to DirectDrawCreate is not a valid DirectDraw driver
   * identifier.
   *)
  DDERR_INVALIDDIRECTDRAWGUID = DDErrorMask or 561 ;

  (*
   * A DirectDraw object representing this driver has already been created
   * for this process.
   *)
  DDERR_DIRECTDRAWALREADYCREATED = DDErrorMask or 562 ;

  (*
   * A hardware only DirectDraw object creation was attempted but the driver
   * did not support any hardware.
   *)
  DDERR_NODIRECTDRAWHW = DDErrorMask or 563 ;

  (*
   * this process already has created a primary surface
   *)
  DDERR_PRIMARYSURFACEALREADYEXISTS = DDErrorMask or 564 ;

  (*
   * software emulation not available.
   *)
  DDERR_NOEMULATION = DDErrorMask or 565 ;

  (*
   * region passed to Clipper::GetClipList is too small.
   *)
  DDERR_REGIONTOOSMALL = DDErrorMask or 566 ;

  (*
   * an attempt was made to set a clip list for a clipper objec that
   * is already monitoring an hwnd.
   *)
  DDERR_CLIPPERISUSINGHWND = DDErrorMask or 567 ;

  (*
   * No clipper object attached to surface object
   *)
  DDERR_NOCLIPPERATTACHED = DDErrorMask or 568 ;

  (*
   * Clipper notification requires an HWND or
   * no HWND has previously been set as the CooperativeLevel HWND.
   *)
  DDERR_NOHWND = DDErrorMask or 569 ;

  (*
   * HWND used by DirectDraw CooperativeLevel has been subclassed,
   * this prevents DirectDraw from restoring state.
   *)
  DDERR_HWNDSUBCLASSED = DDErrorMask or 570 ;

  (*
   * The CooperativeLevel HWND has already been set.
   * It can not be reset while the process has surfaces or palettes created.
   *)
  DDERR_HWNDALREADYSET = DDErrorMask or 571 ;

  (*
   * No palette object attached to this surface.
   *)
  DDERR_NOPALETTEATTACHED = DDErrorMask or 572 ;

  (*
   * No hardware support for 16 or 256 color palettes.
   *)
  DDERR_NOPALETTEHW = DDErrorMask or 573 ;

  (*
   * If a clipper object is attached to the source surface passed into a
   * BltFast call.
   *)
  DDERR_BLTFASTCANTCLIP = DDErrorMask or 574 ;

  (*
   * No blter.
   *)
  DDERR_NOBLTHW = DDErrorMask or 575 ;

  (*
   * No DirectDraw ROP hardware.
   *)
  DDERR_NODDROPSHW = DDErrorMask or 576 ;

  (*
   * returned when GetOverlayPosition is called on a hidden overlay
   *)
  DDERR_OVERLAYNOTVISIBLE = DDErrorMask or 577 ;

  (*
   * returned when GetOverlayPosition is called on a overlay that UpdateOverlay
   * has never been called on to establish a destionation.
   *)
  DDERR_NOOVERLAYDEST = DDErrorMask or 578 ;

  (*
   * returned when the position of the overlay on the destionation is no longer
   * legal for that destionation.
   *)
  DDERR_INVALIDPOSITION = DDErrorMask or 579 ;

  (*
   * returned when an overlay member is called for a non-overlay surface
   *)
  DDERR_NOTAOVERLAYSURFACE = DDErrorMask or 580 ;
 
  (*
   * An attempt was made to set the cooperative level when it was already
   * set to exclusive.
   *)
  DDERR_EXCLUSIVEMODEALREADYSET = DDErrorMask or 581 ;

  (*
   * An attempt has been made to flip a surface that is not flippable.
   *)
  DDERR_NOTFLIPPABLE = DDErrorMask or 582 ;

  (*
   * Can't duplicate primary & 3D surfaces, or surfaces that are implicitly
   * created.
   *)
  DDERR_CANTDUPLICATE = DDErrorMask or 583 ;

  (*
   * Surface was not locked.  An attempt to unlock a surface that was not
   * locked at all, or by this process, has been attempted.
   *)
  DDERR_NOTLOCKED = DDErrorMask or 584 ;

  (*
   * Windows can not create any more DCs
   *)
  DDERR_CANTCREATEDC = DDErrorMask or 585 ;

  (*
   * No DC was ever created for this surface.
   *)
  DDERR_NODC = DDErrorMask or 586 ;

  (*
   * This surface can not be restored because it was created in a different
   * mode.
   *)
  DDERR_WRONGMODE = DDErrorMask or 587 ;

  (*
   * This surface can not be restored because it is an implicitly created
   * surface.
   *)
  DDERR_IMPLICITLYCREATED = DDErrorMask or 588 ;

  (*
   * The surface being used is not a palette-based surface
   *)
  DDERR_NOTPALETTIZED = DDErrorMask or 589 ;

  (*
   * The display is currently in an unsupported mode
   *)
  DDERR_UNSUPPORTEDMODE = DDErrorMask or 590 ;

    (* Alpha bit depth constants *)

  (*
   * 1 bit alpha constants
   *)

  (*
   * Completely transparent. Colors being blended with this constant do not effect
   * the final result.
   *)
  DDALPHA1_TRANSPARENT = 0 ;

  (*
   * Completely opaque. Colors being blended with this constant are the final result.
   *)
  DDALPHA1_OPAQUE = 1 ;

  (*
   * 2 bit alpha constants
   *)

  (*
   * Completely transparent. Colors being blended with this constant do not effect
   * the final result.
   *)
  DDALPHA2_TRANSPARENT = 0 ;

  (*
   * 50-50 blend.  Colors being blended with this constant effect the final result
   * with 50% of their color.
   *)
  DDALPHA2_50 = 2 ;

  (*
   * Completely opaque. Colors being blended with this constant are the final result.
   *)
  DDALPHA2_OPAQUE = 3 ;

  (*
   * 4 bit alpha constants
   *)

  (*
   * Completely transparent. Colors being blended with this constant do not effect
   * the final result.
   *)
  DDALPHA4_TRANSPARENT = 0 ;

  (*
   * Opaque modifier is multiplied by any value between 0 and _OPAQUE to
   * establish gradients of alpha blending.
   *)
  DDALPHA4_OPAQUEMOD = 1 ;

  (*
   * 50-50 blend.  Colors being blended with this constant effect the final result
   * with 50% of their color.
   *)
  DDALPHA4_50 = 8 ;

  (*
   * Completely opaque. Colors being blended with this constant are the final result.
   *)
  DDALPHA4_OPAQUE = 15 ;

  (*
   * 8 bit alpha constants
   *)

  (*
   * Completely transparent. Colors being blended with this constant do not effect
   * the final result.
   *)
  DDALPHA8_TRANSPARENT = 0 ;

  (*
   * Opaque modifier is multiplied by any value between 0 and _OPAQUE to
   * establish gradients of alpha blending.
   *)
  DDALPHA8_OPAQUEMOD = 1 ;

  (*
   * 50-50 blend.  Colors being blended with this constant effect the final result
   * with 50% of their color.
   *)
  DDALPHA8_50 = 128 ;
  (*
   * Completely opaque. Colors being blended with this constant are the final result.
   *)
  DDALPHA8_OPAQUE = 255 ;

  DD_ROP_SPACE     = 256 div 32 ;

