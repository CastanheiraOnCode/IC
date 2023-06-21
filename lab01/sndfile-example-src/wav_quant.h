
#include <iostream>
#include <vector>
#include <map>
#include <sndfile.hh>

constexpr size_t FRAME_BUFFER_SIZE = 65536;

class WAVQuant {
  private:

    std::vector<short> quantdsamples;
    size_t nFrames;

  public:
	WAVQuant(const SndfileHandle& sfh) {
        quantdsamples.resize(FRAME_BUFFER_SIZE * sfh.channels());
	}

    std::vector<short> quantSamples(std::vector<short> samples, size_t quantNumber){
        for(size_t i = 0; i < samples.size(); i++){
            short newsample = samples[i]>>quantNumber;
            newsample = newsample<<quantNumber;
            quantdsamples[i] = newsample;
        }

        return quantdsamples;
    }







};



