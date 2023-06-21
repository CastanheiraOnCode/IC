#include <iostream>
#include <vector>
#include <sndfile.hh>
#include "wav_quant.h"

/*This program quantizices an wav file by a number of bits chosen by the user and writes it in sfhOut*/

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {

	bool verbose { false };

	if(argc < 3) {
		cerr << "Usage: wav_quant [ -v (verbose) ]\n";
		cerr << "              wavFileIn quantNumber wavFileOut\n";
		return 1;
	}

	for(int n = 1 ; n < argc ; n++)
		if(string(argv[n]) == "-v") {
			verbose = true;
			break;
		}
	SndfileHandle sfhIn { argv[argc-3] };
    int quantNumber = { atoi(argv[argc-2])};
    
	if(sfhIn.error()) {
		cerr << "Error: invalid input file\n";
		return 1;
    }

	if((sfhIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		cerr << "Error: file is not in WAV format\n";
		return 1;
	}

	if((sfhIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
		cerr << "Error: file is not in PCM_16 format\n";
		return 1;
	}

	if(verbose) {
		cout << "Input file has:\n";
		cout << '\t' << sfhIn.frames() << " frames\n";
		cout << '\t' << sfhIn.samplerate() << " samples per second\n";
		cout << '\t' << sfhIn.channels() << " channels\n";
	}

	SndfileHandle sfhOut { argv[argc-1], SFM_WRITE, sfhIn.format(),
	  sfhIn.channels(), sfhIn.samplerate() };
	if(sfhOut.error()) {
		cerr << "Error: invalid output file\n";
		return 1;
    }

	size_t nFrames;
    WAVQuant quantizicer{sfhIn};
	vector<short> samples(FRAMES_BUFFER_SIZE * sfhIn.channels());
    vector<short> quantSample(FRAMES_BUFFER_SIZE * sfhIn.channels());
	while((nFrames = sfhIn.readf(samples.data(), FRAMES_BUFFER_SIZE))){
        
        quantSample = quantizicer.quantSamples(samples, quantNumber);
        sfhOut.writef(quantSample.data(),nFrames);
	}

	return 0;
}
