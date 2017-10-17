//Message.h
//Gives messages to user


//messages
#define MESSAGEDELAY	5		//controls the delay of the inittial message (if enabled)
#define SWAPDELAYMSG 10

void Delay100ms(unsigned long count); // time delay in 0.1 seconds
void GameOverMessage(void);					//Prints game over message
void InitMessage(void);				//Prints initial message
void WinMessage(void);
void RestartMessage(void);
