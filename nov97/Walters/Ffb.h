//====================================================================
// Input.h
//========
// Purpose
//		This file defines a small layer of abstraction between the
//		game code and Direct Input.  
//
// Benefits:
//	  -	Group common items into easily identifiable structures.
//	  -	Reduce redundant steps that commonly cause of bugs.
//	  -	Simplify effect management.
//====================================================================
#ifndef _DXINPUT_H_
#define _DXINPUT_H_

#define kDI_MaxJoyButs	32
#define kDI_MaxJoyAxis	8

typedef enum
{
   kDI_Joy1=0,
   kDI_Joy2,
   kDI_Joy3,
   kDI_Joy4,
   kDI_Joy5,
   kDI_Joy6,
   kDI_Joy7,
   kDI_Joy8,
   kDI_Joy9,
   kDI_Joy10,
   kDI_Joy11,
   kDI_Joy12,
   kDI_Joy13,
   kDI_Joy14,
   kDI_Joy15,
   kDI_Joy16,
   kDI_MaxJoy,
	kDI_Mouse,
	kDI_KeyBoard,
	kDI_AllDevices
}tDI_Device;

typedef enum
{
	kNoButton=-1,
	kButton0=0,
	kButton1,
	kButton2,
	kButton3,
	kButton4,
	kButton5,
	kButton6,
	kButton7,
	kButton8,
	kButton9,
	kButton10,
	kButton31=31,
	kButtonMax

}tJoyButtons;

typedef enum 
{
   kConstant,
   kRamp,
   kCustom,

   kWave_Square,
   kWave_Sine,
   kWave_Triange,
   kWave_SawUp,
   kWave_SawDown,

   kCondition_Spring,
   kCondition_Damper,
   kCondition_Inertia,
   kCondition_Friction,

   kMaxEffectSubTypes

}tEffType;

typedef struct tEffConstant
{
	long  Mag;
}tEffConstant;

typedef struct tEffRamp
{
   long  Start;
   long  End;
}tEffRamp;

typedef struct tEffWave
{
	int 	Mag;
	long  Offset;
	int	Phase;
	int	Period;
}tEffWave;

typedef struct tEffCondition
{
   long  Offset;
   long  PositiveCoefficient;
   long  NegativeCoefficient;
   int   PositiveSaturation;
   int   NegativeSaturation;
   long  DeadBand;
}tEffCondition;

typedef struct tEffCustom
{
    int 	Channels;
    int	Period;
    int	Samples;
    long *ForceData;
}tEffCustom;

typedef union tEffectInfo
{
   tEffConstant   Constant;
   tEffRamp       Ramp;
   tEffWave       Wave;
   tEffCondition  Condition;
   tEffCustom     Custom;
}tEffInfo;

typedef struct tEffectEnvelope
{
   int 			AttackLevel; 
   int 			AttackTime; 
   int			FadeLevel; 
   int			FadeTime; 
}tEffEnvelope;

typedef struct tFFB_Effect
{
	tEffType			Type;
	tEffInfo			TypeInfo;
   int 				Duration;
   int 				Period;
   int 				Gain;
   tJoyButtons		Trigger;						
   int 				TriggerRepeatTime;
   long				Direction;
	tEffEnvelope	Envelope;
}tFFB_Effect;


// ===================================================================
//									Function Prototypes
// ===================================================================
// Init
// ----
int	ffb_Init(void);
void	Input_Init(void);
void	Input_Exit(void);

// Use
// ---
int	joy_Read(tDI_Device, int* buttons, int* x, int* y, int* z, int* r, int* u, int* v, int* pov); 

void	ffb_Pause(tDI_Device);
void	ffb_Continue(tDI_Device);

void	ffb_Enable(tDI_Device);
void	ffb_Disable(tDI_Device);

int	ffb_effectCreate(tDI_Device, tFFB_Effect*);
void	ffb_effectPlay(short effectID);
void	ffb_effectStop(short effectID);
void	ffb_effectStopAll(tDI_Device);
void	ffb_effectModify(short effectID, int*	Direction, int* Duration, int* Gain, 
								int* Period, tEffInfo* TypeInfo, tEffEnvelope* Envelope);

#endif // _DXINPUT_H_