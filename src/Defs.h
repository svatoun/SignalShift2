#ifndef __defs_h__
#define __defs_h__

const int debug = 1;            // set to >0 to activate debug output on the serial console. The collector will use delays 200ms and will print stats each 2s
const int debugSensors = 0;
const int debugLow = 0;
const int debugControl = 1;     // debug control commands
const int debugLed = 0;
const int debugMgmt = 0;
const int debugS88 = 0;
const int debugS88Low = 0;
const int debugVirtual = 0;
const int numChannels = 8;      // number of sensors used. Max 5 on Arduino UNO, 8 on Nano.
const int maxVirtualSensors = 16; // maximum number of virtual sensors

#endif
