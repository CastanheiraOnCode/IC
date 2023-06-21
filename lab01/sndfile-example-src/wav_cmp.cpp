#include <iostream>
#include <vector>
#include <sndfile.hh>
#include <cmath>

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading frames

int main(int argc, char *argv[]) {

	if(argc < 3) {
		cerr << "Usage: wav_cmp [ -v (verbose) ]\n";
		cerr << "              wavFileIn wavFileOut\n";
		return 1;
	}

	SndfileHandle sndFile { argv[argc-2] };
    SndfileHandle sndFile2 { argv[argc-1] };
	if(sndFile.error() || sndFile2.error()) {
		cerr << "Error: invalid input file\n";
		return 1;
    }

	if((sndFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV || (sndFile2.format() & (SF_FORMAT_TYPEMASK != SF_FORMAT_WAV))) {
		cerr << "Error: file is not in WAV format\n";
		return 1;
	}

	if((sndFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 || (sndFile2.format() & (SF_FORMAT_SUBMASK != SF_FORMAT_PCM_16))) {
		cerr << "Error: file is not in PCM_16 format\n";
		return 1;
	}


    //Print Signal to noise Ratio



    //Maximum per Sample Absolute Error
	size_t nFrames;
	size_t nFrames2;
    //WAVCMP cmp{sndFile, sndFile2};
    //vector sample tem as samples do ficheiro 
	vector<short> samples1(FRAMES_BUFFER_SIZE * sndFile.channels());
	
	vector<short> samples2(FRAMES_BUFFER_SIZE * sndFile2.channels());
	int32_t Ex = {};
	int32_t noise = {};
	int32_t maxError = {};
	while((nFrames = sndFile.readf(samples1.data(), FRAMES_BUFFER_SIZE) && (nFrames2 = sndFile2.readf(samples2.data(), FRAMES_BUFFER_SIZE)))) { //escreve com samplesdata
		samples1.resize(nFrames * sndFile.channels());
        //read Frames
		samples2.resize(nFrames2 * sndFile2.channels());
        for(size_t i = 0; i<samples1.size(); i++){

			Ex = Ex + pow(samples1[i],2);//frame ^2

		}
		for(uint32_t j = 0; j<samples2.size(); j++){
			
			noise = noise + pow(samples2[j] - samples1[j],2);
			if (maxError < abs(samples2[j] - samples1[j])){
				maxError = abs(samples2[j] - samples1[j]);
			}

		}
	}

	float snr =10*log10(Ex / noise);

	
	//SIGNAL TO NOISE RATIO - (FRAME ^2)(energia do sinal) / (FRAME QUANTIZADA - FRAME)^2(energia do ruida)	
	printf("The signal to noise ratio is: %f\n", snr);
	printf("The maximum error is %d\n", maxError);

	return 0;
}

