#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#define REGISTER_CMD(cmdRef, callback, before) XPLMRegisterCommandHandler(cmdRef, callback, before, (void*)0)

#define UNREGISTER_CMD(cmdRef, callback) XPLMUnregisterCommandHandler(cmdRef, callback, 0, 0)

// nws.h - Nose Wheel Steering Commands

#endif