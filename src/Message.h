//Message.h
//Gives messages to user
#ifndef _MESSAGE_H
#define _MESSAGE_H

//messages
#define MESSAGEDELAY	5		//controls the delay of the inittial message (if enabled)
#define SWAPDELAYMSG 10

#ifdef UNUSED
EXPORTED void Delay100ms(unsigned long count); // time delay in 0.1 seconds
#endif
EXPORTED void GameOverMessage(void);		   // Prints game over message
#ifndef TEST_WITHOUT_IO
EXPORTED void InitMessage(void);			   // Prints initial message
#endif
EXPORTED void WinMessage(void);
EXPORTED void RestartMessage(void);

#endif
