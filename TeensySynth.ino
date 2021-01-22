#include <Audio.h>
// use bandlimit waveforms from https://github.com/MarkTillotson/Audio/tree/band_limited_waveforms
#define USE_BANDLIMIT_WAVEFORMS 1

// set SYNTH_DEBUG to enable debug logging (1=most,2=all messages)
#define SYNTH_DEBUG 1

// define MIDI channel
#define SYNTH_MIDICHANNEL 1

// inital poly mode (POLY, MONO or PORTAMENTO)
#define SYNTH_INITIALMODE POLY

// define tuning of A4 in Hz
#define SYNTH_TUNING 440

// gain at oscillator/filter input stage (1:1)
// keep low so filter does not saturate with resonance
#define GAIN_OSC 0.5

// gain in final mixer stage for polyphonic mode (4:1)
// (0.25 is the safe value but larger sounds better :) )
//#define GAIN_POLY 1.
#define GAIN_POLY 0.25

// gain in final mixer stage for monophonic modes
//#define GAIN_MONO 1.
#define GAIN_MONO 0.25

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
#define NPROGS 13
uint16_t progs[NPROGS] = {
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
#define NPROGS 8
uint8_t progs[NPROGS] = {
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

enum FilterMode_t {
  LOWPASS,
  BANDPASS,
  HIGHPASS,
  FILTEROFF,
  FILTERMODE_N,
};

//////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////
float   masterVolume   = 0.8;
#ifdef USE_BANDLIMIT_WAVEFORMS
uint16_t currentProgram = WAVEFORM_BANDLIMIT_SAWTOOTH;
# else
uint8_t currentProgram = WAVEFORM_SAWTOOTH;
#endif

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

// filter
FilterMode_t filterMode;
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
bool  flangerOn;
int   flangerOffset;
int   flangerDepth;
float flangerFreqCoarse;
float flangerFreqFine;

// portamento
bool     portamentoOn;
uint32_t portamentoTime;
int8_t   portamentoDir;
float    portamentoStep;
float    portamentoPos;

// latching values
int lastProgCC=0;

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
    for (uint8_t fm=0; fm<FILTERMODE_N; ++fm) {
      if (fm == filterMode) o->mix->gain(fm,filtAtt);
      else                  o->mix->gain(fm,0);
    }
  } while (++o < end);
}

inline void updateFilter() {
  Oscillator *o=oscs,*end=oscs+NVOICES;
  do {
    o->filt->octaveControl(filtOct);
    o->filt->frequency(filtFreq);
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

void resetAll() {
  polyOn     = true;
  omniOn     = true;
  velocityOn = true;

	layerWidth = 1./127.;
	layerMix1	 = 1.;
	layerMix2	 = 1.;
  
  filterMode     = LOWPASS;
  sustainPressed = false;
  channelVolume  = 1.0;
  panorama       = 0.5;
  pulseWidth     = 0.5;
  pitchBend      = 0;
  pitchScale     = 1;
  octCorr        = currentProgram == WAVEFORM_PULSE ? 1 : 0;
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

  // FX
  flangerOn         = false;
  flangerOffset     = DELAY_LENGTH/4;
  flangerDepth      = DELAY_LENGTH/16;
  flangerFreqCoarse = 0;
  flangerFreqFine   = .5;

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

inline void updateProgram() {
  if (currentProgram==WAVEFORM_PULSE) octCorr = 1;
  else                   octCorr = 0;
    
  Oscillator *o=oscs,*end=oscs+NVOICES;
  do {
    if (currentProgram==WAVEFORM_PULSE || currentProgram==WAVEFORM_BANDLIMIT_PULSE){
			o->wf->pulseWidth(pulseWidth);
			o->wf2->pulseWidth(pulseWidth);
		}
    o->wf->begin(progs[currentProgram]);
		o->wf2->begin(progs[currentProgram]);
  } while(++o < end);
}

inline void updatePulseWidth() {
	if (currentProgram==WAVEFORM_PULSE || currentProgram==WAVEFORM_BANDLIMIT_PULSE) {
		Oscillator *o=oscs,*end=oscs+NVOICES;
		do {
			if (o->note < 0) continue;
			o->wf->pulseWidth(pulseWidth);
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
          curOsc = o;
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
  if (!omniOn && channel != SYNTH_MIDICHANNEL) return;

  switch (control) {
  case 0: // bank select, do nothing (switch sounds via program change only)
    break;
  case 13: // portamento time
  {
    float portamentoRange = portamentoStep*portamentoTime;
    portamentoTime = value*value*value;
    portamentoStep = portamentoRange/portamentoTime;
    break;
  }
  case 14: // volume
    channelVolume = value/127.;
    updateVolume();
    break;
  case 2: // attack
    envAttack = value*11880./127.;
    updateEnvelope();
    break;
  case 3: // decay
    envDecay = value*11880./127.;
    updateEnvelope();
    break;
  case 4: // sustain
    envSustain = value/127.;
    updateEnvelope();
    break;
  case 5: // release
    envRelease = value*11880./127.;
    updateEnvelope();
    break;
  case 6: // attack
    env2Attack = value*5000./127.;
    updateEnvelope2();
    break;
  case 8: // decay
    env2Decay = value*5000./127.;
    updateEnvelope2();
    break;
  case 9: // sustain
    env2Sustain = value/127.;
    updateEnvelope2();
    break;
  case 12: // release
    env2Release = value*5000./127.;
    updateEnvelope2();
    break;
  case 22: // filter frequency
    //filtFreq = value/2.5*AUDIO_SAMPLE_RATE_EXACT/127.;
    filtFreq = (1.0*value)/127;
		filtFreq=pow(filtFreq, 3)*17000;
    updateFilter();
    break;
  case 21: // filter resonance
    //filtReso = value*4.1/127.+0.9;
    filtReso = (1.0*value)/127;
		filtReso=map(filtReso, 0, 1, 1.11, 5.0);
    updateFilter();
    break;
  case 20: // filter octaves
    filtOct = 7.*value/127.;
    updateFilter();
    break;
  case 18: // layer mix
		if (value>63) {
			layerMix1 = map((1.0*value), 64., 127., 1., 0.);
			layerMix2 = 1.;
		} else {
			layerMix1 = 1.;
			layerMix2 = map((1.0*value), 63., 0., 1., 0.);
		}
    updateVolume();
    break;
  case 19: // filter mode
    if (value < FILTERMODE_N) {
      filterMode = FilterMode_t(value);
    } else {
      filterMode = FilterMode_t((filterMode+1)%FILTERMODE_N);
    }
    updateFilterMode();
    break;
	case 23: // poly mode
		switch (value){
			case 127:
				// turn on poly mode
				polyOn=true;
				portamentoOn=false;
    		updatePolyMode();
				break;
		}
		break;
	case 33: // mono mode
		switch (value){
			case 127:
				// turn off poly mode
				polyOn=false;
				portamentoOn=true;
    		updatePolyMode();
				break;
		}
		break;
  case 16: // layer width
    layerWidth = ((1+value)/127.);
  	updateLayer();
    break;
  case 17: // pulse width
    pulseWidth = (value/127.)*0.9+0.05;
    updatePulseWidth();
    break;
	case 47: // program change down
		if (value>=127 && lastProgCC != value){
  		if (value>0) {
				currentProgram--;
				updateProgram();
			}
		}
		lastProgCC=value;
		break;
	case 48: // program change up
		if (value>=127 && lastProgCC != value){
  		if (currentProgram<NPROGS) {
				currentProgram++;
				updateProgram();
			}
		}
		lastProgCC=value;
		break;
  case 64: // sustain/damper pedal
    if (value > 63) sustainPressed = true;
    else {
      sustainPressed = false;
      Oscillator *o=oscs, *end=oscs+NVOICES;
      do {
        if (o->note != -1 && !notesFind(notesPressed,o->note)) oscOff(*o);
      } while (++o < end);
    }
    break;
  case 31: // portamento on
    if (polyOn) break;
    if (value > 63) {
      portamentoOn = true;
      if (oscs->note != -1) portamentoPos = oscs->note;
    }
    break;
	case 41: // portamento off
    if (polyOn) break;
    if (value > 126) {
    	portamentoOn = false;
		}
    break;
  case 25: // transpose
		if (value>=127){	
			transpose++;
		}
    break;
  case 35: // transpose
		if (value>=127){	
			transpose--;
		}
    break;
	/*
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
  default:
#if SYNTH_DEBUG > 0
    SYNTH_SERIAL.print("Unhandled Control Change: channel ");
    SYNTH_SERIAL.print(channel);
    SYNTH_SERIAL.print(", control ");
    SYNTH_SERIAL.print(control);
    SYNTH_SERIAL.print(", value ");
    SYNTH_SERIAL.println(value);
#endif
    break;
  }    
#if SYNTH_DEBUG > 0
  SYNTH_SERIAL.print("Control Change: channel ");
  SYNTH_SERIAL.print(channel);
  SYNTH_SERIAL.print(", control ");
  SYNTH_SERIAL.print(control);
  SYNTH_SERIAL.print(", value ");
  SYNTH_SERIAL.println(value);
#endif
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

  if (program <NPROGS) {
    if( program!=currentProgram) {
      currentProgram = program;
      updateProgram();
    }
  }
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
  SYNTH_SERIAL.print("Current Program:      ");
  SYNTH_SERIAL.println(currentProgram);
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
  SYNTH_SERIAL.println(filterMode);
  SYNTH_SERIAL.print("Filter Frequency:     ");
  SYNTH_SERIAL.println(filtFreq);
  SYNTH_SERIAL.print("Filter Resonance:     ");
  SYNTH_SERIAL.println(filtReso);
  SYNTH_SERIAL.print("Filter Attenuation:   ");
  SYNTH_SERIAL.println(filtAtt);
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
  
  AudioMemory(AMEMORY);
  sgtl5000_1.enable();
  sgtl5000_1.unmuteHeadphone();
  sgtl5000_1.volume(masterVolume);

  resetAll();
  updateProgram();
  
  flangerL.begin(delaylineL,DELAY_LENGTH,FLANGE_DELAY_PASSTHRU,0,0);
  flangerR.begin(delaylineR,DELAY_LENGTH,FLANGE_DELAY_PASSTHRU,0,0);
  updateFlanger();

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
	// ns - no pot, so disable this for now
	//updateMasterVolume();
  updatePortamento();

#if SYNTH_DEBUG > 0
  performanceCheck();
  while (SYNTH_SERIAL.available())
    selectCommand(SYNTH_SERIAL.read());
#endif
}
