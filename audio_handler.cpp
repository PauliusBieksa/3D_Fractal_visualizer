#include "audio_handler.h"
#include <iostream>


// Structure for fast reading and writing wit hoverwrite protection
struct semaphore_frame_mono
{
private:
	static int read;
	static int write;
	static float frame[3][FRAME_SIZE];


	// Increment the write counter
	void w_incr()
	{
		if (write > 2)
			if (read == 0)
				write = 1;
			else
				write = 0;
		else
			if (write++ == read)
				write++;
	}

	// Sets the read flag
	void set_read()
	{
		if (write == 0)
			read = 2;
		else
			read = write - 1;
	}

public:

	void init()
	{
		read = -1;
		write = 0;
	}

	// Writes sample
	void write_frame(float *s)
	{
		w_incr();
		for (unsigned int i = 0; i < FRAME_SIZE; i++)
			frame[write][i] = s[i];
	}

	// Reads last sample. Returned through result (result has to be 'FRAME_SIZE' sized)
	void read_frame(float *result)
	{
		set_read();
		for (unsigned int i = 0; i < FRAME_SIZE; i++)
			result[i] = frame[read][i];
		read = -1;
	}
};


static semaphore_frame_mono transfer_frame;
static unsigned int no_input_count;


// Default constructor
Audio_handler::Audio_handler()
{
	err = Pa_Initialize();
	if (err != paNoError)
		printf("PortAudio initialization failed\n%s\n", Pa_GetErrorText(err));

	transfer_frame.init();
}



// Default destructor
Audio_handler::~Audio_handler()
{
	// If stream has been started, close it
	if (stream != nullptr)
	{
		err = Pa_CloseStream(stream);
		if (err != paNoError)
			printf("Failed to close stream\n");
	}

	err = Pa_Terminate();
	if (err != paNoError)
		printf("PortAudio termination failed\n%s\n", Pa_GetErrorText(err));
}



// Open a stream with default parameters
void Audio_handler::initialize_default()
{
	if (stream != nullptr)
	{
		printf("Stream already open");
		return;
	}

	// Set up default input device
	input_pars.device = Pa_GetDefaultInputDevice(); // Default device
	if (input_pars.device == paNoDevice)
	{
		printf("No default input device");
	}
	input_pars.channelCount = 2; // stereo input
	input_pars.sampleFormat = paFloat32;
	input_pars.suggestedLatency = Pa_GetDeviceInfo(input_pars.device)->defaultLowInputLatency;
	input_pars.hostApiSpecificStreamInfo = NULL;

	// Set up default output device
	output_pars.device = Pa_GetDefaultOutputDevice(); // default device
	if (output_pars.device == paNoDevice)
	{
		printf("No default output device");
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
		NULL
	);
	if (err != paNoError)
		printf("Failed opening stream");

	// Start audio processing
	err = Pa_StartStream(stream);
	if (err != paNoError)
		printf("Failed to start audio processing");
}



// Choose an input device and open stream
void Audio_handler::initialize_choose_input()
{
	if (stream != nullptr)
	{
		printf("Stream already open");
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

			if (i == Pa_GetDefaultOutputDevice())
				printf("### DEFAULT OUTPUT ###\n");

			printf("Name: %s\n", dev_info->name);
			printf("Max inputs = %d\n", dev_info->maxInputChannels);
			//printf("Max outputs = %d\n", dev_info->maxOutputChannels);
			printf("Default low input latency   = %8.4f\n", dev_info->defaultLowInputLatency);
			//printf("Default low output latency  = %8.4f\n", dev_info->defaultLowOutputLatency);
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

	// Set up selected input device
	input_pars.device = dev_selection;
	input_pars.channelCount = dev_info->maxInputChannels > 1 ? 2 : 1; // stereo input. ### Uses stereo by default
	input_pars.sampleFormat = paFloat32;
	input_pars.suggestedLatency = Pa_GetDeviceInfo(input_pars.device)->defaultLowInputLatency;
	input_pars.hostApiSpecificStreamInfo = NULL;

	// Set up default outpu device
	output_pars.device = Pa_GetDefaultOutputDevice(); // default output device
	if (output_pars.device == paNoDevice)
	{
		printf("No default output device");
		return;
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
		NULL
	);
	if (err != paNoError)
		printf("Failed opening stream");

	// Start audio processing
	err = Pa_StartStream(stream);
	if (err != paNoError)
		printf("Failed to start audio processing");
}



// Callback function for audio processing
// ### TIME CRITICAL - avoid anything slow ###
int Audio_handler::callback(const void *input, void *output, unsigned long frameCount,
	const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	SAMPLE *out = (SAMPLE*)output;
	const SAMPLE *in = (const SAMPLE*)input;
	unsigned int i;
	(void)timeInfo; // Prevent unused variable warnings aparently
	(void)statusFlags;
	(void)userData;

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
			single_mono_frame[i] = *in;
			*out++ = *in++; // Left cahnnel - direct from input
			*out++ = *in++; // Right cahnnel - direct from input
		}
	}

	transfer_frame.write_frame(single_mono_frame);
	return 0;
}