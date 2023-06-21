#include <iostream>
#include <vector>
#include <sndfile.hh>
#include <cmath>
#include <string.h>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]) {

	if(argc < 5) {
		cerr << "Usage: wav_effects [ -v (verbose) ]\n";
		cerr << "              wavFileIn effectName alpha delay wavFileOut\n";
		return 1;
	}
	//ampmodulation = x(n) = cos(2pifn) - f é input
	SndfileHandle sndFile { argv[argc-5] };
    string effect {argv[argc-4]};
	float alpha =  stof({argv[argc-3]});
	int delay =  stoi({argv[argc-2]});


	//cout << effect << "\n";
	vector<string> availableEffect {"echo","multipleEcho","ampModulation"};
	if(sndFile.error()) {
		cerr << "Error: invalid input file\n";
		return 1;
    }

	if((sndFile.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		cerr << "Error: file is not in WAV format\n";
		return 1;
	}

	if((sndFile.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
		cerr << "Error: file is not in PCM_16 format\n";
		return 1;
	}

	if(alpha < 0 || alpha >1 ){
		cerr << "Error: alpha must be between 0 or 1\n";
	}

	if(alpha < 0 || alpha >1 ){
		cerr << "Error: alpha must be between 0 or 1\n";
	}

	if((delay *10) % 10 != 0 ){
		cerr << "Error: delay must be a whole number\n";
	}

	uint32_t opt = {};
	bool found = false;
    //create opt to switch effect -> echo = 0, multipleEcho = 1, ampModulation = 2
	for(uint32_t i = 0; i<availableEffect.size(); i++){
		if (effect == availableEffect[i]){
			opt = i;
			found = true;
			break;
		}
		if(i  == availableEffect.size() && !found){
			cerr << "Error: effect is not found";
		return 1;
		}
	}	

	SndfileHandle sfhOut { argv[argc-1], SFM_WRITE, sndFile.format(),
	  sndFile.channels(), sndFile.samplerate() };
	if(sfhOut.error()) {
		cerr << "Error: invalid output file\n";
		return 1;
    }


	vector<short> effectSample(sndFile.frames() * sndFile.channels());
	uint32_t delay_sampleRate = delay * sndFile.samplerate();//input para argumento(s)


	//vector to hold all the sound file's frames
	vector<short> all_samples(sndFile.frames() * sndFile.channels());
	//read into all_samples the number of frames the file has
	size_t nframys = sndFile.readf(all_samples.data(), sndFile.frames());

	//print it for debugging pourpouses
	// int count = 0;
	// for (auto i = all_samples.begin(); i != all_samples.end(); ++i){
	// 	cout << *i << " " ;
	// 	count++;
	// }

	cout <<"\n\n->read "<<nframys << " frames successfully\n"; 
	cout << "->found "<< sndFile.frames() <<" total frames in sndFile\n";
	//cout << "->counted: "<< count << " spaces in vector\n";
	switch (opt){
	case 0 :
		for(size_t i = 0; i<all_samples.size(); i++){
			//cout << i << " ";
			if (i<delay_sampleRate){
				effectSample[i] = all_samples[i];
			}else{
				effectSample[i] = (all_samples[i] + alpha * (all_samples[i- (delay_sampleRate)]))/(1+alpha);
			}
			
		}
		break;
		
	case 1 :
		for(size_t i = 0; i<all_samples.size(); i++){
			if(i<delay_sampleRate){
				effectSample[i] = all_samples[1];
			}
			else{
				short new_sample = (all_samples[i] + alpha * (all_samples[i- (delay_sampleRate)]))/(1+alpha);
				effectSample[i] = new_sample;
				short new_sample2 = (all_samples[i] + alpha * (effectSample[i- (delay_sampleRate)]))/(1+alpha);
				effectSample[i] = new_sample2;
			}
			
		}
		break;

	default:
		break;
	}

		sfhOut.writef(effectSample.data(), sndFile.frames());

	return 0;
}