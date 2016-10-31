//====================================================================
// input.c
//==========
// Direct Input Force Feedback code
//====================================================================
#include "Input.h"
#include "dinput.h" 

//====================================================================
//                     Private Declarations 
//====================================================================
#define kJOY_COOP_FLAGS (DISCL_EXCLUSIVE | DISCL_BACKGROUND)
#define kMaxEffects  30

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
   DICONDITION       condition[2];
   DICUSTOMFORCE     custom;

}tEffectClasses;

typedef struct tEffect
{
   DIEFFECT       general;
   tEffectClasses specific;
   DIENVELOPE     envelope;
   LONG           direction[2];

}tEffect;

static tEffect                Effect[kMaxEffects];

// DInput Data
// -----------
static LPDIRECTINPUT          DI                   = NULL; 
static LPDIRECTINPUTDEVICE2   DID2_Joy[kMaxJoy];
static LPDIRECTINPUTEFFECT    DIE_hEffect[kMaxEffects];

// Joystick Data
// -------------
tJoyInfo       JoyInfo[kMaxJoy];
static int     maskFFB                       = 0;
static int     numJoy                        = 0;
static int     numEffects                    = 0;
static DWORD   dwAxis[2];

// Private Functions
// -----------------
BOOL CALLBACK  EnumJoy_Callback (LPDIDEVICEINSTANCE pdinst, LPVOID pvRef);
static int     DeviceAcquireErr (HRESULT res, int dev_num);
static int	   Device_SetCoopLevel (tDevice,DWORD coop_level);

// -------------------------------------------------------------------
//    Initializes all attached force feedback devices.
// -------------------------------------------------------------------
int 
ForceInit(void)
{
   if (!numJoy)
      JoyInit();

   if (maskFFB)
   {int i;  
      for (i=kJoy1; i<kMaxJoy; i++)
      {  if (DID2_Joy[i] && (maskFFB & (1<<i)))
            IDirectInputDevice2_SendForceFeedbackCommand(
               DID2_Joy[i], DISFFC_RESET);
      }
   }
   return maskFFB;
}
// -------------------------------------------------------------------
//    Pause the FFB output on the given device.  Use ffb_Continue to
//    continue where you left off.
// -------------------------------------------------------------------
void
ffb_Pause(tDevice dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(
         DID2_Joy[dev],DISFFC_PAUSE);
}
// -------------------------------------------------------------------
//    Unpause the FFB output on the given device.  Complimentary to
//    ffb_Pause.
// -------------------------------------------------------------------
void
ffb_Continue(tDevice dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(
         DID2_Joy[dev],DISFFC_CONTINUE);
}
// -------------------------------------------------------------------
//    Must be called after initialization in order to activate the 
//    device.  All Effects are stopped prior to enabling the device.
//    Use ffb_Pause & ffb_Continue if you want disable forces
//    temporarily and resume later.
// -------------------------------------------------------------------
void
ffb_Enable(tDevice dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(
         DID2_Joy[dev],DISFFC_SETACTUATORSON);

   ffb_effectStopAll(dev);
   ffb_Continue(dev);
}
// -------------------------------------------------------------------
//    Complimentary to ffb_Enable
// -------------------------------------------------------------------
void
ffb_Disable(tDevice dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev))
      IDirectInputDevice2_SendForceFeedbackCommand(
         DID2_Joy[dev],DISFFC_SETACTUATORSOFF);
}
// -------------------------------------------------------------------
//    Set the Gain on a device for all effects. 0-100%
// -------------------------------------------------------------------
void	
ffb_SetGain(tDevice dev, int percentGain)
{
DIPROPDWORD prop;
int         i;

   prop.diph.dwSize        = sizeof(DIPROPDWORD);
   prop.diph.dwHeaderSize  = sizeof(DIPROPHEADER);
   prop.diph.dwObj         = 0;
   prop.diph.dwHow         = DIPH_DEVICE;

   // Convert 0-100 to DInputs 0-10,000
   // ---------------------------------
   if (percentGain>100)
      prop.dwData = 10000;
   else
   if (percentGain<0)
      prop.dwData = 0;
   else
      prop.dwData = percentGain * 100;

   // See what effects are currently playing
   // --------------------------------------
   // ... NEED TO DO

   // Unload the effects
   // ------------------
   for (i=0; i<numEffects; i++)
      ffb_effectUnload(i);

   IDirectInputDevice2_SetProperty (
      DID2_Joy[dev], DIPROP_FFGAIN, (LPCDIPROPHEADER)&prop.diph);

   // Reload the effects
   // ------------------
   for (i=0; i<numEffects; i++)
      ffb_effectLoad(i);

   // Restart previously playing effects
   // ----------------------------------
   // ... NEED TO DO
}

// -------------------------------------------------------------------
//    Create a single effect for the given device
// -------------------------------------------------------------------
int
ffb_effectCreate(tDevice dev, tFFB_Effect* eff)
{
   // Important stuff
   Effect[numEffects].general.dwDuration              
      = (eff->Duration==kInfinite_Duration?INFINITE:(eff->Duration*1000));
   Effect[numEffects].general.dwGain                  
      = eff->Gain;
   Effect[numEffects].general.dwTriggerButton         
      = (eff->Trigger==kNoButton?DIEB_NOTRIGGER:DIJOFS_BUTTON(eff->Trigger));
   Effect[numEffects].general.dwTriggerRepeatInterval 
      = eff->TriggerRepeatTime;
   Effect[numEffects].direction[0]                    
      = eff->Direction * 100;

   if (eff->Axis == kXAxisOnly || (JoyInfo[dev].DevType & kWheel))
   {
      Effect[numEffects].general.cAxes                = 1; 
      Effect[numEffects].general.rgdwAxes             = dwAxis; 
      dwAxis[0]                                       = DIJOFS_X;
   }
   else
   if (eff->Axis == kYAxisOnly)
   {
      Effect[numEffects].general.cAxes                = 1; 
      Effect[numEffects].general.rgdwAxes             = dwAxis; 
      dwAxis[0]                                       = DIJOFS_Y;
   }
   else
   {
      Effect[numEffects].general.cAxes                = 2;
      Effect[numEffects].general.rgdwAxes             = dwAxis; 
      dwAxis[0]                                       = DIJOFS_X;
      dwAxis[1]                                       = DIJOFS_Y;
   }

   memcpy(&Effect[numEffects].specific,&eff->TypeInfo,sizeof(tEffectClasses));

   // Wacky COM related overhead
   Effect[numEffects].general.dwSize                  
      = sizeof(DIEFFECT); 
   Effect[numEffects].general.dwFlags                 
      = DIEFF_POLAR | DIEFF_OBJECTOFFSETS; 
   Effect[numEffects].general.dwSamplePeriod          
      = 0;//HZ_to_uS(100);
   Effect[numEffects].general.lpvTypeSpecificParams   
      = &Effect[numEffects].specific;  
   Effect[numEffects].general.rglDirection            
      = &Effect[numEffects].direction;
   Effect[numEffects].direction[1]                    
      = 0L;
   Effect[numEffects].general.lpEnvelope              
      = NULL;  // not supported now, maybe later
   Effect[numEffects].general.lpvTypeSpecificParams   
      = &Effect[numEffects].specific;


   switch (eff->Type)
   {
      case kConstant:
         Effect[numEffects].general.cbTypeSpecificParams    
            = sizeof(DICONSTANTFORCE);         break;
      case kRamp:
         Effect[numEffects].general.cbTypeSpecificParams    
            = sizeof(DIRAMPFORCE);             break;
      case kCustom:
         Effect[numEffects].general.cbTypeSpecificParams    
            = sizeof(DICUSTOMFORCE);           break;
      case kWave_Square:
      case kWave_Sine:
      case kWave_Triange:
      case kWave_SawUp:
      case kWave_SawDown:
         Effect[numEffects].general.cbTypeSpecificParams    
            = sizeof(DIPERIODIC);              break;
      case kCondition_Spring1D:
      case kCondition_Damper:
      case kCondition_Inertia:
      case kCondition_Friction:
         Effect[numEffects].general.cbTypeSpecificParams    
            = sizeof(DICONDITION);             break;
      case kCondition_Spring2D:
         Effect[numEffects].general.cbTypeSpecificParams    
            = sizeof(DICONDITION)*2;           break;
      default:
         bug("bad effect subType");
         return(-1);
   }

   IDirectInputDevice2_CreateEffect(   
      DID2_Joy[dev], 
      effectGUID[eff->Type], 
      (LPCDIEFFECT)           &Effect[numEffects], 
      (LPDIRECTINPUTEFFECT*)  &DIE_hEffect[numEffects], 0);

   ffb_effectUnload(numEffects);
   ++numEffects;
   return(numEffects-1);
}
// -------------------------------------------------------------------
//    Play the effect in its current state
// -------------------------------------------------------------------
void
ffb_effectPlay(short eID)
{
   IDirectInputEffect_Start(DIE_hEffect[eID],1,0);
}
// -------------------------------------------------------------------
//    Stop the effect (doesn't matter if it's playing or not)
// -------------------------------------------------------------------
void
ffb_effectStop(short eID)
{
   IDirectInputEffect_Stop(DIE_hEffect[eID]);
}
// -------------------------------------------------------------------
//    Stops all forces on the given device.
// -------------------------------------------------------------------
void
ffb_effectStopAll(tDevice dev)
{
   if (DID2_Joy[dev] && (maskFFB & 1<<dev) )
      IDirectInputDevice2_SendForceFeedbackCommand(
         DID2_Joy[dev],DISFFC_STOPALL);
}
// -------------------------------------------------------------------
//    Send the effect to the device RAM (activates it on MS devices
//    if it is a trigger effect).
// -------------------------------------------------------------------
void
ffb_effectLoad(short eID)
{
   IDirectInputEffect_Download(DIE_hEffect[eID]);
}
// -------------------------------------------------------------------
//    Stops the effect if it is playing, and removes it from
//    device RAM
// -------------------------------------------------------------------
void
ffb_effectUnload(short eID)
{
   IDirectInputEffect_Unload(DIE_hEffect[eID]);
}
// -------------------------------------------------------------------
//    Changes an effect whether or not it is downloaded or playing
// -------------------------------------------------------------------
int
ffb_effectModify(short eID,            tLoadEffect PlayNow, 
                 long* Direction,      tUL* Time,            
                 tUL* Gain,            tEffInfo* TypeInfo, 
                 tJoyButtons* Trigger, tEffEnvelope* Envelope)
{
int flags = 0;

   if (Direction)
   {
      if (*Direction > 360)
         *Direction = 360;
      else
      if (*Direction < 0)
         *Direction = 0;
               
      *Direction *= 100;

      if (Effect[eID].direction[0] != *Direction)
      {
         Effect[eID].direction[0] = *Direction;
         flags |= DIEP_DIRECTION;
      }
   }
   if (Time)
   {
      if (*Time==kInfinite_Duration)
      {  *Time = INFINITE;
      }
      else
      {  if (*Time > 1500)  //1.5 sec
            *Time = 1500;
               
         *Time *= 1000;
      }

      if (Effect[eID].general.dwDuration != *Time)
      {
         Effect[eID].general.dwDuration = *Time;
         flags |= DIEP_DURATION;
      }
   }
   if (Gain)
   {
      if (*Gain > 10000)
         *Gain = 10000;

      if (Effect[eID].general.dwGain != *Gain)
      {
         Effect[eID].general.dwGain = *Gain;
         flags |= DIEP_GAIN;
      }
   }
   if (TypeInfo)
   {
      if (memcmp( &Effect[eID].specific, TypeInfo, 
            Effect[eID].general.cbTypeSpecificParams))
      {
         //print("moding -- typeSpecific\n");
         memcpy( &Effect[eID].specific, TypeInfo, 
            Effect[eID].general.cbTypeSpecificParams);
         flags |= DIEP_TYPESPECIFICPARAMS;
      }
   }
   if (Trigger)
   {
      if (*Trigger < kButtonMax)
      {  
         Effect[eID].general.dwTriggerButton = 
            (*Trigger==kNoButton ? DIEB_NOTRIGGER : DIJOFS_BUTTON(*Trigger));
         flags |= DIEP_TRIGGERBUTTON;
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
   {
      if (PlayNow == kPlayNow)
      {  ffb_effectPlay(eID);
      }
      return(0);
   }
   
   ffb_effectUnload(eID);

   IDirectInputEffect_SetParameters(
      DIE_hEffect[eID], (LPCDIEFFECT)&Effect[eID], flags);

   if (PlayNow != kDontPlayNow)
      ffb_effectPlay(eID);

   return(1);
}

// -------------------------------------------------------------------
//    Init the Direct Input system
// -------------------------------------------------------------------
void
InputInit(void)
{
int      i;

   if (DI)
      return;

   for (i=0; i<kMaxJoy; i++)
   {
      DID2_Joy[i]                = NULL;
      JoyInfo[i].ButtonMask   = 0;
      JoyInfo[i].AxisMask     = 0;
      JoyInfo[i].DevType      = 0;
      JoyInfo[i].Name[0]      = 0;
   }
   for (i=0; i<kMaxEffects; i++)
      DIE_hEffect[i] = 0;

   maskFFB  = 0;
   numJoy   = 0;

   DirectInputCreate(
      (HINSTANCE)GetModuleHandle(NULL), DIRECTINPUT_VERSION, &DI, NULL); 
}
// -------------------------------------------------------------------
//    Tells windows we are no longer using the device.
// -------------------------------------------------------------------
void
DeviceRelease(tDevice dev)
{
   if (DID2_Joy[dev])
      IDirectInputDevice_Unacquire(DID2_Joy[dev]);
}
// -------------------------------------------------------------------
//    Creates and acquires all joysticks (called only once)
// -------------------------------------------------------------------
int
JoyInit(void)
{
static int alreadyInited=0;
   // Make sure Main DInput OBJ has been created
   // ------------------------------------------ 
   if (!DI)
   {    
      InputInit();      
      if (!DI)
         return(FALSE);
      alreadyInited=0;
   }
   if (alreadyInited)
      return(numJoy);
   else
      alreadyInited=1;

   // Don't do this if it's already successfully been done
   // ----------------------------------------------------
   if (!numJoy)
   {
      IDirectInput_EnumDevices(  DI, 
                                 DIDEVTYPE_JOYSTICK, 
                                 EnumJoy_Callback, 
                                 DI, 
                                 DIEDFL_ATTACHEDONLY); 
      JoyQuery(kMaxJoy,NULL,NULL);
   }

   // Only attempt to acquire if a joy device is present
   // --------------------------------------------------
   if (numJoy)
      DeviceAcquire(kMaxJoy);

   return(numJoy);
}
// -------------------------------------------------------------------
//    See if device support FFB
// -------------------------------------------------------------------
tJoyInfo* 
JoyQuery(tDevice dev, int* ButtMask, int* AxMask)
{
int                     i,bit;
DIDEVCAPS               DICaps; 
DIDEVICEOBJECTINSTANCE  DIObjInst; 
DWORD                   DIAxisOFS[kMAX_Axis] = 
                                       {  DIJOFS_X,
                                          DIJOFS_Y,
                                          DIJOFS_Z,
                                          DIJOFS_RX,
                                          DIJOFS_SLIDER(0),
                                          DIJOFS_SLIDER(1) };
   if (ButtMask)
      *ButtMask= 0;
   if (AxMask)
      *AxMask  = 0;

   // Make sure Main DInput OBJ has been created
   // ------------------------------------------ 
   if (!DI)
   {  InputInit();      
      if (!DI)
         return(NULL);
   }

   if (!DID2_Joy[dev])
      return(NULL);

   DeviceAcquire(dev);

   // No need to waste time reQuerying since we have the info saved
   // -------------------------------------------------------------
   if (JoyInfo[dev].ButtonMask || JoyInfo[dev].AxisMask)
   {
      if (ButtMask)
         *ButtMask = JoyInfo[dev].ButtonMask;
      if (AxMask)
         *AxMask   = JoyInfo[dev].AxisMask;
      return(&JoyInfo[dev]);
   }
      
   DICaps.dwSize = sizeof(DIDEVCAPS); 
   
   if (IDirectInputDevice2_GetCapabilities(DID2_Joy[dev],&DICaps) 
      != DI_OK)
      return(NULL);

   if (DICaps.dwFlags & DIDC_FORCEFEEDBACK)
   {  printf("ffb support\n");
      maskFFB |= 1<<dev;
   }

   DIObjInst.dwSize = sizeof(DIDEVICEOBJECTINSTANCE); 

   // Get the Button flags
   // --------------------
   for (i=0,bit=0; i<DICaps.dwButtons && bit < kButtonMax; ++bit)
   {
      if (IDirectInputDevice2_GetObjectInfo(
            DID2_Joy[dev],&DIObjInst,DIJOFS_BUTTON(bit), DIPH_BYOFFSET)
      == DI_OK)
      {
         JoyInfo[dev].ButtonMask |= 1<<bit;
         if (ButtMask)
            *ButtMask = JoyInfo[dev].ButtonMask;
         ++i;
      }
   }

   // Get the Axis flags
   // ------------------
   for (i=0,bit=0; i<DICaps.dwAxes && bit < kMAX_Axis; ++bit)
   {
      if (IDirectInputDevice2_GetObjectInfo(
            DID2_Joy[dev],&DIObjInst,DIAxisOFS[bit], DIPH_BYOFFSET)
      == DI_OK)
      {
         JoyInfo[dev].AxisMask |= 1<<bit;
         if (AxMask)
            *AxMask  = JoyInfo[dev].AxisMask;
         ++i;
      }
   }
   return(&JoyInfo[dev]);
}
// -------------------------------------------------------------------
//    Call this to gain access to a device after the device has been
//    created & after regaining the focus after losing it.
// -------------------------------------------------------------------
int
DeviceAcquire(tDevice dev)
{
int cnt=0;
   
   if (DI)
     if (DID2_Joy[dev])
        return DeviceAcquireErr 
            (IDirectInputDevice2_Acquire(DID2_Joy[dev]),dev);

   return(cnt);
}
// -------------------------------------------------------------------
// -------------------------------------------------------------------
int
ReadJoy(tDevice dev, int* buttons, 
        int* x, int* y, int* z, int* r, int* u, int* v, int* pov) 
{ 
HRESULT     hRes; 
int         i;
DIJOYSTATE  js; 

   if (!DID2_Joy[dev])
   {
      if(pov)     *pov = 0;
      if(buttons) *buttons = 0;
      return(0);
   }

   // poll the joystick to read the current state
   hRes = IDirectInputDevice2_Poll (DID2_Joy[dev]);

   if (hRes != DI_OK) 
   { 
      if(hRes == DIERR_INPUTLOST)
      {  print ("Can't poll, input lost\n");
         if (!DeviceAcquire(dev))
            bug("couldn't reacquire joystick %d\n",dev);
      }
      else
      if(hRes == DIERR_NOTACQUIRED)
      {  print ("Can't poll, not acquired\n");
         if (!DeviceAcquire(dev))
            bug("couldn't reacquire joystick %d\n",dev);
      }
      else
         bug("Can't poll, unknown error\n");
      return(0);
   } 

   // get data from the joystick 
   hRes = IDirectInputDevice2_GetDeviceState(
      DID2_Joy[dev], sizeof(DIJOYSTATE), &js); 
 
   if (hRes != DI_OK) 
   { 
      print ("attempting to reacquire\n");
      if(hRes == DIERR_INPUTLOST) 
         DeviceAcquire(dev);
      return 0; 
   } 
   // Return Axis values for non NULL parameters
   // ------------------------------------------
   if(x)          *x    = js.lX; 
   if(y)          *y    = js.lY; 
   if(z)          *z    = js.lZ;
   if(r)          *r    = js.lRz;
   if(u)          *u    = js.rglSlider[0];
   if(v)          *v    = js.rglSlider[1];
   if(pov)        *pov  = js.rgdwPOV[0];
   // Return the buttons in a bit array
   // ---------------------------------
   if (buttons)
   {  *buttons = 0;        
      for (i=0; i<32; i++)
      {  if (js.rgbButtons[i] & 0x80)
            *buttons |= 1<<i;        
      }
   }
   return 1; 
} 
 

// -------------------------------------------------------------------
// -------------------------------------------------------------------
void
InputExit(void)
{
int i;

   if (DI)
   {
      // Release the Varmit
      // ------------------
      if (DID1_Rat)
      {
         if (mouseEventHandle)
            CloseHandle(mouseEventHandle);
         IDirectInputDevice_Unacquire(DID1_Rat);
         IDirectInputDevice_Release(DID1_Rat);
         DID1_Rat = NULL;
      }
      // Release the Keyboard
      // --------------------
      if (DID1_Key)
      {
         IDirectInputDevice_Unacquire(DID1_Key);
         IDirectInputDevice_Release(DID1_Key);
         DID1_Key = NULL;
      }
      // Release All Joystick Devices
      // ----------------------------
      for (i=0; i<kMaxJoy; i++)
      {
         if (DID2_Joy[i])
         {
            IDirectInputDevice2_Unacquire(DID2_Joy[i]);
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

//====================================================================
//                     Private Fuction Decals 
//====================================================================
// -------------------------------------------------------------------
// Return:
//   DIENUM_CONTINUE    continue calling us with any more devices
//   DIENUM_STOP        all done, don't call us back anymore, go away.
// -------------------------------------------------------------------
BOOL CALLBACK 
EnumJoy_Callback(LPDIDEVICEINSTANCE pdinst, LPVOID pvRef)
{
LPDIRECTINPUT        pdi   = pvRef;
LPDIRECTINPUTDEVICE  pdev;

   // Save Name
   // ---------
   strncpy(JoyInfo[numJoy].Name,pdinst->tszProductName,kMAX_Str);
   JoyInfo[numJoy].Name[kMAX_Str-1] = NULL;

   // Save Type
   // ---------
   switch (GET_DIDEVICE_SUBTYPE(pdinst->dwDevType))
   {
      case DIDEVTYPEJOYSTICK_WHEEL:
         JoyInfo[numJoy].DevType = kWheel;
         break;
      case DIDEVTYPEJOYSTICK_GAMEPAD:
         JoyInfo[numJoy].DevType = kGamePad;
         break;
      default:
         JoyInfo[numJoy].DevType = kStick;
         break;
   }

   // Create an instance of the device
   // --------------------------------   
   if (IDirectInput_CreateDevice 
       (pdi, &pdinst->guidInstance, &pdev, NULL) 
      != DI_OK) 
   { 
      bug("Could not create dinput device obj\n");
      return DIENUM_CONTINUE; 
   }

   // Set the data format to the default
   // ---------------------------------- 
   if (IDirectInputDevice_SetDataFormat 
       (pdev, &c_dfDIJoystick) 
      != DI_OK) 
   { 
      bug("Could not set dinput device data format\n");
      IDirectInputDevice_Release(pdev); 
      return DIENUM_CONTINUE; 
   }

   // Get the DID2 from DID1 
   // ----------------------
   if (IDirectInputDevice_QueryInterface
       (pdev, &IID_IDirectInputDevice2, (void **) &DID2_Joy[numJoy])
      != DI_OK)
   {
      bug("QueryInterface did not return DI_OK\n");
      IDirectInputDevice_Release(pdev);
      return DIENUM_CONTINUE; 
   }
   
   // Set the cooperative level 
   // -------------------------
   if (!Device_SetCoopLevel(numJoy, kJOY_COOP_FLAGS))
   {
      print("Could not set dinput mouse coop level\n");
      return FALSE;
   }

   // Done with Device1
   // -----------------
   IDirectInputDevice_Release(pdev);

   // Device was added successfully
   // -----------------------------
   ++numJoy;

   return DIENUM_CONTINUE;
}

// -------------------------------------------------------------------
//    set the device cooperative level.
//    this controls:
//       1. exclusive or shared access attrib.
//       2. forground or for/back ground use.
//
// -------------------------------------------------------------------
static int
Device_SetCoopLevel(tDevice dev, DWORD coop_level)
{
   // Set a single joystick
   // ---------------------
      if (DID2_Joy[dev])
      {
         if (IDirectInputDevice2_SetCooperativeLevel
             (DID2_Joy[dev], (HWND)getwindowhandle(), coop_level) 
            != DI_OK) 
            return(0); 
      }
      else
         return(0);
         
   return(1);
}
// -------------------------------------------------------------------
// Purpose:
//    Handle success/err reporting
// -------------------------------------------------------------------
static int
DeviceAcquireErr(HRESULT res, int dev_num)
{
   if (res == DI_OK)
   {
      print ("device #%d acquired\n",dev_num);
      return(TRUE);
   }
   else
   if (res == S_FALSE)
   {
      //print ("device #%d already acquired\n",dev_num);
      return(TRUE);
   }
   else
   if (res == DIERR_INVALIDPARAM)
   {
      abortmessage ("device %d DIERR_INVALIDPARAM\n",dev_num);
      return(FALSE);
   }
   else
   if (res == DIERR_OTHERAPPHASPRIO)
   {
      #if DEBUG
      abortmessage ("device %d DIERR_OTHERAPPHASPRIO\n",dev_num);
      #endif
      return(FALSE);
   }
   else
      abortmessage("Unknown Error acquiring device %d\n",dev_num);

   return(FALSE);
}
