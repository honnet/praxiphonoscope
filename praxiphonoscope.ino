#include <ADC.h>  // Teensy 3.1 uncomment this line and install http://github.com/pedvide/ADC
#include <MozziGuts.h>
#include <Sample.h> // Sample template
#include <samples/burroughs1_18649_int8.h>
#include <mozzi_rand.h>
#include <Line.h>
#include <Smooth.h>

// use: Sample <table_size, update_rate, interpolation > SampleName (wavetable)
Sample <BURROUGHS1_18649_NUM_CELLS, AUDIO_RATE, INTERP_LINEAR> aSample(BURROUGHS1_18649_DATA);

//Line to scrub through sample at audio rate, Line target is set at control rate
Line <Q16n16> scrub; // Q16n16 fixed point for high precision

// the number of audio steps the line has to take to reach the next offset
const unsigned int AUDIO_STEPS_PER_CONTROL = AUDIO_RATE / CONTROL_RATE;

int offset = 0;
int offset_advance = -333; // just a guess

// use this smooth out the wandering/jumping rate of scrubbing, gives more convincing reverses
float smoothness = 0.9f;
Smooth <int> kSmoothOffsetAdvance(smoothness);


void setup(){
    randSeed(); // fresh randomness
    aSample.setLoopingOn();
    startMozzi();

    Serial.begin(9600);
    pinMode(13, OUTPUT);
}

///////////////////////////////////////////////////////////////////////////////
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include "Encoder_pullDown.h"

Encoder myEnc(22, 23);
long position = -999;
long newPos;

///////////////////////////////////////////////////////////////////////////////

void updateControl(){

#if 0

    int delta = 0;
    if (newPos != position)
    {
        digitalWrite(13, !digitalRead(13));

        delta = position - newPos;
        delta = map(delta, -999, 999, -333, 333);

        position = newPos;
        Serial.print(delta);
        Serial.print(" ");
        Serial.println(position);
    }

    int smooth_offset_advance = kSmoothOffsetAdvance.next(delta);

    offset += smooth_offset_advance;

#else // simulation

    static int delta = 500;
    delta = delta ? delta-10 : 500;
    offset += delta;

    Serial.print(delta);
    Serial.print(" ");
    Serial.println(offset);

#endif

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




