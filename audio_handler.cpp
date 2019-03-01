#include "audio_handler.h"
#include <iostream>
#include "pa_asio.h"


static unsigned int no_input_count;

// Default constructor
Audio_handler::Audio_handler()
{
	// Initialize ring buffer
	rb.rb_incoming_data = PaUtil_AllocateMemory(sizeof(float) * FRAME_SIZE);
	if (rb.rb_incoming_data == NULL)
		printf("Failed to allocate memory for ringbuffer.\n");
	PaUtil_InitializeRingBuffer(&rb.rb_incoming, sizeof(float), FRAME_SIZE, rb.rb_incoming_data);

	// Initialize PortAudio
	err = Pa_Initialize();
	if (err != paNoError)
		printf("PortAudio initialization failed\n%s\n", Pa_GetErrorText(err));
}



// Default destructor
Audio_handler::~Audio_handler()
{
	// If stream has been started, close it
	if (stream != nullptr)
	{
		err = Pa_StopStream(stream);
		if (err != paNoError)
			printf("Failed to stop stream\n%s\n", Pa_GetErrorText(err));
		err = Pa_CloseStream(stream);
		// Free memory
		if (rb.rb_incoming_data)
			PaUtil_FreeMemory(rb.rb_incoming_data);

		if (err != paNoError)
			printf("Failed to close stream\n%s\n", Pa_GetErrorText(err));
	}

	err = Pa_Terminate();
	if (err != paNoError)
		printf("PortAudio termination failed\n%s\n", Pa_GetErrorText(err));
}



// Open a stream with default parameters
void Audio_handler::initialize_default()
{
	// If stream already open, return
	if (stream != nullptr)
	{
		printf("Stream already open\n");
		return;
	}

	// Set up default input device
	input_pars.device = Pa_GetDefaultInputDevice(); // Default device
	if (input_pars.device == paNoDevice)
	{
		printf("No default input device\n%s\n", Pa_GetErrorText(err));
	}
	input_pars.channelCount = 2; // stereo input
	input_pars.sampleFormat = paFloat32;
	input_pars.suggestedLatency = Pa_GetDeviceInfo(input_pars.device)->defaultLowInputLatency;
	input_pars.hostApiSpecificStreamInfo = NULL;

	// Set up default output device
	output_pars.device = Pa_GetDefaultOutputDevice(); // default device
	if (output_pars.device == paNoDevice)
	{
		printf("No default output device\n%s\n", Pa_GetErrorText(err));
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
		FRAME_SIZE,
		paClipOff,
		callback,
		&rb
	);
	if (err != paNoError)
		printf("Failed opening stream\n%s\n", Pa_GetErrorText(err));

	// Start audio processing
	err = Pa_StartStream(stream);
	if (err != paNoError)
		printf("Failed to start audio processing\n%s\n", Pa_GetErrorText(err));
}



// Choose an input device and open stream
void Audio_handler::initialize_choose_input()
{
	// If stream already open, return
	if (stream != nullptr)
	{
		printf("Stream already open\n");
		return;
	}

	const PaDeviceInfo *dev_info;

	printf("Input devices:\n");
	for (int i = 0; i < Pa_GetDeviceCount(); i++)
	{
		dev_info = Pa_GetDeviceInfo(i);
		// Only show input devices
		if (dev_info->maxInputChannels > 0)
		{
			printf("--------------------------------------- device #%d\n", i);

			if (i == Pa_GetDefaultInputDevice())
				printf("### DEFAULT INPUT ###\n");

			printf("Name: %s\n", dev_info->name);
			printf("Max inputs = %d\n", dev_info->maxInputChannels);
			printf("Default low input latency = %8.4f\n", dev_info->defaultLowInputLatency);
		}
	}

	int dev_selection = -1;
	// Get input device selection
	while (dev_selection < 0 || dev_selection >= Pa_GetDeviceCount())
	{
		printf("\n\nSelect one of the input devices.\n");
		std::cin >> dev_selection;
	}
	dev_info = Pa_GetDeviceInfo(dev_selection);

	// Input parameters for asio device
	PaAsioStreamInfo asioInputInfo;
	asioInputInfo.size = sizeof(PaAsioStreamInfo);
	asioInputInfo.hostApiType = paASIO;
	asioInputInfo.version = 1;
	asioInputInfo.flags = paAsioUseChannelSelectors;
	i_channel_selectors[0] = 0;
	i_channel_selectors[1] = 1;
	asioInputInfo.channelSelectors = i_channel_selectors;

	// Set up selected input device
	input_pars.device = dev_selection;
	input_pars.channelCount = dev_info->maxInputChannels > 1 ? 2 : 1; // stereo input. ### Uses stereo by default
	input_pars.sampleFormat = paFloat32;
	input_pars.suggestedLatency = Pa_GetDeviceInfo(input_pars.device)->defaultLowInputLatency;
	// Assign apropriate host api info to the input parameters
	if (Pa_GetHostApiInfo(dev_info->hostApi)->type == paASIO)
		input_pars.hostApiSpecificStreamInfo = &asioInputInfo;
	else
		input_pars.hostApiSpecificStreamInfo = NULL;

	printf("Output devices:\n");
	for (int i = 0; i < Pa_GetDeviceCount(); i++)
	{
		dev_info = Pa_GetDeviceInfo(i);
		// Only show input devices
		if (dev_info->maxOutputChannels > 0)
		{
			printf("--------------------------------------- device #%d\n", i);

			if (i == Pa_GetDefaultOutputDevice())
				printf("### DEFAULT OUTPUT ###\n");

			printf("Name: %s\n", dev_info->name);
			printf("Max outputs = %d\n", dev_info->maxOutputChannels);
			printf("Default low input latency = %8.4f\n", dev_info->defaultLowOutputLatency);
		}
	}
	dev_selection = -1;
	// Get output device selection
	while (dev_selection < 0 || dev_selection >= Pa_GetDeviceCount())
	{
		printf("\n\nSelect one of the output devices.\n");
		std::cin >> dev_selection;
	}
	dev_info = Pa_GetDeviceInfo(dev_selection);

	// Output parameters for asio device
	PaAsioStreamInfo asioOutputInfo;
	asioOutputInfo.size = sizeof(PaAsioStreamInfo);
	asioOutputInfo.hostApiType = paASIO;
	asioOutputInfo.version = 1;
	asioOutputInfo.flags = paAsioUseChannelSelectors;
	o_channel_selectors[0] = 0;
	o_channel_selectors[1] = 1;
	asioOutputInfo.channelSelectors = o_channel_selectors;

	// Set up selected input device
	output_pars.device = dev_selection;
	output_pars.channelCount = dev_info->maxOutputChannels > 1 ? 2 : 1; // stereo output. ### Uses stereo by default
	output_pars.sampleFormat = paFloat32;
	output_pars.suggestedLatency = Pa_GetDeviceInfo(output_pars.device)->defaultLowOutputLatency;
	// Assign apropriate host api info to the input parameters
	if (Pa_GetHostApiInfo(dev_info->hostApi)->type == paASIO)
		output_pars.hostApiSpecificStreamInfo = &asioOutputInfo;
	else
		output_pars.hostApiSpecificStreamInfo = NULL;

	// Open stream
	err = Pa_OpenStream(
		&stream,
		&input_pars,
		&output_pars,
		SAMPLE_RATE,
		FRAME_SIZE,
		paClipOff,
		callback,
		&rb
	);
	if (err != paNoError)
		printf("Failed opening stream\n%s\n", Pa_GetErrorText(err));

	// Start audio processing
	err = Pa_StartStream(stream);
	if (err != paNoError)
		printf("Failed to start audio processing\n%s\n", Pa_GetErrorText(err));
}



// Callback function for audio processing
// ### TIME CRITICAL - avoid anything slow ###
int Audio_handler::callback(const void *input, void *output, unsigned long frameCount,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	SAMPLE *out = (SAMPLE*)output;
	const SAMPLE *in = (const SAMPLE*)input;
	unsigned int i;
	// Prevent unused variable warnings
	(void)timeInfo;
	(void)statusFlags;
	
	ring_buffer *data = (ring_buffer*)userData;

	float single_mono_frame[FRAME_SIZE];

	if (input == NULL)
	{
		for (i = 0; i < frameCount; i++)
		{
			single_mono_frame[i] = 0.0f;
			*out++ = 0; // Left channel - silence
			*out++ = 0; // Right channel - silence
		}
		no_input_count++;
	}
	else
	{
		for (i = 0; i < frameCount; i++)
		{
			// ### Sends mono signal to both channels ###
			single_mono_frame[i] = *in;
			*out++ = *in; // Left cahnnel - direct from input
			*out++ = *in++; // Right cahnnel - direct from input
			in++; // Increment input pointer, to skip right channel
		}
	}

	PaUtil_WriteRingBuffer(&data->rb_incoming, single_mono_frame, FRAME_SIZE);
	return paContinue;
}



// Gets latest audio frame on demand // May be changed or removed when audio analysis is added
std::vector<float> Audio_handler::update()
{
	static float f[FRAME_SIZE];
	while (PaUtil_GetRingBufferReadAvailable(&rb.rb_incoming) > 0)
		PaUtil_ReadRingBuffer(&rb.rb_incoming, &f, FRAME_SIZE);
	std::vector<float> v(f, f + FRAME_SIZE);
	
	return v;
}