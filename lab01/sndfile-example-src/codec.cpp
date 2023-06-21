#include <iostream>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <sndfile.hh>
#include "BitStream.h"
#include <bitset>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <bitset>


using namespace std;

string float2bin (float number){
    string complete{};
    std::string str = std::to_string(number);
	int length = str.length();
    for (int i=0; i<length; i++){
        std::string binary = bitset<4>(str[i]).to_string(); //to binary
        complete+=binary;
    }
    complete+="1111";

    return complete;
}

//versao para quando vem apenas os bits de um float
float bin2float(string str){
	string temp="";
	string final {};
	int length = str.length();
	for (int i=0; i<length; i=i+4){
        temp = "";
		//0010
        temp.push_back(str[i]);
        temp.push_back(str[i+1]);
        temp.push_back(str[i+2]);
        temp.push_back(str[i+3]);
		//"3"<-3<-0010
		string number = std::to_string(stoi(temp, 0, 2));
		if (number == "13"){
			final += "-";
		}else if (number == "14"){
			final += ".";
		}else if (number == "15"){
			return std::stof(final);
		}else{
			final += number;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {

	bool verbose { false };
	size_t bs { 1024 };
	double dctFrac { 0.2 };

	if(argc < 3) {
		cerr << "Usage: codec [ -v (verbose) ]\n";
		cerr << "               [ -bs blockSize (def 1024) ]\n";
		cerr << "               [ -frac dctFraction (def 0.2) ]\n";
		cerr << "               wavFileIn";
		return 1;
	}

	for(int n = 1 ; n < argc ; n++)
		if(string(argv[n]) == "-v") {
			verbose = true;
			break;
		}

	for(int n = 1 ; n < argc ; n++)
		if(string(argv[n]) == "-bs") {
			bs = atoi(argv[n+1]);
			break;
		}

	for(int n = 1 ; n < argc ; n++)
		if(string(argv[n]) == "-frac") {
			dctFrac = atof(argv[n+1]);
			break;
		}

	SndfileHandle sfhIn { argv[argc-2] };
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

	SndfileHandle sfhOut { "decoded.wav", SFM_WRITE, sfhIn.format(),
	  sfhIn.channels(), sfhIn.samplerate() };
	if(sfhOut.error()) {
		cerr << "Error: invalid output file\n";
		return 1;
    }

	if(verbose) {
		cout << "Input file has:\n";
		cout << '\t' << sfhIn.frames() << " frames\n";
		cout << '\t' << sfhIn.samplerate() << " samples per second\n";
		cout << '\t' << sfhIn.channels() << " channels\n";
	}

	size_t nChannels { static_cast<size_t>(sfhIn.channels()) };
	size_t nFrames { static_cast<size_t>(sfhIn.frames()) };


	// Read all samples: c1 c2 ... cn c1 c2 ... cn ...
	// Note: A frame is a group c1 c2 ... cn
	vector<short> samples(nChannels * nFrames);
	sfhIn.readf(samples.data(), nFrames);

	size_t nBlocks { static_cast<size_t>(ceil(static_cast<double>(nFrames) / bs)) };


	// Do zero padding, if necessary
	samples.resize(nBlocks * bs * nChannels);

	// Vector for holding all DCT coefficients, channel by channel
	vector<vector<double>> x_dct(nChannels, vector<double>(nBlocks * bs));

	// Vector for holding DCT computations
	vector<double> x(bs);

	// Direct DCT
	fftw_plan plan_d = fftw_plan_r2r_1d(bs, x.data(), x.data(), FFTW_REDFT10, FFTW_ESTIMATE);
	for(size_t n = 0 ; n < nBlocks ; n++)
		for(size_t c = 0 ; c < nChannels ; c++) {
			for(size_t k = 0 ; k < bs ; k++)
				x[k] = samples[(n * bs + k) * nChannels + c];

			fftw_execute(plan_d); 
			// Keep only "dctFrac" of the "low frequency" coefficients
			for(size_t k = 0 ; k < bs * dctFrac ; k++)
				x_dct[c][n * bs + k] = x[k] / (bs << 1);

		}

	fstream file;//file that has the coefficients in binary encoded 
    file.open("temp.txt", ios_base::out);
    uint32_t sizeOffile = {};
	// encoding the x_dct coeficient to a binary value
    for(size_t n = 0 ; n < nBlocks ; n++){
		for(size_t c = 0 ; c < nChannels ; c++) {
			for(size_t k = 0 ; k < bs * dctFrac ; k++){
                //convert x_dct to binary and write on file
				string binary_xdct = float2bin(x_dct[c][n*bs+k]);
				sizeOffile+= binary_xdct.size()/4;
				if (file.is_open()) {
					file.write(binary_xdct.data(), binary_xdct.size());
				}
            }

		}
	}

	cout << "sizeOffile : " << sizeOffile;
	file.close();

	ifstream fileOutEncode("temp.txt", ios::binary);
	BitStream bitStream1;
	bitStream1.encode(fileOutEncode,sizeOffile);
	fileOutEncode.close();

	BitStream bitStream2;
	ifstream fileOutDecode("encode.bin", ios::binary);
	bitStream2.decode(fileOutDecode,sizeOffile);
	fileOutDecode.close();
	
	FILE* bits_text = fopen("decode.txt", "r");

    char character = 0;
    int count = 0;
    string temp = "";
    string current_n ="";
    vector<double> x_dct2 {};

    while (!feof(bits_text)) {
        character = getc(bits_text);
        if (character==EOF) break;
        count++;
        temp.push_back(character);
        if (count == 4){
            current_n+=temp;
            if(temp == "1111"){
                //cout<< current_n<<"\n";
                x_dct2.push_back( bin2float(current_n));
                current_n ="";
            }
            count = 0;
            temp = "";
       }
    }

	fclose(bits_text);

	//only for printing
    //for (int usn=0; usn<900; usn++)
    //    cout<< x_dct2[usn] <<endl;


	vector<vector<double>> new_x_dct(nChannels, vector<double>(nBlocks * bs));
	int conta = 0;

	//for( int r = 0; r<(int)x_dct2.size(); r++)
		for(size_t n = 0 ; n < nBlocks ; n++)
			for(size_t c = 0 ; c < nChannels ; c++) 
				for(size_t k = 0 ; k < bs * dctFrac ; k++){
					new_x_dct[c][n * bs + k] = x_dct2[conta++];
				}

	vector<double> new_x(bs);
	vector<short> new_samples(nChannels * nFrames);
	new_samples.resize(nBlocks * bs * nChannels);

	// Inverse DCT
	fftw_plan plan_i = fftw_plan_r2r_1d(bs, new_x.data(), new_x.data(), FFTW_REDFT01, FFTW_ESTIMATE);
	for(size_t n = 0 ; n < nBlocks ; n++)
		for(size_t c = 0 ; c < nChannels ; c++) {
			for(size_t k = 0 ; k < bs ; k++)
				new_x[k] = new_x_dct[c][n * bs + k];

			fftw_execute(plan_i);
			for(size_t k = 0 ; k < bs ; k++)
				new_samples[(n * bs + k) * nChannels + c] = static_cast<short>(round(new_x[k]));

		}

	sfhOut.writef(new_samples.data(), sfhIn.frames());
    

	/*for(size_t k = 0; k<bs * dctFrac; k++){
		cout << "binary x_dct" << binary_xdct[k] << endl;
	}*/
	return 0;
}

