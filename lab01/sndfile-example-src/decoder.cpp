#include <iostream>
#include <fstream>
#include <string>
#include "BitStream.h"
#include <filesystem>

using namespace std;

int main(int argc, char* argv[]){
    
    //find file from arguments
    string filename = argv[1];
    int n = atoi(argv[2]);
    cout << filename << endl;
    
    //get file extension
    string extension = filename.substr(filename.find_last_of(".") + 1);
    ifstream file(filename, ios::binary);

    //create bitstream object
    BitStream bitstream;
    bitstream.decode(file,n);
    return 0;
}

    