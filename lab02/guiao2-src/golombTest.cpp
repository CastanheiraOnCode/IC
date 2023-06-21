#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "golomb.h"

using namespace std;

int main(int argc, char** argv)
{
    int n = stoi(argv[argc - 2]);
    int m = stoi(argv[argc -1]);
    Golomb g = Golomb(m);
    //string code = g.encode(n);
    string code = " 0000001111101000111111111111111111110000000000";

    cout << "code:"  <<  code << endl;
    
    int decode = g.decode(code);

    cout << "n: " << n << " decode:" << decode <<"\n";

}