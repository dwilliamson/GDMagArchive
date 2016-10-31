unit Main;

interface

uses
  Windows, Messages, SysUtils,
  Classes, Graphics, Controls,
  Forms, Dialogs, DrawBrd1,
  Menus, StdCtrls;

type
  TForm1 = class(TForm)
    SpeedDraw1: TSpeedDraw;
    MainMenu1: TMainMenu;
    Options1: TMenuItem;
    StartWindowed1: TMenuItem;
    StartExclusiveDoubleClickFormtoClose1: TMenuItem;
    Label1: TLabel;
    procedure FormClick(Sender: TObject);
    procedure StartWindowed1Click(Sender: TObject);
    procedure StartExclusiveDoubleClickFormtoClose1Click(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure FormCreate(Sender: TObject);
  private
    procedure EHandler(Sender: TObject; E: Exception);
  end;

var
  Form1: TForm1;

implementation

uses
  DDraw;

{$R *.DFM}

procedure TForm1.FormClick(Sender: TObject);
begin
  Close;
end;

procedure TForm1.StartWindowed1Click(Sender: TObject);
begin
  SpeedDraw1.Run;
end;

procedure TForm1.StartExclusiveDoubleClickFormtoClose1Click(
  Sender: TObject);
begin
  SpeedDraw1.Exclusive := True;
  SpeedDraw1.Run;
end;

procedure TForm1.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if ((ssAlt in Shift) and (key = Ord('X'))) then
    Close;
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
  Application.OnException := EHandler;
end;

procedure TForm1.EHandler(Sender: TObject; E: Exception);
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
