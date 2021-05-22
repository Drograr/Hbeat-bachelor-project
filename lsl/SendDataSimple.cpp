#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <lsl_cpp.h>

/**
 * This is an example of how a simple data stream can be offered on the network.
 * Here, the stream is named SimpleStream, has content-type EEG, and 8 channels.
 * The transmitted samples contain random numbers (and the sampling rate is irregular)
 */

const int nchannels = 1;

int main(int argc, char* argv[]) {
	int i;
	// make a new stream_info (nchannelsch) and open an outlet with it
	lsl::stream_info info(argc > 1 ? argv[1] : "SimpleStream", "EEG", nchannels);
	lsl::stream_outlet outlet(info);



	// send data forever
	float sample[nchannels];
	i = 59;
	while(true) {
		// generate random data
		for (int c=0;c<nchannels;c++) sample[c] = i ;
		// send it
		i = i+1;
		if (i == 160){
			i = 59;
		}
		outlet.push_sample(sample);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}
