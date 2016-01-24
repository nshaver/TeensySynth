#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// define SYNTH_DEBUG to enable debug logging on HWSerial 1 (digital pins 0/1)
#define SYNTH_DEBUG

// define IGNORE_OTHERCHANNELS to only listen to MIDI channel 1
#define IGNORE_OTHERCHANNELS

// define MIDI channel
#define SYNTH_MIDICHANNEL 1

// define tuning of A4 in Hz
#define SYNTH_TUNING 440

// Audio architecture generated with
// http://www.pjrc.com/teensy/gui/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=80,180
AudioSynthWaveform       waveform2;      //xy=80,240
AudioSynthWaveform       waveform3;      //xy=80,300
AudioSynthWaveform       waveform4;      //xy=80,360
AudioSynthWaveform       waveform5;      //xy=80,420
AudioSynthWaveform       waveform6;      //xy=80,480
AudioSynthWaveform       waveform7;      //xy=80,540
AudioSynthWaveform       waveform8;      //xy=80,600
AudioFilterStateVariable filter1;        //xy=260,180
AudioFilterStateVariable filter2;        //xy=260,240
AudioFilterStateVariable filter3;        //xy=260,300
AudioFilterStateVariable filter4;        //xy=260,360
AudioFilterStateVariable filter5;        //xy=260,420
AudioFilterStateVariable filter6;        //xy=260,480
AudioFilterStateVariable filter7;        //xy=260,540
AudioFilterStateVariable filter8;        //xy=260,600
AudioMixer4              mixer1;         //xy=440,180
AudioMixer4              mixer2;         //xy=440,240
AudioMixer4              mixer3;         //xy=440,300
AudioMixer4              mixer4;         //xy=440,360
AudioMixer4              mixer5;         //xy=440,420
AudioMixer4              mixer6;         //xy=440,480
AudioMixer4              mixer7;         //xy=440,540
AudioMixer4              mixer8;         //xy=440,600
AudioEffectEnvelope      envelope1;      //xy=640,180
AudioEffectEnvelope      envelope2;      //xy=640,240
AudioEffectEnvelope      envelope3;      //xy=640,300
AudioEffectEnvelope      envelope4;      //xy=640,360
AudioEffectEnvelope      envelope5;      //xy=640,420
AudioEffectEnvelope      envelope6;      //xy=640,480
AudioEffectEnvelope      envelope7;      //xy=640,540
AudioEffectEnvelope      envelope8;      //xy=640,600
AudioMixer4              envmixer1;      //xy=900,210
AudioMixer4              envmixer2;      //xy=900,330
AudioMixer4              envmixer3;      //xy=900,450
AudioMixer4              envmixer4;      //xy=900,570
AudioMixer4              mixerL;         //xy=1160,270
AudioMixer4              mixerR;         //xy=1160,510
AudioOutputI2S           i2s1;           //xy=1300,390
AudioConnection          patchCord1(waveform1, 0, filter1, 0);
AudioConnection          patchCord2(waveform1, 0, mixer1, 3);
AudioConnection          patchCord3(waveform2, 0, filter2, 0);
AudioConnection          patchCord4(waveform2, 0, mixer2, 3);
AudioConnection          patchCord5(waveform3, 0, filter3, 0);
AudioConnection          patchCord6(waveform3, 0, mixer3, 3);
AudioConnection          patchCord7(waveform4, 0, filter4, 0);
AudioConnection          patchCord8(waveform4, 0, mixer4, 3);
AudioConnection          patchCord9(waveform5, 0, filter5, 0);
AudioConnection          patchCord10(waveform5, 0, mixer5, 3);
AudioConnection          patchCord11(waveform6, 0, filter6, 0);
AudioConnection          patchCord12(waveform6, 0, mixer6, 3);
AudioConnection          patchCord13(waveform7, 0, filter7, 0);
AudioConnection          patchCord14(waveform7, 0, mixer7, 3);
AudioConnection          patchCord15(waveform8, 0, filter8, 0);
AudioConnection          patchCord16(waveform8, 0, mixer8, 3);
AudioConnection          patchCord17(filter1, 0, mixer1, 0);
AudioConnection          patchCord18(filter1, 1, mixer1, 1);
AudioConnection          patchCord19(filter1, 2, mixer1, 2);
AudioConnection          patchCord20(filter2, 0, mixer2, 0);
AudioConnection          patchCord21(filter2, 1, mixer2, 1);
AudioConnection          patchCord22(filter2, 2, mixer2, 2);
AudioConnection          patchCord23(filter3, 0, mixer3, 0);
AudioConnection          patchCord24(filter3, 1, mixer3, 1);
AudioConnection          patchCord25(filter3, 2, mixer3, 2);
AudioConnection          patchCord26(filter4, 0, mixer4, 0);
AudioConnection          patchCord27(filter4, 1, mixer4, 1);
AudioConnection          patchCord28(filter4, 2, mixer4, 2);
AudioConnection          patchCord29(filter5, 0, mixer5, 0);
AudioConnection          patchCord30(filter5, 1, mixer5, 1);
AudioConnection          patchCord31(filter5, 2, mixer5, 2);
AudioConnection          patchCord32(filter6, 0, mixer6, 0);
AudioConnection          patchCord33(filter6, 1, mixer6, 1);
AudioConnection          patchCord34(filter6, 2, mixer6, 2);
AudioConnection          patchCord35(filter7, 0, mixer7, 0);
AudioConnection          patchCord36(filter7, 1, mixer7, 1);
AudioConnection          patchCord37(filter7, 2, mixer7, 2);
AudioConnection          patchCord38(filter8, 0, mixer8, 0);
AudioConnection          patchCord39(filter8, 1, mixer8, 1);
AudioConnection          patchCord40(filter8, 2, mixer8, 2);
AudioConnection          patchCord41(mixer1, 0, envelope1, 0);
AudioConnection          patchCord42(mixer1, 0, envmixer1, 2);
AudioConnection          patchCord43(mixer2, 0, envelope2, 0);
AudioConnection          patchCord44(mixer2, 0, envmixer1, 3);
AudioConnection          patchCord45(mixer3, 0, envelope3, 0);
AudioConnection          patchCord46(mixer3, 0, envmixer2, 2);
AudioConnection          patchCord47(mixer4, 0, envelope4, 0);
AudioConnection          patchCord48(mixer4, 0, envmixer2, 3);
AudioConnection          patchCord49(mixer5, 0, envelope5, 0);
AudioConnection          patchCord50(mixer5, 0, envmixer3, 2);
AudioConnection          patchCord51(mixer6, 0, envelope6, 0);
AudioConnection          patchCord52(mixer6, 0, envmixer3, 3);
AudioConnection          patchCord53(mixer7, 0, envelope7, 0);
AudioConnection          patchCord54(mixer7, 0, envmixer4, 2);
AudioConnection          patchCord55(mixer8, 0, envelope8, 0);
AudioConnection          patchCord56(mixer8, 0, envmixer4, 3);
AudioConnection          patchCord57(envelope1, 0, envmixer1, 0);
AudioConnection          patchCord58(envelope2, 0, envmixer1, 1);
AudioConnection          patchCord59(envelope3, 0, envmixer2, 0);
AudioConnection          patchCord60(envelope4, 0, envmixer2, 1);
AudioConnection          patchCord61(envelope5, 0, envmixer3, 0);
AudioConnection          patchCord62(envelope6, 0, envmixer3, 1);
AudioConnection          patchCord63(envelope7, 0, envmixer4, 0);
AudioConnection          patchCord64(envelope8, 0, envmixer4, 1);
AudioConnection          patchCord65(envmixer1, 0, mixerL, 0);
AudioConnection          patchCord66(envmixer1, 0, mixerR, 0);
AudioConnection          patchCord67(envmixer2, 0, mixerL, 1);
AudioConnection          patchCord68(envmixer2, 0, mixerR, 1);
AudioConnection          patchCord69(envmixer3, 0, mixerL, 2);
AudioConnection          patchCord70(envmixer3, 0, mixerR, 2);
AudioConnection          patchCord71(envmixer4, 0, mixerL, 3);
AudioConnection          patchCord72(envmixer4, 0, mixerR, 3);
AudioConnection          patchCord73(mixerL, 0, i2s1, 0);
AudioConnection          patchCord74(mixerR, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=80,60
// GUItool: end automatically generated code

//////////////////////////////////////////////////////////////////////
// Data types and lookup tables
//////////////////////////////////////////////////////////////////////
struct Oscillator {
  AudioSynthWaveform*       wf;
  AudioFilterStateVariable* filt;
  AudioMixer4*              mix;
  AudioEffectEnvelope*      env;
  int note;
  uint8_t velocity;
};

#define NVOICES 8

#define AMEMORY 40
Oscillator oscs[NVOICES] = {
  { &waveform1, &filter1, &mixer1, &envelope1, -1, 0 },
  { &waveform2, &filter2, &mixer2, &envelope2, -1, 0 },
  { &waveform3, &filter3, &mixer3, &envelope3, -1, 0 },
  { &waveform4, &filter4, &mixer4, &envelope4, -1, 0 },
  { &waveform5, &filter5, &mixer5, &envelope5, -1, 0 },
  { &waveform6, &filter6, &mixer6, &envelope6, -1, 0 },
  { &waveform7, &filter7, &mixer7, &envelope7, -1, 0 },
  { &waveform8, &filter8, &mixer7, &envelope8, -1, 0 },
};

enum PolyMode_t {
  POLY,
  MONO,
  //MONO_L,
  //MONO_R,
  POLYMODE_N,
};

enum FilterMode_t {
  LOWPASS,
  BANDPASS,
  HIGHPASS,
  FILTEROFF,
  FILTERMODE_N,
};

#define NPROGS 7
uint8_t progs[NPROGS] = {
  WAVEFORM_SINE,
  WAVEFORM_SQUARE,
  WAVEFORM_TRIANGLE,
  WAVEFORM_SAWTOOTH,
  WAVEFORM_SAWTOOTH_REVERSE,
  WAVEFORM_PULSE,
  WAVEFORM_SAMPLE_HOLD,
};

//////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////
float   masterVolume   = 0.3;
uint8_t currentProgram = 3;

PolyMode_t polyMode;
bool       sustainPressed;
float      channelVolume;
float      panorama;
float      pulseWidth;
float      pitchBend;

// filter
FilterMode_t filterMode;
float filtFreq; // 20-AUDIO_SAMPLE_RATE_EXACT/2.5
float filtReso; // 0.9-5.0
float filtOcta; // 0-7

// envelope
bool  envOn;
float envDelay;   // 0-200
float envAttack;  // 0-200
float envHold;    // 0-200
float envDecay;   // 0-200
float envSustain; // 0-1
float envRelease; // 0-200

int8_t notesOn[NVOICES]      = { -1, -1, -1, -1, -1, -1, -1, -1, };
int8_t notesPressed[NVOICES] = { -1, -1, -1, -1, -1, -1, -1, -1, };

inline void notesReset(int8_t* notes) {
  for (uint8_t i=0; i<NVOICES; ++i) {
    notes[i] = -1;
  }
}

inline void notesAdd(int8_t* notes, uint8_t note) {
  for (uint8_t i=0; i<NVOICES; ++i) {
    if(notes[i]==-1) {
      notes[i] = note;
      break;
    }
  }
}

inline void notesDel(int8_t* notes, uint8_t note) {
  uint8_t i=1;
  for ( ; i < NVOICES && notes[i-1] != note; ++i);
  for ( ; i < NVOICES; ++i) {
    notes[i-1] = notes[i];
    if (notes[i]==-1)
      break;
  }
  if (notes[NVOICES-1]==note || i == NVOICES)
    notes[NVOICES-1] = -1;
}

inline bool notesFind(int8_t* notes, uint8_t note) {
  for (uint8_t i=0; i<NVOICES; ++i) {
    if (notes[i]==note) return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////
// Parameter control functions
//////////////////////////////////////////////////////////////////////
inline void updateFilterMode() {
  for (uint8_t i=0; i<NVOICES; ++i) {
    for (uint8_t fm=0; fm<FILTERMODE_N; ++fm) {
      if (fm == filterMode)
        oscs[i].mix->gain(fm,1);
      else
        oscs[i].mix->gain(fm,0);
    }
  }
}

inline void updateFilter() {
  for (uint8_t i=0; i<NVOICES; ++i) {
    oscs[i].filt->frequency(filtFreq);
    oscs[i].filt->resonance(filtReso);
    oscs[i].filt->octaveControl(filtOcta);
  }
}

inline void updateEnvelope() {
  for (uint8_t i=0; i<NVOICES; ++i) {
    oscs[i].env->delay(envDelay);
    oscs[i].env->attack(envAttack);
    oscs[i].env->hold(envHold);
    oscs[i].env->decay(envDecay);
    oscs[i].env->sustain(envSustain);
    oscs[i].env->release(envRelease);
  }
}

inline void toggleEnvelope() {
  float env   = envOn ? 0.5 : 0;
  float noenv = envOn ? 0 : 0.5;
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

void resetAll() {
  polyMode       = POLY;
  filterMode     = FILTEROFF;
  sustainPressed = false;
  channelVolume  = 1.0;
  panorama       = 0.5;
  pulseWidth     = 0.5;
  pitchBend      = 0;

  // filter
  filtFreq = 10000.;
  filtReso = 0.7;
  filtOcta = 0;

  // envelope
  envOn      = true;
  envDelay   = 0;
  envAttack  = 50;
  envHold    = 50;
  envDecay   = 50;
  envSustain = 0.75;
  envRelease = 50;
  
  updateFilterMode();
  updateEnvelope();
  updatePan();
}

inline void updateProgram() {
  for (uint8_t i=0; i<NVOICES; ++i) {
    if (currentProgram==5) {
      oscs[i].wf->pulseWidth(pulseWidth);
    }
    oscs[i].wf->begin(progs[currentProgram]);
  }
}

inline void updatePulseWidth() {
  if (currentProgram!=5) return;
  for (uint8_t i=0; i<NVOICES; ++i) {
    if (oscs[i].note < 0) continue;
    oscs[i].wf->pulseWidth(pulseWidth);
  }
}

inline void updatePitch() {
  for (uint8_t i=0; i<NVOICES; ++i) {
    int8_t note = oscs[i].note;
    if (note < 0) continue;
    note -= 69; 
    oscs[i].wf->frequency(SYNTH_TUNING * pow(2,note/12.+pitchBend));
  }
}

inline void updateVolume() {
  for (uint8_t i=0; i<NVOICES; ++i) {
    if (oscs[i].note < 0) continue;
    oscs[i].wf->amplitude(oscs[i].velocity/127.*channelVolume);
  }
}

inline void updatePan() {
  for (uint8_t i=0; i<4; ++i) {
    mixerL.gain(i,(1-panorama)/4.);
    mixerR.gain(i,panorama/4.);
  }
}

inline void updateMasterVolume() {
  // read the volume knob
  float vol = (float) analogRead(A1) / 1280.0;
  if( fabs(vol-masterVolume) > 0.01) {
    masterVolume = vol;
    sgtl5000_1.volume(masterVolume);
#ifdef SYNTH_DEBUG
    Serial1.print("Volume: ");
    Serial1.println(vol);
#endif
  }
}

//////////////////////////////////////////////////////////////////////
// Oscillator control functions
//////////////////////////////////////////////////////////////////////
inline void oscOn( Oscillator& osc, int8_t note, uint8_t velocity) {
  if (osc.note!=note) {
    // Sets all notes as an offset of A4 (#69)
    int8_t noteNumberAfterOffset = note - 69; 
    osc.wf->frequency(SYNTH_TUNING * pow(2,noteNumberAfterOffset/12.+pitchBend));
    notesAdd(notesOn,note);
    osc.wf->amplitude(velocity/127.*channelVolume);
    osc.velocity = velocity;
    osc.note = note;
    if (envOn)
      osc.env->noteOn();
  } else {
    if (velocity > osc.velocity) {
      osc.wf->amplitude(velocity/127.*channelVolume);
      osc.velocity = velocity;
    }
  }
}

inline void oscOff( Oscillator& osc) {
  if (envOn)
    osc.env->noteOff();
  else
    osc.wf->amplitude(0);
  notesDel(notesOn,osc.note);
  osc.note = -1;
}

inline void allOff() {
  for (uint8_t i=0; i<NVOICES; ++i) {
    oscOff(oscs[i]);
    oscs[i].wf->amplitude(0);
  }
  notesReset(notesOn);
}

//////////////////////////////////////////////////////////////////////
// MIDI handlers
//////////////////////////////////////////////////////////////////////
void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
#ifdef IGNORE_OTHERCHANNELS
  if (channel != SYNTH_MIDICHANNEL) return;
#endif

#if 0 //#ifdef SYNTH_DEBUG
  Serial1.println("NoteOn");
#endif

  notesAdd(notesPressed,note);

  Oscillator* curOsc = 0;
  if (sustainPressed && notesFind(notesOn,note)) {
    for (uint8_t i=0; i<NVOICES; ++i) {
      if (oscs[i].note == note) {
        curOsc = &oscs[i];
        break;
      }
    }
  }
  
  switch(polyMode) {
    case(POLY):
      for (uint8_t i=0; i<NVOICES && !curOsc; ++i) {
        if (oscs[i].note < 0) {
          curOsc = &oscs[i];
          break;
        }
      }
      if (!curOsc && notesOn[0] != -1) {
        Serial1.println("Stealing voice");
        curOsc = OnNoteOffReal(channel,notesOn[0],velocity,true);
      }
      if (!curOsc) return;
      oscOn( *curOsc, note, velocity);
      break;
    case(MONO):
      curOsc = &oscs[0];
      oscOn( *curOsc, note, velocity);
      break;
    default:
      break;
  }

  return;
}

Oscillator* OnNoteOffReal(uint8_t channel, uint8_t note, uint8_t velocity, bool ignoreSustain) {
#ifdef IGNORE_OTHERCHANNELS
  if (channel != SYNTH_MIDICHANNEL) return 0;
#endif

#if 0 //#ifdef SYNTH_DEBUG
  Serial1.println("NoteOff");
#endif
  notesDel(notesPressed,note);

  if (sustainPressed && !ignoreSustain) return 0;

  Oscillator* curOsc = 0;
  switch(polyMode) {
    case(POLY):
      for (uint8_t i=0; i<NVOICES; ++i) {
        if (oscs[i].note == note) {
          curOsc = &oscs[i];
          break;
        }
      }
      if (!curOsc) return 0;
      oscOff(*curOsc);
      break;
    case (MONO):
      curOsc = &oscs[0];
      if (curOsc->note == note)
        oscOff(*curOsc);
      break;
    default:
      break;
  }
  
  return curOsc;
}

inline void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  OnNoteOffReal(channel,note,velocity,false);
}

void OnControlChange(uint8_t channel, uint8_t control, uint8_t value) {
#ifdef IGNORE_OTHERCHANNELS
  if (channel != SYNTH_MIDICHANNEL) return;
#endif

#ifdef SYNTH_DEBUG
  Serial1.print("ControlChange: channel ");
  Serial1.print(channel);
  Serial1.print(", control ");
  Serial1.print(control);
  Serial1.print(", value ");
  Serial1.println(value);
#endif

  switch (control) {
    case 0: // bank select, do nothing (switch sounds via program change only)
      break;
    case 7: // volume
      channelVolume = value/127.;
      updateVolume();
      break;
    case 10: // pan
      panorama = value/127.;
      updatePan();
      break;
    case 12: // attack
      envAttack = value*200./127.;
      updateEnvelope();
      break;
    case 13: // release
      envRelease = value*200./127.;
      updateEnvelope();
      break;
    case 14: // filter frequency
      filtFreq = value/2.5*AUDIO_SAMPLE_RATE_EXACT/127.;
      updateFilter();
      Serial1.print("Frequency: ");
      Serial1.println(filtFreq);
      break;
    case 15: // filter resonance
      filtReso = value*4.1/127.+0.9;
      updateFilter();
      Serial1.print("Resonance: ");
      Serial1.println(filtReso);
      break;
    case 16: // filter octave control
      filtOcta = value*7./127.;
      updateFilter();
      Serial1.print("Octave Control: ");
      Serial1.println(filtOcta);
     break;
    case 17: // filter mode
      if (value < FILTERMODE_N) {
        filterMode = FilterMode_t(value);
      } else {
        filterMode = FilterMode_t((filterMode+1)%FILTERMODE_N);
      }
      updateFilterMode();
      break;
    case 18: // poly mode
      if (value < POLYMODE_N) {
        polyMode = PolyMode_t(value);
      } else {
        polyMode = PolyMode_t((polyMode+1)%POLYMODE_N);
      }
      allOff();
      break;
    case 19: // envelope mode
      allOff();
      envOn = !envOn;
      toggleEnvelope();
      break;
    case 20: // delay
      envDelay = value*200./127.;
      updateEnvelope();
      break;
    case 21: // hold
      envHold = value*200./127.;
      updateEnvelope();
      break;
    case 22: // decay
      envDecay = value*200./127.;
      updateEnvelope();
      break;
    case 23: // sustain
      envSustain = value/127.;
      updateEnvelope();
      break;
    case 24: // pulse width
      pulseWidth = value/127.;
      updatePulseWidth();
      break;
    case 64: // sustain/damper pedal
      if (value > 64) {
        sustainPressed = true;
      } else {
        sustainPressed = false;
        for (uint8_t i=0; i<NVOICES; ++i) {
          if (oscs[i].note != -1 && !notesFind(notesPressed,oscs[i].note)) {
            oscOff(oscs[i]);
          }
        }
      }
      break;
    case 121: // controller reset
      resetAll();
      break;
    case 123: // all notes off
      allOff();
      break;
    default:
      Serial1.print("Unhandled Control Change ");
      Serial1.println(control);
      break;
  }    
}

void OnPitchChange(uint8_t channel, int pitch) {
#ifdef IGNORE_OTHERCHANNELS
  if (channel != SYNTH_MIDICHANNEL) return;
#endif

#if 0 //#ifdef SYNTH_DEBUG
  Serial1.print("PitchChange: channel ");
  Serial1.print(channel);
  Serial1.print(", pitch ");
  Serial1.println(pitch);
#endif

  if (pitch == 8192)
    pitchBend = 0;
  else
    pitchBend = (pitch-8192)/8192.;
  updatePitch();
}

void OnProgramChange(uint8_t channel, uint8_t program) {
#ifdef IGNORE_OTHERCHANNELS
  if (channel!=SYNTH_MIDICHANNEL) return;
#endif

#if 0 //#ifdef SYNTH_DEBUG
  Serial1.print("ProgramChange: channel ");
  Serial1.print(channel);
  Serial1.print(", program ");
  Serial1.println(program);
#endif

  if (program <NPROGS) {
    if( program!=currentProgram) {
      currentProgram = program;
      updateProgram();
    }
  }
}

void OnAfterTouch(uint8_t channel, uint8_t pressure) {
#ifdef IGNORE_OTHERCHANNELS
  if (channel!=SYNTH_MIDICHANNEL) return;
#endif

#ifdef SYNTH_DEBUG
  Serial1.print("AfterTouch: channel ");
  Serial1.print(channel);
  Serial1.print(", pressure ");
  Serial1.println(pressure);
#endif
}

void OnSysEx( const uint8_t *data, uint16_t length, bool complete) {
  Serial1.print("SysEx: length ");
  Serial1.print(length);
  Serial1.print(", complete ");
  Serial1.println(complete);
}

void OnRealTimeSystem(uint8_t realtimebyte) {
  Serial1.print("RealTimeSystem: ");
  Serial1.println(realtimebyte);
}

void OnTimeCodeQFrame(uint16_t data)
{
  Serial1.print("TimeCodeQuarterFrame: ");
  Serial1.println(data);  
}
        
//////////////////////////////////////////////////////////////////////
// Debugging functions
//////////////////////////////////////////////////////////////////////
inline void notesDump(int8_t* notes) {
  for (uint8_t i=0; i<NVOICES; ++i) {
    Serial1.print(' ');
    Serial1.print(notes[i]);
  }
  Serial1.println();
}

inline void printResources( float cpu, uint8_t mem) {
  Serial1.print( "CPU Usage: ");
  Serial1.print(cpu);
  Serial1.print( "%, Memory: ");
  Serial1.println(mem);
}

void performanceCheck() {
  static unsigned long last = 0;
  static float   lastCpu = 0;
  static uint8_t lastMem = 0;
  unsigned long now = millis();
  if ((now-last)>1000) {
    last = now;
    float   cpu = AudioProcessorUsageMax();
    uint8_t mem = AudioMemoryUsageMax();
    if( (lastMem!=mem) || fabs(lastCpu-cpu)>1) {
      printResources( cpu, mem);
    }   
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();
    last = now;
    lastCpu = cpu;
    lastMem = mem;
  }
}

//////////////////////////////////////////////////////////////////////
// setup() and loop()
//////////////////////////////////////////////////////////////////////
void setup() {
  Serial1.begin(115200);
  
  AudioMemory(AMEMORY);
  sgtl5000_1.enable();
  sgtl5000_1.volume(masterVolume);

  resetAll();
  toggleEnvelope();
  updateProgram();
  delay(1000);
  
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandlePitchChange(OnPitchChange);
  usbMIDI.setHandleProgramChange(OnProgramChange);
  usbMIDI.setHandleAfterTouch(OnAfterTouch);
  usbMIDI.setHandleSysEx(OnSysEx);
  //usbMIDI.setHandleRealTimeSystem(OnRealTimeSystem);
  usbMIDI.setHandleTimeCodeQuarterFrame(OnTimeCodeQFrame);
 
  Serial1.println();
  Serial1.println("TeensySynth v0.1");
}

void loop() {
  usbMIDI.read();
  updateMasterVolume();
#ifdef SYNTH_DEBUG
  performanceCheck();
  while (Serial1.available())
  {
    
    switch (Serial1.read()) {
      case ('\r'):
        Serial1.println();
        break;
      case ('b'):
        Serial1.print("Notes Pressed:");
        notesDump(notesPressed);
        Serial1.print("Notes On:     ");
        notesDump(notesOn);
        break;
      default:
        break;
    }   
  }
#endif
}
