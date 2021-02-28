
#define ARTWORK_FIRE		1		// Artwork display fires after this amount of timeout after the last user input

long int xclock();
void timers_Print(long int start, long int end, char* name, int enabled);
int timers_FireArt(long int last);
