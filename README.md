## TeensySynth - a polyphonic MIDI synthesizer for Teensy 4.x

Forked from https://github.com/jmechnich/TeensySynth

### Changes from original fork
* features three per-voice oscillators, each with tuning and volume
* added bandlimit waveforms to eliminate aliasing
* added filter envelope
* added noise generator
* LFO
* sample and hold
* modified as needed to support teensy 4.1 and multiple MIDI input devices. allows a keyboard to control notes and a separate control surface to control synth parameters
* misc playability fixes. allow retrigger of notes when sustain is held. in mono mode retrigger still-held notes when current note is released.
* reorganized CC changes into a mapped array. created a mapped array for all parameters.
* patch save/load/select into EEPROM
* added optional support for inexpensive 128x64 i2c OLED display
* added support for much-cheaper PT8211 DAC instead of Teensy Audio Shield.

### Useful links
* [Teensy Audio Library](http://www.pjrc.com/teensy/td_libs_Audio.html)
* [Audio System Design Tool for Teensy Audio Library](http://www.pjrc.com/teensy/gui/)
* [MIDI controller number](http://www.indiana.edu/~emusic/cntrlnumb.html)
