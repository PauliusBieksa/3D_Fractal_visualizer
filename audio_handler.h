#include "Lib/portaudio/include/portaudio.h"

#define SAMPLE_RATE 44100
typedef float SAMPLE;

class Audio_handler
{
private:
//	typedef int PaStreamCallback(const void *input, void *output, unsigned long frameCount,
//			const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	static int test_callback(const void *input, void *output, unsigned long frameCount,
		const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
	void initialize();

	PaStream * stream;
	void *data;
	PaStreamParameters input_pars;
	PaStreamParameters output_pars;
public:
	Audio_handler();
	~Audio_handler();
};