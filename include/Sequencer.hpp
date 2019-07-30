#ifndef MIDISEQUENCER_H
#define MIDISEQUENCER_H

#include "Util.hpp"
#include <condition_variable>
#include <functional>
#include <vector>
#include <algorithm>

namespace tsal {

/** @class Sequencer
 * @brief A sequencer that handles the scheduling of events
 * 
 * Sequencer handles the real-time scheduling of the process
 * based upon BPM and PPQ parameters
 */
class Sequencer {
  public:
    void tick();
    void start();
    void stop();
    void setSampleRate(unsigned sampleRate) { mSampleRate = sampleRate; };
    void setTick(unsigned tick);
    void setBPM(unsigned bpm);
    void setPPQ(unsigned ppq);
    void schedule(std::function<void ()> callback, Timing::NoteScale scale, unsigned duration = 1);
    unsigned getTick() const;
    unsigned getTicksInNote(Timing::NoteScale note) const;
    void waitForTick(unsigned tick);
    struct Event {
        Event(unsigned t, std::function<void ()> f) {
          tick = t;
          callback = f;
        }
        unsigned tick;
        std::function<void ()> callback;
    };
  private:
    std::mutex mMutex; 
    std::condition_variable mCondition;
    std::vector<unsigned> mTickEvents; // Replace with mEvents eventually
    std::vector<std::unique_ptr<Event>> mEvents;
    void setSamplesPerTick();
    unsigned mSampleTime = 0;
    unsigned mTick = 0;


    unsigned mPPQ = 240;
    unsigned mBPM = 100;
    unsigned mSampleRate = 0;
    double mSamplesPerTick;

    static Util::ParameterRange<unsigned> mBPMRange;
    static Util::ParameterRange<unsigned> mPPQRange;
};

}
#endif
