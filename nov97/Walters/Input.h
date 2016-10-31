//====================================================================
// input.h
//===========
// Direct Input Force Feedback code
//====================================================================
#ifndef _INPUT_H_
#define _INPUT_H_

#define kMAX_Str		80
#define kMAX_Axis			8
#define kMAX_Sliders		2
#define kMAX_POVs			4
	
#define kInfinite_Duration 0xFFFFFF
#define HZ_to_uS(hz)		((int)(1000000.0/(double)(hz) + 0.5))

typedef signed char   	tSC;
typedef signed int    	tSI;
typedef signed long   	tSL;

typedef unsigned char 	tUC;
typedef unsigned int  	tUI;
typedef unsigned long 	tUL;

typedef enum
{
   kJoy1=0,
   kJoy2,
   kJoy3,
   kJoy4,
   kJoy5,
   kJoy6,
   kJoy7,
   kJoy8,
   kJoy9,
   kJoy10,
   kJoy11,
   kJoy12,
   kJoy13,
   kJoy14,
   kJoy15,
   kJoy16,
   kMaxJoy,
	kMouse,
	kKeyBoard,
	kAllDevices
}tDevice;

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
	kStick  = 1<<0,
	kWheel  = 1<<1,
	kGamePad= 1<<2,
}tDeviceMask;

typedef struct
{
	int 				  ButtonMask;
	int 				  AxisMask;
	tDeviceMask      DevType;
	char				  Name[kMAX_Str];
}tJoyInfo;

typedef struct
{
	int 	ButtonMask;

   tSL  	X_pos;
	tSL	Y_pos;
	tSL	Z_pos;

	tSL	X_rot;
	tSL	Y_rot;
	tSL	Z_rot;

	tSL	Slide_pos[kMAX_Sliders];
   tUL	POV_dir[kMAX_POVs];

}tJoyData;

typedef enum
{
	kDontPlayNow=0,
	kPlayNow,
	kPlayNowIfModified,

}tLoadEffect;

typedef enum 
{
   kConstant=0,
   kRamp,
   kCustom,

   kWave_Square,
   kWave_Sine,
   kWave_Triange,
   kWave_SawUp,
   kWave_SawDown,

   kCondition_Spring1D,
   kCondition_Spring2D,
   kCondition_Damper,
   kCondition_Inertia,
   kCondition_Friction,

   kMaxEffectSubTypes

}tEffType;

typedef struct tEffectConstant
{
	long  Mag;							// +- 10,000
}tEffConstant;

typedef struct tEffectRamp
{
   long  Start;
   long  End;
}tEffRamp;

typedef struct tEffectWave
{
	tUL	Mag;							// 0 to 10,000
	long  Offset;						// +- 10,000
	tUL	Phase;						// 0 to 35,999
	tUL	Period;						// uS
}tEffWave;

typedef struct tEffectCondition
{
   long  Offset;						// +- 10,000
   long  PositiveCoefficient;		// +- 10,000
   long  NegativeCoefficient;		// +- 10,000
   tUL 	PositiveSaturation;		// 0 to 10,000
   tUL 	NegativeSaturation;		// 0 to 10,000
   long  DeadBand;					// 0 to 10,000
}tEffCondition;

typedef struct tEffectCustom
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
   tUL  AttackLevel; 
   tUL  AttackTime; 
   tUL  FadeLevel; 
   tUL  FadeTime; 
}tEffEnvelope;

typedef enum
{
	kXAxisOnly,
	kYAxisOnly,
	kBothAxes
}tEffAxis;

typedef struct tFFB_Effect
{
	tEffType			Type;
	tEffInfo			TypeInfo[2];
   tUL				Duration;
   tUL				Gain;							// 0-10000 -- scales all magnitudes and envelope
	tEffAxis			Axis;
   tJoyButtons		Trigger;						
   tUL				TriggerRepeatTime;
   long				Direction;					// 0 to 360 deg.
	tEffEnvelope	Envelope;
}tFFB_Effect;


// ===================================================================
//									Function Prototypes
// ===================================================================
// General
// -------
void	InputInit(void);
void	InputExit(void);
int	DeviceAcquire(tDevice);
void	DeviceRelease(tDevice);

// Joy
// ---
int			JoyInit(void);
int			ReadJoy(tDevice, int* buttons, int* x, int* y, int* z, int* r, int* u, int* v, int* pov); 
tJoyInfo*   JoyQuery(tDevice dev, int* ButtMask, int* AxMask);

// Force
// -----
int	ForceInit(void);

void	ffb_Pause(tDevice);
void	ffb_Continue(tDevice);
void	ffb_Enable(tDevice);
void	ffb_Disable(tDevice);
void	ffb_SetGain(tDevice, int percentGain);
void	ffb_effectLoad(short eID);
void	ffb_effectUnload(short eID);
int	ffb_effectCreate(tDevice, tFFB_Effect*);
void	ffb_effectPlay(short effectID);
void	ffb_effectStop(short effectID);
void	ffb_effectStopAll(tDevice);
int	ffb_effectModify(short     eID,       tLoadEffect  PlayNow,
							  long*     Direction, tUL* 	   Duration, tUL*        Gain, 
							  tEffInfo* TypeInfo,  tJoyButtons* Trigger,  tEffEnvelope* Envelope);

#endif // _INPUT_H_