@echo off
echo  ------------------------------------------------
echo  You might need to edit the DCC32.CFG file 
echo  before you can compile this program from the
echo  DOS prompt
echo  ------------------------------------------------
echo  :
dcc32 -b -u..\units Simple2.dpr
