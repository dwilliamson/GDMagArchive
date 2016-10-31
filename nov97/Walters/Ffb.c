//====================================================================
// Input.c
//==========
//
// Description:
//    Direct Input initialization and use
//
//====================================================================
#include "input.h"
#include "dinput.h" 

#define kJOY_COOP_FLAGS (DISCL_EXCLUSIVE | DISCL_BACKGROUND)

#define kDI_MaxEffects  30       // Hard coded limit (not a DX limit)

// Force Feedback Effect Data
// --------------------------
REFGUID effectGUID[kMaxEffectSubTypes] =
{
   &GUID_ConstantForce,
   &GUID_RampForce,   
   &GUID_CustomForce, 
   // period
   &GUID_Square,      
   &GUID_Sine,        
   &GUID_Triangle,    
   &GUID_SawtoothUp,  
   &GUID_SawtoothDown,
   // condition
   &GUID_Spring,      
   &GUID_Damper,      
   &GUID_Inertia,     
   &GUID_Friction
};

typedef union 
{
   DICONSTANTFORCE   constant;
   DIRAMPFORCE       ramp;
   DIPERIODIC        period;
   DICONDITION       condition;
   DICUSTOMFORCE     custom;

}tEffectClasses;

typedef struct tEffect
{
   DIEFFECT       general;
   tEffectClasses specific;
   DIENVELOPE     envelope;
   LONG           direction[2];

}tEffect;

static tEffect                Effect[kDI_MaxEffects];

// DInput Data
// -----------
static LPDIRECTINPUT          DI                   = NULL; 
static LPDIRECTINPUTDEVICE    DID1_Rat             = NULL;
static LPDIRECTINPUTDEVICE    DID1_Key             = NULL;
static LPDIRECTINPUTDEVICE2   DID2_Joy[kDI_MaxJoy];
static LPDIRECTINPUTEFFECT    DIE_hEffect[kDI_MaxEffects];

// Joystick Data
// -------------
static int           maskFFB                       = 0;
static int           numJoy                        = 0;
static int           numEffects                    = 0;
static char          isWheel[kDI_MaxJoy];
static char          joyName[kDI_MaxJoy][MAX_PATH];
static DWORD         dwAxes[2]                     = { DIJOFS_X, DIJOFS_Y };

// Private Functions
// -----------------
BOOL CALLBACK  DIEnumJoysticks_Callback(LPDIDEVICEINSTANCE pdinst, LPVOID pvRef);
static int     DXInput_AcquireErr(HRESULT res, int dev_num);
static int	   DXInput_SetCoopLevel(tDI_Device,int coop_level);


// ------------
// Input_Init
// -------------------------------------------------------------------
// Purpose:
//    Initialize the main Direct Input object.
//    This is used by the joystick, mouse, keyboard & force feedback
// -------------------------------------------------------------------
void
Input_Init(void)
{
HRESULT  hr; 
int      i;

   if (DI)
   {  printf ("DXInput_Init -- already initialized\n");
      return;
   }

   for (i=0; i<kDI_MaxJoy; i++)
      DID2_Joy[i] = NULL;

   maskFFB  = 0;
   numJoy   = 0;

   hr = DirectInputCreate((HINSTANCE)GetModuleHandle(NULL), DIRECTINPUT_VERSION, &DI, NULL); 
   if (hr != DI_OK) 
   {  printf ("DirectInputCreate failed"); 
      DI = NULL;
      return;
   } 
}



// ------------------
// Input_SetCoopLevel
// -------------------------------------------------------------------
// -------------------------------------------------------------------
static int
Input_SetCoopLevel(tDI_Device dev, int coop_level)
{
HWND hwin;

   if (!(hwin=(HWND)getwindowhandle()))
      printf("couldn't get window handle\n");

   // Set a single joystick
   // ---------------------
   if (dev < kDI_MaxJoy)
   {
      if (DID2_Joy[dev])
      {
         // Set the cooperative level to share the device
         // ---------------------------------------------
         if (IDirectInputDevice2_SetCooperativeLevel
             (DID2_Joy[dev], (HWND)getwindowhandle(), coop_level) 
            != DI_OK) 
         { 
            printf("Could not set dinput device coop level\n");
            return(0); 
         }
      }
   }
   // Set all single joysticks
   // ------------------------
   else 
   if (dev == kDI_MaxJoy)
   {
   tDI_Device i;
      for (i=kDI_Joy1; i<kDI_MaxJoy; i++)
      {
         DXInput_SetCoopLevel(i, coop_level);
      }
   }

   return(1);
}

// -------------
// Input_Acquire
// -------------------------------------------------------------------
// Purpose:
//    Acquires a direct input device for use.
//
// Input:
//    The device to acquire (use kDI_MaxJoy to acquire all available 
//    joysticks).
//
// Return:
//    # of devices acquired.
//
// Description:
//    Call this to gain access to a device after the device has been
//    created & after regaining the focus after losing it.
//
// -------------------------------------------------------------------
int
Input_Acquire(tDI_Device dev)
{
int i,
    cnt=0;
   
   if (DI)
   {
      if (dev == kDI_AllDevices)
      {
         cnt += DXInput_Acquire(kDI_MaxJoy);
         return(cnt);
      }

      switch (dev)
      {
         case kDI_MaxJoy:
            {
               for (i=0; i<kDI_MaxJoy; i++)
               {
                  if (DID2_Joy[i])
                  {
                     if (DXInput_AcquireErr (IDirectInputDevice2_Acquire(DID2_Joy[i]),i))
                        ++cnt;
                  }
               }
               return(cnt);
            }
            break;
         default:
            if (dev > kDI_AllDevices)
            {
               printf("Invalid device ID, out of range\n");
               return(0);
            }
            else
            if (DID2_Joy[dev])
            {
               return DXInput_AcquireErr (IDirectInputDevice2_Acquire(DID2_Joy[dev]),dev);
            }
            break;
      }
   }
   else
      printf ("Direct Input object not initialized...\n");

   return(cnt);
}

// ------------
// Input_Exit
// -------------------------------------------------------------------
// -------------------------------------------------------------------
void
Input_Exit(void)
{
int i;

   if (DI)
   {  // Release All Joystick Devices
      // ----------------------------
      for (i=0; i<kDI_MaxJoy; i++)
      {  if (DID2_Joy[i])
         {  IDirectInputDevice2_Unacquire(DID2_Joy[i]);
            IDirectInputDevice2_Release(DID2_Joy[i]);
            DID2_Joy[i] = NULL;
         }
      }
      // Release Direct Input
      // --------------------
      IDirectInput_Release(DI);
      DI = NULL;
   }

   maskFFB  = 0;
   numJoy   = 0;
}



// --------
// joy_Init
// -------------------------------------------------------------------
// Purpose:
//    Creates and acquires all joysticks
//
// Input:
//    None
//
// Return:
//    # of sticks acquired
//
// Description:
//
// -------------------------------------------------------------------
int
joy_Init(void)
{
   // Make sure Main DInput OBJ has been created
   // ------------------------------------------ 
   if (!DI)
   {
      DXInput_Init();      
      if (!DI)
      {
         printf("Dinput not initialized yet\n");
         return(FALSE);
      }
   }
   // Don't do this if it's already successfully been done
   // ----------------------------------------------------
   if (!numJoy)
   {
   int i;
      IDirectInput_EnumDevices(  DI, 
                                 DIDEVTYPE_JOYSTICK, 
                                 DIEnumJoysticks_Callback, 
                                 DI, 
                                 DIEDFL_ATTACHEDONLY); 
      for (i=0; i<numJoy; i++)
      {  joy_query(i, NULL, NULL, NULL);
      }
   }
   // Only attempt to acquire if a joy device is present
   // --------------------------------------------------
   if (numJoy)
      DXInput_Acquire(kDI_MaxJoy);

   return(numJoy);
}

// ---------
// joy_Query
// -------------------------------------------------------------------
// Purpose:
//    Besides checking what buttons/axis are available, this function
//    also checks for force feedback support.
// -------------------------------------------------------------------
int
joy_Query(int dev, int* but_flags, int* axis_flags)
{
int                     i,bit;
DIDEVCAPS               DICaps; 
DIDEVICEOBJECTINSTANCE  DIObjInst; 
DWORD                   DIAxisOFS[kDI_MaxJoyAxis] = 
                                       {  DIJOFS_X,
                                          DIJOFS_Y,
                                          DIJOFS_Z,
                                          DIJOFS_RX,
                                          DIJOFS_SLIDER(0),
                                          DIJOFS_SLIDER(1) };

   // Make sure Main DInput OBJ has been created
   // ------------------------------------------ 
   if (!DI)
   {
      DXInput_Init();      
      if (!DI)
      {
         printf("Dinput not initialized yet\n");
         return(FALSE);
      }
   }

   if (!DID2_Joy[dev])
   {
      print("device not found #%d\n",dev);
      return(0);
   }

   if (!numJoy)
   {
      if (but_flags)*but_flags=0;
      if (axis_flags)*axis_flags=0;
      return(0);
   }

   DXInput_Acquire(dev);

   DICaps.dwSize = sizeof(DIDEVCAPS); 
   
   if (IDirectInputDevice2_GetCapabilities(DID2_Joy[dev],&DICaps) != DI_OK)
   {
      print("Failed getting device caps\n");
      return(0);
   }

   if (DICaps.dwFlags & DIDC_FORCEFEEDBACK)
   {
      print("ffb support\n");
      maskFFB |= 1<<dev;
   }

   DIObjInst.dwSize = sizeof(DIDEVICEOBJECTINSTANCE); 

   // Get the Axis flags
   // ------------------
   if (but_flags)
   {
      *but_flags = 0;

      for (i=0,bit=0; i<DICaps.dwButtons && bit < 32; ++bit)
      {
         if (IDirectInputDevice2_GetObjectInfo(DID2_Joy[dev],&DIObjInst,DIJOFS_BUTTON(bit), DIPH_BYOFFSET)
         == DI_OK)
         {
            *but_flags |= 1<<bit;
            ++i;
         }
      }
   }

   // Get the Axis flags
   // ------------------
   if (axis_flags)
   {
      *axis_flags = 0;

      for (i=0,bit=0; i<DICaps.dwAxes && bit < 8; ++bit)
      {
         if (IDirectInputDevice2_GetObjectInfo(DID2_Joy[dev],&DIObjInst,DIAxisOFS[bit], DIPH_BYOFFSET)
         == DI_OK)
         {
            *axis_flags |= 1<<bit;
            ++i;
         }
      }
   }
   return(1);
}


// --------
// joy_Read
// -------------------------------------------------------------------
// -------------------------------------------------------------------
int
joy_Read(tDI_Device dev, int* buttons, int* x, int* y, int* z, int* r, int* u, int* v, int* pov) 
{ 
HRESULT     hRes; 
int         i;
DIJOYSTATE  js; 

   if (!DID2_Joy[dev])
      return(0);

   // poll the joystick to read the current state
   // -------------------------------------------
   hRes = IDirectInputDevice2_Poll (DID2_Joy[dev]);

   if (hRes != DI_OK) 
   { 
      if(hRes == DIERR_INPUTLOST)
      {
         printf ("Can't poll, input lost\n");
         if (!DXInput_Acquire(dev))
         {
            printf ("couldn't reacquire joystick %d\n",dev);
         }
      }
      else
      if(hRes == DIERR_NOTACQUIRED)
      {
         printf ("Can't poll, not acquired\n");
         if (!DXInput_Acquire(dev))
         {
            printf ("couldn't reacquire joystick %d\n",dev);
         }
      }
      return(0);
   } 

   // get data from the joystick
   // --------------------------
   hRes = IDirectInputDevice2_GetDeviceState (DID2_Joy[dev], sizeof(DIJOYSTATE), &js); 
 
   if (hRes != DI_OK) 
   { 
      printf ("attempting to reacquire\n");
      if(hRes == DIERR_INPUTLOST) 
         DXInput_Acquire(dev);
      return 0; 
   } 
   
   // Return Axis values for non NULL parameters
   // ------------------------------------------
   if(x)    
      *x = js.lX; 
   if(y)    
      *y = js.lY; 
   if(z)    
      *z = js.lZ;
   if(r)    
      *r = js.lRz;
   if(u)    
      *u = js.rglSlider[0];
   if(v)    
      *v = js.rglSlider[1];

   if(pov)  
      *pov = js.rgdwPOV[0];

   // Return the buttons in a bit array
   // ---------------------------------
   if (buttons)
   {  *buttons = 0;        
      for (i=0; i<32; i++)
      {  if (js.rgbButtons[i] & 0x80)
         {  *buttons |= 1<<i;        
         }
      }
   }
   return 1; 
} 
 


// --------
// ffb_Init
// -------------------------------------------------------------------
// Purpose:
//    Initialize force feedback if available.
// -------------------------------------------------------------------
void
ffb_Init(void)
{
   if (!numJoy)
      joy_init();

   if (maskFFB)
   {int i;  
      for (i=kDI_Joy1; i<kDI_MaxJoy; i++)
      {  if (DID2_Joy[i] && (maskFFB & (1<<i)))
            IDirectInputDevice2_SendForceFeedbackCommand(DID2_Joy[i],DISFFC_RESET);
      }
   }
   return maskFFB;
}

// ---------
// ffb_Pause
// -------------------------------------------------------------------
// Purpose:
//    Pause the FFB output on the given device.  Use ffb_Continue to
//    continue where you left off.
// -------------------------------------------------------------------
void
ffb_Pause(tDI_Device dev)
{
   if (dev == kDI_MaxJoy)
   {int i;
      for (i=0; i<kDI_MaxJoy; i++)
      {  ffb_Pause(i);
      }
      return;
   }

   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(DID2_Joy[dev],DISFFC_PAUSE);
}
   
// ------------
// ffb_Continue
// -------------------------------------------------------------------
// Purpose:
//    Unpause the FFB output on the given device.  Complimentary to
//    ffb_Pause.
// -------------------------------------------------------------------
void
ffb_Continue(tDI_Device dev)
{
   if (dev == kDI_MaxJoy)
   {int i;
      for (i=0; i<kDI_MaxJoy; i++)
      {  ffb_Continue(i);
      }
      return;
   }

   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(DID2_Joy[dev],DISFFC_CONTINUE);
}

// ----------
// ffb_Enable
// -------------------------------------------------------------------
// Purpose:
//    Must be called after initialization in order to activate the 
//    device.
//    Use ffb_Pause & ffb_Continue if you want disable forces
//    temporarily and resume later.
// -------------------------------------------------------------------
void
ffb_Enable(tDI_Device dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(DID2_Joy[dev],DISFFC_SETACTUATORSON);
}

// -----------
// ffb_Disable
// -------------------------------------------------------------------
// Purpose:
//    Turns off FFB, but effects still play on processor.
// -------------------------------------------------------------------
void
ffb_Disable(tDI_Device dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(DID2_Joy[dev],DISFFC_SETACTUATORSOFF);
}

// ----------------
// ffb_effectCreate
// -------------------------------------------------------------------
// Purpose:
//    Create a single effect for future playback.
//    Effect is given a logical ID
// -------------------------------------------------------------------
int
ffb_effectCreate(tDI_Device dev, tFFB_Effect* eff)
{
HRESULT hr;

   if (numEffects >= kDI_MaxEffects)
   {
      printf ("Reached hardcoded limit for # of effects.");
      return(-1);
   }
   if (!(DID2_Joy[dev] && (maskFFB & 1<<dev)) )
      return(-1);

   // Important stuff
   Effect[numEffects].general.dwDuration              = eff->Duration;             
   Effect[numEffects].general.dwSamplePeriod          = eff->Period;
   Effect[numEffects].general.dwGain                  = eff->Gain;
   Effect[numEffects].general.dwTriggerButton         = (eff->Trigger==kNoButton?DIEB_NOTRIGGER:DIJOFS_BUTTON(eff->Trigger));
   Effect[numEffects].general.dwTriggerRepeatInterval = eff->TriggerRepeatTime;
   Effect[numEffects].direction[0]                    = eff->Direction;
   memcpy(&Effect[numEffects].specific,&eff->TypeInfo,sizeof(tEffectClasses));

   // Wacky COM related overhead
   Effect[numEffects].general.dwSize                  = sizeof(DIEFFECT); 
   Effect[numEffects].general.cAxes                   = isWheel[dev]?1:2; 
   Effect[numEffects].general.rgdwAxes                = &dwAxes[0]; 
   Effect[numEffects].general.dwFlags                 = DIEFF_POLAR | DIEFF_OBJECTOFFSETS; 
   Effect[numEffects].general.lpvTypeSpecificParams   = &Effect[numEffects].specific;  
   Effect[numEffects].general.rglDirection            = &Effect[numEffects].direction;
   Effect[numEffects].direction[1]                    = 0L;
   Effect[numEffects].general.lpEnvelope              = NULL;
   Effect[numEffects].general.lpvTypeSpecificParams   = &Effect[numEffects].specific;


   switch (eff->Type)
   {
      case kConstant:
         Effect[numEffects].general.cbTypeSpecificParams    = sizeof(DICONSTANTFORCE);
         break;
      case kRamp:
         Effect[numEffects].general.cbTypeSpecificParams    = sizeof(DIRAMPFORCE);
         break;
      case kCustom:
         Effect[numEffects].general.cbTypeSpecificParams    = sizeof(DICUSTOMFORCE);
         break;
      case kWave_Square:
      case kWave_Sine:
      case kWave_Triange:
      case kWave_SawUp:
      case kWave_SawDown:
         Effect[numEffects].general.cbTypeSpecificParams    = sizeof(DIPERIODIC);
         break;
      case kCondition_Spring:
      case kCondition_Damper:
      case kCondition_Inertia:
      case kCondition_Friction:
         Effect[numEffects].general.cbTypeSpecificParams    = sizeof(DICONDITION);
         break;
      default:
         printf("BUG: bad effect subType");
         return(-1);
   }


   hr=IDirectInputDevice2_CreateEffect(   DID2_Joy[dev], 
                                          effectGUID[eff->Type], 
                                          (LPCDIEFFECT)           &Effect[numEffects], 
                                          (LPDIRECTINPUTEFFECT*)  &DIE_hEffect[numEffects], 0);
   switch (hr)
   {
   case DI_OK:
      print ("effect created\n");
      ffb_effectUnload(numEffects);
      ++numEffects;
      return(numEffects-1);
      
   case DIERR_DEVICENOTREG:
      printf ("effect not created, DIERR_DEVICENOTREG\n");
      return(-1);
      
   case DIERR_DEVICEFULL:
      printf ("effect not created, DIERR_DEVICEFULL\n");
      return (-1);
   }
   return(-1);
}

// --------------
// ffb_effectPlay
// -------------------------------------------------------------------
// Purpose:
//    Play an effect that was previously created.
// -------------------------------------------------------------------
void
ffb_effectPlay(short eID)
{
   IDirectInputEffect_Start(DIE_hEffect[eID],1,0);
}

// --------------
// ffb_effectStop
// -------------------------------------------------------------------
// Purpose:
//    Stop a single effect.
// -------------------------------------------------------------------
void
ffb_effectStop(short eID)
{
   IDirectInputEffect_Stop(DIE_hEffect[eID]);
}

// -----------------
// ffb_effectStopAll
// -------------------------------------------------------------------
// Purpose:
//    Stops all forces on the given device.
// -------------------------------------------------------------------
void
ffb_effectStopAll(tDI_Device dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev) )
      IDirectInputDevice2_SendForceFeedbackCommand(DID2_Joy[dev],DISFFC_STOPALL);
}

// ----------------
// ffb_effectUnload
// -------------------------------------------------------------------
// Purpose:
//    Unload a single effect...  Necessary to make room for other
//    effects.
// -------------------------------------------------------------------
void
ffb_effectUnload(short eID)
{
   IDirectInputEffect_Unload(DIE_hEffect[eID]);
}


// ----------------
// ffb_effectModify
// -------------------------------------------------------------------
// Purpose:
//    Modifies a single effect, only if the given parameters are
//    different from what's currently loaded.
// -------------------------------------------------------------------
void
ffb_effectModify(short eID, int*	Direction, int* Duration, int* Gain, 
					  int* Period, tEffInfo* TypeInfo, tEffEnvelope* Envelope)
{
int flags = 0;

   return;
   if (Direction)
   {
      if (Effect[eID].direction[0] != *Direction)
      {
         Effect[eID].direction[0] = *Direction;
         flags |= DIEP_DIRECTION;
      }
   }
   if (Duration)
   {
      if (Effect[eID].general.dwDuration != *Duration)
      {
         Effect[eID].general.dwDuration = *Duration;
         flags |= DIEP_DURATION;
      }
   }
   if (Gain)
   {
      if (Effect[eID].general.dwGain != *Gain)
      {
         Effect[eID].general.dwGain = *Gain;
         flags |= DIEP_GAIN;
      }
   }
   if (Period)
   {
      if (Effect[eID].general.dwSamplePeriod != *Period)
      {
         Effect[eID].general.dwSamplePeriod = *Period;
         flags |= DIEP_SAMPLEPERIOD;
      }
   }
   if (TypeInfo)
   {
      if (!memcmp( &Effect[eID].specific, TypeInfo, Effect[eID].general.cbTypeSpecificParams))
      {
         memcpy( &Effect[eID].specific, TypeInfo, Effect[eID].general.cbTypeSpecificParams);
         flags |= DIEP_TYPESPECIFICPARAMS;
      }
   }
   if (Envelope)
   {
      if (Effect[eID].envelope.dwAttackLevel != Envelope->AttackLevel ||
          Effect[eID].envelope.dwAttackTime  != Envelope->AttackTime  ||      
          Effect[eID].envelope.dwFadeLevel   != Envelope->FadeLevel   ||
          Effect[eID].envelope.dwFadeTime    != Envelope->FadeTime     )      
      {
         Effect[eID].envelope.dwAttackLevel  = Envelope->AttackLevel;
         Effect[eID].envelope.dwAttackTime   = Envelope->AttackTime;
         Effect[eID].envelope.dwFadeLevel    = Envelope->FadeLevel;
         Effect[eID].envelope.dwFadeTime     = Envelope->FadeTime;
         flags |= DIEP_ENVELOPE;
      }
   }
   
   if (!flags)
      return;

   IDirectInputEffect_SetParameters(DIE_hEffect[eID], (LPCDIEFFECT)&Effect[eID], flags);
}


//====================================================================
//                     Private Fuction Decals 
//====================================================================

// ------------------------
// DIEnumJoysticks_Callback
// -------------------------------------------------------------------
// Purpose:
//    Initialize all connected joysticks.
//
// Input:
//    pdinst   info about the current joystick being enumed.
//    pvRef    the direct input object that was passed in before
//             starting the enum process.
// Return:
//   DIENUM_CONTINUE    continue calling us with any more devices
//   DIENUM_STOP        all done, don't call us back anymore, go away.
// -------------------------------------------------------------------
BOOL CALLBACK 
DIEnumJoysticks_Callback(LPDIDEVICEINSTANCE pdinst, LPVOID pvRef)
{
LPDIRECTINPUT        pdi   = pvRef;
LPDIRECTINPUTDEVICE  pdev;

   if (DIDEVTYPEJOYSTICK_WHEEL == GET_DIDEVICE_SUBTYPE(pdinst->dwDevType))
      isWheel[numJoy] = TRUE;
   else
      isWheel[numJoy] = FALSE;

   // Create an instance of the device
   // --------------------------------   
   if (IDirectInput_CreateDevice 
       (pdi, &pdinst->guidInstance, &pdev, NULL) 
      != DI_OK) 
   { 
      printf ("Could not create dinput device obj\n");
      return DIENUM_CONTINUE; 
   }

   // Set the data format to the default
   // ---------------------------------- 
   if (IDirectInputDevice_SetDataFormat 
       (pdev, &c_dfDIJoystick) 
      != DI_OK) 
   { 
      printf ("Could not set dinput device data format\n");
      IDirectInputDevice_Release (pdev); 
      return DIENUM_CONTINUE; 
   }

   // Get the DID2 from DID1 
   // ----------------------
   if (IDirectInputDevice_QueryInterface
       (pdev, &IID_IDirectInputDevice2, (void **) &DID2_Joy[numJoy])
      != DI_OK)
   {
      printf ("QueryInterface did not return DI_OK\n");
      IDirectInputDevice_Release (pdev);
      return DIENUM_CONTINUE; 
   }
   
   // Set the cooperative level 
   // -------------------------
   if (!DXInput_SetCoopLevel (numJoy, kJOY_COOP_FLAGS))
   {
      printf ("Could not set dinput mouse coop level\n");
      return FALSE;
   }

   // Done with Device1
   // -----------------
   IDirectInputDevice_Release (pdev);

   // Device was added successfully
   // -----------------------------
   ++numJoy;

   return DIENUM_CONTINUE;
}


// ----------------
// Input_AcquireErr
// -------------------------------------------------------------------
// Purpose:
//    Handle success/err reporting
// -------------------------------------------------------------------
static int
Input_AcquireErr(HRESULT res, int dev_num)
{
   if (res == DI_OK)
   {
      printf ("device #%d acquired\n",dev_num);
      return(TRUE);
   }
   else
   if (res == S_FALSE)
   {
      printf ("device #%d already acquired\n",dev_num);
      return(TRUE);
   }
   else
   if (res == DIERR_INVALIDPARAM)
   {
      printf ("device %d DIERR_INVALIDPARAM\n",dev_num);
      return(FALSE);
   }
   else
   if (res == DIERR_OTHERAPPHASPRIO)
   {
      printf ("device %d DIERR_OTHERAPPHASPRIO\n",dev_num);
      return(TRUE);
   }
   else
      printf ("Unknown Error acquiring device %d\n",dev_num);

   return(FALSE);
}


