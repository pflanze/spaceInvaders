//Message.h
//Gives messages to user

#define MESSAGEDELAY	5		//controls the delay of the inittial message (if enabled)

void Delay100ms(unsigned long count); // time delay in 0.1 seconds
void GameOverMessage(void);					//Prints game over message
void InitMessage(void);				//Prints initial message
void WinMessage(void);
void RestartMessage(void);
