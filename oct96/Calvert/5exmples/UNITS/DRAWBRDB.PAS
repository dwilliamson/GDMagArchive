unit DrawBrdb;

interface

uses
  Windows, Messages, SysUtils,
  Classes, Graphics, Controls,
  Forms, Dialogs, StdCtrls,
  Buttons, ExtCtrls;

type
  TBackDlg = class(TForm)
    Edit1: TEdit;
    Edit2: TEdit;
    BitBtn1: TBitBtn;
    Label1: TLabel;
    Label2: TLabel;
    Bevel1: TBevel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Edit3: TEdit;
    Edit4: TEdit;
    Edit5: TEdit;
    Edit6: TEdit;
    Bevel2: TBevel;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  BackDlg: TBackDlg;

implementation

{$R *.DFM}

end.
