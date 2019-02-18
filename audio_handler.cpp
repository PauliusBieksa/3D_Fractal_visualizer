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
	err = Pa_Terminate();
	if (err != paNoError)
		std::cout << "PortAudio termination failed" << std::endl
		<< Pa_GetErrorText(err) << std::endl;
}


int Audio_handler::PaStreamCallback(const void *input, void *output, unsigned long frameCount,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	return 0;
}