//====================================================================
// Main.c
//========
// Purpose
//    Sample to show sequence of events from initialization to
//    playback.
//====================================================================
#include "input.h"


void
myMain(void)
{
tFFB_Effect  effect;
short        eID;

   Input_Init();
   joy_Init();
   joy_Query(kDI_Joy1,NULL,NULL);      // Assume joy1 is FFB
   ffb_Init();

   // Constant Effect (simple jolt)
   // -----------------------------
   effect.Type                  = kConstant; 
   effect.TypeInfo.Constant.Mag = 10000; 
   effect.Duration              = 1000000 >> 1; // half second
   effect.Period                = 1000;               
   effect.Gain                  = 10000;              
   effect.Trigger               = kNoButton;
   effect.TriggerRepeatTime     = 0;      
   effect.Direction             = 0; 

   eID = ffb_effectCreate(kDI_Joy1, &effect);

   if (eID >= 0)
   {
      ffb_effectPlay(eID);

      // wait a little for effect to play before next play
      // -------------------------------------------------
      sleep(100);          
      
      // Reduce the duration and play again
      // ----------------------------------
      ffb_effectModify(eID, NULL, 1000000 >> 2, NULL,NULL,NULL,NULL);
      ffb_effectPlay(eID);
      sleep(500);

      ffb_effectStopAll(kDI_Joy1);
   }
   Input_Exit();
}
