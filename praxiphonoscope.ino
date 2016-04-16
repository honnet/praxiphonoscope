//#include <ADC.h>  // Teensy 3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <MozziGuts.h>
#include <Sample.h> // Sample template
#include <samples/burroughs1_18649_int8.h>
#include <mozzi_rand.h>
#include <Line.h>
#include <Smooth.h>
//#define CONTROL_RATE 128

// use: Sample <table_size, update_rate, interpolation > SampleName (wavetable)
Sample <BURROUGHS1_18649_NUM_CELLS, AUDIO_RATE, INTERP_LINEAR> aSample(BURROUGHS1_18649_DATA);

//Line to scrub through sample at audio rate, Line target is set at control rate
Line <Q16n16> scrub; // Q16n16 fixed point for high precision

// the number of audio steps the line has to take to reach the next offset
const unsigned int AUDIO_STEPS_PER_CONTROL = AUDIO_RATE / CONTROL_RATE;

int offset = 0;
int offset_advance = -333; // just a guess

// use this smooth out the wandering/jumping rate of scrubbing, gives more convincing reverses
float smoothness = 0.9;
Smooth <int> kSmoothOffsetAdvance(smoothness);

#define DEBUG_PRINT 1

void setup(){
    randSeed(); // fresh randomness
    aSample.setLoopingOn();
    startMozzi();

#if DEBUG_PRINT > 0
    Serial.begin(115200);
#endif
    pinMode(13, OUTPUT);
}

///////////////////////////////////////////////////////////////////////////////
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include "Encoder_pullDown.h"

Encoder myEnc(2, 3);
long position = 0;
long newPos = 0;

///////////////////////////////////////////////////////////////////////////////

void updateControl(){

    int delta = 0;
    int smooth_offset_advance = 0;

    if (newPos != position)
    {
        digitalWrite(13, !digitalRead(3));

        delta = position - newPos;
        delta *= 60;

        smooth_offset_advance = kSmoothOffsetAdvance.next(delta);
        if (abs(smooth_offset_advance) < 10)
            smooth_offset_advance = 0;

#if DEBUG_PRINT > 0
        Serial.print("-900 900 ");
        Serial.print(smooth_offset_advance);
        Serial.print(" ");
        Serial.println(delta);
#endif
        position = newPos;
    }

    offset += smooth_offset_advance;

    // keep offset in range
    if (offset >= BURROUGHS1_18649_NUM_CELLS) offset -= BURROUGHS1_18649_NUM_CELLS;
    if (offset < 0) offset += BURROUGHS1_18649_NUM_CELLS;

    // set new target for interpolating line to scrub to
    scrub.set(Q16n0_to_Q16n16(offset), AUDIO_STEPS_PER_CONTROL);

}


int updateAudio(){
    newPos = myEnc.read();

    unsigned int index = Q16n16_to_Q16n0(scrub.next());
    return aSample.atIndex(index);
}


void loop(){
    audioHook();
}




