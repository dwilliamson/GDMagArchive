unit drawbrd1;
{.$DEFINE DEBUG}

{
   Copyright (c) 1996 by Charlie Calvert

   This unit compiles two VERY different ways, depending on
   whether or not USECOM is defined. If USECOM is defined, then
   the key objects in this unit are defined as descendants of
   IUnknown, otherwise, they are descendants of TComponent, and
   can be placed on the Component Palette.

   As a rule, you should only define USECOM if you are compiling
   this code into a COM DLL. Merely compiling the unit with
   USECOM defined will not cause trouble, from either inside our
   outside the IDE, but you need to be aware that the DCU for
   the unit will then be different from the one in CMPLIB32.DCL, that is,
   from one that is compiled as TComponent descendant that can be
   placed on the Component Palette. (In Delphi 2.0, all components
   shown on the component palette are part of CMPLIB32.DCL.)

   If you define DEBUG, above, then a file called DEBUGFILE will
   be opened and you can use it to write variables to a text file.
   See the initialization section at the bottom of this unit. If
   you don't define DEBUG, then all your WriteLn statements will
   no longer compile, which is a quick easy way of finding them. It's
   a cheap debugging technique, but it works, and is handy when
   dealing with COM objects.


 Docs on TSpeedDraw:


 No Clipper Object:
 
  Don't try to blit something from the WorkSurface into the
  backsurface that is larger than the backsurface itself!
  On the same note, don't blit off the edges, either. For instance,
  If your bitmap in the WorkSurface is the same size as the
  BackSurface then BackOrigin.X and Y have to be zero! This limitation
  exists becausethere is no clipper object in these components at
  this time.

}

interface

uses
  DDraw, Windows, Classes,
  OleAuto, DDUtils, Dialogs,
  SysUtils, Forms, DsgnIntf,
  ExtCtrls, Graphics
  {$IFDEF USECOM}, Ole2 {$ENDIF};

const
  Timer1 = 1;
  MaxMapRows = 12;
  MaxMapCols = 20;

const
  ftSage = 100;
  ftSnake = 101;
  ftMonastery = 102;
  ftHero = 103;

const
  {$IFDEF USECOM}
  CLSID_ISPEEDDRAW: TGUID = (
    D1:$185FF680;D2:$DD53;D3:$11CF;D4:($92,$C4,$00,$80,$C8,$0C,$F1,$D2));
  {$ENDIF}

  MaxCreatures = 4;
  CAry: array[0..(MaxCreatures - 1)] of string =
    ('Sage', 'Snake', 'Monastery', 'Hero');
  TotalMapType = 53;

type
{ --- SPEEDDRAW CLASSES --- }

  TMoveHeroEvent = procedure(Sender: TObject; NewPos: TPoint;
    NewType: Integer; var MoveOk: Boolean) of object;
    
  EDDError = class(Exception);

  {$IFDEF USECOM}
  TObjectDestroyed = procedure;

  IDrawBase = class(IUnknown)
  protected
    FRefCount: LongInt;
    FObjectDestroyed: TObjectDestroyed;
  public
    constructor Create; virtual;
    function QueryInterface(const iid: TIID; var obj): HResult; override; stdcall;
    function AddRef: Longint; override; stdcall;
    function Release: Longint; override; stdcall;
  end;

  IMyClassFactory = class(IClassFactory)
  private
    FRefCount: LongInt;
  public
    constructor Create; virtual;
    function QueryInterface(const iid: TIID; var obj): HResult; override; stdcall;
    function AddRef: Longint; override; stdcall;
    function Release: Longint; override; stdcall;
    function CreateInstance(unkOuter: IUnknown; const iid: TIID;
      var obj): HResult; override; stdcall;
    function LockServer(fLock: BOOL): HResult; override; stdcall;
  end;

var
  LockCount: Integer;
  ObjCount: Integer;
  
type
  TSpeedDraw = class(IDrawBase)
  {$ELSE}
  TSpeedDraw = class(TComponent)
  {$ENDIF}
  private
    FActive: Boolean;
    FBackColor: TColor;
    FBackgroundMap: string;
    FBackSurface: IDirectDrawSurface;
    FClipper: IDirectDrawClipper;
    FPalette: IDirectDrawPalette;
    FDirectDraw: IDirectDraw;
    FPrimarySurface: IDirectDrawSurface;
    FHandle: HWnd;
    FDllName: string;
    FUseExclusive: Boolean;
    FUseTimer: Boolean;
    FTimerOdd: Boolean;
    FTimerInterval: Integer;
    FTransColor: Integer;
    FWorkSurface: IDirectDrawSurface;
    FBackOrigin: TPoint;
    FBackRect: TRect;
    FFillBackGround: Boolean;
    FShowBackBmp: Boolean;
    FBackGroundProc: TNotifyEvent;
    FInitObjectsProc: TNotifyEvent;
    function CreatePrimary: Boolean;
    function SetUpBack: Boolean;
    procedure SetupWorkSurface;
    function GetDib(Instance: THandle; S: string): THandle;
    function CreateSurface(Bitmap: THandle): IDirectDrawSurface;
    function LoadPalette(Instance: THandle;
      const BitmapName: string): IDirectDrawPalette;
    function MakeItSo(DDResult: HResult): Boolean;
    procedure SetActive(Value: Boolean);
  protected
    function BackgroundBlits: Boolean; virtual;
    procedure DDTest(hr: HResult; S: string);
    procedure ReLoadBitmap(Surface: IDirectDrawSurface;
      const BitmapName : string);
    function Pause: Boolean; virtual; stdcall;
    procedure Restore; virtual; stdcall;
  public
   {$IFDEF USECOM}
    constructor Create; override;
    procedure InitParams(AHandle: THandle; BackGroundMapStr: PChar;
      TransColor: Integer; DllName: PChar); virtual; stdcall;
   {$ELSE}
    constructor Create(AOwner: TComponent);  override;
    constructor CreateParams(AOwner: TComponent; BackGroundMapStr: string;
      TransColor: Integer; DllName: string); virtual;
   {$ENDIF}
    procedure Run; virtual; stdcall;
    procedure DestroyObjects; virtual; stdcall;
    function CreateDDSurface(var DDS: IDirectDrawSurface;
                             BitsName: string; UsePalette: BOolean): Boolean;
    procedure ErrorEvent(S: string);
    procedure DoFlip; virtual; stdcall;
    procedure InitFailed;
    procedure Initialize;
    procedure InitObjects; virtual; stdcall;
    procedure Move(Value: Integer); virtual; stdcall;
    procedure SetupPalette;
    destructor Destroy; override;
    function CreateBlankSurface(Width, Height: Integer): IDirectDrawSurface;
    {$IFDEF USECOM}
    procedure MakeActive(Value: Boolean); virtual; stdcall;
    procedure SetTimerOdd(Value: Boolean); virtual; stdcall;
    procedure SetBackOrigin(Value: TPoint); virtual; stdcall;
    {$ELSE}
    property Active: Boolean read fActive write SetActive;
    property BackGroundMap: string read FBackGroundMap write FBackGroundMap;
    property DirectDraw: IDirectDraw read FDirectDraw write FDirectDraw;
    property PrimarySurface: IDirectDrawSurface read FPrimarySurface write FPrimarySurface;
    property BackSurface: IDirectDrawSurface
      read FBackSurface write FBackSurface;
    property WorkSurface: IDirectDrawSurface read FWorkSurface write FWorkSurface;
    property Palette: IDirectDrawPalette read FPalette write FPalette;
    property Handle: HWnd read FHandle write FHandle;
    property BackRect: TRect read FBackRect write FBackRect;
    property BackOrigin: TPoint read FBackOrigin write FBackOrigin;
    property TimerOdd: Boolean read FTimerOdd write FTimerOdd;
  published
    property Exclusive: Boolean read FUseExclusive write FUseExclusive;
    property BlankBackGround: Boolean read FFillBackGround write FFillBackGround;
    property BackColor: TColor read FBackColor write FBackColor;
    property ShowBackBmp: Boolean read FShowBackBmp write FShowBackBmp;
    property UseTimer: Boolean read FUseTimer write FUseTimer;
    property TimerInterval: Integer read FTimerInterval write FTimerInterval;
    property BackgroundFile: string read FBackGroundmap write FBackGroundmap;
    property DllName: string read FDllName write FDllName;
    property TransparentColor: Integer read FTransColor write FTransColor;
    property PaintProc: TNotifyEvent read FBackGroundProc
      write FBackGroundProc;
    property InitObjectsProc: TNotifyEvent read FInitObjectsProc
      write FInitObjectsProc;
    property BackOrigX: LongInt read FBackOrigin.X write FBackOrigin.X;
    property BackOrigY: LongInt read FBackOrigin.Y write FBackOrigin.Y;
    {$ENDIF}
  end;

  TBackGrndBmpProperty = class(TStringProperty)
  public
    function GetAttributes: TPropertyAttributes; override;
    procedure Edit; override;
  end;

  TDllNameProperty = class(TStringProperty)
  public
    function GetAttributes: TPropertyAttributes; override;
    procedure Edit; override;
  end;

  TScreenFileProperty = class(TStringProperty)
  public
    function GetAttributes: TPropertyAttributes; override;
    procedure Edit; override;
  end;

  TSpeedDrawEditor = class(TComponentEditor)
    procedure Edit; override;
  end;

{$IFNDEF USECOM}
procedure Register;
{$ENDIF}

function DDCreateSurface(DD: IDirectDraw; Width, Height: DWord;
  SysMem, Trans: Boolean; dwColorKey: DWord): IDirectDrawSurface;
function CreateClipper(DD: IDirectDraw; Handle: HWnd): IDirectDrawClipper;

implementation

uses
  CodeBox, DrawBrdb;

var
  ASpeedDraw: TSpeedDraw;
{$IFDEF Debug}
  DebugFile: TextFile;
{$EndIf}

{$IFNDEF USECOM}
procedure Register;
begin
  RegisterComponents('Unleash', [TSpeedDraw]);
  RegisterPropertyEditor(TypeInfo(string),
                 TSpeedDraw, 'BackGroundFile',
                 TBackGrndBmpProperty);
  RegisterPropertyEditor(TypeInfo(string),
                 TSpeedDraw, 'DllName',
                 TDllNameProperty);

  RegisterComponentEditor(TSpeedDraw, TSpeedDrawEditor);
//  RegisterComponents('Unleash', [TCreatureList]);
end;
{$ENDIF}

{ --- TBackGrndBmpProperty --- }

procedure TBackGrndBmpProperty.Edit;
var
  MPFileOpen: TOpenDialog;
begin
  MPFileOpen := TOpenDialog.Create(Application);
  MPFileOpen.Filename := GetValue;
  MPFileOpen.Filter := 'Bitmap|*.bmp';
  MPFileOpen.HelpContext := 0;
  MPFileOpen.Options := MPFileOpen.Options + [ofShowHelp, ofPathMustExist,
    ofFileMustExist];
  try
    if MPFileOpen.Execute then SetValue(MPFileOpen.Filename);
  finally
    MPFileOpen.Free;
  end;
end;

function TBackGrndBmpProperty.GetAttributes: TPropertyAttributes;
begin
  Result := [paDialog, paRevertable];
end;

{ TDllNameProperty }

procedure TDllNameProperty.Edit;
var
  MPFileOpen: TOpenDialog;
begin
  MPFileOpen := TOpenDialog.Create(Application);
  MPFileOpen.Filename := GetValue;
  MPFileOpen.Filter := 'Bitmap|*.dll';
  MPFileOpen.HelpContext := 0;
  MPFileOpen.Options := MPFileOpen.Options + [ofShowHelp, ofPathMustExist,
    ofFileMustExist];
  try
    if MPFileOpen.Execute then SetValue(MPFileOpen.Filename);
  finally
    MPFileOpen.Free;
  end;
end;

function TDllNameProperty.GetAttributes: TPropertyAttributes;
begin
  Result := [paDialog, paRevertable];
end;

{ --- TSpeedDrawEditor --- }

procedure TSpeedDrawEditor.Edit;
var
  B: TBackDlg;
  X, Y: Integer;
  X1, Y1: Integer;
begin
  B := TBackDlg.Create(Application);
  B.Edit1.Text := IntToStr(TSpeedDraw(Component).FBackOrigin.X);
  B.Edit2.Text := IntToStr(TSpeedDraw(Component).FBackOrigin.Y);
  B.Edit3.Text := IntToStr(TSpeedDraw(Component).FBackRect.Left);
  B.Edit4.Text := IntToStr(TSpeedDraw(Component).FBackRect.Top);
  B.Edit5.Text := IntToStr(TSpeedDraw(Component).FBackRect.Right);
  B.Edit6.Text := IntToStr(TSpeedDraw(Component).FBackRect.Bottom);
  B.ShowModal;
  X := StrToInt(B.Edit1.Text);
  Y := StrToInt(B.Edit2.Text);
  TSpeedDraw(Component).FBackOrigin := Point(X, Y);
  X := StrToInt(B.Edit3.Text);
  Y := StrToInt(B.Edit4.Text);
  X1 := StrToInt(B.Edit5.Text);
  Y1 := StrToInt(B.Edit6.Text);
  TSpeedDraw(Component).FBackRect := Rect(X, Y, X1, Y1);
  B.Free;
  Designer.Modified;
end;

{ TScreenFileProperty }

procedure TScreenFileProperty.Edit;
var
  MPFileOpen: TOpenDialog;
begin
  MPFileOpen := TOpenDialog.Create(Application);
  MPFileOpen.Filename := GetValue;
  MPFileOpen.Filter := 'Bitmap|*.dta';
  MPFileOpen.HelpContext := 0;
  MPFileOpen.Options := MPFileOpen.Options + [ofShowHelp, ofPathMustExist,
    ofFileMustExist];
  try
    if MPFileOpen.Execute then SetValue(MPFileOpen.Filename);
  finally
    MPFileOpen.Free;
  end;
end;

function TScreenFileProperty.GetAttributes: TPropertyAttributes;
begin
  Result := [paDialog, paRevertable];
end;

{$IFDEF USECOM}

{ ----------------- }
{ --- IDRAWBASE --- }
{ ----------------- }

{ This routine is passed as a parameter to IDrawBase }
procedure ObjectDestroyed;
begin
  Dec(ObjCount)
end;

constructor IDrawBase.Create;
begin
  inherited Create;
  FObjectDestroyed := ObjectDestroyed;
  FRefCount := 0;
end;

function IDrawBase.QueryInterface(const iid: TIID; var obj): HResult;
begin
  if IsEqualIID(iid, IID_IUnknown) or
     IsEqualIID(iid, CLSID_ISPEEDDRAW) then begin
    Pointer(obj) := Self;
    AddRef;
    Result := S_OK;
  end else begin
    Pointer(obj) := nil;
    Result := E_NOINTERFACE;
  end;
end;

function IDrawBase.AddRef: Longint;
begin
  Inc(FRefCount);
  Result := FRefCount;
end;

function IDrawBase.Release: Longint;
begin
  Dec(FRefCount);
  if FRefCount = 0 then begin
    FObjectDestroyed;
    Free;
    Result := 0;
  end else
    Result := FRefCount;
end;

{ --- IMyClassFactory --- }

constructor IMyClassFactory.Create;
begin
  inherited Create;
  FRefCount := 0;
end;

function IMyClassFactory.QueryInterface(const iid: TIID; var obj): HResult;
begin
  if IsEqualIID(iid, IID_IClassFactory) or
     IsEqualIID(iid, IID_IUnknown) then begin
    Pointer(obj) := Self;
    AddRef;
    Result := S_OK;
  end else begin
    Pointer(obj) := nil;
    Result := E_NOINTERFACE;
  end;
end;

function IMyClassFactory.AddRef: Longint;
begin
  Inc(FRefCount);
  Result := FRefCount;
end;

function IMyClassFactory.Release: Longint;
begin
  Dec(FRefCount);
  if FRefCount = 0 then begin
    Destroy;
    Result := 0;
  end else
    Result := FRefCount;
end;

function IMyClassFactory.CreateInstance(UnkOuter: IUnknown;
                                        const iid: Tiid;
                                        var Obj): hResult;
var
  hr: HResult;
  MyObject: TSpeedDraw;
begin
  if UnkOuter <> nil then begin
    Result := E_Fail;
    Exit;
  end;
  if (not isEqualIID(iid, IID_IUnknown)) and
     (not isEqualIID(iid, CLSID_ISPEEDDRAW)) then begin
    Result := E_Fail;
    Exit;
  end;

  MyObject := TSpeedDraw.Create;

  if MyObject = nil then begin
    Pointer(Obj) := nil;
    Result := E_OutOfMemory;
    Exit;
  end;

  hr := MyObject.QueryInterface(iid, Obj);
  if Failed(hr) then
    MyObject.Free
  else
    Inc(ObjCount);

  Result := hr;
end;

function IMyClassFactory.LockServer(fLock: BOOL): HResult;
begin
  if fLock then
    Inc(LockCount)
  else
    Dec(LockCount);
  Result := S_Ok;
end;

{$ENDIF} 

{ ------------------ }
{ --- TSpeedDraw --- }
{ ------------------ }

procedure Timer2Timer(H: HWnd; Msg: UInt; Event: UInt; Time: DWord);
begin
  ASpeedDraw.DoFlip;
end;

{$IFDEF USECOM}
procedure TSpeedDraw.InitParams(AHandle: THandle; BackGroundMapStr: PChar;
  TransColor: Integer; Dllname: PChar);
begin
  FHandle := AHandle;
{$ELSE}
constructor TSpeedDraw.CreateParams(AOwner: TComponent; BackGroundMapStr: string;
  TransColor: Integer; Dllname: string);
begin
  inherited Create(AOwner);
  FHandle := TForm(Owner).Handle;
{$ENDIF}
  FDllName := DllName;
  FActive := False;
  FTransColor:= TransColor;            // Transparent color
  FBackGroundMap := BackGroundMapStr;  // Name of background bitmap
  Initialize;
end;

{$IFDEF USECOM}
constructor TSpeedDraw.Create;
begin
  inherited Create;
{$ELSE}
constructor TSpeedDraw.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  FHandle := TForm(AOwner).Handle;
{$ENDIF}
  Initialize;
end;

procedure TSpeedDraw.Initialize;
begin
  FShowBackBmp := True;
  FTimerInterval := 250;
end;

destructor TSpeedDraw.Destroy;
begin
  if FActive then
    KillTimer(FHandle, Timer1);
  DestroyObjects;
  inherited Destroy;
end;

procedure TSpeedDraw.Run;
begin
  if (FBackGroundmap = '') and (FShowBackBmp = True) then
    raise EDDError.Create('If ShowBackMap is is True, you must fill in BackgroundFile');
  if (FDllName <> '') then
    if not FileExists(FDllName) then 
      raise EDDError.Create('Can''t find DLL: ' + FDllName);
  InitObjects;
  DoFlip;
end;

procedure TSpeedDraw.ErrorEvent(S: string);
begin
  FActive := False;
  FDirectDraw.FlipToGDISurface;
  if not FUseExclusive then
    FDirectDraw.SetCooperativeLevel(FHandle, DDSCL_Normal);
  raise EDDError.Create(S);
end;

procedure TSpeedDraw.DDTest(hr: HResult; S: string);
begin
  if not Succeeded(hr) then
    raise EDDError.CreateFmt('DDTest Error: %s $%x %s',
      [S, hr, GetOleError(hr)]);
end;

procedure TSpeedDraw.DestroyObjects;
begin
  FActive := False;
  if (FDirectDraw <> nil) and (FUseExclusive <> False) then
    FDirectDraw.SetCooperativeLevel(FHandle, DDSCL_Normal);
  if FDirectDraw <> nil then begin
    if FBackSurface <> nil then FBackSurface.Release;
    if FClipper <> nil then FClipper.Release;
    if FPrimarySurface <> nil then FPrimarySurface.Release;
    if FPalette <> nil then FPalette.Release;
    if FWorkSurface <> nil then FWorkSurface.Release;
    if FDirectDraw <> nil then FDirectDraw.Release;
  end;
  FDirectDraw := nil;
  FBackSurface := nil;
  FClipper := nil;
  FPrimarySurface := nil;
  FWorkSurface := nil;
  FPalette := nil;
end;

procedure TSpeedDraw.SetUpPalette;
begin
end;

// Don't try to blit something from the WorkSurface into the
// backsurface that is larger than the backsurface itself!
// On the same note, don't blit off the edges, either. For instance,
// If your bitmap in the WorkSurface is the same size as the
// BackSurface then BackOrigin.X and Y have to be zero! In short,
// there is no clipper object in these components at this time.
function TSpeedDraw.BackgroundBlits: Boolean;
var
  hr: HResult;
  DC: HDC;
  OldBrush, Brush: HBrush;
begin
  Result := True;
  if FFillBackGround then begin
    FBackSurface.GetDC(DC);                // Don't step through!!
    Brush := CreateSolidBrush(FBackColor);
    OldBrush := SelectObject(DC, Brush);
    Rectangle(DC, 0, 0, 640, 480);
    SelectObject(DC, OldBrush);
    DeleteObject(Brush);
    FBackSurface.ReleaseDC(DC);
  end;
  if FShowBackBmp then begin
    hr := FBackSurface.BltFast(FBackOrigin.X, FBackOrigin.Y,
          FWorkSurface, FBackRect,
          DDBLTFAST_WAIT or DDBLTFAST_SRCCOLORKEY);
    if not Succeeded(hr) then
      raise EDDError.CreateFmt('TSpeedDraw.BackGroundBlits : $%x'+CR+'%s'+CR+'%s',
        [hr, GetOleError(hr), 'Check BackRect, BackOrigin???']);
  end;
  if Assigned(FBackGroundProc) then
    FBackGroundProc(Self);
end;

{ utility function to help restore surfaces }
function TSpeedDraw.MakeItSo(DDResult: HResult): Boolean;
begin
  case DDResult of
    DD_OK: Result := true;
  else
    Result := DDResult <> DDERR_WASSTILLDRAWING;
  end;
end;

/////////////////////////////////////////////////
// Description:  Draw image to background flip it to the front
//   Background blits is for special processing. In Exclusive
//   mode we call Flip, in windowed mode we blit backsurface
//   in to front surface.
/////////////////////////////////////////////////
procedure TSpeedDraw.DoFlip;
var
  R1, R: TRect;
begin
  FTimerOdd := not FTimerOdd;
  if FBackSurface = nil then Exit;
  if not FUseExclusive then begin
    try
      BackGroundBlits;
      Windows.GetWindowRect(FHandle, R);
      R1 := Rect(0, 0, 640, 480);
      DDTest(FPrimarySurface.Blt(R, FBackSurface, R1, 0, nil), 'DoFlip');
    except
      on E:Exception do
        ErrorEvent('Flipping');
    end;
  end else begin
    try
      if FActive then begin
        BackGroundBlits;
        repeat until MakeItSo(FPrimarySurface.Flip(nil, DDFLIP_WAIT));
      end;
    except
      on Exception do
        ErrorEvent('Flipping');
    end;
  end;
end;

function TSpeedDraw.LoadPalette(Instance: THandle;
                                 const BitmapName: string): IDirectDrawPalette;
var
  HR: HResult;
  i, NumPalEntries: integer;
  APE: T256PalEntry;
  bfHeader: TBitmapFileHeader;
  biHeader: TBitmapInfoHeader;
  Temp: byte;
begin
  // build a 332 palette as the default.
  for i := 0 to 255 do with APE[ i ] do begin
    peRed := (((i shr 5) and $07) * 255 div 7);
    peGreen := (((i shr 2) and $07) * 255 div 7);
    peBlue := ((i and $03) * 255 div 3);
    peFlags := 0;
  end;

  if BitmapName = '' then begin
    ErrorEvent('No bitmapname in LoadPalette');
    Exit;
  end;

  // get a pointer to the bitmap resource.
  if Instance <> 0 then begin
    APE := GetPaletteFromResFile(Instance, BitmapName, NumPalEntries);
  end else begin
    with TFileStream.Create(BitmapName, fmOpenRead) do
    try
      Read(bfHeader, SizeOf(bfHeader));
      Read(biHeader, SizeOf(biHeader));
      Read(APE, SizeOf(APE));
    finally
      Free;
    end;

    //  get the number of colors in the color table
    if biHeader.biSize <> SizeOf(TBitmapInfoHeader) then
      NumPalEntries := 0
    else if biHeader.biBitCount > 8 then
      NumPalEntries := 0
    else if biHeader.biClrUsed = 0 then
      NumPalEntries := 1 SHL biHeader.biBitCount
    else
      NumPalEntries := biHeader.biClrUsed;

    //  a DIB color table has its colors stored BGR not RGB
    //  so flip them around.
    for i := 0 to NumPalEntries - 1 do with APE[i] do begin
      Temp := peRed;
      peRed := peBlue;
      peBlue := Temp;
    end;
  end;

  // create the DD palette
  HR := FDirectDraw.CreatePalette(DDPCAPS_8BIT, @APE[0], Result, NIL);
  if HR <> DD_OK then
    raise EDDError.CreateFmt('DirectDraw.CreatePalette: %d > %s', [HR, GetOleError(HR)]);
end;

/////////////////////////////////////////////////
// Load a bitmap into memory
/////////////////////////////////////////////////
function TSpeedDraw.GetDib(Instance: THandle; S: string): THandle;
var
  Flags: UINT;
begin
  if Instance <> 0 then
    Flags := LR_CREATEDIBSECTION
  else
    Flags := LR_LOADFROMFILE or LR_CREATEDIBSECTION;

  Result := LoadImage(Instance, PChar(S), Image_Bitmap, 0, 0, Flags);
  if Result = 0 then
    raise EDDError.Create('TSpeedDraw.GetDib: Could not load bitmap: ' + S);
end;

function TSpeedDraw.CreateBlankSurface(Width, Height: Integer): IDirectDrawSurface;
var
  SurfaceDesc: TDDSurfaceDesc;
begin
  try
    FillChar(SurfaceDesc, SizeOf(SurfaceDesc), 0);
    with SurfaceDesc do begin
      dwSize := SizeOf(SurfaceDesc);
      dwFlags := DDSD_CAPS or DDSD_HEIGHT or DDSD_WIDTH;
      ddsCaps.dwCaps := DDSCAPS_OFFSCREENPLAIN;
      dwWidth := Width;
      dwHeight := Height;
    end;

    if FDirectDraw.CreateSurface(SurfaceDesc, Result, NIL) <> DD_OK then
      raise Exception.Create('CreateSurface failed');
  except
    on E: Exception do
      ErrorEvent('TSpeedDraw.CreateBlankSurface: ' + E.Message);
  end;
end;

/////////////////////////////////////////////////
// CreateSurface.
/////////////////////////////////////////////////
function TSpeedDraw.CreateSurface(Bitmap: THandle): IDirectDrawSurface;
var
  SurfaceDesc: TDDSurfaceDesc;
  BM: Windows.TBitmap;
begin
  if Bitmap = 0 then
    raise Exception.Create('No Bitmap in CreateSurface');
  try
    try
      GetObject(Bitmap, SizeOf(BM), @BM);

      FillChar(SurfaceDesc, SizeOf(SurfaceDesc), 0);
      with SurfaceDesc do begin
        dwSize := SizeOf(SurfaceDesc);
        dwFlags := DDSD_CAPS or DDSD_HEIGHT or DDSD_WIDTH;
        ddsCaps.dwCaps := DDSCAPS_OFFSCREENPLAIN;
        dwWidth := BM.bmWidth;
        dwHeight := BM.bmHeight;
      end;

      if FDirectDraw.CreateSurface(SurfaceDesc, Result, NIL) <> DD_OK then
        raise Exception.Create('CreateSurface failed');

        DDCopyBitmap(Result, Bitmap, 0, 0, 0, 0);
    except
      on E: Exception do
        ErrorEvent('TSpeedDraw.CreateSurface: ' + E.Message);
    end;
  finally
    if Bitmap <> 0 then DeleteObject(Bitmap);
  end;
end;

/////////////////////////////////////////////////
// Called when you need to get a bitmap. You don't need a
// bitmap to create the primary surface. Most of the
// work is done in GetDib and CreateSurface
/////////////////////////////////////////////////
function TSpeedDraw.CreateDDSurface(var DDS: IDirectDrawSurface;
                                     BitsName: string; UsePalette: Boolean): Boolean;
var
  ColorKey: TDDColorKey;
  hr: HResult;
  Lib: THandle;
  Dib: THandle;
begin
  Result := False;
  Lib := 0;

  if FDllName <> '' then begin
    Lib := LoadLibrary(PChar(FDllName));
    if Lib < 32 then Exit;
  end;

  if UsePalette then begin
    FPalette := LoadPalette(Lib, PChar(FBackGroundMap));
    if FPalette = nil then begin
      InitFailed;
      Exit;
    end;

    FPrimarySurface.SetPalette(FPalette);
  end;

  Dib := GetDib(Lib, BitsName);
  DDS := CreateSurface(Dib);

  if DDS = nil then begin
    InitFailed;
    Exit;
  end;

  ColorKey.dwColorSpaceLowValue := FTransColor;
  ColorKey.dwColorSpaceHighValue := FTransColor;
  hr := DDS.SetColorKey(DDCKEY_SRCBLT, ColorKey);
  if hr <> DD_OK then
    raise EDDError.CreateFmt('TSpeedDraw.CreateDDSurface: %d %s', [hr, GetOleError(hr)])
  else
    Result := True;

  if Lib <> 0 then FreeLibrary(Lib);
end;

/////////////////////////////////////////////////
// Create the primary surface
/////////////////////////////////////////////////
function TSpeedDraw.CreatePrimary: Boolean;
var
  SurfaceDesc: TDDSurfaceDesc;
  hr: HResult;
begin
  FillChar(SurfaceDesc, sizeOf(TDDSurfaceDesc), 0);
  SurfaceDesc.dwSize := sizeof(TDDSurfaceDesc);

  if not FUseExclusive then begin
    SurfaceDesc.dwFlags := DDSD_CAPS;
    SurfaceDesc.ddsCaps.dwCaps := DDSCAPS_PRIMARYSURFACE;
  end else begin
    SurfaceDesc.dwFlags := DDSD_CAPS or DDSD_BACKBUFFERCOUNT;
    SurfaceDesc.ddsCaps.dwCaps := DDSCAPS_PRIMARYSURFACE or
                                  DDSCAPS_FLIP or
                                  DDSCAPS_COMPLEX;
    SurfaceDesc.dwBackBufferCount := 1;
  end;

  hr := FDirectDraw.CreateSurface(SurfaceDesc, FPrimarySurface, nil);
  if hr <> DD_OK then
    raise EDDError.CreateFmt('TSpeedDraw.CreatePrimary: %d %s', [hr, GetOleError(hr)])
  else
    Result := True;
end;

// In exclusive mode we get the back surface by calling
// GetAttached surface, in windowed mode we just create
// a second surface. See DoFlip.
function TSpeedDraw.SetUpBack: Boolean;
var
  hr: HResult;
  DDSCaps: TDDSCaps;
begin
  Result := False;
  if not FUseExclusive then begin
    FBackSurface := DDCreateSurface(FDirectDraw, 640, 480, False, False, 0);
    if FBackSurface = nil then
    raise EDDError.Create('Can''t set up back surface');
    FClipper := CreateClipper(FDirectDraw, FHandle);
    hr := FPrimarySurface.SetClipper(FClipper);
    if( hr <> DD_OK ) then
      raise EDDError.Create('Can''t attach clipper to front buffer');
  end else begin
    FillChar(DDSCaps, SizeOf(TDDSCaps), 0);
    ddscaps.dwCaps := DDSCAPS_BACKBUFFER;
      hr := FPrimarySurface.GetAttachedSurface(ddscaps, FBackSurface);
    if hr <> DD_OK then
      raise EDDError.CreateFmt('TSpeedDraw.SetUpBack: %d %s', [hr, GetOleError(hr)])
    else
      Result := True;
   end;
end;

procedure TSpeedDraw.SetUpWorkSurface;
var
  SurfaceDesc: TDDSurfaceDesc;

  hr: HResult;
begin
  if not CreateDDSurface(FWorkSurface, FBackGroundMap, True) then
    raise EDDError.Create('TSpeedDraw.SetupWorkSurface: No WorkSurface: ' + FBackGroundMap)
  else begin
    SurfaceDesc.dwSize := SizeOf(TDDSurfaceDesc);
    hr := FWorkSurface.GetSurfaceDesc(SurfaceDesc);
    if hr <> DD_OK then
      raise EDDError.CreateFmt('TSpeedDraw.SetupWorkSurface: No Surface Desc %d %s', [hr, GetOleError(hr)]);
    FBackRect := Rect(0, 0, SurfaceDesc.dwWidth, SurfaceDesc.dwHeight);
  end;
end;


{ Here are the steps in the initialization:
    Create DirectDraw Object
    SetCooperativeLevel
    if Exclusive then SetDisplayMode
    CreatePrimary
    SetupBack
    Create the work surface
    Set Active to true }
procedure TSpeedDraw.InitObjects;
var
  hr: hResult;
  Flags: DWORD;
begin
  ASpeedDraw := Self;
  DDTest(DirectDrawCreate(nil, FDirectDraw, nil), 'InitObjects1');
  if not FUseExclusive then
    Flags := DDSCL_NORMAL
  else
    Flags := DDSCL_EXCLUSIVE or DDSCL_FULLSCREEN;
  DDTest(FDirectDraw.SetCooperativeLevel(FHandle, Flags), 'SetCooperativeLevel');
  if FUseExclusive then begin
    hr := FDirectDraw.SetDisplayMode(640, 480, 8);
    if(hr <> DD_OK) then
      raise EDDError.CreateFmt('TSpeedDraw.InitObjects: %d %s', [hr, GetOleError(hr)]);
  end;
  
  CreatePrimary;
  SetUpBack;
  if FBackgroundMap <> '' then SetUpWorkSurface;

  if FUseTimer then
    SetTimer(FHandle, Timer1, FTimerInterval, @Timer2Timer);

  if Assigned(FInitObjectsProc) then
    FInitObjectsProc(Self);

  FActive := True;
end;

procedure TSpeedDraw.InitFailed;
begin
  FActive := False;
  DestroyObjects;
  ShowMessage('Failed');
end;

procedure TSpeedDraw.Move(Value: Integer);
begin
end; 

function TSpeedDraw.Pause: Boolean;
var
  hr: hResult;
begin
  Result := False;
  FActive := False;
  if FUseTimer then
    KillTimer(FHandle, Timer1);
  hr := FDirectDraw.FlipToGDISurface;
  if hr <> DD_OK then Exit;
  if not FUseExclusive then
    hr := FDirectDraw.SetCooperativeLevel(FHandle, DDSCL_Normal);
  Result := Succeeded(hr);
end;

procedure TSpeedDraw.SetActive(Value: Boolean);
begin
  if Value then
    Restore
  else
    Pause;
end;

procedure TSpeedDraw.ReLoadBitmap(Surface: IDirectDrawSurface; const BitmapName : string);
var
  Bitmap: HBitmap;
  Lib: THandle;
begin
  Lib := 0;
  Bitmap := 0;

  if FDllName <> '' then begin
    Lib := LoadLibrary(PChar(FDllName));
    if Lib < 32 then Exit;
  end;
  
  //  try to load the bitmap as a resource, if that fails, try it as a file
  if Lib <> 0 then
    Bitmap := LoadImage(Lib, PChar(BitmapName),
                        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  try
    if Bitmap = 0 then
      Bitmap := LoadImage( 0, PChar( BitmapName ), IMAGE_BITMAP,
                           0, 0, LR_LOADFROMFILE or LR_CREATEDIBSECTION ) ;
    if Bitmap = 0 then
      Raise Exception.CreateFmt( 'Unable to load bitmap %s', [ BitmapName ]);

    DDCopyBitmap(Surface, Bitmap, 0, 0, 0, 0);
  finally
    DeleteObject(Bitmap);
  end;
  if Lib <> 0 then FreeLibrary(Lib);
end;

/////////////////////////////////////////////////
// Restore the WorkSurface
// May need to be overridden in children to
// restore surfaces other than the backsurface
/////////////////////////////////////////////////
procedure TSpeedDraw.Restore;
var
  hr: HResult;
begin
  FActive := True;
  ddTest(FPrimarySurface.Restore, 'Restore');
  hr := FWorkSurface.Restore;
  if hr = DD_OK then
    ReLoadBitmap(FWorkSurface, FBackGroundMap);
  if FUseTimer then
    SetTimer(FHandle, Timer1, FTimerInterval, @Timer2Timer);
end;


(*
 * DDCreateSurface
 *)
function DDCreateSurface(DD: IDirectDraw; Width, Height: DWord;
  SysMem, Trans: Boolean; dwColorKey: DWord): IDirectDrawSurface;
var
  SurfaceDesc:  TDDSURFACEDESC;
  Hr: HResult;
  ColorKey:  TDDCOLORKEY;
  Surface:  IDIRECTDRAWSURFACE;

begin
 
  // fill in surface desc
  FillChar(SurfaceDesc, SizeOf(TDDSurfaceDesc), 0);
  SurfaceDesc.dwSize := sizeof(SurfaceDesc);
  SurfaceDesc.dwFlags := DDSD_CAPS or DDSD_HEIGHT or DDSD_WIDTH;

  SurfaceDesc.ddsCaps.dwCaps := DDSCAPS_OFFSCREENPLAIN;
  //if sysmem or bUseSysMem  then
  //  ddsd.ddsCaps.dwCaps := SurfaceDesc.ddsCaps.dwCaps or DDSCAPS_SYSTEMMEMORY;

  SurfaceDesc.dwHeight := height;
  SurfaceDesc.dwWidth := width;

  hr := DD.CreateSurface(SurfaceDesc, Surface, nil);

  // set the color key for this bitmap

  if( hr = DD_OK ) then begin
    if( trans ) then begin
      ColorKey.dwColorSpaceLowValue := dwColorKey;
      ColorKey.dwColorSpaceHighValue := dwColorKey;
      Surface.SetColorKey(DDCKEY_SRCBLT, ColorKey);
    end
  end else
    raise EDDError.Create('CreateSurface Failed in DDCreateSurface');

  Result := Surface;
end; // DDCreateSurface


        //
        // now create a DirectDrawClipper object.
        //

function CreateClipper(DD: IDirectDraw; Handle: HWnd): IDirectDrawClipper;
var
  hr: HResult;
  lpClipper: IDirectDrawClipper;
begin
  hr := DD.CreateClipper(0, lpClipper, nil);

  if( hr <> DD_OK ) then begin
    raise EDDError.Create('No Clipper');
    Exit;
  end;

  Hr := lpClipper.SetHWnd(0, Handle);

  if(hr <> DD_OK ) then begin
    raise EDDError.Create('Can''t set clipper window handle');
    Exit;
  end;
  Result := lpClipper;
end;

{$IFDEF USECOM}
procedure TSpeedDraw.MakeActive(Value: Boolean);
begin
  FActive := Value;
end;

procedure TSpeedDraw.SetTimerOdd(Value: Boolean);
begin
  FTimerOdd := Value;
end;

procedure TSpeedDraw.SetBackOrigin(Value: TPoint);
begin
  FBackOrigin := Value;
end;

{$ENDIF}

{$IFDEF Debug}
initialization
  AssignFile(DebugFile, 'C:\DrawBrd1.dbg');
  ReWrite(DebugFile);
finalization
  CloseFile(DebugFile);
{$ENDIF}
end.
