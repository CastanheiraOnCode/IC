#ifndef WAVHIST_H
#define WAVHIST_H

#include <iostream>
#include <vector>
#include <map>
#include <sndfile.hh>

class WAVHist {
  private:
	std::vector<std::map<short, size_t>> counts;
	std::vector<short>ch0;
	std::vector<short>ch1;
	std::map<short, size_t> mid_channel;
	std::map<short, size_t> side_channel;


  public:
	WAVHist(const SndfileHandle& sfh) {
		counts.resize(sfh.channels());
	}

	void update(const std::vector<short>& samples) {
		size_t n { };
		for(auto s : samples)
			counts[n++ % counts.size()][s]++;

		separate_samples(samples);
	}

	void dump(const size_t channel) const {
		for(auto [value, counter] : counts[channel])
			std::cout << value << '\t' << counter << '\n';
	}

	void separate_samples(const std::vector<short>& samples){
		size_t divider { };
		for (auto s : samples){
			if(divider%2 == 0)
				ch0.insert(ch0.end(), s);
			else
				ch1.insert(ch1.end(), s);
			divider++;
		}
	}

	void dump_mid(){
		createMidChannel();
		for(auto [value, counter] : mid_channel)
			std::cout << value << '\t' << counter << '\n';
	}

	void createMidChannel(){
		short L { }; 
		short R { };

		for(size_t i = 0; i<ch0.size(); i++){
			L = ch0[i];
			R = ch1[i];
			mid_channel[(int)(L+R)/2]++;
		} 
	}

	void dump_side(){
		createSideChannel();
		for(auto [value, counter] : side_channel)
			std::cout << value << '\t' << counter << '\n';
	}

	void createSideChannel(){
		short L { }; 
		short R { };

		for(size_t i = 0; i<ch0.size(); i++){
			L = ch0[i];
			R = ch1[i];
			side_channel[(int)(L-R)/2]++;
		} 
	}
};

#endif