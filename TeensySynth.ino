#include <Audio.h>
// use bandlimit waveforms from https://github.com/MarkTillotson/Audio/tree/band_limited_waveforms
#define USE_BANDLIMIT_WAVEFORMS 1

// set SYNTH_DEBUG to enable debug logging (1=most,2=all messages)
#define SYNTH_DEBUG 0

#define USE_OLED 1
#ifdef USE_OLED
//////////////////////////
// oled - on esp8266 connect sda=d1 and scl=d2, connect 3.3v and ground
//			- on nano connect sda=a4 and scl=a5, connect 5v and ground
//			- on teensy connect sda=a4 and scl=a5, connect 5v and ground
//////////////////////////
#include <i2c_t3.h>
//#include <SPI.h>
//#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/TomThumb.h>
#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
unsigned long next_oled_update_ms=0;
#endif
// create these string vars regardless so we don't have to put so many ifdefs all over the place
// text updates immediately, text1-4 update periodically via the bottom of the loop.
String text;
String text1;
String text2;
String text3;
String text4;

#include <i2c_t3.h>

#include <EEPROM.h>

// define MIDI channel
#define SYNTH_MIDICHANNEL 1

// inital poly mode (POLY, MONO or PORTAMENTO)
#define SYNTH_INITIALMODE POLY

// define tuning of A4 in Hz
#define SYNTH_TUNING 440

// gain at oscillator/filter input stage (1:1)
// keep low so filter does not saturate with resonance
#define GAIN_OSC 0.7

// gain in final mixer stage for polyphonic mode (4:1)
// (0.25 is the safe value but larger sounds better :) )
//#define GAIN_POLY 1.
#define GAIN_POLY 0.45

// gain in final mixer stage for monophonic modes
//#define GAIN_MONO 1.
#define GAIN_MONO 0.45

// define delay lines for modulation effects
#define DELAY_LENGTH (16*AUDIO_BLOCK_SAMPLES)
short delaylineL[DELAY_LENGTH];
short delaylineR[DELAY_LENGTH];

// audio memory
#define AMEMORY 50

// switch between USB and UART MIDI
#define USB_MIDI 1
#ifdef USB_MIDI

#include "USBHost_t36.h"
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
MIDIDevice usbMIDI1(myusb);
MIDIDevice usbMIDI2(myusb);
#define SYNTH_SERIAL Serial

#else // 'real' MIDI via UART
#define SYNTH_SERIAL Serial
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
#endif

//////////////////////////////////////////////////////////////////////
// Data types and lookup tables
//////////////////////////////////////////////////////////////////////
struct Oscillator {
	AudioSynthWaveform*       wf;
	AudioSynthWaveform*       wf2;
	AudioFilterStateVariable* filt;
	AudioMixer4*              mix;
	AudioEffectEnvelope*      env;
	AudioEffectEnvelope*      env2;
	int8_t  note;
	uint8_t velocity;
};

// synth architecture in separate file
#include "SynthArch.h"

#define NVOICES 8
Oscillator oscs[NVOICES] = {
	{ &waveform1, &waveform9, &filter1, &mixer1, &envelope1,  &filt_env_1, -1, 0 },
	{ &waveform2, &waveform10, &filter2, &mixer2, &envelope2, &filt_env_2, -1, 0 },
	{ &waveform3, &waveform11, &filter3, &mixer3, &envelope3, &filt_env_3, -1, 0 },
	{ &waveform4, &waveform12, &filter4, &mixer4, &envelope4, &filt_env_4, -1, 0 },
	{ &waveform5, &waveform13, &filter5, &mixer5, &envelope5, &filt_env_5, -1, 0 },
	{ &waveform6, &waveform14, &filter6, &mixer6, &envelope6, &filt_env_6, -1, 0 },
	{ &waveform7, &waveform15, &filter7, &mixer7, &envelope7, &filt_env_7, -1, 0 },
	{ &waveform8, &waveform16, &filter8, &mixer8, &envelope8, &filt_env_8, -1, 0 },
};

#ifdef USE_BANDLIMIT_WAVEFORMS
#define COUNT_OF_WAVEFORMS 13
uint16_t waveforms[COUNT_OF_WAVEFORMS] = {
	WAVEFORM_SINE,
	WAVEFORM_SAWTOOTH,
	WAVEFORM_SQUARE,
	WAVEFORM_TRIANGLE,
	WAVEFORM_ARBITRARY,
	WAVEFORM_PULSE,
	WAVEFORM_SAWTOOTH_REVERSE,
	WAVEFORM_SAMPLE_HOLD,
	WAVEFORM_TRIANGLE_VARIABLE,
	WAVEFORM_BANDLIMIT_SAWTOOTH,
	WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE,
	WAVEFORM_BANDLIMIT_SQUARE,
	WAVEFORM_BANDLIMIT_PULSE,
};
#else
#define COUNT_OF_WAVEFORMS 8
uint8_t waveforms[COUNT_OF_WAVEFORMS] = {
	WAVEFORM_SINE,
	WAVEFORM_SQUARE,
	WAVEFORM_TRIANGLE,
	WAVEFORM_SAWTOOTH,
	WAVEFORM_SAWTOOTH_REVERSE,
	WAVEFORM_PULSE,
	WAVEFORM_SAMPLE_HOLD,
	WAVEFORM_ARBITRARY,
};
#endif

enum FilterModes {
	FILTERMODE_LOWPASS,
	FILTERMODE_BANDPASS,
	FILTERMODE_HIGHPASS,
	FILTERMODE_OFF,
	FILTERMODE_COUNT_OF_MODES
};
byte FILTERmode=FILTERMODE_LOWPASS;

//////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////
float   masterVolume   = 0.8;
#ifdef USE_BANDLIMIT_WAVEFORMS
uint16_t osc1_wf = WAVEFORM_BANDLIMIT_SAWTOOTH;
uint16_t osc2_wf = WAVEFORM_BANDLIMIT_SAWTOOTH;
# else
uint8_t osc1_wf = WAVEFORM_SAWTOOTH;
uint8_t osc2_wf = WAVEFORM_SAWTOOTH;
#endif

const float DIV127 = (1.0 / 127.0);

bool  polyOn;
bool  omniOn;
bool  velocityOn;

float layerWidth;
float layerMix1;
float layerMix2;

bool  sustainPressed;
float channelVolume;
float panorama;
float pulseWidth; // 0.05-0.95
float pitchBend;  // -1/+1 oct
float pitchScale;
int   octCorr;
int		transpose;

// LFO
float LFO = 0;
float LFOnoshape = 0;
unsigned int LFOspeed = 2000;
float LFOpitch = 1.;
float LFOdepth = 0.5;
float LFOsamplehold = 1.;
enum LFOmodes {
	LFO_MODE_OFF,
	LFO_MODE_UPDOWN,
	LFO_MODE_UP,
	LFO_MODE_DOWN,
	LFO_MODE_SAMPLE_HOLD,
	LFO_COUNT_OF_MODES
};
byte LFOmode = LFO_MODE_OFF;
enum LFOshapes {
	LFO_SINE,
	LFO_SQUARE,
	LFO_COUNT_OF_SHAPES
};
byte LFOshape = LFO_SINE;

// filter
float filtFreq; // 20-AUDIO_SAMPLE_RATE_EXACT/2.5
float filtReso; // 0.9-5.0
float filtAtt;  // 0-1
float filtOct;  // 0-7

// envelope
bool  envOn;
float envDelay;   // 0-11880
float envAttack;  // 0-11880
float envHold;    // 0-11880
float envDecay;   // 0-11880
float envSustain; // 0-1
float envRelease; // 0-11880

// envelope2
bool  env2On;
float env2Delay;   // 0-11880
float env2Attack;  // 0-11880
float env2Hold;    // 0-11880
float env2Decay;   // 0-11880
float env2Sustain; // 0-1
float env2Release; // 0-11880

// FX
/*
bool  flangerOn;
int   flangerOffset;
int   flangerDepth;
float flangerFreqCoarse;
float flangerFreqFine;
*/

// portamento
bool     portamentoOn;
uint32_t portamentoTime;
int8_t   portamentoDir;
float    portamentoStep;
float    portamentoPos;

#ifdef USE_OLED
//////////////////////////////////////////////////////////////////////
// OLED functions
//////////////////////////////////////////////////////////////////////
void showtext(int line_number, String text, boolean clearDisplay){
	// if debugging is enabled then it makes sense to send this new text to the debug window
#if SYNTH_DEBUG > 0
			SYNTH_SERIAL.println(text);
#endif

	if (clearDisplay) {
		display.clearDisplay();
		display.drawRect(0, 0, 128, 64, WHITE);

		display.drawRect(0, 0, 128, 22, WHITE);
		display.drawRect(0, 21,128, 22, WHITE);
		display.drawRect(0, 42,128, 23, WHITE);
		//display.drawRect(0, 42,128, 11, WHITE);
		//display.drawRect(0, 52,128, 12, WHITE);
	}

	int tx=2, ty=0;
	int rx=2, ry=0;
	int rh=21;
	if (line_number==1){
		display.setFont(&FreeSans9pt7b);
		ty=15;
		ry=1;
		rh=20;
	} else if (line_number==2){
		display.setFont(&FreeSans9pt7b);
		ty=37;
		ry=22;
		rh=20;
	} else if (line_number==3){
		display.setFont(&TomThumb);
		ty=51;
		ry=43;
		rh=9;
	} else if (line_number==4){
		display.setFont(&TomThumb);
		ty=61;
		ry=53;
		rh=10;
	} else {
		// just do two lines, but 3 would be y=52 if needed in the future
		return;
	}
	display.fillRect(1, ry, 126, rh, BLACK);
	display.setTextColor(WHITE);
	display.setCursor(tx, ty);
	display.print(text);
	display.display();
}
#endif

//////////////////////////////////////////////////////////////////////
// EEPROM storage variables and functions
//////////////////////////////////////////////////////////////////////
int patch_number=0;

// define patch variables here. new variables should be put at the end of PROGRAMvars, but before the count
enum PROGRAMvars {
	PRG_KEY_1,
	PRG_KEY_2,
	PRG_OSC1_TUNE,
	PRG_OSC2_TUNE,
	PRG_OSC_MIX,
	PRG_OSC1_WF,
	PRG_OSC2_WF,
	PRG_PULSE_WIDTH,
	PRG_PORT_SPEED,
	PRG_POLY_MODE,
	PRG_FILT_FREQ,
	PRG_FILT_RES,
	PRG_FILT_OCT,
	PRG_FILT_MODE,
	PRG_LFO_SPEED,
	PRG_LFO_DEPTH,
	PRG_LFO_SHAPE,
	PRG_LFO_MODE,
	PRG_LFO_DEST,
	PRG_ENV_A,
	PRG_ENV_D,
	PRG_ENV_S,
	PRG_ENV_R,
	PRG_FILT_ENV_A,
	PRG_FILT_ENV_D,
	PRG_FILT_ENV_S,
	PRG_FILT_ENV_R,
	PRG_MASTER_VOL,
	PRG_COUNT_OF_VARS
};

byte CC_PARAM_MAP[127]={};
// map MIDI control channels to parameter control here
void init_cc_param_map(){
	CC_PARAM_MAP[PRG_OSC1_TUNE]		= 0;
	CC_PARAM_MAP[PRG_OSC2_TUNE]		= 18;
	CC_PARAM_MAP[PRG_OSC_MIX]			= 15;
	CC_PARAM_MAP[PRG_OSC1_WF]			= 26;
	CC_PARAM_MAP[PRG_OSC2_WF]			= 36;
	CC_PARAM_MAP[PRG_PULSE_WIDTH]	= 0;
	CC_PARAM_MAP[PRG_PORT_SPEED]	= 13;
	CC_PARAM_MAP[PRG_POLY_MODE]		= 23;
	CC_PARAM_MAP[PRG_FILT_FREQ]		= 22;
	CC_PARAM_MAP[PRG_FILT_RES]		= 21;
	CC_PARAM_MAP[PRG_FILT_OCT]		= 20;
	CC_PARAM_MAP[PRG_FILT_MODE]		= 31;
	CC_PARAM_MAP[PRG_LFO_SPEED]		= 16;
	CC_PARAM_MAP[PRG_LFO_DEPTH]		= 17;
	CC_PARAM_MAP[PRG_LFO_SHAPE]		= 35;
	CC_PARAM_MAP[PRG_LFO_MODE]		= 25;
	CC_PARAM_MAP[PRG_LFO_DEST]		= 0;
	CC_PARAM_MAP[PRG_ENV_A]				= 2;
	CC_PARAM_MAP[PRG_ENV_D]				= 3;
	CC_PARAM_MAP[PRG_ENV_S]				= 4;
	CC_PARAM_MAP[PRG_ENV_R]				= 5;
	CC_PARAM_MAP[PRG_FILT_ENV_A]	= 6;
	CC_PARAM_MAP[PRG_FILT_ENV_D]	= 8;
	CC_PARAM_MAP[PRG_FILT_ENV_S]	= 9;
	CC_PARAM_MAP[PRG_FILT_ENV_R]	= 12;
	CC_PARAM_MAP[PRG_MASTER_VOL]	= 14;
}

// setup a key1/2 that will be used to verify whether or not a load from an eeprom 
// slot is probably a patch that was stored by this program
byte PROGRAM_KEY_1=123;
byte PROGRAM_KEY_2=45;
byte ProgramArr[50];
void init_program(){
	// initialize a default program
	for (uint8_t i=0; i<PRG_COUNT_OF_VARS; i++) {
		ProgramArr[PRG_KEY_1]=PROGRAM_KEY_1;
		ProgramArr[PRG_KEY_2]=PROGRAM_KEY_2;
		ProgramArr[PRG_OSC1_TUNE]=0;
		ProgramArr[PRG_OSC2_TUNE]=0;
		ProgramArr[PRG_OSC_MIX]=65;
		ProgramArr[PRG_OSC1_WF]=WAVEFORM_BANDLIMIT_SAWTOOTH;
		ProgramArr[PRG_OSC2_WF]=WAVEFORM_BANDLIMIT_SAWTOOTH;
		ProgramArr[PRG_PULSE_WIDTH]=65;
		ProgramArr[PRG_PORT_SPEED]=0;
		ProgramArr[PRG_POLY_MODE]=1;
		ProgramArr[PRG_FILT_FREQ]=50;
		ProgramArr[PRG_FILT_RES]=50;
		ProgramArr[PRG_FILT_OCT]=50;
		ProgramArr[PRG_FILT_MODE]=FILTERMODE_LOWPASS;
		ProgramArr[PRG_LFO_MODE]=LFO_MODE_OFF;
		ProgramArr[PRG_LFO_SPEED]=65;
		ProgramArr[PRG_LFO_DEPTH]=65;
		ProgramArr[PRG_LFO_SHAPE]=LFO_SINE;
		ProgramArr[PRG_LFO_DEST]=0;
		ProgramArr[PRG_ENV_A]=0;
		ProgramArr[PRG_ENV_D]=0;
		ProgramArr[PRG_ENV_S]=127;
		ProgramArr[PRG_ENV_R]=0;
		ProgramArr[PRG_FILT_ENV_A]=0;
		ProgramArr[PRG_FILT_ENV_D]=0;
		ProgramArr[PRG_FILT_ENV_S]=127;
		ProgramArr[PRG_FILT_ENV_R]=0;
		ProgramArr[PRG_MASTER_VOL]=127;
	}
	set_program();
}

void save_program(){
	if (patch_number>=0 && patch_number<50){
		ProgramArr[PRG_KEY_1]=PROGRAM_KEY_1;
		ProgramArr[PRG_KEY_2]=PROGRAM_KEY_2;

		// loop through each CC_PARAM_MAP and send the patch value for that variable for a CC change
		for (uint8_t i=0; i<50; i++) {
			long this_address=(50*patch_number) + i;
			byte this_value=ProgramArr[i];
			EEPROM.write(this_address, this_value);
		}
#if SYNTH_DEBUG > 0
			SYNTH_SERIAL.print("patch_number ");
			SYNTH_SERIAL.print(patch_number);
			SYNTH_SERIAL.println(" saved");
#endif
	} else {
#if SYNTH_DEBUG > 0
		SYNTH_SERIAL.print("save_program: cannot save, invalid patch_number: ");
		SYNTH_SERIAL.println(patch_number);
#endif
	}
}

void load_program(){
	if (patch_number>=0 && patch_number<50){
		ProgramArr[PRG_KEY_1]=PROGRAM_KEY_1;
		ProgramArr[PRG_KEY_2]=PROGRAM_KEY_2;

		bool load_key_matches=true;
		long this_address;
		byte this_value;

		// loop through each CC_PARAM_MAP and send the patch value for that variable for a CC change
		for (uint8_t i=0; i<50; i++) {
			this_address=(50*patch_number) + i;
			this_value;
			this_value=EEPROM.read(this_address);
			
			if (i==PRG_KEY_1){
				if (this_value!=PROGRAM_KEY_1){
					load_key_matches=false;
				}
			} else if (i==PRG_KEY_2){
				if (this_value!=PROGRAM_KEY_2){
					load_key_matches=false;
				}
			} else if (i>PRG_KEY_2){
				if (load_key_matches==false){
					// keys did not match, cannot load this patch
#if SYNTH_DEBUG > 0
					SYNTH_SERIAL.print("load_program: no key match for patch_number ");
					SYNTH_SERIAL.println(patch_number);
#endif
					break;
				} else {
					ProgramArr[i]=this_value;
				}
			}
		}
		set_program();

#if SYNTH_DEBUG > 0
		SYNTH_SERIAL.print("patch_number ");
		SYNTH_SERIAL.print(patch_number);
		SYNTH_SERIAL.println(" loaded");
#endif
#ifdef USE_OLED
	text=(String)"Preset " + patch_number;
	showtext(1, text, false);
#endif
	} else {
#if SYNTH_DEBUG > 0
		SYNTH_SERIAL.print("load_program: cannot load, invalid patch_number: ");
		SYNTH_SERIAL.println(patch_number);
#endif
	}
}

void set_program(){
	if (ProgramArr[PRG_KEY_1]!=PROGRAM_KEY_1 ||
			ProgramArr[PRG_KEY_2]!=PROGRAM_KEY_2){
		// does not seem to be a valid program
		return;
	}

	// loop through each CC_PARAM_MAP and send the patch value for that variable for a CC change
	for (uint8_t i=0; i<PRG_COUNT_OF_VARS; i++) {
		if (CC_PARAM_MAP[i]>0){
			// send MIDI channel=99 to tell the OnControlChange program that for latching values you just want to 
			// use whatever value is sent in, not expect 127 to cycle through values
			OnControlChange(99, CC_PARAM_MAP[i], ProgramArr[i]);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Handling of sounding and pressed notes
//////////////////////////////////////////////////////////////////////
int8_t notesOn[NVOICES]      = { -1, -1, -1, -1, -1, -1, -1, -1, };
int8_t notesPressed[NVOICES] = { -1, -1, -1, -1, -1, -1, -1, -1, };
int8_t lastVelocity; // used for retrigger in mono mode of previously-held keys

inline void notesReset(int8_t* notes) {
	memset(notes,-1,NVOICES*sizeof(int8_t));
}

inline void notesAdd(int8_t* notes, uint8_t note, uint8_t velocity) {
	int8_t *end=notes+NVOICES;
	int8_t noteNumber=0;
	do {
		if(*notes == -1) {
			*notes = note;
			lastVelocity=velocity;
			break;
		}
		noteNumber++;
	} while (++notes < end);
}

inline int8_t notesDel(int8_t* notes, uint8_t note) {
	int8_t lastNote = -1;
	int8_t *pos=notes, *end=notes+NVOICES;
	while (++pos < end && *(pos-1) != note);
	if (pos-1 != notes) lastNote = *(pos-2);
	while (pos < end) {
		*(pos-1) = *pos;
		if (*pos++ == -1) break;
	}
	if (*(end-1)==note || pos == end)
			*(end-1) = -1;
	return lastNote;
}

inline bool notesFind(int8_t* notes, uint8_t note) {
	int8_t *end=notes+NVOICES;
	do {
		if (*notes == note) return true;
	} while (++notes < end);
	return false;
}

//////////////////////////////////////////////////////////////////////
// Parameter control functions
//////////////////////////////////////////////////////////////////////
inline void updateFilterMode() {
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		// enable mixer input that corresponds to the lfo type, expects 3=unfiltered
		for (uint8_t fm=0; fm<FILTERMODE_COUNT_OF_MODES; ++fm) {
			if (fm == FILTERmode) o->mix->gain(fm,filtAtt);
			else                  o->mix->gain(fm,0);
		}
	} while (++o < end);
}

inline void updateFilter() {
	float this_filtFreq=filtFreq*LFO*(LFOsamplehold*LFOdepth);
	if (this_filtFreq>17000) this_filtFreq=17000;
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		o->filt->octaveControl(filtOct);
		o->filt->frequency(this_filtFreq);
		o->filt->resonance(filtReso);
	} while (++o < end);
}

inline void updateEnvelope() {
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		o->env->delay(envDelay);
		o->env->attack(envAttack);
		o->env->hold(envHold);
		o->env->decay(envDecay);
		o->env->sustain(envSustain);
		o->env->release(envRelease);
	} while (++o < end);
}

inline void updateEnvelope2() {
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		o->env2->delay(env2Delay);
		o->env2->attack(env2Attack);
		o->env2->hold(env2Hold);
		o->env2->decay(env2Decay);
		o->env2->sustain(env2Sustain);
		o->env2->release(env2Release);
	} while (++o < end);
}

inline void updateEnvelopeMode() {
	float env    = envOn ? 1 : 0;
	float noenv  = envOn ? 0 : 1;
	for (uint8_t i=0; i<2; ++i) {
		// env
		envmixer1.gain(i,env);
		envmixer2.gain(i,env);
		envmixer3.gain(i,env);
		envmixer4.gain(i,env);
		// no env
		envmixer1.gain(i+2,noenv);
		envmixer2.gain(i+2,noenv);
		envmixer3.gain(i+2,noenv);
		envmixer4.gain(i+2,noenv);
	}
}

void LFOupdate(bool retrig){
	if (LFOmode==LFO_MODE_OFF) {
		LFO=1.;
		LFOdepth=1.;
		return;
	}
  static unsigned long LFOtime = 0;
  static bool LFOdirection = false;
  unsigned long currentMicros = micros();
  static bool retriggered = false;

  if (retrig == true) {
		retriggered = true;
		if (LFOmode==LFO_MODE_UP){
			// start
			// UP really means go 0->1, 0->1...
			LFO=0.;
			LFOdirection=false;
		} else {
			// everything other than UP really means go 1->0->1->0...
			LFO=1.;
			LFOdirection=true;
		}
		LFOnoshape=LFO;
	}

  if (currentMicros - LFOtime >= LFOspeed || retrig==true) {
    LFOtime = currentMicros;

    retriggered = false;

		// generate new random depth for sample&hold
		bool gen_rand_depth=false;

    // Update LFO
    if (LFOdirection == false) {
			// going up
      LFOnoshape = (LFOnoshape + 0.01);
      if (LFOnoshape >= 1.) {
				// top reached
				if (LFOmode==LFO_MODE_SAMPLE_HOLD){
					gen_rand_depth=true;
				}
				
				if (LFOmode==LFO_MODE_UP){
					// UP really means go 0->1, 0->1
					LFOdirection = false;
					LFOnoshape=0.;
				} else {
					LFOdirection = true;
					LFOnoshape=1.;
				}
      }
    } else {
			// going down
      LFOnoshape = (LFOnoshape - 0.01);
      if (LFOnoshape <= 0) {
				// bottom reached
				if (LFOmode==LFO_MODE_SAMPLE_HOLD){
					gen_rand_depth=true;
				}
				if (LFOmode==LFO_MODE_DOWN){
        	LFOdirection = true;
					LFOnoshape=1.;
				} else {
        	LFOdirection = false;
					LFOnoshape=0.;
				}
      }
    }

		if (LFOmode==LFO_MODE_SAMPLE_HOLD){
			if (gen_rand_depth){
				// generate a random depth for sample&hold
				long randDepth=random(127);
				LFOsamplehold = (1. * randDepth)/127.;
			}
		} else {
			LFOsamplehold = 1.;
		}

		if (LFOshape==LFO_SQUARE || LFOmode==LFO_MODE_SAMPLE_HOLD){
			// always use square when going to sample&hold
			if (LFOmode==LFO_MODE_SAMPLE_HOLD){
				// always keep LFO on for sample&hold, the LFOsamplehold will be what changes
				LFO=1.;
			} else {
				if (LFOnoshape>=0.5){
					LFO=1.;
				} else {
					LFO=0.;
				}
			}
		} else {
			LFO=LFOnoshape;
		}

#if SYNTH_DEBUG > 3
		SYNTH_SERIAL.print("LFO=");
		SYNTH_SERIAL.print(LFO);
		SYNTH_SERIAL.print("  LFOSPD=");
		SYNTH_SERIAL.print(LFOspeed);
		SYNTH_SERIAL.print("  depth=");
		SYNTH_SERIAL.println(LFOdepth);
#endif
		updateFilter();
  }
}

/*
void updateFlanger() {
	if (flangerOn) {
		AudioNoInterrupts();
		flangerL.voices(flangerOffset,flangerDepth,flangerFreqCoarse+flangerFreqFine);
		flangerR.voices(flangerOffset,flangerDepth,flangerFreqCoarse+flangerFreqFine);
		AudioInterrupts();
	} else {
		flangerL.voices(FLANGE_DELAY_PASSTHRU,0,0);
		flangerR.voices(FLANGE_DELAY_PASSTHRU,0,0);        
	}
}
*/

void resetAll() {
	polyOn     = true;
	omniOn     = true;
	velocityOn = true;

	layerWidth = 1./127.;
	layerMix1	 = 1.;
	layerMix2	 = 1.;

	FILTERmode	 	 = FILTERMODE_LOWPASS;
	sustainPressed = false;
	channelVolume  = 1.0;
	panorama       = 0.5;
	pulseWidth     = 0.5;
	pitchBend      = 0;
	pitchScale     = 1;
	octCorr        = osc1_wf == WAVEFORM_PULSE ? 1 : 0;
	transpose			 = 0; 

	// filter
	filtFreq = 17000.;
	filtReso = 1.11;
	filtAtt  = 1.;
	filtOct  = 2.5;

	// envelope
	envOn      = true;
	envDelay   = 0;
	envAttack  = 0;
	envHold    = 0;
	envDecay   = 0;
	envSustain = 1;
	envRelease = 0;

	// dc, used for filter envelope
	dc1.amplitude(1);
	dc2.amplitude(1);
	dc3.amplitude(1);
	dc4.amplitude(1);
	dc5.amplitude(1);
	dc6.amplitude(1);
	dc7.amplitude(1);
	dc8.amplitude(1);

	// filter envelope 
	env2On      = true;
	env2Delay   = 0;
	env2Attack  = 0;
	env2Hold    = 0;
	env2Decay   = 0;
	env2Sustain = 1;
	env2Release = 0;

	/*
	// FX
	flangerOn         = false;
	flangerOffset     = DELAY_LENGTH/4;
	flangerDepth      = DELAY_LENGTH/16;
	flangerFreqCoarse = 0;
	flangerFreqFine   = .5;
	*/

	// portamento
	portamentoOn   = false;
	portamentoTime = 1000;
	portamentoDir  = 0;
	portamentoStep = 0;
	portamentoPos  = -1;

	updatePolyMode();
	updateFilterMode();
	updateEnvelope();
	updateEnvelope2();
	updatePan();
}

inline void updateWaveform() {
	if (osc1_wf==WAVEFORM_PULSE) octCorr = 1;
	else octCorr = 0;
		
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		if (osc1_wf==WAVEFORM_PULSE || osc1_wf==WAVEFORM_BANDLIMIT_PULSE){
			o->wf->pulseWidth(pulseWidth);
		}
		if (osc2_wf==WAVEFORM_PULSE || osc2_wf==WAVEFORM_BANDLIMIT_PULSE){
			o->wf2->pulseWidth(pulseWidth);
		}
		o->wf->begin(waveforms[osc1_wf]);
		o->wf2->begin(waveforms[osc2_wf]);
	} while(++o < end);
}

inline void updatePulseWidth() {
	if (osc1_wf==WAVEFORM_PULSE || osc1_wf==WAVEFORM_BANDLIMIT_PULSE) {
		Oscillator *o=oscs,*end=oscs+NVOICES;
		do {
			if (o->note < 0) continue;
			o->wf->pulseWidth(pulseWidth);
		} while(++o < end);
	}
	if (osc2_wf==WAVEFORM_PULSE || osc2_wf==WAVEFORM_BANDLIMIT_PULSE) {
		Oscillator *o=oscs,*end=oscs+NVOICES;
		do {
			if (o->note < 0) continue;
			o->wf2->pulseWidth(pulseWidth);
		} while(++o < end);
	}
	return;
}

inline void updatePitch() {
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		if (o->note < 0) continue;
		o->wf->frequency(noteToFreq(o->note, false));
		o->wf2->frequency(noteToFreq(o->note, true));
	} while(++o < end);
}

inline void updateLayer() {
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		if (o->note < 0) continue;
		o->wf->frequency(noteToFreq(o->note, false));
		o->wf2->frequency(noteToFreq(o->note, true));
	} while(++o < end);
}

inline void updateVolume() {
	Oscillator *o=oscs,*end=oscs+NVOICES;
	float velocity;
	do {
		if (o->note < 0) continue;
		velocity = velocityOn ? o->velocity/127. : 1;
		o->wf->amplitude(velocity*channelVolume*GAIN_OSC*layerMix1);
		o->wf2->amplitude(velocity*channelVolume*GAIN_OSC*layerMix2);
	} while(++o < end);
}

inline void updatePan() {
	float norm  = (polyOn && !portamentoOn) ? GAIN_POLY : GAIN_MONO;
	float left=norm, right=norm;
	if (panorama < 0.5) right *= 2*panorama;
	else left *= 2*(1-panorama);

	for (uint8_t i=0; i<4; ++i) {
		mixerL.gain(i,left);
		mixerR.gain(i,right);
	}
}

inline void updateMasterVolume() {
	// read the volume knob
	float vol = (float) analogRead(A1) / 1280.0;
	if( fabs(vol-masterVolume) > 0.01) {
		masterVolume = vol;
		sgtl5000_1.volume(masterVolume);
#if SYNTH_DEBUG > 0
		SYNTH_SERIAL.print("Volume: ");
		SYNTH_SERIAL.println(vol);
#endif
	}
}

inline void updatePolyMode() {
	allOff();
	updateEnvelopeMode();
	updatePan();
}

inline void updatePortamento() {
	if (portamentoDir == 0) return;
	if (oscs->note < 0) {
		portamentoDir = 0;
		return;
	}
	if (portamentoDir < 0) {
		portamentoPos -= portamentoStep;
		if (portamentoPos < oscs->note) {
			portamentoPos = oscs->note;
			portamentoDir = 0;
		}
	} else {
		portamentoPos += portamentoStep;
		if (portamentoPos > oscs->note) {
			portamentoPos = oscs->note;
			portamentoDir = 0;
		}
	}
	oscs->wf->frequency(noteToFreq(portamentoPos, false));
	oscs->wf2->frequency(noteToFreq(portamentoPos, true));
}

//////////////////////////////////////////////////////////////////////
// Oscillator control functions
//////////////////////////////////////////////////////////////////////
inline float noteToFreq(float note, bool useLayer) {
	note=note+transpose;
	// Sets all notes as an offset of A4 (#69)
	if (portamentoOn) note = portamentoPos;
	float thisFreq=SYNTH_TUNING*pow(2,(note - 69)/12.+pitchBend/pitchScale+octCorr);
	if (useLayer==true){
		thisFreq=thisFreq*(1+layerWidth);
	} else {
		thisFreq=SYNTH_TUNING*pow(2,(note - 69)/12.+pitchBend/pitchScale+octCorr);
	}
	return thisFreq;
}

inline void oscOn(Oscillator& osc, int8_t note, uint8_t velocity) {
	float v = velocityOn ? velocity/127. : 1;
	if (osc.note!=note) {
		osc.wf->frequency(noteToFreq(note, false));
		osc.wf2->frequency(noteToFreq(note, true));
		notesAdd(notesOn,note,velocity);
		if (envOn && !osc.velocity) osc.env->noteOn();
		if (env2On) osc.env2->noteOn();
		osc.wf->amplitude(v*channelVolume*GAIN_OSC*layerMix1);
		osc.wf2->amplitude(v*channelVolume*GAIN_OSC*layerMix2);
		osc.velocity = velocity;
		osc.note = note;
	} else if (velocity > osc.velocity) {
		osc.wf->amplitude(v*channelVolume*GAIN_OSC*layerMix1);
		osc.wf2->amplitude(v*channelVolume*GAIN_OSC*layerMix2);
		osc.velocity = velocity;
	}
}

inline void oscOff(Oscillator& osc) {
	if (envOn) {
		osc.env->noteOff();
	} else {
		osc.wf->amplitude(0);
		osc.wf2->amplitude(0);
	}
	if (env2On) osc.env2->noteOff();
	notesDel(notesOn,osc.note);
	osc.note = -1;
	osc.velocity = 0;
}

inline void allOff() {
	Oscillator *o=oscs,*end=oscs+NVOICES;
	do {
		oscOff(*o);
		o->wf->amplitude(0);
		o->wf2->amplitude(0);
		o->env->noteOff();
		o->env2->noteOff();
	} while(++o < end);
	notesReset(notesOn);
}

//////////////////////////////////////////////////////////////////////
// MIDI handlers
//////////////////////////////////////////////////////////////////////
void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
	if (!omniOn && channel != SYNTH_MIDICHANNEL) return;

#if SYNTH_DEBUG > 1
	SYNTH_SERIAL.println("NoteOn");
#endif

	notesAdd(notesPressed,note,velocity);
	LFOupdate(true);

	Oscillator *o=oscs;
	if (portamentoOn) {
		if (portamentoTime == 0 || portamentoPos < 0) {
			portamentoPos = note;
			portamentoDir = 0;
		} else if (portamentoPos > -1) {
			portamentoDir  = note > portamentoPos ? 1 : -1;
			portamentoStep = fabs(note-portamentoPos)/(portamentoTime);
		}
		*notesOn = -1;
		oscOn(*o, note, velocity);
	} else if (polyOn) {
		Oscillator *curOsc=0, *end=oscs+NVOICES;
		if (sustainPressed && notesFind(notesOn,note)) {
			do {
				if (o->note == note) {
					// need to retrigger a sustained note. turn off and reuse this osc below when it retriggers.
					curOsc = OnNoteOffReal(channel,note,velocity,true);
					break;
				}
			} while (++o < end);
		}
		for (o=oscs; o < end && !curOsc; ++o) {
			if (o->note < 0) {
				curOsc = o;
				break;
			}
		}
		if (!curOsc && *notesOn != -1) {
#if SYNTH_DEBUG > 0
			SYNTH_SERIAL.println("Stealing voice");
#endif
			curOsc = OnNoteOffReal(channel,*notesOn,velocity,true);
		}
		if (!curOsc) return;
		oscOn(*curOsc, note, velocity);
	} else {
		*notesOn = -1;
		oscOn(*o, note, velocity);
	}

	return;
}

Oscillator* OnNoteOffReal(uint8_t channel, uint8_t note, uint8_t velocity, bool ignoreSustain) {
	if (!omniOn && channel != SYNTH_MIDICHANNEL) return 0;

#if SYNTH_DEBUG>1
	SYNTH_SERIAL.println("NoteOff");
#endif
	int8_t lastNote = notesDel(notesPressed,note);

	// note off velocity is used later in mono mode to retrigger a still-held note. the velocity of the
	// last note played seems appropriate for the velocity of that retrigger, rather than whatever velocity
	// it was originally played with. that way crescendo and decrescendo can be done with trills
	velocity=lastVelocity;
	
	if (sustainPressed && !ignoreSustain) return 0;

	Oscillator *o=oscs;
	if (portamentoOn) {
		if (o->note == note) {
			if (lastNote != -1) {
				notesDel(notesOn,note);
				if (portamentoTime == 0) {
					portamentoPos = lastNote;
					portamentoDir = 0;
				} else {
					portamentoDir = lastNote > portamentoPos? 1 : -1;
					portamentoStep = fabs(lastNote-portamentoPos)/(portamentoTime);
				}
				oscOn(*o, lastNote, velocity);
			}
			else 
			{
				oscOff(*o);
				portamentoPos = -1;
				portamentoDir = 0;
			}
		}
		if (oscs->note == note) {
			if (lastNote != -1) {
				notesDel(notesOn,o->note);
				oscOn(*o, lastNote, velocity);
			} else {
				oscOff(*o);
			}
		}
	}
	else if (polyOn) {
		Oscillator *end=oscs+NVOICES;
		do {
			if (o->note == note) break;
		} while (++o < end);
		if (o == end) return 0;
		oscOff(*o);
	} else {
		if (oscs->note == note) {
			if (lastNote != -1) {
				notesDel(notesOn,o->note);
				oscOn(*o, lastNote, velocity);
			} else {
				oscOff(*o);
			}
		}
	}
	
	return o;
}

inline void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
	OnNoteOffReal(channel,note,velocity,false);
}

void OnAfterTouchPoly(uint8_t channel, uint8_t note, uint8_t value) {
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("AfterTouchPoly: channel ");
	SYNTH_SERIAL.print(channel);
	SYNTH_SERIAL.print(", note ");
	SYNTH_SERIAL.print(note);
	SYNTH_SERIAL.print(", value ");
	SYNTH_SERIAL.println(value);
#endif
}

void OnControlChange(uint8_t channel, uint8_t control, uint8_t value) {
	// channel=99 is an indicator that the latching controllers should just set their value to value, not cycle through
	// the various values for that parameter
	if (!omniOn && channel != SYNTH_MIDICHANNEL && channel!=99) return;

	// loop through params, see if this control channel controls a particular param
	for (uint8_t c=0; c<PRG_COUNT_OF_VARS; ++c) {

		// layer width
		if (c==PRG_OSC2_TUNE && control==CC_PARAM_MAP[PRG_OSC2_TUNE]){
			ProgramArr[c]=value;
			layerWidth = ((1+value)/127.);
			updateLayer();
			text2=(String)"O2 Tune = " + value;
		}

		// env attack
		if (c==PRG_ENV_A && control==CC_PARAM_MAP[PRG_ENV_A]){
			ProgramArr[c]=value;
			envAttack = value*11880./127.;
			updateEnvelope();
			text2=(String)"Env A = " + value;
		}

		// env decay
		if (c==PRG_ENV_D && control==CC_PARAM_MAP[PRG_ENV_D]){
			ProgramArr[c]=value;
			envDecay = value*11880./127.;
			updateEnvelope();
			text2=(String)"Env D = " + value;
		}

		if (c==PRG_ENV_S && control==CC_PARAM_MAP[PRG_ENV_S]){
			// sustain
			ProgramArr[c]=value;
			envSustain = value/127.;
			updateEnvelope();
			text2=(String)"Env S = " + value;
		}

		if (c==PRG_ENV_R && control==CC_PARAM_MAP[PRG_ENV_R]){
			// release
			ProgramArr[c]=value;
			envRelease = value*11880./127.;
			updateEnvelope();
			text2=(String)"Env R = " + value;
		}

		if (c==PRG_FILT_ENV_A && control==CC_PARAM_MAP[PRG_FILT_ENV_A]){
			// filt attack
			ProgramArr[c]=value;
			env2Attack = value*5000./127.;
			updateEnvelope2();
			text2=(String)"FEnv A = " + value;
		}

		if (c==PRG_FILT_ENV_D && control==CC_PARAM_MAP[PRG_FILT_ENV_D]){
			// filt decay
			ProgramArr[c]=value;
			env2Decay = value*5000./127.;
			updateEnvelope2();
			text2=(String)"FEnv D = " + value;
		}

		if (c==PRG_FILT_ENV_S && control==CC_PARAM_MAP[PRG_FILT_ENV_S]){
			// filt sustain
			ProgramArr[c]=value;
			env2Sustain = value/127.;
			updateEnvelope2();
			text2=(String)"FEnv S = " + value;
		}

		if (c==PRG_FILT_ENV_R && control==CC_PARAM_MAP[PRG_FILT_ENV_R]){
			// filt release
			ProgramArr[c]=value;
			env2Release = value*5000./127.;
			updateEnvelope2();
			text2=(String)"FEnv R = " + value;
		}

		if (c==PRG_PORT_SPEED && control==CC_PARAM_MAP[PRG_PORT_SPEED]){
			// portamento time
			ProgramArr[c]=value;
			float portamentoRange = portamentoStep*portamentoTime;
			portamentoTime = value*value*value;
			portamentoStep = portamentoRange/portamentoTime;
			text2=(String)"Port = " + value;
		}

		if (c==PRG_MASTER_VOL && control==CC_PARAM_MAP[PRG_MASTER_VOL]){
			// volume
			ProgramArr[c]=value;
			channelVolume = value/127.;
			updateVolume();
			text2=(String)"Vol = " + value;
		}

		if (c==PRG_OSC_MIX && control==CC_PARAM_MAP[PRG_OSC_MIX]){
			// layer mix
			ProgramArr[c]=value;
			if (value>63) {
				layerMix1 = map((1.0*value), 64., 127., 1., 0.);
				layerMix2 = 1.;
			} else {
				layerMix1 = 1.;
				layerMix2 = map((1.0*value), 63., 0., 1., 0.);
			}
			updateVolume();
			text2=(String)"Osc Mix = " + value;
		}

		if (c==PRG_LFO_SPEED && control==CC_PARAM_MAP[PRG_LFO_SPEED]){
			// LFO1 speed
			ProgramArr[c]=value;
			float xSpeed = 1.-(value / 127.);
			xSpeed = pow(100, (xSpeed - 1));
			LFOspeed = (70000 * xSpeed)-500;
			text2=(String)"LFO Sp = " + value;
		}

		if (c==PRG_LFO_DEPTH && control==CC_PARAM_MAP[PRG_LFO_DEPTH]){
			// LFO1 depth
			ProgramArr[c]=value;
			float xDepth = value / 127.;
			LFOdepth = xDepth;
			text2=(String)"LFO Dp = " + value;
		}

		if (c==PRG_FILT_OCT && control==CC_PARAM_MAP[PRG_FILT_OCT]){
			// filter octaves
			ProgramArr[c]=value;
			filtOct = 7.*value/127.;
			updateFilter();
			text2=(String)"F Oct = " + value;
		}

		if (c==PRG_FILT_RES && control==CC_PARAM_MAP[PRG_FILT_RES]){
			// filter resonance
			//filtReso = value*4.1/127.+0.9;
			ProgramArr[c]=value;
			filtReso = (1.0*value)/127;
			filtReso=map(filtReso, 0, 1, 1.11, 5.0);
			updateFilter();
			text2=(String)"F Res = " + value;
		}

		if (c==PRG_FILT_FREQ && control==CC_PARAM_MAP[PRG_FILT_FREQ]){
			// filter frequency
			//filtFreq = value/2.5*AUDIO_SAMPLE_RATE_EXACT/127.;
			ProgramArr[c]=value;
			filtFreq = (1.0*value)/127;
			filtFreq=pow(filtFreq, 3)*17000;
			updateFilter();
			text2=(String)"F Freq = " + int(filtFreq);
		}

		if (c==PRG_LFO_MODE && control==CC_PARAM_MAP[PRG_LFO_MODE]){
			// LFO mode
			if (channel==99){
				// loading a specific value, assuming 127 only is used when latch button is pressed on midi controller
				LFOmode=value;
			} else if (value>=127){
				LFOmode++;
				if (LFOmode>=LFO_COUNT_OF_MODES) LFOmode=0;
				ProgramArr[c]=LFOmode;
				text2=(String)"LFO = " + LFOmode;
			}
		}

		if (c==PRG_OSC1_WF && control==CC_PARAM_MAP[PRG_OSC1_WF]){
			// osc1 waveform
			if (channel==99){
				// loading a specific value, assuming 127 only is used when latch button is pressed on midi controller
				osc1_wf=value;
				updateWaveform();
			} else if (value>=127){
				osc1_wf++;
				if (osc1_wf>=COUNT_OF_WAVEFORMS) osc1_wf=0;
				ProgramArr[c]=osc1_wf;
				updateWaveform();
				text2=(String)"WF1 = " + osc1_wf;
			}
		}

		if (c==PRG_OSC2_WF && control==CC_PARAM_MAP[PRG_OSC2_WF]){
			// osc2 waveform
			if (channel==99){
				// loading a specific value, assuming 127 only is used when latch button is pressed on midi controller
				osc2_wf=value;
				updateWaveform();
			} else if (value>=127){
				osc2_wf++;
				if (osc2_wf>=COUNT_OF_WAVEFORMS) osc2_wf=0;
				ProgramArr[c]=osc2_wf;
				updateWaveform();
				text2=(String)"WF2 = " + osc2_wf;
			}
		}

		if (c==PRG_FILT_MODE && control==CC_PARAM_MAP[PRG_FILT_MODE]){
			// filter mode
			if (channel==99){
				// loading a specific value, assuming 127 only is used when latch button is pressed on midi controller
				FILTERmode=value;
				updateFilterMode();
			} else if (value>=127){
				FILTERmode++;
				if (FILTERmode >= FILTERMODE_COUNT_OF_MODES) FILTERmode=0;
				ProgramArr[c]=FILTERmode;
				updateFilterMode();
				text2=(String)"F Mode = " + FILTERmode;
			}
		}

		if (c==PRG_LFO_SHAPE && control==CC_PARAM_MAP[PRG_LFO_SHAPE]){
			// LFO shape
			if (channel==99){
				// loading a specific value, assuming 127 only is used when latch button is pressed on midi controller
				LFOshape=value;
			} else if (value>=127){
				LFOshape++;
				if (LFOshape>=LFO_COUNT_OF_SHAPES) LFOshape=0;
				ProgramArr[c]=LFOshape;
				text2=(String)"LFO Shape = " + LFOshape;
			}
		}

		if (c==PRG_POLY_MODE && control==CC_PARAM_MAP[PRG_POLY_MODE]){
			// poly mode
			if (channel==99){
				// loading a specific value, assuming 127 only is used when latch button is pressed on midi controller
				polyOn=value;
				updatePolyMode();
			} else if (value>=127){
				if (polyOn){
					// mono mode
					// turn off poly mode
					polyOn=false;
					portamentoOn=true;
					updatePolyMode();
					text2=(String)"Poly Mode Off";
				} else {
					// turn on poly mode
					polyOn=true;
					portamentoOn=false;
					updatePolyMode();
					text2=(String)"Poly Mode On";
				}
				ProgramArr[c]=polyOn;
			}
		}
	}

	if (control==45){
		if (value >= 127){
			// load patch
			load_program();
		}
	} else if (control==44){
		if (value >= 127){
			// save patch
			save_program();
		}
	} else if (control==47){
		if (value >= 127){
			// previous patch
			// next patch
			patch_number--;
			if (patch_number<0) patch_number=0;
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("Next patch: ");
	SYNTH_SERIAL.println(patch_number);
#endif
			text3=(String)"Patch " + patch_number;
		}
	} else if (control==48){
		if (value >= 127){
			// next patch
			patch_number++;
			if (patch_number>=50) patch_number=50;
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("Next patch: ");
	SYNTH_SERIAL.println(patch_number);
#endif
			text3=(String)"Patch " + patch_number;
		}
	} else if (control==49){
		if (value >= 127){
			init_program();
			text1=(String)"Init Preset";
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.println("init program:");
#endif
			for (uint8_t i=0; i<PRG_COUNT_OF_VARS; i++) {
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("ProgramArr[");
	SYNTH_SERIAL.print(i);
	SYNTH_SERIAL.print("]=");
	SYNTH_SERIAL.println(ProgramArr[i]);
#endif
			}
		}
	} else if (control==64){
		// sustain/damper pedal
		if (value > 63) sustainPressed = true;
		else {
			sustainPressed = false;
			Oscillator *o=oscs, *end=oscs+NVOICES;
			do {
				if (o->note != -1 && !notesFind(notesPressed,o->note)) oscOff(*o);
			} while (++o < end);
		}
	}

	/*
	case x: // pulse width
		pulseWidth = (value/127.)*0.9+0.05;
		updatePulseWidth();
		break;
	case x: // controller reset
		resetAll();
		break;
	case x: // all notes off
		allOff();
		break;
	case x: // omni off
		allOff();
		omniOn = false;
		break;
	case x: // omni on
		allOff();
		omniOn = true;
		break;
	case x: // filter attenuation
		filtAtt = value/127.;
		updateFilterMode();
		break;
	case x: // fix volume
		switch (value) {
		case 0:
			velocityOn = false;
			break;
		case 1:
			velocityOn = true;
			break;
		default:
			velocityOn = !velocityOn;
			break;
		}
		break;
	case x: // pan
		panorama = value/127.;
		updatePan();
		break;
	case x: // envelope mode
		allOff();
		envOn = !envOn;
		updateEnvelopeMode();
		break;
	case x: // flanger toggle
		if (value < 2)
				flangerOn = bool(value);
		else
				flangerOn = !flangerOn;
		updateFlanger();
		break;
	case x: // flanger offset
		flangerOffset = int(value/127.*8)*DELAY_LENGTH/8;
		updateFlanger();
		break;
	case x: // flanger depth
		flangerDepth = int(value/127.*8)*DELAY_LENGTH/8;
		updateFlanger();
		break;
	case x: // flanger coarse frequency
		break;
		flangerFreqCoarse = value/127.*10.;
		updateFlanger();
		break;
	case x: // flanger fine frequency
		flangerFreqFine = value/127.;
		updateFlanger();
		break;
	case x: // pitch range in semitones
		pitchScale = 12./value;
		break;
	*/

#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("Control Change: channel ");
	SYNTH_SERIAL.print(channel);
	SYNTH_SERIAL.print(", control ");
	SYNTH_SERIAL.print(control);
	SYNTH_SERIAL.print(", value ");
	SYNTH_SERIAL.println(value);
#endif
	text4=(String)"CC " + control + " = " + value;;
}

void OnPitchChange(uint8_t channel, int pitch) {
	if (!omniOn && channel != SYNTH_MIDICHANNEL) return;

#if SYNTH_DEBUG > 1
	SYNTH_SERIAL.print("PitchChange: channel ");
	SYNTH_SERIAL.print(channel);
	SYNTH_SERIAL.print(", pitch ");
	SYNTH_SERIAL.println(pitch);
#endif

#ifdef USB_MIDI
	if (pitch == 8192)
		pitchBend = 0;
	else
		pitchBend = (pitch-8192)/8192.;
#else
	pitchBend = pitch/8192.;
#endif
	
	updatePitch();
}

void OnProgramChange(uint8_t channel, uint8_t program) {
	if (!omniOn && channel!=SYNTH_MIDICHANNEL) return;

#if SYNTH_DEBUG > 1
	SYNTH_SERIAL.print("ProgramChange: channel ");
	SYNTH_SERIAL.print(channel);
	SYNTH_SERIAL.print(", program ");
	SYNTH_SERIAL.println(program);
#endif
}

void OnAfterTouch(uint8_t channel, uint8_t pressure) {
	if (!omniOn && channel!=SYNTH_MIDICHANNEL) return;

#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("AfterTouch: channel ");
	SYNTH_SERIAL.print(channel);
	SYNTH_SERIAL.print(", pressure ");
	SYNTH_SERIAL.println(pressure);
#endif
}

void OnSysEx( const uint8_t *data, uint16_t length, bool complete) {
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("SysEx: length ");
	SYNTH_SERIAL.print(length);
	SYNTH_SERIAL.print(", complete ");
	SYNTH_SERIAL.println(complete);
#endif
}

void OnRealTimeSystem(uint8_t realtimebyte) {
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("RealTimeSystem: ");
	SYNTH_SERIAL.println(realtimebyte);
#endif
}

/*
void OnTimeCodeQFrame(uint16_t data)
{
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.print("TimeCodeQuarterFrame: ");
	SYNTH_SERIAL.println(data);
#endif
}
*/
				
//////////////////////////////////////////////////////////////////////
// Debugging functions
//////////////////////////////////////////////////////////////////////
#if SYNTH_DEBUG > 0
float   statsCpu = 0;
uint8_t statsMem = 0;

void oscDump(uint8_t idx) {
	SYNTH_SERIAL.print("Oscillator ");
	SYNTH_SERIAL.print(idx);
	oscDump(oscs[idx]);
}

void oscDump(const Oscillator& o) {
	SYNTH_SERIAL.print(" note=");
	SYNTH_SERIAL.print(o.note);
	SYNTH_SERIAL.print(", velocity=");
	SYNTH_SERIAL.println(o.velocity);  
}

inline void notesDump(int8_t* notes) {
	for (uint8_t i=0; i<NVOICES; ++i) {
		SYNTH_SERIAL.print(' ');
		SYNTH_SERIAL.print(notes[i]);
	}
	SYNTH_SERIAL.println();
}

inline void printResources( float cpu, uint8_t mem) {
	SYNTH_SERIAL.print( "CPU Usage: ");
	SYNTH_SERIAL.print(cpu);
	SYNTH_SERIAL.print( "%, Memory: ");
	SYNTH_SERIAL.println(mem);
}

void performanceCheck() {
	static unsigned long last = 0;
	unsigned long now = millis();
	if ((now-last)>1000) {
		last = now;
		float cpu = AudioProcessorUsageMax();
		uint8_t mem = AudioMemoryUsageMax();
		if( (statsMem!=mem) || fabs(statsCpu-cpu)>1) {
			printResources( cpu, mem);
		}   
		AudioProcessorUsageMaxReset();
		AudioMemoryUsageMaxReset();
		last = now;
		statsCpu = cpu;
		statsMem = mem;
	}
}

void printInfo() {
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.print("Master Volume:        ");
	SYNTH_SERIAL.println(masterVolume);
	SYNTH_SERIAL.print("OSC1 Waveform  :      ");
	SYNTH_SERIAL.println(osc1_wf);
	SYNTH_SERIAL.print("OSC2 Waveform  :      ");
	SYNTH_SERIAL.println(osc2_wf);
	SYNTH_SERIAL.print("Poly On:              ");
	SYNTH_SERIAL.println(polyOn);
	SYNTH_SERIAL.print("Omni On:              ");
	SYNTH_SERIAL.println(omniOn);
	SYNTH_SERIAL.print("Velocity On:          ");
	SYNTH_SERIAL.println(velocityOn);
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.print("Sustain Pressed:      ");
	SYNTH_SERIAL.println(sustainPressed);
	SYNTH_SERIAL.print("Channel Volume:       ");
	SYNTH_SERIAL.println(channelVolume);
	SYNTH_SERIAL.print("Panorama:             ");
	SYNTH_SERIAL.println(panorama);
	SYNTH_SERIAL.print("Pulse Width:          ");
	SYNTH_SERIAL.println(pulseWidth);
	SYNTH_SERIAL.print("Pitch Bend:           ");
	SYNTH_SERIAL.println(pitchBend);
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.print("Filter Mode:          ");
	SYNTH_SERIAL.println(FILTERmode);
	SYNTH_SERIAL.print("Filter Octave:        ");
	SYNTH_SERIAL.println(filtOct);
	SYNTH_SERIAL.print("Filter Frequency:     ");
	SYNTH_SERIAL.println(filtFreq);
	SYNTH_SERIAL.print("Filter Resonance:     ");
	SYNTH_SERIAL.println(filtReso);
	SYNTH_SERIAL.print("Filter Attenuation:   ");
	SYNTH_SERIAL.println(filtAtt);
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.print("LFO Mode:          ");
	SYNTH_SERIAL.println(LFOmode);
	SYNTH_SERIAL.print("LFO Speed:     ");
	SYNTH_SERIAL.println(LFOspeed);
	SYNTH_SERIAL.print("LFO Depth:     ");
	SYNTH_SERIAL.println(LFOdepth);
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.print("Envelope On:          ");
	SYNTH_SERIAL.println(envOn);
	SYNTH_SERIAL.print("Envelope Delay:       ");
	SYNTH_SERIAL.println(envDelay);
	SYNTH_SERIAL.print("Envelope Attack:      ");
	SYNTH_SERIAL.println(envAttack);
	SYNTH_SERIAL.print("Envelope Hold:        ");
	SYNTH_SERIAL.println(envHold);
	SYNTH_SERIAL.print("Envelope Decay:       ");
	SYNTH_SERIAL.println(envDecay);
	SYNTH_SERIAL.print("Envelope Sustain:     ");
	SYNTH_SERIAL.println(envSustain);
	SYNTH_SERIAL.print("Envelope Release:     ");
	SYNTH_SERIAL.println(envRelease);
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.print("Envelope2 On:          ");
	SYNTH_SERIAL.println(env2On);
	SYNTH_SERIAL.print("Envelope2 Delay:       ");
	SYNTH_SERIAL.println(env2Delay);
	SYNTH_SERIAL.print("Envelope2 Attack:      ");
	SYNTH_SERIAL.println(env2Attack);
	SYNTH_SERIAL.print("Envelope2 Hold:        ");
	SYNTH_SERIAL.println(env2Hold);
	SYNTH_SERIAL.print("Envelope2 Decay:       ");
	SYNTH_SERIAL.println(env2Decay);
	SYNTH_SERIAL.print("Envelope2 Sustain:     ");
	SYNTH_SERIAL.println(env2Sustain);
	SYNTH_SERIAL.print("Envelope2 Release:     ");
	SYNTH_SERIAL.println(env2Release);
	SYNTH_SERIAL.println();
	/*
	SYNTH_SERIAL.print("Flanger On:           ");
	SYNTH_SERIAL.println(flangerOn);
	SYNTH_SERIAL.print("Flanger Offset:       ");
	SYNTH_SERIAL.println(flangerOffset);
	SYNTH_SERIAL.print("Flanger Depth:        ");
	SYNTH_SERIAL.println(flangerDepth);
	SYNTH_SERIAL.print("Flanger Freq. Coarse: ");
	SYNTH_SERIAL.println(flangerFreqCoarse);
	SYNTH_SERIAL.print("Flanger Freq. Fine:   ");
	SYNTH_SERIAL.println(flangerFreqFine);
	*/
	SYNTH_SERIAL.print("Delay Line Length:    ");
	SYNTH_SERIAL.println(DELAY_LENGTH);
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.print("Portamento On:        ");
	SYNTH_SERIAL.println(portamentoOn);
	SYNTH_SERIAL.print("Portamento Time:      ");
	SYNTH_SERIAL.println(portamentoTime);
	SYNTH_SERIAL.print("Portamento Step:      ");
	SYNTH_SERIAL.println(portamentoStep);
	SYNTH_SERIAL.print("Portamento Direction: ");
	SYNTH_SERIAL.println(portamentoDir);
	SYNTH_SERIAL.print("Portamento Position:  ");
	SYNTH_SERIAL.println(portamentoPos);
}

void selectCommand(char c) {
	switch (c) {
	case '\r':
		SYNTH_SERIAL.println();
		break;
	case 'b':
		// print voice statistics
		SYNTH_SERIAL.print("Notes Pressed:");
		notesDump(notesPressed);
		SYNTH_SERIAL.print("Notes On:     ");
		notesDump(notesOn);
		break;
	case 'o':
		// print oscillator status
		for (uint8_t i=0; i<NVOICES; ++i)
				oscDump(i);
		break;
	case 's':
		// print cpu and mem usage
		printResources(statsCpu,statsMem);
		break;
	case 'r':
		// reset parameters
		resetAll();
		break;
	case 'i':
		// print info
		printInfo();
		break;
	case '\t':
		// reboot Teensy
		*(uint32_t*)0xE000ED0C = 0x5FA0004;
		break;
	case ' ':
		// send note off
		allOff();
		break;
	default:
		break;
	}
}

#endif

//////////////////////////////////////////////////////////////////////
// setup() and loop()
//////////////////////////////////////////////////////////////////////
void setup() {
#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.begin(115200);
#endif
	
#ifdef USE_OLED
	//////////////////////////////////////////////
	// start oled setup
	//////////////////////////////////////////////
	Wire.setClock(2400000UL); // set i2c frequency to 400kHz
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);	// for 128x64 use 0x3D, for 128x32 use 0x3C
	display.setFont(&FreeSans9pt7b);
	text=(String)"Teensy Synth";
	showtext(1, text, true);
#endif

	init_cc_param_map();

	AudioMemory(AMEMORY);
	sgtl5000_1.enable();
	sgtl5000_1.unmuteHeadphone();
	sgtl5000_1.volume(masterVolume);

	resetAll();

	/*
	flangerL.begin(delaylineL,DELAY_LENGTH,FLANGE_DELAY_PASSTHRU,0,0);
	flangerR.begin(delaylineR,DELAY_LENGTH,FLANGE_DELAY_PASSTHRU,0,0);
	updateFlanger();
	*/

	mixer9.gain(0, 1);
	mixer10.gain(0, 1);
	mixer11.gain(0, 1);
	mixer12.gain(0, 1);
	mixer13.gain(0, 1);
	mixer14.gain(0, 1);
	mixer15.gain(0, 1);
	mixer16.gain(0, 1);

	mixer9.gain(1, 1);
	mixer10.gain(1, 1);
	mixer11.gain(1, 1);
	mixer12.gain(1, 1);
	mixer13.gain(1, 1);
	mixer14.gain(1, 1);
	mixer15.gain(1, 1);
	mixer16.gain(1, 1);

#ifdef USB_MIDI
	// see arduino/hardware/teensy/avr/libraries/USBHost_t36/USBHost_t36.h
	myusb.begin();
	usbMIDI1.setHandleNoteOff(OnNoteOff);
	usbMIDI1.setHandleNoteOn(OnNoteOn);
	usbMIDI1.setHandleVelocityChange(OnAfterTouchPoly);
	usbMIDI1.setHandleControlChange(OnControlChange);
	usbMIDI1.setHandlePitchChange(OnPitchChange);
	usbMIDI1.setHandleProgramChange(OnProgramChange);
	usbMIDI1.setHandleAfterTouch(OnAfterTouch);
	usbMIDI1.setHandleSysEx(OnSysEx);
	//usbMIDI1.setHandleRealTimeSystem(OnRealTimeSystem);
	//usbMIDI1.setHandleTimeCodeQuarterFrame(OnTimeCodeQFrame);
	usbMIDI2.setHandleNoteOff(OnNoteOff);
	usbMIDI2.setHandleNoteOn(OnNoteOn);
	usbMIDI2.setHandleVelocityChange(OnAfterTouchPoly);
	usbMIDI2.setHandleControlChange(OnControlChange);
	usbMIDI2.setHandlePitchChange(OnPitchChange);
	usbMIDI2.setHandleProgramChange(OnProgramChange);
	usbMIDI2.setHandleAfterTouch(OnAfterTouch);
	usbMIDI2.setHandleSysEx(OnSysEx);
	//usbMIDI2.setHandleRealTimeSystem(OnRealTimeSystem);
	//usbMIDI2.setHandleTimeCodeQuarterFrame(OnTimeCodeQFrame);
#else
	// see arduino/hardware/teensy/avr/libraries/MIDI/src/MIDI.h
	MIDI.begin();
	MIDI.setHandleNoteOff(OnNoteOff);
	MIDI.setHandleNoteOn(OnNoteOn);
	MIDI.setHandleAfterTouchPoly(OnAfterTouchPoly);
	MIDI.setHandleControlChange(OnControlChange);
	MIDI.setHandlePitchBend(OnPitchChange);
	MIDI.setHandleProgramChange(OnProgramChange);
	MIDI.setHandleAfterTouchChannel(OnAfterTouch);
	// the following functions need a different callback signature but they are
	// not used anyways, so...
	//MIDI.setHandleSystemExclusive(OnSysEx);
	//MIDI.setHandleTimeCodeQuarterFrame(OnTimeCodeQFrame);
#endif
 
	delay(1000);

#if SYNTH_DEBUG > 0
	SYNTH_SERIAL.println();
	SYNTH_SERIAL.println("TeensySynth v0.1");
#ifdef USB_MIDI
	SYNTH_SERIAL.println("USB_MIDI enabled");
#else
	SYNTH_SERIAL.println("UART_MIDI enabled");
#endif // USB_MIDI
#endif // SYNTH_DEBUG
}

void loop() {
#ifdef USB_MIDI
	usbMIDI1.read();
	usbMIDI2.read();
#else
	MIDI.read();
#endif

	// ns - no pot, so disable updateMasterVolume for now
	//updateMasterVolume();
  LFOupdate(false);
	updatePortamento();

#if SYNTH_DEBUG > 0
	performanceCheck();
	while (SYNTH_SERIAL.available())
		selectCommand(SYNTH_SERIAL.read());
#endif

#ifdef USE_OLED
	// if text1-4 are used, only allow update of screen every once it a while. do not let it
	// block other more-important processes by spamming the screen with updates.
	if (next_oled_update_ms<millis()){
		if (text1!=""){
			showtext(1, text1, false);
			text1="";
		}
		if (text2!=""){
			showtext(2, text2, false);
			text2="";
		}
		if (text3!=""){
			showtext(3, text3, false);
			text3="";
		}
		if (text4!=""){
			showtext(4, text4, false);
			text4="";
		}
		next_oled_update_ms=millis()+100;
	}
#endif
}
