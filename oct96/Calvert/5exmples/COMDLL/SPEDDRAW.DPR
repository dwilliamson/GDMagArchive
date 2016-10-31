library SpedDraw;

uses
  SysUtils, Classes, Ole2,
  DrawBrd1;

function DllGetClassObject(const rclsid: TCLSID; const riid: TGUID;
                           var ppv): hResult; stdcall export;
var
  HR:  HRESULT;
  MyClassFactory: IMyClassFactory;
begin
  if not IsEqualIID(CLSID_ISpeedDraw, rclsid) then begin
    result := E_FAIL;
    Exit;
  end;

  try
    MyClassFactory := IMyClassFactory.Create;
  except
    Result := E_OUTOFMEMORY;
    Exit;
  end;

  hr := MyClassFactory.QueryInterface(riid, ppv);

  if (FAILED(hr)) then begin
    MyClassFactory.Free;
    Result := hr;
    Exit;
  end;

  Result := hr;
end;

function DllCanUnloadNow: hResult;
begin
  if (LockCount = 0) and (ObjCount = 0) then
    Result := S_Ok
  else
    Result := S_False;
end;

exports
  DllGetClassObject name 'DllGetClassObject',
  DllCanUnloadNow name 'DllCanUnloadNow';

begin
end.
