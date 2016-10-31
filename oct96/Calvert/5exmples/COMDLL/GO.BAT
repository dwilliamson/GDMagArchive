@echo off
echo  ------------------------------------------------
echo  You might need to edit the DCC32.CFG file in
echo  DELPHI32\BIN subdirectory before you can compile
echo  this program from the DOS prompt
echo  ------------------------------------------------
echo  :                                              :
call dcc32 -DUSECOM -b SpedDraw.dpr
echo  :                                              :
echo  ------------------------------------------------
echo  Don't forget to register this dll. You can do so
echo  by typing START SPEDDRAW.REG at the DOS prompt,
echo  or by running ..\windows\regedit.exe and loading
echo  SPEDDRAW.REG into that program
echo  You may have to edit the INPROCSERVER32 statement
echo  in SPEDDRAW.REG to get the path straight.
echo  ------------------------------------------------
