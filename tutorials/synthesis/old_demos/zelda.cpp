// Started code from 01_SineEnv.cpp
// Song: Zelda's Lubally from Legend of Zelda: Ocarina of Time

#include <cstdio> // for printing to stdout

#include <vector>

// #include <bits/stdc++.h>
//  #include "./stdc++.h"
#include <numeric>

#include "Gamma/Analysis.h"
#include "Gamma/Effects.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/Spatial.h"
#include "Gamma/DFT.h"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/scene/al_PolySynth.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/sound/al_Reverb.hpp"
#include "al/math/al_Random.hpp"

#include "theoryOne.h"

// using namespace gam;

#define FFT_SIZE 4048

using namespace al;

using namespace std;

using namespace theory;

// This example shows how to use SynthVoice and SynthManagerto create an audio
// visual synthesizer. In a class that inherits from SynthVoice you will
// define the synth's voice parameters and the sound and graphic generation
// processes in the onProcess() functions.

/*
Tempos

*/
// Tempo: 104 bpm, 3/4 time signature

theory::Tempo tpo(104, 3, 4);

float wholeNote = tpo.duration(Tempo::whole);
float halfNote = tpo.duration(Tempo::half);
float quarterNote = tpo.duration(Tempo::quarter); 
float eighthNote = tpo.duration(Tempo::eighth);
float sixteenthNote = tpo.duration(Tempo::sixteenth);

float dottedHalfNote = tpo.duration(Tempo::half, true);
float dottedQuarterNote = tpo.duration(Tempo::quarter, true);
float dottedEighthNote = tpo.duration(Tempo::eighth, true);
float dottedSixteenthNote = tpo.duration(Tempo::sixteenth, true);

/*

musical notes used in ocarina

*/

// NOTE: FREQUENCIES

// 7 Scale notes

double G7 = 3135.96;

// 6 Scale notes

double C6 = 1046.50;
double D6 = 1174.66;
double G6 = 1567.98;
double F6 = 1396.91;
double a6 = 1760.00;
double B6 = 1975.53;

// 5 Scale notes
double a5 = 880.00;
double E5 = 659.25;
double D5 = 587.33;
double C5 = 523.25;
double F5 = 698.46;
double G5 = 783.99;
double B5 = 987.77;

// 4 scale notes
double F4 = 349.23;
double a4 = 440.00;
double B4 = 493.88;
double G4 = 392.00;
double E4 = 329.63;
double D4 = 293.66;
double C4 = 261.63;

// 3 Scale notes
double a3 = 220.00;
double B3 = 246.94;
double C3 = 130.81;
double E3 = 164.81;
double D3 = D4 / 2.0;
double G3 = G4 / 2.0;

// 3 Scale Sharp
double C3_sharp = 138.59;
double F3_sharp = 184.99;

// 2 Scale notes
double G2 = 98.00;
double C2 = 65.41;
double a2 = 110.00;
double D2 = D3 / 2.0;
double F2 = F4 / 4.0;
double E2 = E3 / 2.0;
double B2 = B3 / 2.0;
double a2_sharp = 116.54;

// 1 Scale notes

double B1 = B4 / 8.0;
double a1 = a4 / 8.0;
double a1_sharp = 58.27;

// Vector for spinner
Vec3f randomVec3f(float scale)
{
  return Vec3f(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS()) * scale;
}

class SineEnv : public SynthVoice
{
public:
  // Unit generators
  gam::Pan<> mPan;
  gam::Sine<> mOsc;
  // envelope follower to connect audio output to graphics
  gam::EnvFollow<> mEnvFollow;
  gam::Square<> squareOsc;

  Reverb<float> reverb;
  gam::STFT stft = gam::STFT(FFT_SIZE, FFT_SIZE / 4, 0, gam::HANN, gam::MAG_FREQ);

  // gam::ReverbMS<> mReverb;

  gam::Env<3> mAmpEnv;

  // Additional members
  Mesh mMesh;

  // VISUALS: this is for translating effect
  double a_rotate = 0;
  double b_rotate = 0;
  double timepose = 0;
  Vec3f spinner;

  // Mesh circleMesh;

  // Initialize voice. This function will only be called once per voice when
  // it is created. Voices will be reused if they are idle.
  void init() override
  {

    // Intialize envelope
    mAmpEnv.curve(0); // make segments lines
    mAmpEnv.levels(0, 1, 1, 0);
    mAmpEnv.sustainPoint(2); // Make point 2 sustain until a release is issued

    // We have the mesh be a sphere
    // addDisc(mMesh, 1.0);
    addOctahedron(mMesh, 2.0);
    addCircle(mMesh, 0.4);
    // addOctahedron(circleMesh, 2.0);

    // This is a quick way to create parameters for the voice. Trigger
    // parameters are meant to be set only when the voice starts, i.e. they
    // are expected to be constant within a voice instance. (You can actually
    // change them while you are prototyping, but their changes will only be
    // stored and aplied when a note is triggered.)

    createInternalTriggerParameter("amplitude", 0.3, 0.0, 1.0);
    createInternalTriggerParameter("frequency", 60, 20, 5000);
    createInternalTriggerParameter("attackTime", 1.0, 0.01, 3.0);
    createInternalTriggerParameter("releaseTime", 1.0, 0.1, 10.0);
    createInternalTriggerParameter("pan", 0.0, -1.0, 1.0);
    createInternalTriggerParameter("reverberation", 0.5, 0.0, 1.0);
  }

  // The audio processing function
  void onProcess(AudioIOData &io) override
  {
    // Get the values from the parameters and apply them to the corresponding
    // unit generators. You could place these lines in the onTrigger() function,
    // but placing them here allows for realtime prototyping on a running
    // voice, rather than having to trigger a new voice to hear the changes.
    // Parameters will update values once per audio callback because they
    // are outside the sample processing loop.

    // updateFromParameters();

    updateFromParameters();

    mOsc.freq(getInternalParameterValue("frequency"));
    squareOsc.freq(getInternalParameterValue("frequency"));
    mAmpEnv.lengths()[0] = getInternalParameterValue("attackTime");
    mAmpEnv.lengths()[2] = getInternalParameterValue("releaseTime");
    mPan.pos(getInternalParameterValue("pan"));

    while (io())
    {
      /*

        Notes: When I have 5 wave oscilators, there is a white noise

        With 3 sine waves, and one square wave it sounds descent

        With 2 sine waves, it sounds descent

        NOTE:

        When you add more susntain points, the sound pitch increases, but it kinda cuts it short
I also increased the release and attack time to 5 and it sounds a bit smoother

With 4 sustain points, it sounds more of a whistle/ flute

When you increase the number on levels:

  mAmpEnv.levels(0, 1, 1, 0);

  it sounds louder but did not produce poppoing noise ?



// These two wave ocillators sound kinda bad, they produce popping sounds
  gam::SineR<> mOsc;
  gam::SineD<> mOsc;

      */

      float s1 = (mOsc() * mOsc()) * mAmpEnv() * getInternalParameterValue("amplitude");
      float s2;

      // float wet1, wet2;
      auto f1 = stft();

      reverb(s1, f1, s2);

      mEnvFollow(s1);
      mPan(s1, s1, s2);
      io.out(0) += s1;
      io.out(1) += s2;
    }
    // We need to let the synth know that this voice is done
    // by calling the free(). This takes the voice out of the
    // rendering chain
    if (mAmpEnv.done() && (mEnvFollow.value() < 0.001f))
      free();
  }

  // The graphics processing function
  void onProcess(Graphics &g) override
  {

    /*
    NOTES:

      // In this way the color starts as red but then immedialy changes to pink
        g.color(255 - mEnvFollow.value(), 0, mEnvFollow.value() * amplitude * 10, 0.4);




    */

    // Get the paramter values on every video frame, to apply changes to the
    // current instance
    float frequency = getInternalParameterValue("frequency");
    float amplitude = getInternalParameterValue("amplitude");
    float pan = getInternalParameterValue("pan");

    // Translating parameters
    float radius = frequency / 150;
    b_rotate += 1.1;
    timepose -= 0.04;

    // Now draw
    g.pushMatrix();
    // Move x according to frequency, y according to amplitude

    // ( negative x moves it)
    // g.translate(frequency * 1.7 / 200 - 5, amplitude * 2, -8);

    // g.translate(frequency * 1.7 / 200 - 5, frequency / 1000 , -8);

    g.translate(radius * sin(timepose) - 2.5, radius * cos(timepose), -25 + pan);

    // Rotate
    g.rotate(b_rotate, spinner);

    // Scale in the x and y directions according to amplitude
    // g.scale(1 - amplitude, 1 - amplitude, 1);

    g.scale(frequency / 900, frequency / 900, 1);

    // Set the color. Red and Blue according to sound amplitude and Green
    // according to frequency. Alpha fixed to 0.4

    // This produces an after effect
    // g.color(255 - mEnvFollow.value(), 0, mEnvFollow.value() * amplitude * 10, 0.4);
// HSV(frequency / 1000 - spectrum[i] * 100)

    g.color(HSV(frequency/900));

    // g.color(255 - mEnvFollow.value(), frequency / (4000 * mEnvFollow.value()), frequency / (mEnvFollow.value() * 4000), 0.5);

    g.draw(mMesh);
    // g.color(255, 255, 255, 0.5);

    // g.draw(circleMesh);
    g.popMatrix();
  }

  void updateFromParameters()
  {
    reverb.damping(1 - getInternalParameterValue("reverberation")); // Tail decay factor, in [0,1]
  }

  // The triggering functions just need to tell the envelope to start or release
  // The audio processing function checks when the envelope is done to remove
  // the voice from the processing chain.
  void onTriggerOn() override
  {
    mAmpEnv.reset();
    updateFromParameters();

    // Translation stuff
    timepose = 0; // Initiate timeline
    b_rotate = al::rnd::uniform(0, 360);
    spinner = randomVec3f(1);

    reverb.zero();
  }

  void onTriggerOff() override { mAmpEnv.release(); }
};

// Adding the string instrument
// 09 Plucked_string
class PluckedString : public SynthVoice
{
public:
  float mAmp;
  float mDur;
  float mPanRise;
  gam::Pan<> mPan;
  gam::NoiseWhite<> noise;
  gam::Decay<> env;

  // When the moving average increases, it kind of creates a smoothing delay
  gam::MovingAvg<> fil{3};

  gam::Delay<float, gam::ipl::Trunc> delay;
  gam::ADSR<> mAmpEnv;
  gam::EnvFollow<> mEnvFollow;
  gam::Env<2> mPanEnv;
  gam::STFT stft = gam::STFT(FFT_SIZE, FFT_SIZE / 4, 0, gam::HANN, gam::MAG_FREQ);
  // This time, let's use spectrograms for each notes as the visual components.
  Mesh mSpectrogram;
  vector<float> spectrum;
  double a = 0;
  double b = 0;

  double timepose = 10;
  // Additional members
  Mesh mMesh;

  virtual void init() override
  {
    // Declare the size of the spectrum
    spectrum.resize(FFT_SIZE / 2 + 1);
    mSpectrogram.primitive(Mesh::POINTS);
    mAmpEnv.levels(0, 1, 1, 0);
    mPanEnv.curve(4);
    env.decay(0.1);
    delay.maxDelay(1. / 27.5);
    delay.delay(1. / 440.0);

    addDisc(mMesh, 4.0, 30);
    createInternalTriggerParameter("amplitude", 0.1, 0.0, 1.0);
    createInternalTriggerParameter("frequency", 60, 20, 5000);
    createInternalTriggerParameter("attackTime", 0.001, 0.001, 1.0);
    createInternalTriggerParameter("releaseTime", 3.0, 0.1, 10.0);
    createInternalTriggerParameter("sustain", 0.7, 0.0, 1.0);
    createInternalTriggerParameter("Pan1", 0.0, -1.0, 1.0);
    createInternalTriggerParameter("Pan2", 0.0, -1.0, 1.0);
    createInternalTriggerParameter("PanRise", 0.0, 0, 3.0); // range check
  }

  //    void reset(){ env.reset(); }

  float operator()()
  {
    return (*this)(noise() * env());
  }
  float operator()(float in)
  {
    return delay(
        fil(delay() + in));
  }

  virtual void onProcess(AudioIOData &io) override
  {

    while (io())
    {
      mPan.pos(mPanEnv());
      float s1 = (*this)() * mAmpEnv() * mAmp;
      float s2;
      mEnvFollow(s1);
      mPan(s1, s1, s2);
      io.out(0) += s1;
      io.out(1) += s2;
      // STFT for each notes
      if (stft(s1))
      { // Loop through all the frequency bins
        for (unsigned k = 0; k < stft.numBins(); ++k)
        {
          // Here we simply scale the complex sample
          spectrum[k] = tanh(pow(stft.bin(k).real(), 1.3));
        }
      }
    }
    if (mAmpEnv.done() && (mEnvFollow.value() < 0.001))
      free();
  }

  virtual void onProcess(Graphics &g) override
  {
    float frequency = getInternalParameterValue("frequency");
    float amplitude = getInternalParameterValue("amplitude");

    a += 0.25;
    b += 0.29;
    // b += 0.8;

    timepose -= 0.1;

    // Commenting this out makes it have like an after effect
    mSpectrogram.reset();

    // mSpectrogram.primitive(Mesh::LINE_STRIP);

    for (int i = 0; i < FFT_SIZE / 2; i++)
    {
      // mSpectrogram.color(HSV(0.5 - spectrum[i] * 100));

      // This created the effect
      mSpectrogram.color(HSV(frequency / 1000 - spectrum[i] * 100));

      mSpectrogram.vertex(i, spectrum[i], 0.0);
    }
    g.meshColor(); // Use the color in the mesh
    g.pushMatrix();

    g.translate(0, 0, -12);

    g.rotate(a, Vec3f(0, 1, 0));
    // g.rotate(a, Vec3f(1,1,1));

    g.rotate(b, Vec3f(1));
    // g.scale(3.5 * (frequency / 40.0) / FFT_SIZE, frequency * 30, frequency / 50.0);
    g.scale(3.5 * (frequency / 40.0) / FFT_SIZE, frequency *25, frequency/50 );

    g.pointSize(frequency / 20.0);
    // cout <<"This is the frequency" << frequency/50.0 << endl;
    g.draw(mSpectrogram);
    g.popMatrix();
  }

  virtual void onTriggerOn() override
  {
    mAmpEnv.reset();
    timepose = 0;
    updateFromParameters();
    env.reset();
    delay.zero();
    mPanEnv.reset();
  }

  virtual void onTriggerOff() override
  {
    mAmpEnv.triggerRelease();
  }

  void updateFromParameters()
  {
    mPanEnv.levels(getInternalParameterValue("Pan1"),
                   getInternalParameterValue("Pan2"),
                   getInternalParameterValue("Pan1"));
    mPanRise = getInternalParameterValue("PanRise");
    delay.freq(getInternalParameterValue("frequency"));
    mAmp = getInternalParameterValue("amplitude");
    mAmpEnv.levels()[1] = 1.0;
    mAmpEnv.levels()[2] = getInternalParameterValue("sustain");
    mAmpEnv.lengths()[0] = getInternalParameterValue("attackTime");
    mAmpEnv.lengths()[3] = getInternalParameterValue("releaseTime");
    mPanEnv.lengths()[0] = mPanRise;
    mPanEnv.lengths()[1] = mPanRise;
  }
};

// We make an app.
class MyApp : public App
{
public:
  // GUI manager for SineEnv voices
  // The name provided determines the name of the directory
  // where the presets and sequences are stored
  SynthGUIManager<SineEnv> synthManager{"SineEnv"};

  // This function is called right after the window is created
  // It provides a grphics context to initialize ParameterGUI
  // It's also a good place to put things that should
  // happen once at startup.
  void onCreate() override
  {
    navControl().active(false); // Disable navigation via keyboard, since we
                                // will be using keyboard for note triggering

    // Set sampling rate for Gamma objects from app's audio
    gam::sampleRate(audioIO().framesPerSecond());

    imguiInit();

    // Function to start the music

    // startMusic();

    synthManager.synthRecorder().verbose(true);
  }

  // The audio callback function. Called when audio hardware requires data
  void onSound(AudioIOData &io) override
  {
    synthManager.render(io); // Render audio
  }

  void onAnimate(double dt) override
  {
    // The GUI is prepared here
    imguiBeginFrame();
    // Draw a window that contains the synth control panel
    synthManager.drawSynthControlPanel();
    imguiEndFrame();
  }

  // The graphics callback function.
  void onDraw(Graphics &g) override
  {
    g.clear();
    // Render the synth's graphics
    synthManager.render(g);

    // GUI is drawn here
    imguiDraw();
  }

  // Whenever a key is pressed, this function is called
  bool onKeyDown(Keyboard const &k) override
  {
    if (ParameterGUI::usingKeyboard())
    { // Ignore keys if GUI is using
      // keyboard
      return true;
    }

    switch (k.key())
    {
    case 'a':
      std::cout << "Music started" << std::endl;

      startMusic();
      return false;
    }

    return true;
  }

  // Whenever a key is released this function is called
  bool onKeyUp(Keyboard const &k) override
  {
    int midiNote = asciiToMIDI(k.key());
    if (midiNote > 0)
    {
      synthManager.triggerOff(midiNote);
    }
    return true;
  }

  void onExit() override { imguiShutdown(); }

  /*

  Starting added code

  */

  // Function to play individual note

  void playNote(float freq, float time, float duration, float amp = .4, float attack = 0.01, float decay = 0.01)
  {
    auto *voice = synthManager.synth().getVoice<SineEnv>();

    voice->setInternalParameterValue("frequency", freq);
    voice->setInternalParameterValue("amplitude", amp);
    voice->setInternalParameterValue("attackTime", attack);
    voice->setInternalParameterValue("releaseTime", decay);
    voice->setInternalParameterValue("pan", 0.0);

    synthManager.synthSequencer().addVoiceFromNow(voice, time, duration);
  }

  void playString(float freq, float time, float duration, float amp = 0.4)
  {
    auto *voice = synthManager.synth().getVoice<PluckedString>();

    voice->setInternalParameterValue("frequency", freq);
    voice->setInternalParameterValue("amplitude", amp);

    synthManager.synthSequencer().addVoiceFromNow(voice, time, duration);
  }

  void ocaSameTempo(float time, std::vector<double> notes, double tempo, float amp = 0.4)
  {
    for (auto &i : notes)
    {
      playNote(i, time, tempo, amp);
      time += tempo;
    }
  }

  void playOcaSequence(float time, std::vector<double> notes, std::vector<double> tempos, float amp = 0.25)
  {
    for (int i = 0; i < notes.size(); i++)
    {
      playNote(notes[i], time, tempos[i], amp);
      time += tempos[i];
    }
  }

  void playStringSequence(float time, std::vector<double> notes, std::vector<double> tempos, float amp = 0.2)
  {
    for (int i = 0; i < notes.size(); i++)
    {
      playString(notes[i], time, tempos[i], amp);
      time += tempos[i];
    }
  }

  void playOcarina()
  {

    /*
    Parameters for playNote function:


    playNote(
      float freq, // Frequency of the note
      float time, // Time to start playing the note
      float duration, // Duration of the note
      float amp = .2, // Amplitude of the note
      float attack = 0.01, // Attack time of the note
      float decay = 0.01 // Decay time of the note
    );

    */

    double delay = 0;

    // piano chorus for Zelda's lubally from The Legend of Zelda Ocarina of Time

    // It has 17 "parts" in total
    std::vector<double> notes_1_4 = {B4, D5, a4, G4, a4, B4, D5, a4, B4, D5, a5, G5};

    std::vector<double> tempos_1_4 = {halfNote, quarterNote,
                                      halfNote, eighthNote, eighthNote,
                                      halfNote, quarterNote,
                                      dottedHalfNote,
                                      halfNote, quarterNote,
                                      halfNote, quarterNote};

    vector<double> notes_5_8 = {D5, C5, B4, a4, G4, a4, B4, D5, a4, B4, D5, a5};

    vector<double> tempos_5_8 = {halfNote, eighthNote, eighthNote,
                                 halfNote, eighthNote, eighthNote,
                                 halfNote, quarterNote,
                                 dottedHalfNote,
                                 halfNote, quarterNote,
                                 dottedHalfNote};

    // There is another note that has to be played at the same time
    vector<double> notes_9_12 = {B4, D5, a5, G5, D6, G5,
                                 a5, B5,
                                 C6, D6,
                                 C6, B5, C6, B5};

    // On tempo 10, 11, 13
    // vector<double> doubleNotes_9_12 = {D5, C5, B4, C5, B4};

    // vector<double> doubleDelay_9_12 = {halfNote, quarterNote,
    //                                    halfNote, quarterNote,
    //                                    dottedHalfNote + halfNote, sixteenthNote, sixteenthNote, sixteenthNote, sixteenthNote};

    // double doubleTempo = accumulate(doubleDelay_9_12.begin(), doubleDelay_9_12.end(), 0.0);

    // vector<double> doubleTempos_9_12 = {halfNote, eighthNote, eighthNote, eighthNote, eighthNote};

    vector<double> tempos_9_12 = {halfNote, quarterNote,
                                  halfNote, quarterNote,
                                  dottedHalfNote, sixteenthNote, sixteenthNote, sixteenthNote, sixteenthNote,
                                  halfNote, eighthNote, eighthNote,
                                   eighthNote, eighthNote};

    double tempos_1_4_time = accumulate(tempos_1_4.begin(), tempos_1_4.end(), 0.0);

    double tempos_5_8_time = accumulate(tempos_5_8.begin(), tempos_5_8.end(), 0.0);

    double tempos_9_12_time = accumulate(tempos_9_12.begin(), tempos_9_12.end(), 0.0);

    vector<double> notes_13_17 = {G5, C6, B5, a5, B5, a5, E5, D6, C6, B5, C6, B5, G5, C6, G6, a5, C6, F6, a6, D6, B6};

    vector<double> tempos_13_17 = {halfNote,
                                         halfNote, eighthNote, eighthNote,
                                         eighthNote, eighthNote, halfNote,
                                         halfNote, eighthNote, eighthNote,
                                         eighthNote, eighthNote, quarterNote, quarterNote,
                                         wholeNote,
                                         eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote};

    playOcaSequence(delay, notes_1_4, tempos_1_4);

    delay += tempos_1_4_time;

    playOcaSequence(delay, notes_5_8, tempos_5_8);

    delay += tempos_5_8_time;

    // doubleTempo += tempos_1_4_time + tempos_5_8_time;

    // Double notes
    // playOcaSequence(doubleTempo, doubleNotes_9_12, doubleTempos_9_12);

    playOcaSequence(delay, notes_9_12, tempos_9_12);

    delay += tempos_9_12_time;

    playOcaSequence(delay, notes_13_17, tempos_13_17);
  }

  void playStringInstrument()
  {
    double delay = 0;

    vector<double> stringSeqNotes1 = {C2, G2, E3, C2, a2, F3_sharp, C2, G2, E3, C2, a2, F3_sharp, B1, D3, a3, a1_sharp, C3_sharp, G3};
    vector<double> stringSeqTempos1 = {
        eighthNote,
        eighthNote,
        halfNote,
        eighthNote,
        eighthNote,
        halfNote,
        eighthNote,
        eighthNote,
        halfNote,
        eighthNote,
        eighthNote,
        halfNote,
        eighthNote,
        eighthNote,
        halfNote,
        eighthNote,
        eighthNote,
        halfNote,
    };

    vector<double> stringSeqNotes2 = {
        a1, C3, G3, D2, a2, F3_sharp, C2, G2, E3, C2, a2, F3_sharp, C2, G2, E3, C2, a2, F3_sharp};

    vector<double> stringSeqTempos2 = stringSeqTempos1;

    vector<double> stringSeqNotes3 = {B1, D3, a3, a1_sharp, C3_sharp, G3, a1, C3, G3, D2, a2, F3_sharp, F2, a2, C3, E3, C3, a2, E2, G2};

    vector<double> stringSeqNotes4 = {B2, D3, B2, G2, D2, F2, a2, C3, a2, F2, C2, E2, G2, B2, G2, E2, F2, a2, C3, E3, C3, a2, E2, G2, B2, D3, B2, G2, D2, G2, a2_sharp, C3_sharp, a2_sharp, G2, a2, G2};

    vector<double> stringSeqTempos3 = {
        eighthNote, eighthNote, halfNote,
        eighthNote, eighthNote, halfNote,
        eighthNote, eighthNote, halfNote,
        eighthNote, eighthNote, halfNote,
        eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote};

    vector<double> stringSeqTempos4 = {
        eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote,
        eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote,
        eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote,
        eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote,
        eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote,
        eighthNote, eighthNote, eighthNote, eighthNote, eighthNote, eighthNote,
        halfNote, quarterNote + dottedHalfNote};

    double tempos_1_time = accumulate(stringSeqTempos1.begin(), stringSeqTempos1.end(), 0.0);

    double tempos_2_time = accumulate(stringSeqTempos2.begin(), stringSeqTempos2.end(), 0.0);

    double tempos_3_time = accumulate(stringSeqTempos3.begin(), stringSeqTempos3.end(), 0.0);

    // playString(C2, delay, eighthNote);
    playStringSequence(delay, stringSeqNotes1, stringSeqTempos1);
    delay += tempos_1_time;

    playStringSequence(delay, stringSeqNotes2, stringSeqTempos2);
    delay += tempos_2_time;

    playStringSequence(delay, stringSeqNotes3, stringSeqTempos3);
    delay += tempos_3_time;

    playStringSequence(delay, stringSeqNotes4, stringSeqTempos4);
  }

  void startMusic()
  {
    // playSnareBackground();
    playOcarina();
    playStringInstrument();
  }

  /*
    Added code ends here
  */
};

int main()
{
  // Create app instance
  MyApp app;

  // Set up audio
  app.configureAudio(48000., 512, 2, 0);

  app.start();
  return 0;
}