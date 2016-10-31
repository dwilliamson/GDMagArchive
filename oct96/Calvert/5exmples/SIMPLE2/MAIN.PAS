unit Main;

interface

uses
  Windows, Messages, SysUtils,
  Classes, Graphics, Controls,
  Forms, Dialogs, Menus,
  DrawBrd1, DDraw, StdCtrls;

type
  TForm1 = class(TForm)
    SpeedDraw1: TSpeedDraw;
    MainMenu1: TMainMenu;
    Start1: TMenuItem;
    Label1: TLabel;
    procedure Start1Click(Sender: TObject);
    procedure SpeedDraw1InitObjectsProc(Sender: TObject);
    procedure SpeedDraw1PaintProc(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure FormDblClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    FNewSurface: IDirectDrawSurface;
    FDinoRect: TRect;
    procedure ExceptionHandler(Sender: TObject; E: Exception);
  end;

var
  Form1: TForm1;

implementation

uses
  CodeBox;

{$R *.DFM}

procedure TForm1.Start1Click(Sender: TObject);
begin
  SpeedDraw1.Run;
end;

procedure TForm1.SpeedDraw1InitObjectsProc(Sender: TObject);
var
  SurfaceDesc: TDDSurfaceDesc;
  hr: HResult;
begin
  SpeedDraw1.CreateDDSurface(FNewSurface, 'dino.bmp', True);
  SurfaceDesc.dwSize := SizeOf(TDDSurfaceDesc);
  hr := FNewSurface.GetSurfaceDesc(SurfaceDesc);
  if hr <> DD_OK then
    raise EDDError.CreateFmt('No Surface Desc $%x %s', [hr, GetOleError(hr)]);
  FDinoRect := Rect(0, 0, SurfaceDesc.dwWidth, SurfaceDesc.dwHeight);
end;

procedure TForm1.SpeedDraw1PaintProc(Sender: TObject);
var
  hr: HResult;
begin
  hr := SpeedDraw1.BackSurface.BltFast(200, 75,
          FNewSurface, FDinoRect,
          DDBLTFAST_WAIT or DDBLTFAST_SRCCOLORKEY);
  if hr <> DD_OK then
    raise EDDError.CreateFmt('No blit $%x %s', [hr, GetOleError(hr)]);
end;

procedure TForm1.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if (ssalt in Shift) and (Ord('X') = Key) then
    Close;
end;

procedure TForm1.FormDblClick(Sender: TObject);
begin
  Close;
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
  Application.OnException := ExceptionHandler;
end;

procedure TForm1.ExceptionHandler(Sender: TObject; E: Exception);
begin
  with SpeedDraw1 do begin
    Active := False;
    DirectDraw.FlipToGDISurface;
    if Exclusive then
      DirectDraw.SetCooperativeLevel(Handle, DDSCL_Normal);
    ShowMessage(E.Message);
  end;
end;

end.
