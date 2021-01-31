#ifndef SYNTHARCH_H
#define SYNTHARCH_H

#include <Audio.h>

// GUItool: begin automatically generated code
#ifdef TEENSY_DAC_PT8211
AudioOutputPT8211        i2s1;           //xy=1144.5,479.00000286102295
#else
AudioOutputI2S           i2s1;           //xy=1144.5,479.00000286102295
AudioControlSGTL5000     sgtl5000_1;     //xy=1064.5000076293945,265.00003576278687
#endif

AudioSynthNoiseWhite		 noise1;
AudioSynthNoiseWhite		 noise2;
AudioSynthNoiseWhite		 noise3;
AudioSynthNoiseWhite		 noise4;
AudioSynthNoiseWhite		 noise5;
AudioSynthNoiseWhite		 noise6;
AudioSynthNoiseWhite		 noise7;
AudioSynthNoiseWhite		 noise8;
AudioSynthWaveform       waveform11;      //xy=72.5,24.0000057220459
AudioSynthWaveform       waveform12;     //xy=70.50000762939453,135.99996280670166
AudioSynthWaveform       waveform13;     //xy=77.99999237060547,247.00001525878906
AudioSynthWaveform       waveform14;     //xy=70.99999237060547,350.99999141693115
AudioSynthWaveform       waveform15;     //xy=70.00000762939453,465.0000023841858
AudioSynthWaveform       waveform16;     //xy=70,580.0000038146973
AudioSynthWaveform       waveform17;     //xy=69.00000953674316,686.999933719635
AudioSynthWaveform       waveform18;     //xy=68,790.9999542236328

AudioSynthWaveform       waveform21;      //xy=73.49999618530273,59.00000762939453
AudioSynthWaveform       waveform22;      //xy=74.49999237060547,171.99998569488525
AudioSynthWaveform       waveform23;      //xy=80.00000762939453,282.9999885559082
AudioSynthWaveform       waveform24;      //xy=74,383.99999237060547
AudioSynthWaveform       waveform25;      //xy=75,500.0000009536743
AudioSynthWaveform       waveform26;      //xy=70.99998092651367,613.9999856948853
AudioSynthWaveform       waveform27;      //xy=72.9999771118164,717.9999895095825
AudioSynthWaveform       waveform28;      //xy=70.99994659423828,822.9999303817749

AudioSynthWaveform       waveform31;      //xy=73.49999618530273,59.00000762939453
AudioSynthWaveform       waveform32;      //xy=74.49999237060547,171.99998569488525
AudioSynthWaveform       waveform33;      //xy=80.00000762939453,282.9999885559082
AudioSynthWaveform       waveform34;      //xy=74,383.99999237060547
AudioSynthWaveform       waveform35;      //xy=75,500.0000009536743
AudioSynthWaveform       waveform36;      //xy=70.99998092651367,613.9999856948853
AudioSynthWaveform       waveform37;      //xy=72.9999771118164,717.9999895095825
AudioSynthWaveform       waveform38;      //xy=70.99994659423828,822.9999303817749

AudioSynthWaveformDc     dc2;            //xy=80.49999237060547,206.9999761581421
AudioSynthWaveformDc     dc4;            //xy=80.99999618530273,421.99994373321533
AudioSynthWaveformDc     dc7;            //xy=79.99999237060547,748.999924659729
AudioSynthWaveformDc     dc5;            //xy=80.99999237060547,535.9999618530273
AudioSynthWaveformDc     dc1;            //xy=83.49990844726562,97.00000953674316
AudioSynthWaveformDc     dc6;            //xy=81.99999618530273,645.9999628067017
AudioSynthWaveformDc     dc8;            //xy=81.99999237060547,854.9999151229858
AudioSynthWaveformDc     dc3;            //xy=90,316.00000381469727
AudioEffectEnvelope      filt_env_2;     //xy=209.50001525878906,205.9999761581421
AudioMixer4              mixer14;        //xy=209.99999237060547,593.999960899353
AudioMixer4              mixer10;        //xy=213.5,154.99989414215088
AudioMixer4              mixer9;         //xy=214.50000762939453,46.999969482421875
AudioMixer4              mixer16;        //xy=212.9999771118164,804.9999132156372
AudioMixer4              mixer15;        //xy=214.00001525878906,703.0000162124634
AudioEffectEnvelope      filt_env_7;     //xy=215.00000762939453,751.999997138977
AudioMixer4              mixer11;        //xy=218.9999885559082,260.9999122619629
AudioEffectEnvelope      filt_env_1;     //xy=220.49989318847656,97.99999618530273
AudioMixer4              mixer13;        //xy=218.9999771118164,478.99992084503174
AudioEffectEnvelope      filt_env_6;     //xy=220.00000762939453,643.0000257492065
AudioEffectEnvelope      filt_env_5;     //xy=221,528.0000247955322
AudioEffectEnvelope      filt_env_8;     //xy=220.00000762939453,852.999979019165
AudioEffectEnvelope      filt_env_3;     //xy=223.00001525878906,311.99995517730713
AudioMixer4              mixer12;        //xy=223.0000228881836,363.9998998641968
AudioEffectEnvelope      filt_env_4;     //xy=223.00001525878906,416.0000009536743
AudioFilterStateVariable filter6;        //xy=346.99998474121094,609.9999704360962
AudioFilterStateVariable filter4;        //xy=347.9999809265137,373.0000123977661
AudioFilterStateVariable filter7;        //xy=346.99998474121094,724.9999704360962
AudioFilterStateVariable filter5;        //xy=349.99996185302734,496.99998569488525
AudioFilterStateVariable filter3;        //xy=351,260.99998474121094
AudioFilterStateVariable filter8;        //xy=348.99998474121094,819.9999723434448
AudioFilterStateVariable filter2;        //xy=352.49997329711914,185.99996662139893
AudioFilterStateVariable filter1;        //xy=363.5000190734863,85.00000190734863
AudioMixer4              mixer3;         //xy=479.9999694824219,260.9999885559082
AudioMixer4              mixer2;         //xy=481.49996185302734,189.99999141693115
AudioMixer4              mixer4;         //xy=481.9999694824219,370.0000009536743
AudioMixer4              mixer7;         //xy=481.9999694824219,722.0000066757202
AudioMixer4              mixer5;         //xy=483.9999694824219,496.0000333786011
AudioMixer4              mixer8;         //xy=484.9999694824219,822.0000085830688
AudioMixer4              mixer6;         //xy=485.9999694824219,608.000002861023
AudioMixer4              mixer1;         //xy=495.49996185302734,85.00000381469727
AudioEffectEnvelope      envelope2;      //xy=620.5000305175781,189.99997806549072
AudioEffectEnvelope      envelope4;      //xy=620.0000381469727,367.00000190734863
AudioEffectEnvelope      envelope7;      //xy=619.0000381469727,720.0000066757202
AudioEffectEnvelope      envelope6;      //xy=621.0000381469727,606.0000047683716
AudioEffectEnvelope      envelope3;      //xy=623.0000381469727,260.00000381469727
AudioEffectEnvelope      envelope5;      //xy=623.0000381469727,494.0000333786011
AudioEffectEnvelope      envelope8;      //xy=622.0000381469727,818.000039100647
AudioEffectEnvelope      envelope1;      //xy=633.5000305175781,81.0000057220459
AudioMixer4              envmixer2;      //xy=779.0000228881836,316.0000057220459
AudioMixer4              envmixer4;      //xy=781.0000228881836,761.0000066757202
AudioMixer4              envmixer3;      //xy=783.0000228881836,544.000002861023
AudioMixer4              envmixer1;      //xy=800.5000152587891,149.99999809265137
AudioMixer4              mixerL;         //xy=966.5,362.99997901916504
AudioMixer4              mixerR;         //xy=989.4999885559082,599.0000076293945

AudioConnection					 patchNoise1(noise1, 0, mixer9, 3);
AudioConnection					 patchNoise2(noise2, 0, mixer10, 3);
AudioConnection					 patchNoise3(noise3, 0, mixer11, 3);
AudioConnection					 patchNoise4(noise4, 0, mixer12, 3);
AudioConnection					 patchNoise5(noise5, 0, mixer13, 3);
AudioConnection					 patchNoise6(noise6, 0, mixer14, 3);
AudioConnection					 patchNoise7(noise7, 0, mixer15, 3);
AudioConnection					 patchNoise8(noise8, 0, mixer16, 3);

AudioConnection          wavepatchCord11(waveform11, 0, mixer9, 0);
AudioConnection          wavepatchCord12(waveform12, 0, mixer10, 0);
AudioConnection          wavepatchCord13(waveform13, 0, mixer11, 0);
AudioConnection          wavepatchCord14(waveform14, 0, mixer12, 0);
AudioConnection          wavepatchCord15(waveform15, 0, mixer13, 0);
AudioConnection          wavepatchCord16(waveform16, 0, mixer14, 0);
AudioConnection          wavepatchCord17(waveform17, 0, mixer15, 0);
AudioConnection          wavepatchCord18(waveform18, 0, mixer16, 0);

AudioConnection          wavepatchCord21(waveform21, 0, mixer9, 1);
AudioConnection          wavepatchCord22(waveform22, 0, mixer10, 1);
AudioConnection          wavepatchCord23(waveform23, 0, mixer11, 1);
AudioConnection          wavepatchCord24(waveform24, 0, mixer12, 1);
AudioConnection          wavepatchCord25(waveform25, 0, mixer13, 1);
AudioConnection          wavepatchCord26(waveform26, 0, mixer14, 1);
AudioConnection          wavepatchCord27(waveform27, 0, mixer15, 1);
AudioConnection          wavepatchCord28(waveform28, 0, mixer16, 1);

AudioConnection          wavepatchCord31(waveform31, 0, mixer9, 2);
AudioConnection          wavepatchCord32(waveform32, 0, mixer10, 2);
AudioConnection          wavepatchCord33(waveform33, 0, mixer11, 2);
AudioConnection          wavepatchCord34(waveform34, 0, mixer12, 2);
AudioConnection          wavepatchCord35(waveform35, 0, mixer13, 2);
AudioConnection          wavepatchCord36(waveform36, 0, mixer14, 2);
AudioConnection          wavepatchCord37(waveform37, 0, mixer15, 2);
AudioConnection          wavepatchCord38(waveform38, 0, mixer16, 2);

AudioConnection          patchCord17(dc2, filt_env_2);
AudioConnection          patchCord18(dc4, filt_env_4);
AudioConnection          patchCord19(dc7, filt_env_7);
AudioConnection          patchCord20(dc5, filt_env_5);
AudioConnection          patchCord21(dc1, filt_env_1);
AudioConnection          patchCord22(dc6, filt_env_6);
AudioConnection          patchCord23(dc8, filt_env_8);
AudioConnection          patchCord24(dc3, filt_env_3);
AudioConnection          patchCord25(filt_env_2, 0, filter2, 1);
AudioConnection          patchCord26(mixer14, 0, filter6, 0);
AudioConnection          patchCord27(mixer14, 0, mixer6, 3);
AudioConnection          patchCord28(mixer10, 0, filter2, 0);
AudioConnection          patchCord29(mixer10, 0, mixer2, 3);
AudioConnection          patchCord30(mixer9, 0, filter1, 0);
AudioConnection          patchCord31(mixer9, 0, mixer1, 3);
AudioConnection          patchCord32(mixer16, 0, filter8, 0);
AudioConnection          patchCord33(mixer16, 0, mixer8, 3);
AudioConnection          patchCord34(mixer15, 0, filter7, 0);
AudioConnection          patchCord35(mixer15, 0, mixer7, 3);
AudioConnection          patchCord36(filt_env_7, 0, filter7, 1);
AudioConnection          patchCord37(mixer11, 0, filter3, 0);
AudioConnection          patchCord38(mixer11, 0, mixer3, 3);
AudioConnection          patchCord39(filt_env_1, 0, filter1, 1);
AudioConnection          patchCord40(mixer13, 0, filter5, 0);
AudioConnection          patchCord41(mixer13, 0, mixer5, 3);
AudioConnection          patchCord42(filt_env_6, 0, filter6, 1);
AudioConnection          patchCord43(filt_env_5, 0, filter5, 1);
AudioConnection          patchCord44(filt_env_8, 0, filter8, 1);
AudioConnection          patchCord45(filt_env_3, 0, filter3, 1);
AudioConnection          patchCord46(mixer12, 0, filter4, 0);
AudioConnection          patchCord47(mixer12, 0, mixer4, 3);
AudioConnection          patchCord48(filt_env_4, 0, filter4, 1);
AudioConnection          patchCord49(filter6, 0, mixer6, 0);
AudioConnection          patchCord50(filter6, 1, mixer6, 1);
AudioConnection          patchCord51(filter6, 2, mixer6, 2);
AudioConnection          patchCord52(filter4, 0, mixer4, 0);
AudioConnection          patchCord53(filter4, 1, mixer4, 1);
AudioConnection          patchCord54(filter4, 2, mixer4, 2);
AudioConnection          patchCord55(filter7, 0, mixer7, 0);
AudioConnection          patchCord56(filter7, 1, mixer7, 1);
AudioConnection          patchCord57(filter7, 2, mixer7, 2);
AudioConnection          patchCord58(filter5, 0, mixer5, 0);
AudioConnection          patchCord59(filter5, 1, mixer5, 1);
AudioConnection          patchCord60(filter5, 2, mixer5, 2);
AudioConnection          patchCord61(filter3, 0, mixer3, 0);
AudioConnection          patchCord62(filter3, 1, mixer3, 1);
AudioConnection          patchCord63(filter3, 2, mixer3, 2);
AudioConnection          patchCord64(filter8, 0, mixer8, 0);
AudioConnection          patchCord65(filter8, 1, mixer8, 1);
AudioConnection          patchCord66(filter8, 2, mixer8, 2);
AudioConnection          patchCord67(filter2, 0, mixer2, 0);
AudioConnection          patchCord68(filter2, 1, mixer2, 1);
AudioConnection          patchCord69(filter2, 2, mixer2, 2);
AudioConnection          patchCord70(filter1, 0, mixer1, 0);
AudioConnection          patchCord71(filter1, 1, mixer1, 1);
AudioConnection          patchCord72(filter1, 2, mixer1, 2);
AudioConnection          patchCord73(mixer3, envelope3);
AudioConnection          patchCord74(mixer3, 0, envmixer2, 2);
AudioConnection          patchCord75(mixer2, envelope2);
AudioConnection          patchCord76(mixer2, 0, envmixer1, 3);
AudioConnection          patchCord77(mixer4, envelope4);
AudioConnection          patchCord78(mixer4, 0, envmixer2, 3);
AudioConnection          patchCord79(mixer7, envelope7);
AudioConnection          patchCord80(mixer7, 0, envmixer4, 2);
AudioConnection          patchCord81(mixer5, envelope5);
AudioConnection          patchCord82(mixer5, 0, envmixer3, 2);
AudioConnection          patchCord83(mixer8, envelope8);
AudioConnection          patchCord84(mixer8, 0, envmixer4, 3);
AudioConnection          patchCord85(mixer6, envelope6);
AudioConnection          patchCord86(mixer6, 0, envmixer3, 3);
AudioConnection          patchCord87(mixer1, envelope1);
AudioConnection          patchCord88(mixer1, 0, envmixer1, 2);
AudioConnection          patchCord89(envelope2, 0, envmixer1, 1);
AudioConnection          patchCord90(envelope4, 0, envmixer2, 1);
AudioConnection          patchCord91(envelope7, 0, envmixer4, 0);
AudioConnection          patchCord92(envelope6, 0, envmixer3, 1);
AudioConnection          patchCord93(envelope3, 0, envmixer2, 0);
AudioConnection          patchCord94(envelope5, 0, envmixer3, 0);
AudioConnection          patchCord95(envelope8, 0, envmixer4, 1);
AudioConnection          patchCord96(envelope1, 0, envmixer1, 0);
AudioConnection          patchCord97(envmixer2, 0, mixerL, 1);
AudioConnection          patchCord98(envmixer2, 0, mixerR, 1);
AudioConnection          patchCord99(envmixer4, 0, mixerL, 3);
AudioConnection          patchCord100(envmixer4, 0, mixerR, 3);
AudioConnection          patchCord101(envmixer3, 0, mixerL, 2);
AudioConnection          patchCord102(envmixer3, 0, mixerR, 2);
AudioConnection          patchCord103(envmixer1, 0, mixerL, 0);
AudioConnection          patchCord104(envmixer1, 0, mixerR, 0);
AudioConnection          patchCord105(mixerL, 0, i2s1, 0);
AudioConnection          patchCord106(mixerR, 0, i2s1, 1);
// GUItool: end automatically generated code


#endif
