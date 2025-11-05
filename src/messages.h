#ifndef __messages_h__
#define __messages_h__

#include <Arduino.h>

extern void initSharedMessages();

extern const __FlashStringHelper * msg_InvalidMastID;
extern const __FlashStringHelper * msg_InitialMessage;
extern const __FlashStringHelper * msg_InvalidOutput;
extern const __FlashStringHelper * msg_MastNotNotOpened;
extern const __FlashStringHelper * msg_InvalidAspect;

#endif