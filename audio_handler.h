#include "portaudio.h"
#include "Lib/portaudio/src/common/pa_ringbuffer.h"
#include "Lib/portaudio/src/common/pa_util.h"
#include <vector>

#define SAMPLE_RATE 44100
#define FRAME_SIZE 256
typedef float SAMPLE;



struct ring_buffer
{
	float frame[256];

	PaUtilRingBuffer rb_incoming;
	void *rb_incoming_data;
};


class Audio_handler
{
public:
	Audio_handler();
	~Audio_handler();

	void initialize_default();
	void initialize_choose_input();
	std::vector<float> update();// { PaUtil_ReadRingBuffer(&rb.rb_incoming, rb.frame, 256); return &rb.frame; }

private:
	static int callback(const void *input, void *output, unsigned long frameCount,
		const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	ring_buffer rb = { 0 };
	PaStream *stream = nullptr;
	void *data;
	PaStreamParameters input_pars;
	PaStreamParameters output_pars;
	PaError err;
};