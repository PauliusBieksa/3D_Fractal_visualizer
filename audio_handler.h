#include "Lib/portaudio/include/portaudio.h"

#define SAMPLE_RATE 44100
#define FRAME_SIZE 256
typedef float SAMPLE;


class Audio_handler
{
public:
	Audio_handler();
	~Audio_handler();

	void initialize_default();
	void initialize_choose_input();

private:
//	typedef int PaStreamCallback(const void *input, void *output, unsigned long frameCount,
//			const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	static int callback(const void *input, void *output, unsigned long frameCount,
		const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	PaStream * stream = nullptr;
	void *data;
	PaStreamParameters input_pars;
	PaStreamParameters output_pars;
	PaError err;
};