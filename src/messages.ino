#include <Arduino.h>

const __FlashStringHelper * msg_InvalidMastID;
const __FlashStringHelper * msg_InitialMessage;
const __FlashStringHelper * msg_InvalidOutput;
const __FlashStringHelper * msg_MastNotNotOpened;
const __FlashStringHelper * msg_InvalidAspect;

void initSharedMessages() {
    msg_InvalidMastID = F("Bad mast ID ");
    msg_InvalidOutput = F("Bad output ID ");
    msg_MastNotNotOpened = F("Mast not opened");
    msg_InvalidAspect = F("Bad aspect ");
    msg_InitialMessage = F("Copyright (c) 2018, Petr Sidlo <sidlo64@seznam.cz>\nCopyright (c) 2022, Svatopluk Dedic <svatopluk.dedic@gmail.com>, APL 2.0 License");
}