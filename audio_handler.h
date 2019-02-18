#include "Lib/portaudio/include/portaudio.h"

class Audio_handler
{
private:
	int PaStreamCallback(const void *input, void *output, unsigned long frameCount,
			const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
public:
	Audio_handler();
	~Audio_handler();
};