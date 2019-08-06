#include "Mixer.hpp"
#include "Channel.hpp"

namespace tsal {

void Mixer::errorCallback( RtAudioError::Type type, const std::string &errorText ) {
  // This example error handling function does exactly the same thing
  // as the embedded RtAudio::error() function.
  std::cout << "in errorCallback" << std::endl;
  if ( type == RtAudioError::WARNING )
    std::cerr << '\n' << errorText << "\n\n";
  else if ( type != RtAudioError::WARNING )
    throw( RtAudioError( errorText, type ) );
}

/* This is the main function that we give to the audio buffer to call for sampling
 * Depending on how it's configured, this will usually get called 44100 per second
 */
int Mixer::streamCallback(void *outputBuffer,
                   __attribute__((unused)) void *inputBuffer,
                   unsigned nBufferFrames, 
                   __attribute__((unused)) double streamTime,
                   RtAudioStreamStatus status,
                   void *data) {
  
  BIT_DEPTH *buffer = (BIT_DEPTH *) outputBuffer;
  Mixer *audio = (Mixer *) data;

  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;

  for (unsigned i = 0; i < nBufferFrames; i++) {
    *buffer++ = (BIT_DEPTH) (audio->getInput() * SCALE);
  }

  return 0;        
}

void Mixer::initalizeStream() {
  if (mRtAudio.getDeviceCount() < 1) {
    std::cout << "\nNo audio devices found!\n";
    exit(1);
  }
  
  unsigned deviceId = mRtAudio.getDefaultOutputDevice(); 
  RtAudio::DeviceInfo info = mRtAudio.getDeviceInfo(deviceId);
  // If the sample rate hasn't been set, use the highest sample rate supported
  if (mSampleRate == 0) {
    mSampleRate = info.sampleRates[info.sampleRates.size() - 1];
  }
  
  mRtAudio.showWarnings(true);

  mBufferFrames = 512;
  RtAudio::StreamParameters oParams;
  oParams.deviceId = deviceId;
  oParams.nChannels = mChannels;
  oParams.firstChannel = 0;

  RtAudio::StreamOptions options;
  // Investigate what exactly these flags do
  //options.flags = RTAUDIO_HOG_DEVICE | RTAUDIO_SCHEDULE_REALTIME;
  
  std::cout << "Sample rate: " << mSampleRate 
            << "\nBuffer frames: " << mBufferFrames
            << std::endl;

  mSequencer.setSampleRate(mSampleRate);
  mSequencer.setBPM(60);
  mSequencer.setPPQ(240);

  // Add a compressor so people don't break their sound cards
  mMaster.add(mCompressor);

  try {
    mRtAudio.openStream(&oParams, NULL, FORMAT, mSampleRate, &mBufferFrames, 
      &streamCallback, (void *)this, &options, &errorCallback);
    mRtAudio.startStream();
  } catch (RtAudioError& e) {
    e.printMessage();
  } 
}

Mixer::Mixer() : mMaster(this), mCompressor(this) {
  mChannels = 1;
  initalizeStream();
}

/**
 * @brief Construct a new Mixer
 * 
 * @param sampleRate if left blank, TSAudio will default to the highest sample rate supported
 */
Mixer::Mixer(unsigned sampleRate)  : mMaster(this), mCompressor(this) {
  mSampleRate = sampleRate;
  // Eventually it would be nice to play in stereo, but that sounds hard for now
  mChannels = 1;
  initalizeStream();
}

Mixer::~Mixer() {
  if (mRtAudio.isStreamOpen())
    mRtAudio.closeStream();
}

double Mixer::getInput() {
  mSequencer.tick();
  return mMaster.getOutput();
}

/**
 * @brief Add a channel 
 *
 * @param channel
 */
void Mixer::add(Channel& channel) {
  mMaster.add(channel);
}

/**
 * @brief Remove a channel
 * @param channel
 */
void Mixer::remove(Channel& channel) {
  mMaster.remove(channel);
}

/**
 * @brief Add an instrument
 * 
 * Add an instrument to the default master channel
 *
 * @param instrument
 */
void Mixer::add(Instrument& instrument) {
  mMaster.add(instrument);
}

/**
 * @brief Remove an instrument
 * 
 * Remove an instrument that was added to the default master channel
 * 
 * @param instrument
 */
void Mixer::remove(Instrument& instrument) {
  mMaster.remove(instrument);
}

/**
 * @brief Add an effect
 * 
 * Add an effect to the default master channel
 *
 * @param effect
 */
void Mixer::add(Effect& effect) {
  mMaster.add(effect);
}


/**
 * @brief Remove an effect
 * 
 * Remove an effect that was added to the default master channel
 * 
 * @param effect
 */
void Mixer::remove(Effect& effect) {
  mMaster.remove(effect);
}


}
