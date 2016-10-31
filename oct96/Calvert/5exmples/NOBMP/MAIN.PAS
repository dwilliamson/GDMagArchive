unit Main;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  drawbrd1, Menus, StdCtrls;

type
  TForm1 = class(TForm)
    SpeedDraw1: TSpeedDraw;
    MainMenu1: TMainMenu;
    Options1: TMenuItem;
    StartWindowed1: TMenuItem;
    StartExclusive1: TMenuItem;
    StartExclusiveMode21: TMenuItem;
    StartWindowedMode21: TMenuItem;
    N1: TMenuItem;
    procedure SpeedDraw1PaintProc(Sender: TObject);
    procedure FormDblClick(Sender: TObject);
    procedure StartWindowed1Click(Sender: TObject);
    procedure StartExclusive1Click(Sender: TObject);
    procedure StartExclusiveMode21Click(Sender: TObject);
    procedure StartWindowedMode21Click(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
  private
    FUseModeTwo: Boolean;
    FCount: Integer;
    procedure CloseMenus;
  end;

var
  Form1: TForm1;

implementation

{$R *.DFM}

procedure TForm1.SpeedDraw1PaintProc(Sender: TObject);
const
  S = 'DblClick Form To Exit when in Exclusive mode.';
var
  DC: HDC;
  Brush, OldBrush: HBrush;
begin
  SpeedDraw1.BackSurface.GetDC(DC);
  Brush := CreateSolidBrush(RGB(0, 0, 255));
  OldBrush := SelectObject(DC, Brush);
  SetBkMode(DC, Transparent); 
  TextOut(DC, 10, 10, PChar(S), Length(S));

  if not FUseModeTwo then begin
   if SpeedDraw1.TimerOdd then
      Rectangle(DC, 100, 100, 200, 200)
    else
      Rectangle(DC, 200, 100, 300, 200);
  end else begin
    if FCount > 300 then FCount := 0;
    FCount := FCount + 2;
    Rectangle(DC, 100 + FCount, 250, 200 + FCount, 350);
  end;

  SelectObject(DC, OldBrush);
  DeleteObject(Brush);
  SpeedDraw1.BackSurface.ReleaseDC(DC);
end;

procedure TForm1.FormDblClick(Sender: TObject);
begin
  Close;
end;

procedure TForm1.CloseMenus;
var
  i: Integer;
begin
  for i := 0 to ComponentCount - 1 do
    if Components[i] is TMenuItem then
      TMenuItem(Components[i]).Enabled := False;
end;

procedure TForm1.StartWindowed1Click(Sender: TObject);
begin
  CloseMenus;
  SpeedDraw1.TimerInterval := 250;
  SpeedDraw1.Exclusive := False;
  SpeedDraw1.Run;
end;

procedure TForm1.StartWindowedMode21Click(Sender: TObject);
begin
  CloseMenus;
  FUseModeTwo := True;
  SpeedDraw1.Exclusive := False;
  SpeedDraw1.Run;
end;

procedure TForm1.StartExclusive1Click(Sender: TObject);
begin
  with SpeedDraw1 do begin
    TimerInterval := 250;
    Exclusive := True;
    Run;
  end;
end;

procedure TForm1.StartExclusiveMode21Click(Sender: TObject);
begin
  with SpeedDraw1 do begin
    FUseModeTwo := True;
    Exclusive := True;
    Run;
  end;
end;

// Alt X to close...
procedure TForm1.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if ((ssAlt in Shift) and (key = Ord('X'))) then
    Close;
end;

end.
