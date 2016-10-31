unit CodeBox;

{ This unit contains various utility routines that I have written
  or collected over the years.

  There are four major categories of routines:

  Bit manipulation
  Math
  Ole
  Palette
  String

  This unit will compile to a smaller size if you define
  DELPHI_LEAN_AND_MEAN. Might do that on shipping builds...
}

interface

uses
  Windows, Classes, SysUtils, Ole2;

{ --- Bit Manipulation --- }

function ShowBits(B: Byte): string;
procedure SetBit(Position: Integer; Value: Byte; var ChangeByte: Byte);
function BitOn(Position: Integer; TestByte: Byte) : Boolean;

{ --- Math Declarations --- }

function ArcCos(x: Real): Real;
function ArcSin(x: Real): Real;
function Comp2Str(N: Comp): String;
function Int2StrPad0(N: LongInt; Len: Integer): String;
function Int2Str(N: LongInt): String;
function IsEqual(R1, R2: Double): Boolean;
function LogXY(x, y: Real): Real;
function Pennies2Dollars(C: Comp): String;
function Power(X: Integer; Y: Integer): Real;
function Real2Str(N: Real; Width, Places: integer): String;
function Str2Comp(MyString: string): Comp;
function Str2Pennies(S: String): Comp;
function Str2Real(MyString: string): Double;
function XToTheY(x, y: Real): Real;

{ --- OLE DECLARATIONS --- }

const
  SOleError = 62211;
  CLSCTX_REMOTE_SERVER = $10;

type
  PMultiQi = ^TMultiQI;
  TMultiQi = record
    IID: PIID;
    Unknown: IUnknown;
    hr: HRESULT;
  end;

PCoAuthIdentity = ^TCoAuthIdentity;
TCoAuthIdentity = record
//  User: PUSHORT;
  UserLength: ULong;
//  Domain: PUShort;
  DomainLength: ULong;
//  Password: PUShort;
  PasswordLength: ULong;
  Flags: ULong;
end;

PCoAuthInfo = ^TCoAuthInfo;
TCoAuthInfo = record
  dwAuthnSvc: DWord;
  dwAuthzSvc: DWord;
  pwszServerPrincName: PWideChar;
  dwImpersonationLevel: DWord;
  pAuthIdentityData: PCoAuthIdentity;
  dwCapabilities: Dword;
end;

PCoServerInfo = ^TCoServerInfo;
TCoserverinfo = record
  dwReserved1: DWord;
  pwszName: PWideChar;
  pAuthInfo: PCoAuthInfo;
  dwReserved2: DWord;
end;

{  PCoServerInfo = ^TCoServerInfo;
  TCoServerInfo = record
    dwSize: DWORD;
    pszName: POleStr;
  end; }

  EOleError = class(Exception);

  PClassInfo = ^TClassInfo;
  TClassInfo = record
    FileName: string;
    ProgID: string;
    ClassID: string;
    Description: string;
  end;

  TMakeGuid = class
  private
    FClassInfo: TClassInfo;
    FGuid: TGuid;
    FClassName: string;
    function GUIDToString: string;
  protected
    destructor Destroy; override;
  public
    constructor Create(AClassName: string); virtual;
    function GUIDToPascalRecord: string;
    function StringGUIDToPascalRecord(S: string): string;
    function GUIDToCStruct: string;
    function CreateRegFile: string;
    procedure UpdateRegistry(DoRegister: Boolean);
    function CreateClassInfo(FileName, ProgID, Description: string;
                             UpdateReg: Boolean): TClassInfo;
    property GUID: TGUID read FGuid;
    property GuidAsString: string read GuidToString;
  end;

function CoCreateInstanceEx(const clsid: TCLSID; unkOuter: IUnknown;
  dwClsContext: Longint; CoServer: PCoServerInfo;
  const CMQ: LongInt; rgmqResults: PMultiQI): HResult; stdcall;


function AnsiToUnicode(S: string; var NewSize: Integer): PWideChar;
function CLSIDToStr(ID: TCLSID): string;
// function CreateRemoteOleObject(const ClassName, Server: string): Variant;
function CreateRemoteOleObject(ClassID: TCLSID; const Server: string): Variant;
function CreateLocalOleObject(ClassID: TGUID): Variant;
procedure CreateRegKey(const Key, Value: string);
procedure DeleteRegKey(const Key: string);
function GetCLSIDName(iid: TCLSID): string;
function GetNameOfCLSID(iid: TIID): string;
// Given an Interface, find it's name
function GetNameOfInterfaceID(iid: TIID): string;
procedure OleError(ErrorCode: HResult);
procedure OleSucceeded(hr: HResult);
function GetOleError(ErrorCode: HResult): ShortString;
function UnicodeToAnsi(S: PWideChar): string;

{ --- Palette Manipulation --- }

type
  T256PalEntry = array[0..255] of TPALETTEENTRY;

  PRGB = ^TRGB;
  TRGB = array[ 0..255 ] of TRGBQuad;

  { Given a PSP text file palette create a windows palette }

  TFilePalette = class
  private
    FDC: HDC;
    FFileName: string;
    FHandle: HWnd;
    FOldPal: HPalette;
    FPalette: HPalette;
    FPalEntries: T256PalEntry;
    function MakePalette: Boolean;
    function ReadPalette: Boolean;
  protected
    destructor Destroy; override;
  public
    constructor Create(AHandle: HWnd; AFileName: string); virtual;
    function GetPalette: HPalette;
    function RealizePalette: HDC;
    property Palette: HPalette read FPalette write FPalette;
  end;

procedure DrawPalette(DC: HDC);
function GetPaletteFromResFile(Instance: THandle;
  BitmapName: string; var NumPalEntries: Integer): T256PalEntry;
procedure MakePaletteCurrent(Handle: HWnd; Pal: T256PalEntry);
procedure ReadPal(FileName: string; var P: T256PalEntry);
procedure WritePal(FileName: string; var P: T256PalEntry);
// procedure WriteError(ErrorCode: HResult);
procedure AppendError(ErrorCode: HResult; ErrStr: string);
procedure SaveClipBoardBitmap(BitMap: HBitMap; FileName: string);
procedure ShowFilePalette(Handle: HWnd; AFileName: string);
procedure ShowDCCaps(DC: HDC);

{ --- STRING DECLARATIONS --- }

const
  CR = #13#10;

type
  Str12 = string[12];
  DirStr = string[67];
  PathStr = string[79];
  NameStr = string[8];
  ExtStr = string[4];

function Address2Str(Addr : Pointer) : string;
function AddBackSlash(S: string): string;
function CleanString(S: string): string;
function GetFirstWord(S: string): string;
function GetFirstToken(S: string; Token: Char): string;
function GetHexWord(w: Word): string;
function GetLastToken(S: string; Token: Char): string;
function GetLastWord(S: string): string;
{$IFNDEF WIN32}
function GetLogicalAddr(A: Pointer): Pointer;
{$ENDIF}
function GetStartDir: string;
function GetTodayName(Pre, Ext: string): string;
function GetTodaysDate: string;
function GetTimeString: string;
function GetTimeFormat: string;
function IsNumber(Ch: Char): Boolean;
function LeftSet(src: string; Width:Integer; var Trunc: Boolean): String;
function ReplaceChars(S: string; OldCh, NewCh: Char): string;
function RightCharSet(Src: string; Width: Integer;
                      Ch: Char; var Trunc: Boolean): string;
function RemoveFirstWord(var S : String) : String;
function ReplaceString(NewStr, ReplaceStr, Data: string): string;
function ReplaceAllInstancesOfString(NewStr, ReplaceStr: string;
                                     var Data: string): Boolean;
function ReverseStr(S: string): string;
function Shorten(S: string; Cut: Integer): string;
procedure SplitDirName(Path : PathStr; var Dir: DirStr; var WName: Str12);
function StripBlanks(S: string): string;
function StripEndChars(S: string; Ch: Char): string;
function StripFirstWord(S : string) : string;
function StripFirstToken(S: string; Ch: Char): string;
function StripFrontChars(S: string; Ch: Char): string;
function StripFromFront(S: string; Len: Integer): string;
function StripLastToken(S: string; Token: Char): string;
{$IFNDEF WIN32}
procedure SetLength(var S: string; i: Integer);
{$ENDIF}

{ --- Storage --- }

type
  TSafeStore = class(TObject)
  private
    FStorageStrings: TStringList;
    FStorage: IStorage;
    procedure CreateStorage(FileName: string);
    procedure OpenStorage(FileName: string);
    function ShowStorageElement(S: string; StatStg: TStatStg): Integer;
    procedure HandleProperty(Storage: IStorage);
    procedure HandleSubStorage(var Storage: IStorage; StatStg: TStatStg);
    procedure EnumStorageElements(var Storage: IStorage);
    function RefreshStorageStrings: TStringList; 
    destructor Destroy; override;
  public
    constructor Create(FileName: string); virtual;
    procedure DestroyElement(S: string);
    function GetNewStream(StreamName: string): IStream;
    function OpenStream(StreamName: string): IStream;
    procedure WriteTextToStorage(StreamName: string; Value: string);
    function ReadTextFromStream(StreamName: string): string;
    procedure ReadInteger(Stream: IStream; var Num: Integer);
    procedure WriteInteger(Stream: IStream; Num: Integer);
    procedure ReadString(Stream: IStream; var S: string);
    procedure WriteString(Stream: IStream; S: string);
    property StgStrings: TStringList read FStorageStrings;
    property Storage: IStorage read FStorage;
  end;

function ReadStringFromStorage(StorageName: string; StreamName: string): string;
procedure WriteStreamToStorage(StorageName, StreamName, Value: string);
implementation

uses
  Graphics, OleAuto,
  Dialogs, ClipBrd, DDraw,
  Registry;

{ ------------------------- }
{ ---  BIT MANIPULATION --- }
{ ------------------------- }

{ This function accepts a byte parameter and returns
  a string of eight ones and zeros indicating the
  binary form of a bite. }
function ShowBits(B : Byte): string;
var
  i: Integer;
  bt: Byte;
  s: string;
begin
  bt := $01;
  s := '';
  for i := 1 to 8 do begin
    if (b And bt) > 0 then
       S := '1' + s
    else
      s := '0' + s;
     {$R-}
      bt := bt shl 1;
     {$R+}
    end;
  ShowBits := s;
end;

{ This procedure sets a particular bit in the byte changebyte
  to either 1 or 0. The bit is specified by Position, which can range
  from 0 to 7. In Value, put 1 if you want the bit Position set to 1
  and put 0 if you want bit Position set to 0. The right byte is Position
  0, the far left is Position 7. Based on a routine found in
  Turbo Pascal by Stephen K O'Brian. }
procedure SetBit(Position : Integer; Value : Byte; var ChangeByte : Byte);
var
  Bt : Byte;
begin
  bt := $01;
  bt := bt shl Position;
  if Value = 1 then
    ChangeByte := ChangeByte or bt
  else begin
    bt := bt xor $FF;
    ChangeByte := ChangeByte and bt;
  end;
end;

{ This function tests if a bit in TestByte is turned on (equal to 1).
  If the bit indicated by Position is turned on, then BitOn returns True. }
function BitOn(Position : Integer; TestByte : Byte) : Boolean;
var
  bt : Byte;
begin
  bt := $01;
  bt := bt shl Position;
  BitOn := (bt and TestByte) > 0;
end;


{ ------------------------ }
{ ---  MATH ROUTINES --- }
{ ------------------------ }

{----------------------------------------------------
       Name: ArcCos function
Declaration: function ArcCos(x: Real): Real;
       Unit: MathBox
       Code: N
       Date: 02/20/94
Description: Find the ArcCos of a Real
-----------------------------------------------------}
function ArcCos(x: Real): Real;
begin
  ArcCos := ArcTan(Sqrt(1 - Sqr(x)) / x);
end;


{----------------------------------------------------
       Name: ArcSin function
Declaration: function ArcSin(x: Real): Real;
       Unit: MathBox
       Code: N
       Date: 02/20/94
Description: Find the ArcSin of a Real
-----------------------------------------------------}
function ArcSin(x: Real): Real;
begin
  ArcSin := ArcTan(x / Sqrt( 1 - Sqr(x)));
end;

{----------------------------------------------------
       Name: Comp2Str function
Declaration: Comp2Str(N: real; Width, Places: integer)
       Unit: MathBox
       Code: N
       Date: 02/17/94
Description: Converts a Comp into a String
-----------------------------------------------------}
function Comp2Str(N: Comp): String;
var
  TempString: String;
begin
  Str(N:0:0, TempString);
  Comp2Str := TempString;
end;

{----------------------------------------------------
       Name: Int2Str function
Declaration: Int2Str(N: LongInt): String;
       Unit: MathBox
       Code: N
       Date: 06/25/94
Description: Converts a number into a string and pads
             the string with zeros if it is less than
             Len characters long.
-----------------------------------------------------}
function Int2Str(N: LongInt): String;
var
  S : String;
begin
  Str(N:0,S);
  Int2Str := S;
end;

{----------------------------------------------------
       Name: Int2StrPad0 function
Declaration: Int2StrPad0(N: LongInt; Len: Integer): String;
       Unit: MathBox
       Code: N
       Date: 03/01/94
Description: Converts a number into a string and pads
             the string with zeros if it is less than
             Len characters long.
-----------------------------------------------------}
function Int2StrPad0(N: LongInt; Len: Integer): string;
begin
  FmtStr(Result, '%d', [N]);
  while Length(Result) < Len do
    Result := '0' + Result;
end;

{----------------------------------------------------
       Name: IsEqual function
Declaration: IsEqual(R1, R2: Double): Boolean;
       Unit: MathBox
       Code: N
       Date: 07/04/94
Description: Tests to see if two doubles are effectively
             equal. Floating point numbers are never
             exact, so we need an approximation.
-----------------------------------------------------}
function IsEqual(R1, R2: Double): Boolean;
var
  R : Double;
begin
  R := Abs(R1 - R2);
  if R > 0.0001 then
    IsEqual := False
  else
    IsEqual := True;
end;

{----------------------------------------------------
       Name: LogXY function
Declaration: function LogXY(x: Real): Real;
       Unit: MathBox
       Code: N
       Date: 02/20/94
Description: Log of X Y
-----------------------------------------------------}
function LogXY(x, y: Real): Real;
begin
  LogXY := Ln(x) / Ln(y);
end;

{----------------------------------------------------
       Name: Pennies2Dollars function
Declaration: Pennies2Dollars(C: Comp): String;
       Unit: MathBox
       Code: N
       Date: 02/17/94
Description: Converts a Comp type that represents a
	     certain number of pennies into a string
             with two decimal places. 123 => $1.23
-----------------------------------------------------}
function Pennies2Dollars(C: Comp): String;
var
  S: string;
begin
  S := Comp2Str(C);
  Insert('.', S, Length(S) - 1);

  if S[1] = '-' then begin           { Number negative? }
    S := StripFrontChars(S, '-');
    S := '-$' + S;
  end else
    S := '$' + S;

  Pennies2Dollars := S;
end;

{----------------------------------------------------
       Name: Power function
Declaration: Power(X: Integer; Y: Integer): Real;
       Unit: MathBox
       Code: N
       Date: 02/20/94
Description: Raise X to the Y power
-----------------------------------------------------}
function Power(X: Integer; Y: Integer): Real;
var
  Count: Integer;
  OutCome: Real;
begin
  OutCome := 1;
  for Count := 1 to Y do
    OutCome := OutCome * X;
  Power := OutCome;
end;

{----------------------------------------------------
       Name: Real2Str function
Declaration: Real2Str(N: real; Width, Places: integer)
       Unit: MathBox
       Code: N
       Date: 02/17/94
Description: Converts a Real number into a String
-----------------------------------------------------}
function Real2Str(N: Real; Width, Places: integer): String;
var
  TempString: String;
begin
  Str(N:Width:Places, TempString);
  Real2Str := TempString;
end;

{----------------------------------------------------
       Name: Str2Comp function
Declaration: Str2Real(MyString: string)
       Unit: MathBox
       Code: N
       Date: 02/17/94
Description: Converts a String to a Comp
-----------------------------------------------------}
function Str2Comp(MyString: string): Comp;
var
  ErrCode: Integer;
  Temp: Comp;
begin
  If Length(Mystring) = 0 then Str2Comp := 0
  else begin
    Val(Mystring, Temp, ErrCode);
    if ErrCode = 0 then
      Str2Comp := temp
    else
      Str2Comp := 0;
  end;
end;

{----------------------------------------------------
       Name: Str2Pennies function
Declaration: Str2Pennies(MyString: string)
       Unit: MathBox
       Code: N
       Date: 02/17/94
Description: Converts a String to a Comp
-----------------------------------------------------}
function Str2Pennies(S: String): Comp;
var
  C: Comp;
  i: Integer;
  begin
    if S[1] = '$' then Delete(S, 1, 1);
    i := Pos('.', S);
    if i = Length(S) then begin   { Is last character a period? }
      Delete(S, i, 1);
      S := S + '00';
    end else
      if i <> 0 then begin        { Some pennies?               }
        Delete(S, i, 1);
        if i = (Length(S)) then   { Only one char after decimal?}
          S := S + '0'
      end else
        S := S + '00';            { No decimal, no pennies       }
    C := Str2Comp(S);
    Str2Pennies := C;
end;

{----------------------------------------------------
       Name: Str2Real function
Declaration: Str2Real(MyString: string)
       Unit: MathBox
       Code: N
       Date: 02/17/94
Description: Converts a String to Real number
-----------------------------------------------------}
function Str2Real(MyString: string): Double;
var
  ErrCode: Integer;
  Temp: Double;
begin
  If Length(Mystring) = 0 then Str2Real := 0
  else begin
    Val(Mystring, Temp, ErrCode);
    if ErrCode = 0 then
      Str2Real := temp
    else
      Str2Real := 0;
  end;
end;

{----------------------------------------------------
       Name: XToTheY function
Declaration: XToTheY(x, y: Real): Real;
       Unit: MathBox
       Code: N
       Date: 02/20/94
Description: Raise X to the Y Power
-----------------------------------------------------}
function XToTheY(x, y: Real): Real;
begin
  XToTheY := Exp(y * Ln(x));
end;

{ ------------------------------------------------------ }
{ ---  OLE ROUTINES  ----------------------------------- }
{ ------------------------------------------------------ }

{ --- TMakeGuid --- }

// Not exactly rocket science....
constructor TMakeGuid.Create(AClassName: string);
begin
  CoInitialize(nil);
  CoCreateGuid(FGUID);
  FClassName := AClassName;
end;

destructor TMakeGuid.Destroy;
begin
  CoUninitialize;
  inherited Destroy;
end;

function TMakeGuid.GUIDToString: string;
var
  P: PWideChar;
begin
  StringFromCLSID(FGUID, P);
  Result := WideCharToString(P);
end;

function TMakeGuid.StringGuidToPascalRecord(S: string): string;
var
  Len, i: Integer;
begin
  S := ReplaceString('D1:$', '{', S);
  S := ReplaceString(';D2:$', '-', S);
  S := ReplaceString(';D3:$', '-', S);
  S := ReplaceString(';D4:($', '-', S);
  S := ReplaceString(',$', '-', S);
  S := ReplaceString('));', '}', S);
  for i := 1 to 7 do begin
    Len := Length(S);
    if i <> 6 then
      Insert(',$', S, Len - (4 * i));
  end;
  S := '  CLSID_' + FClassName + ': TGUID = (' + #13#10#32#32#32#32 +  S;
  Result := S;
end;

// Convert Windows GUID to Delphi GUID
function TMakeGuid.GUIDToPascalRecord: string;
var
  S: string;
begin
  S := GUIDToString;
  Result := StringGuidToPascalRecord(S);
end;

function TMakeGuid.GUIDToCStruct: string;
var
  Len, i: Integer;
  AClassName, S: string;
begin
  AClassName := UpperCase(FClassName);
  S := GuidToString;
  S := ReplaceString('0x', '{', S);
  S := ReplaceString(', 0x', '-', S);
  S := ReplaceString(', 0x', '-', S);
  S := ReplaceString(', 0x', '-', S);
  S := ReplaceString('', '-', S);
  S := ReplaceString(');', '}', S);
  for i := 1 to 7 do begin
    Len := Length(S);
    Insert(',0x', S, Len - (3 + ((i - 1) * 5)));
  end;
  S := 'DEFINE_GUID(CLSID_' + AClassname + ', ' + S;
  Result := S;
end;

function TMakeGuid.CreateRegFile: string;
var
  GuidAsString: string;
const
  RegString = 'REGEDIT' + CR +
   'HKEY_CLASSES_ROOT\%s1.0 = %s Object' + CR +
   'HKEY_CLASSES_ROOT\%s1.0\CLSID = %s' + CR +
   'HKEY_CLASSES_ROOT\%s = %s Object' + CR +
   'HKEY_CLASSES_ROOT\%s\CurVer = %s1.0' +  CR +
   'HKEY_CLASSES_ROOT\%s\CLSID = %s' + CR +
   'HKEY_CLASSES_ROOT\CLSID\%s = %s Object'+ CR +
   'HKEY_CLASSES_ROOT\CLSID\%s\ProgID = %s1.0' + CR +
   'HKEY_CLASSES_ROOT\CLSID\%s\VersionIndependentProgID = %s' + CR +
   'HKEY_CLASSES_ROOT\CLSID\%s\InprocServer32 = e:\src\Maze\%s\%s.dll' + CR +
   'HKEY_CLASSES_ROOT\CLSID\%s\NotInsertable';
begin
  GuidAsString := GUIDToString;
  Result := Format(RegString, [FClassName, FClassName, FClassName, GuidAsString,
    FClassName, FClassName, FClassName, FClassName, FClassName, GuidAsString,
    GuidAsString, FClassName, GuidAsString, FClassName, GuidAsString, FClassName, GuidAsString, FClassName,
    FClassName, GuidAsString]);
end;

function TMakeGuid.CreateClassInfo(FileName, ProgID, Description: string;
                                   UpdateReg: Boolean): TClassInfo;
begin
  FClassInfo.ClassID := GUIDAsString;
  FClassInfo.FileName := FileName;
  FClassInfo.ProgID := ProgID;
  FClassInfo.Description := Description;
  If UpdateReg then UpdateRegistry(True);
  Result := FClassInfo;
end;

/////////////////////////////////////////////////
// Given a TClassInfo structure, as defined in this unit,
// either register, or unregister the class in the
// registration database. Use REGEDIT.EXE to view results.
/////////////////////////////////////////////////
procedure TMakeGuid.UpdateRegistry(DoRegister: Boolean);
begin
  if DoRegister then begin
    CreateRegKey(FClassInfo.ProgID, FClassInfo.Description);
    CreateRegKey(FClassInfo.ProgID + '\Clsid', FClassInfo.ClassID);
    CreateRegKey('CLSID\' + FClassInfo.ClassID, FClassInfo.Description);
    CreateRegKey('CLSID\' + FClassInfo.ClassID + '\ProgID', FClassInfo.ProgID);
    CreateRegKey('CLSID\' + FClassInfo.ClassID + '\InprocServer32', FClassInfo.FileName);
  end else begin
    DeleteRegKey('CLSID\' + FClassInfo.ClassID + '\InprocServer32');
    DeleteRegKey('CLSID\' + FClassInfo.ClassID + '\ProgID');
    DeleteRegKey('CLSID\' + FClassInfo.ClassID);
    DeleteRegKey(FClassInfo.ProgID + '\Clsid');
    DeleteRegKey(FClassInfo.ProgID);
  end;
end;

{ This function returns the size of the allocated string in NewSize.
  YOu have to free up this memory yourself. }
function AnsiToUnicode(S: string; var NewSize: Integer): PWideChar;
var
  Size: Integer;
  P: PWideChar;
begin
  Size := Length(S);
  NewSize := Size * 2;
  P := VirtualAlloc(nil, Size, Mem_Commit, Page_ReadWrite);
  MultiByteToWideChar(CP_ACP, 0, PChar(S), Size, P, NewSize);
  Result := P;
end;

function CLSIDToStr(ID: TCLSID): string;
var
  hr: hResult;
  WideString: PWideChar;
begin
  hr := StringFromCLSID(ID, WideString);
  if hr < 0 then OleError(hr);
  Result := UnicodeToAnsi(WideString);
end;


const
  ole32    = 'ole32.dll';

function CoCreateInstanceEx; external ole32 name 'CoCreateInstanceEx';

{ TCoserverinfo = record
  dwReserved1: DWord;
  pwszName: PWideChar;
  pAuthInfo: PCoAuthInfo;
  dwReserved2: DWord;
end; }

function GetRemoteOleObject(ClassID: TGUID; const Server: string): Variant;
var
  Unknown: IUnknown;
  ClassFactory: IClassFactory;
  Info: TCoServerInfo;
  Dest: Array[0..127] of WideChar;
begin
  ClassFactory := nil;
  Info.dwReserved1 := 0;
  Info.pwszName := StringToWideChar(Server, Dest, SizeOf(Dest) div 2);
  Info.pAuthInfo := nil;
  Info.dwReserved2 := 0; 
  OleCheck(CoGetClassObject(ClassID, CLSCTX_REMOTE_SERVER, @Info,
                            IID_IClassFactory, ClassFactory));
  if ClassFactory = nil then
    ShowMessage('No Class Factory')
  else
    ClassFactory.CreateInstance(nil, IID_IUnknown, Unknown);
  try
    Result := VarFromInterface(Unknown);
  finally
    ClassFactory.Release;
    Unknown.Release;
  end;
end;

function CreateRemoteOleObject(ClassID: TGUID; const Server: string): Variant;
var
  Info: TCoServerInfo;
  Dest: Array[0..127] of WideChar;
  MultiQI: TMultiQi;
begin
  MultiQi.IID := @IID_IDispatch;
  MultiQI.Unknown := nil;
  FillChar(Info, sizeOF(Info), #0);
  Info.pwszName := StringToWideChar(Server, Dest, SizeOf(Dest) div 2);
  OleCheck(CoCreateInstanceEx(ClassID, nil, CLSCTX_REMOTE_SERVER,
                              @Info, 1, @MultiQI));
  try
    Result := VarFromInterface(MultiQI.Unknown);
  finally
    MultiQi.Unknown.Release;
  end;
end;

function CreateLocalOleObject(ClassID: TGUID): Variant;
var
  Unknown: IUnknown;
  ClassFactory: IClassFactory;
begin
  ClassFactory := nil;
  OleCheck(CoGetClassObject(ClassID, CLSCTX_LOCAL_SERVER, nil,
                            IID_IClassFactory, ClassFactory));
  if ClassFactory = nil then
    ShowMessage('No Class Factory')
  else
    ClassFactory.CreateInstance(nil, IID_IUnknown, Unknown);
  try
    Result := VarFromInterface(Unknown);
  finally
    ClassFactory.Release;
    Unknown.Release;
  end;
end;

procedure CreateRegKey(const Key, Value: string);
begin
  RegSetValue(HKEY_CLASSES_ROOT, PChar(Key), REG_SZ, PChar(Value),
    Length(Value));
end;

procedure DeleteRegKey(const Key: string);
begin
  RegDeleteKey(HKEY_CLASSES_ROOT, PChar(Key));
end;

function GetCLSIDName(iid: TCLSID): string;
var
  S: string;
begin
  if IsEqualIID(iid, IID_IUnknown) then
    S := 'IID_IUnknown'
  else if IsEqualIID(iid, IID_IClassFactory) then
    S := 'IID_IClassFactory'
  else if IsEqualIID(iid, IID_IMarshal) then
    S := 'IID_IMarshal'
  else if IsEqualIID(iid, IID_IStdMarshalInfo) then
    S := 'IID_IStdMarshalInfo'
  else if IsEqualIID(iid, IID_IExternalConnection) then
    S := 'IID_IExternalConnection'
  else
    S := CLSIDToStr(iid);
  Result := S;
end;


function GetNameOfCLSID(iid: TIID): string;
var
  Registry: TRegistry;
  S: string;
  P: PWideChar;
begin
  OleCheck(StringFromCLSID(iid, P));
  S := WideCharToString(P);
  Registry := TRegistry.Create;
  Registry.RootKey := HKEY_CLASSES_ROOT;
  Registry.OpenKey('CLSID', False);
  Registry.OpenKey(S, False);
  Result := Registry.ReadString('');
  Registry.Free;
end;

function GetNameOfInterfaceID(iid: TIID): string;
var
  Registry: TRegistry;
  S: string;
  P: PWideChar;
begin
  OleCheck(StringFromCLSID(iid, P));
  S := WideCharToString(P);
  Registry := TRegistry.Create;
  Registry.RootKey := HKEY_CLASSES_ROOT;
  Registry.OpenKey('Interface', False);
  Registry.OpenKey(S, False);
  Result := Registry.ReadString('');
  if Result = '' then
    Result := 'Could Not Find ID: ' + S;
  Registry.Free;
end;

procedure OleError(ErrorCode: HResult);
var
  Message: string;
begin
  Message := SysErrorMessage(ErrorCode);
  if Message = '' then FmtStr(Message, LoadStr(SOleError), [ErrorCode]);
  raise EOleError.Create(Message);
end;

procedure OleSucceeded(hr: HResult);
begin
  if not Succeeded(hr) then
    OleError(hr);
end;

{ Here are the error constants from DDRAW.PAS. I usually
  don't give an error string, but give you a number you can
  search on in DDRAW. If you look in DDRAW.INC, you'll see why
  giving you the raw hex number is useless.

  If the memory used by this method is too heavy, define
  DELPHI_LEAN_AND_MEAN and it will be linked out. 
}

{$IFDEF DELPHI_LEAN_AND_MEAN}
function GetOleError(ErrorCode: HResult): ShortString;
var
  S: string;
begin
  S := SysErrorMessage(ErrorCode);
  if S = '' then
      FmtStr(S, LoadStr(SOleError), [ErrorCode]);
  Result := 'GetOleError: ' + S;
end;

{$ELSE}

function GetOleError(ErrorCode: HResult): ShortString;
var
  S: string;
begin
  case ErrorCode of
    DDERR_ALREADYINITIALIZED: S := 'Already Initialized';
    DDERR_CANNOTATTACHSURFACE: S := 'Can''t Attach Surface';
    DDERR_CANNOTDETACHSURFACE: S := 'CANNOT DETACH SURFACE';
    DDERR_CURRENTLYNOTAVAIL: S := 'Currently Not Available';
    DDERR_EXCEPTION: S := 'Exception';
    DDERR_GENERIC: S := 'E_FAIL';
    DDERR_HEIGHTALIGN: S := 'Height Align';
    DDERR_INCOMPATIBLEPRIMARY: S := 'Incompatible Primary';
    DDERR_INVALIDCAPS: S := 'Invalid Caps';
    DDERR_INVALIDCLIPLIST: S := 'Invalid Clip List';
    DDERR_INVALIDMODE: S := 'Invalid Mode';
    DDERR_INVALIDOBJECT: S := 'Invalid Object';
    DDERR_INVALIDPARAMS: S := 'E_INVALIDARG';
    DDERR_INVALIDPIXELFORMAT: S := 'Invalid Pixel Format';
    DDERR_INVALIDRECT: S := 'Invalid Rectangle';
    DDERR_LOCKEDSURFACES: S := 'Locked Surfaces';
    DDERR_NO3D: S := 'No 3D';
    DDERR_NOALPHAHW: S := 'No Alpha HW';
    DDERR_NOCLIPLIST: S := 'No Clip List';
    DDERR_NOCOLORCONVHW: S := 'No Color Conv HW';
    DDERR_NOCOOPERATIVELEVELSET: S := 'No Cooperative Level Set';
    DDERR_NOCOLORKEY: S := 'No Color Key';
    DDERR_NOCOLORKEYHW: S := 'No Color Key HW';
    DDERR_NODIRECTDRAWSUPPORT: S := 'No Direct Draw Support';
    DDERR_NOEXCLUSIVEMODE: S := 'No Exclusive Mode';
    DDERR_NOFLIPHW: S := 'No Flip HW';
    DDERR_NOGDI: S := 'No GDI';
    DDERR_NOMIRRORHW: S := '250';
    DDERR_NOTFOUND: S := '255';
    DDERR_NOOVERLAYHW: S := '260';
    DDERR_NORASTEROPHW: S := '280';
    DDERR_NOROTATIONHW: S := '290';
    DDERR_NOSTRETCHHW: S := '310';
    DDERR_NOT4BITCOLOR: S := '316';
    DDERR_NOT4BITCOLORINDEX: S := '317';
    DDERR_NOT8BITCOLOR: S := '320';
    DDERR_NOTEXTUREHW: S := '330';
    DDERR_NOVSYNCHW: S := '335';
    DDERR_NOZBUFFERHW: S := '340';
    DDERR_NOZOVERLAYHW: S := '350';
    DDERR_OUTOFCAPS: S := '360';
    DDERR_OUTOFMEMORY: S := 'E_OUTOFMEMORY';
    DDERR_OUTOFVIDEOMEMORY: S := 'Out Of Video Memory';
    DDERR_OVERLAYCANTCLIP: S := '382';
    DDERR_OVERLAYCOLORKEYONLYONEACTIVE: S := 'Overlay Color Key Only One Active';
    DDERR_PALETTEBUSY: S := 'Palette Busy';
    DDERR_COLORKEYNOTSET: S := 'ColorKey Not Set';
    DDERR_SURFACEALREADYATTACHED: S := 'Surface Already Attached';
    DDERR_SURFACEALREADYDEPENDENT: S := 'Surface Already Dependent';
    DDERR_SURFACEBUSY: S := 'Surface Busy';
    DDERR_SURFACEISOBSCURED: S := 'Surface Is Obscured';
    DDERR_SURFACELOST: S := 'Surface Lost';
    DDERR_SURFACENOTATTACHED: S := 'Surface Not Attached';
    DDERR_TOOBIGHEIGHT: S := 'Too Big Height';
    DDERR_TOOBIGSIZE: S := 'Too Big Size';
    DDERR_TOOBIGWIDTH: S := 'Too Big Width';
    DDERR_UNSUPPORTED: S := 'E_NOTIMPL';
    DDERR_UNSUPPORTEDFORMAT: S := 'Unsupported Format';
    DDERR_UNSUPPORTEDMASK: S := '520';
    DDERR_VERTICALBLANKINPROGRESS: S := '537';
    DDERR_WASSTILLDRAWING: S := '540';
    DDERR_XALIGN: S := '560';
    DDERR_INVALIDDIRECTDRAWGUID: S := 'Invalid Direct Draw GUID';
    DDERR_DIRECTDRAWALREADYCREATED: S := '562';
    DDERR_NODIRECTDRAWHW: S := '563';
    DDERR_PRIMARYSURFACEALREADYEXISTS: S := '564';
    DDERR_NOEMULATION: S := '565';
    DDERR_REGIONTOOSMALL: S := '566';
    DDERR_CLIPPERISUSINGHWND: S := '567';
    DDERR_NOCLIPPERATTACHED: S := '568';
    DDERR_NOHWND: S := '569';
    DDERR_HWNDSUBCLASSED: S := '570';
    DDERR_HWNDALREADYSET: S := '571';
    DDERR_NOPALETTEATTACHED: S := '572';
    DDERR_NOPALETTEHW: S := '573';
    DDERR_BLTFASTCANTCLIP: S := '574';
    DDERR_NOBLTHW: S := '575';
    DDERR_NODDROPSHW: S := '576';
    DDERR_OVERLAYNOTVISIBLE: S := '577';
    DDERR_NOOVERLAYDEST: S := '578';
    DDERR_INVALIDPOSITION: S := '579';
    DDERR_NOTAOVERLAYSURFACE: S := '580';
    DDERR_EXCLUSIVEMODEALREADYSET: S := '581';
    DDERR_NOTFLIPPABLE: S := '582';
    DDERR_CANTDUPLICATE: S := '583';
    DDERR_NOTLOCKED: S := '584';
    DDERR_CANTCREATEDC: S := '585';
    DDERR_NODC: S := '586';
    DDERR_WRONGMODE: S := '587';
    DDERR_IMPLICITLYCREATED: S := '588';
    DDERR_NOTPALETTIZED: S := '589';
    DDERR_UNSUPPORTEDMODE: S := '590';
  else
    S := SysErrorMessage(ErrorCode);
    if S = '' then
      FmtStr(S, LoadStr(SOleError), [ErrorCode]);
  end;
  Result := 'GetOleError: ' + S;
end;
{$ENDIF}

function UnicodeToAnsi(S: PWideChar): string;
var
  S1: PChar;
  i: Integer;
begin
  i := lstrlenw(S) + 1;
  GetMem(S1, 500);
  WideCharToMultiByte(CP_ACP, 0, S, i, S1, i * 2, nil, nil);
  Result := S1;
  FreeMem(S1, 500);
end;

{ ------------------------------------------------------ }
{ --- PALETTE ROUTINES --------------------------------- }
{ ------------------------------------------------------ }

{ --- TFilePalette --- }

constructor TFilePalette.Create(AHandle: HWnd; AFileName: string);
begin
  FHandle := AHandle;
  FFileName := AFilename;
  FDC := GetDC(FHandle);
end;

destructor TFilePalette.Destroy;
begin
  if FPalette <> 0 then
    DeleteObject(FPalette);
  if FDC <> 0 then begin
    SelectPalette(FDC, FOldPal, True);
    ReleaseDC(FHandle, FDC);
  end;
  inherited Destroy;
end;

{ Call only if you are not calling RealizePalette }
function TFilePalette.GetPalette: HPalette;
begin
  ReadPalette;
  MakePalette;
  Result := FPalette;
end;

function TFilePalette.MakePalette: Boolean;
var
  Log: PLogPalette;
begin
  GetMem(Log, 4 + (256 * SizeOf(TPaletteEntry)));
  Log^.palVersion := $300;
  Log^.palNumEntries := 256;
  Move(FPalEntries, Log^.palPalEntry, SizeOf(T256PalEntry));
  FPalette := CreatePalette(Log^);
  FreeMem(Log, 4 + (256 * SizeOf(TPaletteEntry)));
  Result := True;
end;

{ Automatically calls GetPallette }
function TFilePalette.RealizePalette: HDC;
begin
  GetPalette;
  FOldPal := SelectPalette(FDC, FPalette, True);
  Windows.RealizePalette(FDC);
  Result := FDC;
end;

function TFilePalette.ReadPalette: Boolean;
begin
  ReadPal(FFileName, FPalEntries);
  Result := True;
end;

{ --- Misc Routines --- }

procedure DrawPalette(DC: HDC);
var
  i, j: Integer;
  AColor: TColorRef;
  AnIndex, X, Y: Integer;
  OldBrush: HBrush;
  Brush: TBrush;
begin
  AnIndex := 0;
  Brush := TBrush.Create;
  for j := 1 to 16 do
    for i := 0 to 15 do begin
      X := i * 25 + 10;
      Y := j * 25 + 10;
      AColor := PaletteIndex(AnIndex);
      Brush.Color := AColor;
      OldBrush := SelectObject(DC, Brush.Handle);
      Rectangle(DC, X, Y, X + 15, Y + 15);
      SelectObject(DC, OldBrush);
      Inc(AnIndex);
    end;
  Brush.Free;
end;

procedure MakePaletteCurrent(Handle: HWnd; Pal: T256PalEntry);
var
  OldPal, hPal: hPalette;
  Log: PLogPalette;
  DC: HDC;
begin
  DC := GetDC(Handle);
  GetMem(Log, 4 + (256 * SizeOf(TPaletteEntry)));
  Log^.palVersion := $300;
  Log^.palNumEntries := 256;
  Move(Pal, Log^.palPalEntry, SizeOf(Pal));
  hPal := CreatePalette(Log^);
  OldPal := SelectPalette(DC, hPal, True);
  ShowMessage(IntToStr( RealizePalette(DC) ));

  SelectPalette(DC, OldPal, True);
  DeleteObject(hPal);
  ReleaseDC(Handle, DC);
  FreeMem(Log, 4 + (256 * SizeOf(TPaletteEntry)));
end;

function GetPaletteFromResFile(Instance: THandle;
  BitmapName: string; var NumPalEntries: Integer): T256PalEntry;
var
  h: HRsrc;
  BitmapInfo: PBitmapInfo;
  RGB: PRGB;
  i: Integer;
  APE: T256PalEntry;
  GLobal: HGlobal;
begin
  h := FindResource(Instance, PChar(BitmapName), RT_BITMAP);
  if h = 0 then begin
    raise Exception.Create('Can''t load resource in GetPaletteFromResFile');
    Exit;
  end;
  AppendError(0, 'H Exists');
  if h <> 0 then begin
    Global := LoadResource(Instance, h);
    BitmapInfo := PBitmapInfo(LockResource(Global));
    RGB := PRGB(@BitmapInfo^.bmiColors);

    if (BitmapInfo = NIL) or
      (BitmapInfo^.bmiHeader.biSize < sizeof(TBITMAPINFOHEADER)) then
      NumPalEntries := 0
    else
    if (BitmapInfo^.bmiHeader.biBitCount > 8) then NumPalEntries := 0
    else
    if (BitmapInfo^.bmiHeader.biClrUsed = 0) then
       NumPalEntries := 1 SHL BitmapInfo^.bmiHeader.biBitCount
    else
       NumPalEntries := BitmapInfo^.bmiHeader.biClrUsed;

    //  a DIB color table has its colors stored BGR not RGB
    //  so flip them around.
    AppendError(0, 'NumPalEntries: ' + IntToStr(NumPalEntries));
    for i := 0 to NumPalEntries - 1 do with APE[ i ], RGB^[ i ] do begin
      peRed := rgbRed;
      peGreen := rgbGreen;
      peBlue := rgbBlue;
      peFlags := 0;
    end;
    FreeResource(Global);
  end;
  Result := APE;
end;

procedure SaveClipBoardBitmap(BitMap: HBitMap; FileName: string);
var
  B: Graphics.TBitmap;
begin
  B := TBitMap.Create;
  B.Assign(ClipBoard);
  B.SaveToFile(FileName);
  B.Free;
end;

procedure WriteError(ErrorCode: HResult);
var
  Message: string;
  F: Text;
begin
  Message := SysErrorMessage(ErrorCode);
  if Message = '' then FmtStr(Message, LoadStr(62211), [ErrorCode]);
  Assign(F, 'c:\err.txt');
  ReWrite(F);
  WriteLn(F, Message, ' Code: ', ErrorCode);
  Close(F);
end;

procedure AppendError(ErrorCode: HResult; ErrStr: string);
var
  Message: string;
  F: Text;
begin
  Message := SysErrorMessage(ErrorCode);
  if Message = '' then FmtStr(Message, LoadStr(62211), [ErrorCode]);
  Assign(F, 'c:\err.txt');
  try
    Append(F);
  except
    ReWrite(F);
  end;
  WriteLn(F, Message, ' Code: ', ErrorCode, ' ', ErrStr);
  Close(F);
end;

procedure GetColors(S: String; P: TRGBQuad);
begin
  WriteLn(S);
  P.rgbBlue := 1;
end;

procedure ShowFilePalette(Handle: HWnd; AFileName: string);
var
  FilePal: TFilePalette;
  ADC: HDC;
begin
  FilePal := TFilePalette.Create(Handle, AFileName);
  ADC := FilePal.RealizePalette;
  DrawPalette(ADC);
  FilePal.Free;
end;

{ This is one of those text version of palettes like PSP creates }
procedure ReadPal(FileName: string; var P: T256PalEntry);
var
  F: Text;
  i: Integer;
  S: String;
begin
  Assign(F, FileName);
  Reset(F);
  ReadLn(F, S);
  ReadLn(F, S);
  ReadLn(F, S);
  for i := 0 to 255 do begin
    ReadLn(F, p[i].peRed, p[i].peGreen, p[i].peblue);
    P[i].peFlags := PC_NOCOLLAPSE;
  end;
  Close(F);
end;

procedure WritePal(FileName: string; var P: T256PalEntry);
var
  F: Text;
  i: Integer;
begin
  Assign(F, FileName);
  ReWrite(F);
  WriteLn(F, 'JASC-PAL');
  WriteLn(F, '0100');
  WriteLn(F, '256');
  for i := 0 to 255 do
    WriteLn(F, P[i].peRed, ' ', P[i].peGreen, ' ', P[i].peBlue);
  Close(F);
end;


procedure ShowFH(D: TBitMapFileHeader);
begin
  WriteLn('File Header');
  WriteLn('===========');
  WriteLn('Type: ', Chr(Lo(D.bfType)), Chr(Hi(D.bfType)));
  WriteLn('Size: ', D.bfSize);
  WriteLn('Offset: ', D.bfOffBits);
end;

function GetPaletteSize(Info: TBitMapInfoHeader): LongInt;
begin
  if Info.biSize = SizeOf(TBitMapCoreHeader) then
    GetPaletteSize := Info.biClrUsed * SizeOf(TRGBTriple)
  else
    GetPaletteSize := Info.biClrUsed * SizeOf(TRGBQuad);
end;

procedure ShowDCCaps(DC: HDC);
const
  CR = #13#10;
var
  S: string;
begin
  S := Format('BitsPerPixel: %d' + CR +
              'Color Planes: %d' + CR +
              'Num Colors: %d',
  [GetDeviceCaps(DC, BitsPixel),
   GetDeviceCaps(DC, Planes),
   GetDeviceCaps(DC, NumColors)]);
  ShowMessage(S);
end;

{ ------------------------ }
{ ---  STRING ROUTINES --- }
{ ------------------------ }

{$IFNDEF WIN32}
procedure SetLength(var S: string; i: Integer);
begin
  S[0] := Chr(i);
end;
{$ENDIF}

function Address2Str(Addr: Pointer): string;
begin
  Result := Format('%p', [Addr]);
end;

function AddBackSlash(S: string): string;
var
 Temp: string;
begin
  Temp := S;
  if S[Length(Temp)] <> '\' then
    Temp := Temp + '\';
  AddBackSlash := Temp;
end;

{----------------------------------------------------
       Name: CleanString function
Declaration: CleanString(S: String): string;
       Unit: StrBox
       Code: S
       Date: 05/05/94
Description: Erase blanks from end and beginning of
             a string
-----------------------------------------------------}
function CleanString(S: string): string;
var
  Temp: String;
begin
  Temp := ''; 
  if Length(S) <> 0 then begin
    Temp := StripFrontChars(S, #32);
    Temp := StripBlanks(Temp);
  end;
  CleanString := Temp;
end;

{----------------------------------------------------
       Name: GetFirstWord function
Declaration: GetFirstWord(var S: string): string;
       Unit: StrBox
       Code: S
       Date: 05/02/94
Description: Get the first word from a string
-----------------------------------------------------}
function GetFirstWord(S: string): string;
  Var
    i: Integer;
    S1: String;
begin
  i := 1;
  SetLength(S1, 250); // Large buffer, changed later
  while (S[i] <> ' ') and (i < Length(S)) do begin
     S1[i] := S[i];
     Inc(i);
  end;
  Dec(i);
  SetLength(S1, i);
  GetFirstWord := S1;
end;

function GetHexWord(w: Word): string;
const
  HexChars: array [0..$F] of Char =  '0123456789ABCDEF';
var
  Addr: string;
begin
  Addr[1] := hexChars[Hi(w) shr 4];
  Addr[2] := hexChars[Hi(w) and $F];
  Addr[3] := hexChars[Lo(w) shr 4];
  Addr[4] := hexChars[Lo(w) and $F];
  SetLength(Addr, 4);
  GetHexWord := addr;
end;

function GetFirstToken(S: string; Token: Char): string;
var
  Temp: string;
  Index: INteger;
begin
  Index := Pos(Token, S);
  if Index < 1 then begin
    GetFirstToken := '';
    Exit;
  end;
  Dec(Index);
  SetLength(Temp, Index); 
  Move(S[1], Temp[1], Index);
  GetFirstToken := Temp;
end;

{ Get the last part of a string, from a token onward.
  Given "Sam.Txt", and "." as a token, this returns "Txt" }
function GetLastToken(S: string; Token: Char): string;
var
  Temp: string;
  Index: INteger;
begin
  S := ReverseStr(S);
  Index := Pos(Token, S);
  if Index < 1 then begin
    GetLastToken := '';
    Exit;
  end;
  Dec(Index);
  SetLength(Temp, Index);
  Move(S[1], Temp[1], Index);
  GetLastToken := ReverseStr(Temp);
end;

function GetLastWord(S: string): string;
begin
  Result := GetLastToken(S, ' ');
end;

{----------------------------------------------------
       Name: GetLogicalAddress function
Declaration: GetLogicalAddr(A: Pointer): Pointer;
       Unit: StrBox
       Code: S
       Date: 02/09/95
Description: Enter a physical address and this function
             will return a logical address.
-----------------------------------------------------}

{$ifdef OLDDELPHI}
function GetLogicalAddr(A: Pointer): Pointer;
var
  APtr: Pointer;
begin
  if A = nil then exit;
  if Ofs(A) = $FFFF then exit;
  asm
    mov ax, A.Word[0]
    mov dx, A.Word[2]
    mov es,dx
    mov dx,es:Word[0]
    mov APtr.Word[0], ax
    mov APtr.Word[2], dx
  end;
  GetLogicalAddr := APtr;
end;
{$endif}

function GetTimeString: string;
begin
  Result := TimeToStr(Time);
end;

function GetTimeFormat: string;
var
 h, m, s, hund: Word;
begin
   DecodeTime(Time, h, m, s, hund);
   GetTimeFormat:= Int2StrPad0(h, 2) + ':' +
           Int2StrPad0(m, 2) + ':' + Int2StrPad0(s, 2);
end;

function GetStartDir: string;
begin
  Result := ExtractFilePath(ParamStr(0));
  if Result[Length(Result)] <> '\' then
    Result := Result + '\';
end;

{----------------------------------------------------
       Name: GetTodayName function
Declaration: GetTodayName(Pre, Ext: string): string;
       Unit: StrBox
       Code: S
       Date: 03/01/94
Description: Return a filename of type PRE0101.EXT,
             where PRE and EXT are user supplied strings,
             and 0101 is today's date.
-----------------------------------------------------}
function GetTodayName(Pre, Ext: string): string;
var
  y, m, d: Word;
  Year: String;
begin
  DecodeDate(Date,y,m,d);
  Year := Int2StrPad0(y, 4);
  Delete(Year, 1, 2);
  GetTodayName := Pre + Int2StrPad0(m, 2) + Int2StrPad0(d, 2) +
                    Year + '.' + Ext;
end;

{----------------------------------------------------
       Name: GetTodaysDate function
Declaration: GetTodaysDate: string;
       Unit: StrBox
       Code: S
       Date: 08/16/94
Description: Return a string of type MM/DD/YY.
-----------------------------------------------------}
function GetTodaysDate: string;
var
  y, m, d: Word;
  Year: String;
begin
  DecodeDate(Date, y,m,d);
  Year := Int2StrPad0(y, 4);
  Delete(Year, 1, 2);
  GetTodaysDate := Int2StrPad0(m, 2) + '/' + Int2StrPad0(d, 2) + '/' + Year;
end;

function IsNumber(Ch: Char): Boolean;
begin
  IsNumber := ((Ch >= '0') and (Ch <= '9'));
end;

{----------------------------------------------------
       Name: LeftSet function
Declaration: LeftSet(src: string; Width: Integer;
                     var Trunc: Boolean): string;
       Unit: StrBox
       Code: S
       Date: 03/01/94
Description: Pad a string on the left
-----------------------------------------------------}
function LeftSet(src: string; Width: Integer; var Trunc: Boolean): String;
var
  I: Integer;
  Temp: string[80];
begin
  Trunc := False;
  Temp := src;
  if(Length(Temp) > Width) and (Width > 0) then begin
    Temp[0] := CHR(Width);
    Trunc := True;
  end else
    for i := Length(Temp) to width do
      Temp := Temp + ' ';
  LeftSet := Temp;
end;

{----------------------------------------------------
       Name: RemoveFirstWord function
Declaration: RemoveFirstWord(var S: String): String;
       Unit: StrBox
       Code: S
       Date: 03/02/94
Description: Strip the first word from a sentence,
             return word and a shortened sentence.
             Return an empty string if there is no
             first word.
-----------------------------------------------------}
function RemoveFirstWord(var S: String): String;
var
  i, Size: Integer;
  S1: String;
begin
  i := Pos(#32, S);
  if i = 0 then begin
    RemoveFirstWord := '';
    Exit;
  end;
  SetLength(S1, i);
  Move(S[1], S1[1], i);
  SetLength(S1, i-1);
  Size := (Length(S) - i);
  Move(S[i + 1], S[1], Size);
  SetLength(S, Size);
  RemoveFirstWord := S1;
end;

function ReplaceAllInstancesOfString(NewStr, ReplaceStr: string;
  var Data: string): Boolean;
var
  i: Integer;
begin
  i := 0;
  while Pos(ReplaceStr, Data) > 0 do begin
    Data := ReplaceString(NewStr, ReplaceStr, Data);
    Inc(i);
  end;
  Result := i > 0;
end;

{----------------------------------------------------
       Name: ReplaceString
Declaration: ReplaceString(NewStr, ReplaceStr, Data: string): string;
       Unit: StrBox
       Code: S
       Date: 06/06/95
Description: Given a long string, replace one substring with another.
             Take the string: "Football Delight"
             The job is to replace the word Delight with Night:
             S := ReplaceString('Night', 'Delight', 'Football Delight');
             where S ends up equaling "Football Night'; 
-----------------------------------------------------}
function ReplaceString(NewStr, ReplaceStr, Data: string): string;
var
  OffSet: Integer;
begin
  OffSet := Pos(ReplaceStr, Data);
  Delete(Data, OffSet, Length(ReplaceStr));
  Insert(NewStr, Data, OffSet);
  Result := Data;
end;

function ReplaceChars(S: string; OldCh, NewCh: Char): string;
var
  Len: Integer;
  i: Integer;
begin
  Len := Length(S);
  for i := 1 to Len do
    if S[i] = OldCh then
      S[i] := NewCh;
  Result := S;
end;

function ReverseStr(S: string): string;
var
  Len: Integer;
  Temp: String;
  i,j: Integer;
begin
  Len := Length(S);
  SetLength(Temp, Len);
  j := Len;
  for i := 1 to Len do begin
    Temp[i] := S[j];
    dec(j);
  end;
  ReverseStr := Temp;
end;

function RightCharSet(Src: string; Width: Integer;
                      Ch: Char; var Trunc: Boolean): String;
var
  I: Integer;
  Temp: string[80];
begin
  Trunc := False;
  Temp := Src;
  if(Length(Temp) > Width) and (Width > 0) then begin
    Temp[0] := CHR(Width);
    Trunc := True;
  end else
    for i := Length(Temp) to (width - 1) do
      Temp := Ch + Temp;
  RightCharSet := Temp;
end;

function Shorten(S: string; Cut: Integer): string;
begin
  SetLength(S, Length(S) - Cut);
  Shorten := S;
end;

procedure SplitDirName(Path: PathStr; var Dir: DirStr; var WName: Str12);
begin
  Dir := ExtractFilePath(Path);
  WName := ExtractFileName(Path);
end;

{
procedure SplitDirName(Path: PathStr; var Dir: DirStr; var WName: Str12);
begin
  FSplit(Path, Dir, Name, Ext);
  WName := ExtractFileName(Path);
end;
}

{----------------------------------------------------
       Name: StripBlanks function
Declaration: function StripBlanks(var S: string): String;
       Unit: StrBox
       Code: S
       Date: 03/02/94
Description: Strip any stray spaces from the end of
             a string
-----------------------------------------------------}
function StripBlanks(S: string): string;
var
  i: Integer;
begin
  i := Length(S);
  while (Length(S) <= i) and (Length(S) > 0) and (S[i] = ' ') do begin
    Delete(S,i,1);
    Dec(i);
  end;
  StripBlanks := S;
end;

function StripEndChars(S: string; Ch: Char): string;
var
  i: Integer;
begin
  i := Length(S);
  while (length(S) > 0) and (S[i] = Ch) do begin
    Delete(S,i,1);
    Dec(i);
  end;
  StripEndChars := S;
end;


function StripFirstToken(S: string; Ch: Char): string;
var
  i, Size: Integer;
begin
  i := Pos(Ch, S);
  if i = 0 then begin
    StripFirstToken := S;
    Exit;
  end;
  Size := (Length(S) - i);
  Move(S[i + 1], S[1], Size);
  SetLength(S, Size);
  StripFirstToken := S;
end;

{----------------------------------------------------
       Name: StripFirstWord function
Declaration: StripFirstWord(S: string): string;
       Unit: StrBox
       Code: S
       Date: 03/02/94
Description: Strip the first word from a sentence,
             return the shortened sentence. Return original
             string if there is no first word.
-----------------------------------------------------}
function StripFirstWord(S: string): string;
var
  i, Size: Integer;
begin
  i := Pos(#32, S);
  if i = 0 then begin
    StripFirstWord := S;
    Exit;
  end;
  Size := (Length(S) - i);
  Move(S[i + 1], S[1], Size);
  SetLength(S, Size);
  StripFirstWord := S;
end;

{----------------------------------------------------
       Name: StripFrontChars function
Declaration: StripFrontChars(S: string; Ch: Char): String;
       Unit: StrBox
       Code: S
       Date: 03/02/94
Description: Strips any occurances of charact Ch that
             might precede a string.
-----------------------------------------------------}
function StripFrontChars(S: string; Ch: Char): string;
begin
  while (Length(S) > 0) and (S[1] = Ch) do
    S := Copy(S,2,Length(S) - 1);
  StripFrontChars := S;
end;

function StripFromFront(S: string; Len: Integer): string;
begin
  S := ReverseStr(S);
  S := Shorten(S, Len);
  S := ReverseStr(S);
  StripFromFront := S;
end;

{----------------------------------------------------
       Name: StripLastToken function
Declaration: function RemoveLastToken(var S: String): String;
       Unit: StrBox
       Code: S
       Date: 03/02/94
Description: Given a string like "c:\sam\file.txt"
             This returns: "c:\sam"
             But not specific to files any token will do
-----------------------------------------------------}
function StripLastToken(S: string; Token: Char): string;
var
  Temp: string;
  Index: INteger;
begin
  SetLength(Temp, Length(S));
  S := ReverseStr(S);
  Index := Pos(Token, S);
  Inc(Index);
  Move(S[Index], Temp[1], Length(S) - (Index - 1));
  SetLength(Temp, Length(S) - (Index - 1));
  StripLastToken := ReverseStr(Temp);
end;

{ --- Storage Code --- }

constructor TSafeStore.Create(FileName: string);
begin
  inherited Create;
  FStorageStrings := TStringList.Create;
  if not FileExists(FileName) then
    CreateStorage(FileName)
  else
    OpenStorage(FileName);
end;

destructor TSafeStore.Destroy;
begin
  FStorageStrings.Free;
  FStorage.Release;
  inherited Destroy;
end;

procedure TSafeStore.CreateStorage(FileName: string);
var
  Hr: HResult;
  Dest: array[0..127] of WideChar;
begin
  hr := StgCreateDocFile(StringToWideChar(FileName, Dest, SizeOf(Dest) div 2),
      STGM_DIRECT or STGM_CREATE or STGM_WRITE or STGM_SHARE_EXCLUSIVE,
      0, FStorage);
  OleCheck(hr);
end;

{----------------------------------------------------
       Name: OpenStorage
Declaration: OpenStorage(FileName: string);
       Unit: Main
Description: Given a filename, try to open it as a storage
             file.
-----------------------------------------------------}
procedure TSafeStore.OpenStorage(FileName: string);
var
  hr: HResult;
  S: PWideChar;
  Size: Integer;
  Failed: Boolean;
begin
  Failed := False;
  S := nil;
  try
    try
      S := AnsiToUnicode(FileName, Size);
      hr := StgIsStorageFile(S);
      if hr <> NoError then
        raise Exception.Create('Not a valid storage file.');
      FStorageStrings.Add('Storage ' + FileName);
      hr := StgOpenStorage(S, nil,
              Stgm_Direct or Stgm_ReadWrite or Stgm_Share_Exclusive,
              nil, LongInt(nil), FStorage);
      if Ole2.Failed(hr) then
        raise Exception.Create('Call to StgOpenStorage failed');
    except
      Failed := True;
      raise;
    end; { try..except }
  finally
    VirtualFree(S, Size, Mem_Release);
  end; { try..finally }

  if not Failed then begin
    EnumStorageElements(FStorage);
  end;
end;

function Test(hr: HResult): Boolean;
begin
  if Succeeded(hr) then
    Result := True
  else begin
    ShowMessage('Enum Failed');
    Result := False;
  end;
end;


{----------------------------------------------------
       Name: HandleProperty
Declaration:
       Unit: Main
Description: Show Summary Info
-----------------------------------------------------}
procedure TSafeStore.HandleProperty(Storage: IStorage);
begin
  { Not Implemented }
end;

{----------------------------------------------------
       Name: ShowStorageElement
Declaration: ShowStringType(S: string; StatStg: TStatStg);
       Unit: Main
Description: Nonroot storage elements may have a first
             character between #1 and #6 that has a special meaning.
             We deal with that here.
-----------------------------------------------------}
function TSafeStore.ShowStorageElement(S: string; StatStg: TStatStg): Integer;
var
  Temp: string;
begin
  if S = 'Unknown' then begin
    StgStrings.Add('End Storage (Unknown)');
    Result := -1;
    Exit;
  end;
  Temp := UnicodeToAnsi(StatStg.pwcsName) + ' Size: ' + IntToStr(Round(StatStg.cbSize));
    case Temp[1] of
      #1,#2,#3,#4,#6: Temp := '(Special: ' +
                        IntToStr(Ord(Temp[1])) + ') ' +
                        StripFromFront(Temp, 1);

      #5: begin
        Temp := StripFromFront(Temp, 1);
        Temp := '(Property) ' + Temp;
      end;
  end;
  StgStrings.Add(S + ' ' + Temp);
  Result := Ord(Temp[1]);
end;

procedure TSafeStore.HandleSubStorage(var Storage: IStorage; StatStg: TStatStg);
var
  hr: HResult;
  SubStorage: IStorage;
begin
  hr := Storage.OpenStorage(StatStg.pwcsName, nil,
          Stgm_Read or Stgm_Share_Exclusive,
          nil, LongInt(nil), SubStorage);
  if Succeeded(hr) then
    EnumStorageElements(SubStorage)
  else
    ShowMessage('Count not open subStorage');
end;

{----------------------------------------------------
       Name: EnumStorageElements
Declaration: EnumStorageElements(var Storage: IStorage);
       Unit: Main
Description: Enumerate the elements inside a storage.
             This is a recursive routine, but the
             recursion occurs in the HandleSubStorage routine.
-----------------------------------------------------}

procedure TSafeStore.EnumStorageElements(var Storage: IStorage);
var
  Enum: IEnumStatStg;
  hr: hResult;
  StatStg: TStatStg;
  Count: LongInt;
  S: string;
begin
  if not Test(FStorage.EnumElements(0, nil, 0, Enum)) then Exit;

  repeat
    hr := Enum.Next(1, StatStg, @Count);
    OleCheck(hr);
    case StatStg.dwType of
      STGTY_STREAM: S := 'Stream';
      STGTY_STORAGE: S := 'Storage';
      STGTY_LOCKBYTES: S := 'LockBytes';
      STGTY_PROPERTY: S := 'Property';
    else
      S := 'Unknown';
    end;

    if ShowStorageElement(S, StatStg) = 5 then
      HandleProperty(Storage);

    if S = 'Storage' then HandleSubStorage(Storage, StatStg);
  until HR <> S_OK;
  Enum.Release;
end;


procedure TSafeStore.DestroyElement(S: string);
var
  Dest: array[0..127] of WideChar;
begin
  FStorage.DestroyElement(StringToWideChar(S, Dest, SizeOf(Dest) div 2));
end;

// You must Release the stream when done: Stream.Release;
// Stream is for writing only
function TSafeStore.GetNewStream(StreamName: string): IStream;
var
  Hr: HResult;
  Stream: IStream;
  Dest: array[0..127] of WideChar;
begin
  Hr := FStorage.CreateStream(StringToWideChar(StreamName, Dest, SizeOf(Dest) div 2),
    STGM_DIRECT or STGM_CREATE or STGM_READWRITE or STGM_SHARE_EXCLUSIVE , 0, 0, Stream);
  OleCheck(HR);
  Result := Stream;
end;

function TSafeStore.OpenStream(StreamName: string): IStream;
var
  Hr: HResult;
  Stream: IStream;
  Dest: array[0..127] of WideChar;
begin
  Hr := FStorage.OpenStream(
    StringToWideChar(StreamName, Dest, SizeOf(Dest) div 2), nil,
    STGM_DIRECT or STGM_READWRITE or STGM_SHARE_EXCLUSIVE , 0, Stream);
  OleCheck(HR);
  REsult := Stream;
end;

procedure TSafeStore.ReadInteger(Stream: IStream; var Num: Integer);
var
  Size: Integer;
begin
  OleCheck(Stream.Read(@Num, SizeOf(Integer), @Size));
  if Size <> SizeOf(Integer) then
    raise Exception.Create(Self.ClassName + '.ReadInteger');
end;

procedure TSafeStore.WriteInteger(Stream: IStream; Num: Integer);
var
  Size: Integer;
  hr: Integer;
begin
  hr := Stream.Write(@Num, SizeOf(Integer), @Size);
  OleCheck(hr);
  if Size <> SizeOf(Integer) then
    raise Exception.Create(Self.ClassName + '.WriteInteger');
end;

procedure TSafeStore.ReadString(Stream: IStream; var S: string);
var
  Num: Integer;
  Size: Integer;
begin
  ReadInteger(Stream, Num);
  SetLength(S, Num + 1);
  Stream.Read(Pointer(S), Num, @Size);
  S[Num + 1] := #0;
end;

procedure TSafeStore.WriteString(Stream: IStream; S: string);
var
  Size: Integer;
begin
  WriteInteger(Stream, Length(S));
  OleCheck(Stream.Write(PChar(S), Length(S), @Size));
  if Size <> Length(S) then
    raise Exception.Create('Stream');
end;

// This assumes the whole stream will be one string.
// Use if you have a block of text you want to write to storage.
// It's like having a way to create a text file in a storage
// Use with ReadText from stream.
procedure TSafeStore.WriteTextToStorage(StreamName: string; Value: string);
var
  Hr: HResult;
  Stream: IStream;
  Size: LongInt;
  Dest: array[0..127] of WideChar;
begin
  Hr := FStorage.CreateStream(StringToWideChar(StreamName, Dest, SizeOf(Dest) div 2),
    STGM_DIRECT or STGM_CREATE or STGM_WRITE or STGM_SHARE_EXCLUSIVE , 0, 0, Stream);
  OleCheck(HR);
  Stream.Write(Pointer(Value), Length(Value), @Size);
  if Size <> Length(Value) then ShowMessage('Wrong size written');
  Stream.Release;
end;

function TSafeStore.ReadTextFromStream(StreamName: string): string;
var
  Stream: IStream;
  hr: HResult;
  S: PChar;
  Size, ASize: LongInt;
  Dest: array [0..127] of WideChar;
  StatStg: TStatStg;
begin
  hr := FStorage.OpenStream(StringToWideChar(StreamName, Dest, SizeOf(Dest) div 2), nil, STGM_DIRECT or STGM_READ or STGM_SHARE_EXCLUSIVE,
          0, Stream);
  OleCheck(hr);
  Stream.Stat(StatStg, StatFlag_Default);
  Size := Round(StatStg.cbSize);
  GetMem(S, Size + 1);
  Stream.Read(S, Size, @ASize);
  S[Size] := #0;
  Result := S;
  FreeMem(S, Size + 1);
  Stream.Release;
end;

function TSafeStore.RefreshStorageStrings: TStringList;
begin
  EnumStorageElements(FStorage);
  Result := FStorageStrings;
end;

{ === End === }

function ReadStringFromStorage(StorageName: string; StreamName: string): string;
var
  Storage: IStorage;
  Stream: IStream;
  hr: HResult;
  S: PChar;
  Size, ASize: LongInt;
  Dest: array [0..127] of WideChar;
  StatStg: TStatStg;
begin
  hr := StgOpenStorage(StringToWideChar(StorageName, Dest, SizeOf(Dest) div 2), nil, STGM_DIRECT or STGM_READ or STGM_SHARE_EXCLUSIVE,
     nil, 0, Storage);
  OleCheck(hr);

  hr := Storage.OpenStream(StringToWideChar(StreamName, Dest, SizeOf(Dest) div 2), nil, STGM_DIRECT or STGM_READ or STGM_SHARE_EXCLUSIVE,
          0, Stream);
  OleCheck(hr);
  Stream.Stat(StatStg, StatFlag_Default);
  Size := Round(StatStg.cbSize);
  GetMem(S, Size + 1);
  Stream.Read(S, Size, @ASize);
  S[Size] := #0;
  Result := S;
  FreeMem(S, Size + 1);
  Stream.Release;
  Storage.Release;
end;

// Given an existing IStorage file, add a new stream to it.
// Will create an IStorage file if none exists
// For now, this function is meant to work with strings
procedure WriteStreamToStorage(StorageName, StreamName, Value: string);
var
  Hr: HResult;
  Storage: IStorage;
  Stream: IStream;
  Size: LongInt;
  Dest: array[0..127] of WideChar;
begin
  if FileExists(StorageName) then begin
    StgOpenStorage(StringToWideChar(StorageName, Dest, SizeOf(Dest) div 2),
      nil, Stgm_Direct or Stgm_ReadWrite or Stgm_Share_Exclusive,
      nil, LongInt(nil), Storage);
  end else begin
    Hr := StgCreateDocFile(StringToWideChar(StorageName, Dest, SizeOf(Dest) div 2),
      STGM_DIRECT or STGM_CREATE or STGM_WRITE or STGM_SHARE_EXCLUSIVE,
      0, Storage);
    if Hr <> S_OK then
      ShowMessage('Err');
  end;

  Hr := Storage.CreateStream(StringToWideChar(StreamName, Dest, SizeOf(Dest) div 2),
    STGM_DIRECT or STGM_CREATE or STGM_WRITE or STGM_SHARE_EXCLUSIVE , 0, 0, Stream);
  OleCheck(HR);
  Stream.Write(PChar(Value), Length(Value), @Size);
  if Size <> Length(Value) then ShowMessage('Wrong size written');
  Stream.Release;
  Storage.Release;
end;

end.
