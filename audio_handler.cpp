#include "audio_handler.h"
#include <iostream>



Audio_handler::Audio_handler()
{
	PaError err;
	err = Pa_Initialize();
	if (err != paNoError)
		std::cout << "PortAudio initialization failed" << std::endl
		<< Pa_GetErrorText(err) << std::endl;
}

Audio_handler::~Audio_handler()
{
	PaError err;
	err = Pa_CloseStream(stream);
	if (err != paNoError)
		std::cout << "Failed to close stream" << std::endl;

	err = Pa_Terminate();
	if (err != paNoError)
		std::cout << "PortAudio termination failed" << std::endl
		<< Pa_GetErrorText(err) << std::endl;
}

void Audio_handler::initialize()
{
	PaError err;

	input_pars.device = Pa_GetDefaultInputDevice(); // Default device
	if (input_pars.device == paNoDevice)
	{
		std::cout << "No default input device" << std::endl;
	}
	input_pars.channelCount = 2; // stereo input
	input_pars.sampleFormat = paFloat32;
	input_pars.suggestedLatency = Pa_GetDeviceInfo(input_pars.device)->defaultLowInputLatency;
	input_pars.hostApiSpecificStreamInfo = NULL;

	output_pars.device = Pa_GetDefaultOutputDevice(); // default device
	if (output_pars.device == paNoDevice)
	{
		std::cout << "No default output device" << std::endl;
	}
	output_pars.channelCount = 2; // Stereo output
	output_pars.sampleFormat = paFloat32;
	output_pars.suggestedLatency = Pa_GetDeviceInfo(output_pars.device)->defaultLowOutputLatency;
	output_pars.hostApiSpecificStreamInfo = NULL;

	// Open stream
	err = Pa_OpenStream(
		&stream,
		&input_pars,
		&output_pars,
		SAMPLE_RATE,
		256,
		paClipOff,
		test_callback,
		NULL
	);
	if (err != paNoError)
		std::cout << "Failed opening stream" << std::endl;

	err = Pa_StartStream(stream);
	if (err != paNoError)
		std::cout << "Failed to start audio processing" << std::endl;
}

static unsigned int no_input_count;

int Audio_handler::test_callback(const void *input, void *output, unsigned long frameCount,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	SAMPLE *out = (SAMPLE*)output;
	const SAMPLE *in = (const SAMPLE*)input;
	unsigned int i;
	(void)timeInfo; // Prevent unused variable warnings aparently
	(void)statusFlags;
	(void)userData;

	if (input == NULL)
	{
		for (i = 0; i < frameCount; i++)
		{
			*out++ = 0; // Left channel - silence
			*out++ = 0; // Right channel - silence
		}
		no_input_count++;
	}
	else
	{
		for (i = 0; i < frameCount; i++)
		{
			*out++ = *in++; // Left cahnnel - direct from input
			*out++ = *in++; // Right cahnnel - direct from input
		}
	}
	return 0;
}