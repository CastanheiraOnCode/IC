#include <iostream>
#include <math.h>
#include <numeric>
#include <sndfile.hh>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "utils/golomb.h"
#include "utils/BitStream.h"

using namespace std;
using namespace cv;

FILE *fp;
FILE *fp2;



int counter = 0;
int charCount = 0;
int imageHeight=0;
int imageWidth=0;
int imageFrameCount=0;
int imageFps=0;
int m=8;
int b = log2(m);
int t = pow(2,b+1);


//checks if a string is a binary number
bool isBinary(string s){
    for(long unsigned int i = 0; i < s.length(); i++){
        if(s[i] != '0' && s[i] != '1'){
            return false;
        }
    }
    return true;
}



int toDec(string binary){
    int decimal = 0;
    int base = 1;
    int len = binary.length();
    for (int i = len - 1; i >= 0; i--)
    {
        if (binary[i] == '1')
            decimal += base;
        base = base * 2;
    }
    return decimal;
}



void predictor(cv::Mat image){
    vector<int> ret;
    int height = image.rows;
    int width  = image.cols;


    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            //C is upper left pixel
            //B is upper pixel
            //A is left pixel
            if (i == 0 || j == 0)
            {
                image.at<uchar>(i, j) = image.at<uchar>(i, j);
            }

            else{
                //apply non linear prediction
                int A = image.at<uchar>(i, j - 1);
                int B = image.at<uchar>(i - 1, j);
                int C = image.at<uchar>(i - 1, j - 1);

                int P = A + B - C;

                if(C>=max(A,B)){
                    P = min(A,B);
                }
                else if(C<=min(A,B)){
                    P = max(A,B);
                }
                else{
                    P = A + B - C;
                }

                //calculate the residuals
                int R = image.at<uchar>(i, j) + P;
                //write to Received image
                image.at<uchar>(i, j) = R;
            }
        }

    }
}




//decode channel function returning a vector with the values of the channel

vector<int> decodeChannel(int channelSize){
    vector<int> ret;
    fp2=fopen("testDecoder.txt","a");



    //counts the number of values read from the file
    int numberOfValues=0; 

    //create golomb object
    Golomb g = Golomb(m);

    //read height*width values from the file and store them in Y
   
    while(numberOfValues<channelSize){

        //get q  counting the number of 1s
        int q = 0;
        char c;
        string qPart = "";
        while((c = fgetc(fp)) != '0'){
            if(c == '1'){
                q++;
                qPart += c;
            }
            else{
                break;
            }
        }

        //move file pointer back one position to read the 0 
        fseek(fp, -1, SEEK_CUR);

        //get read next b bits
        string rString = "";
        for(int i = 0; i < b+1; i++){
            rString += fgetc(fp);
        }

        //convert rString to int
        int r = toDec(rString);
        int R = r;
        
        //if r'  < 2^b - m
        if(r < t - m){
            //print qPart
            rString = qPart + rString;
            //R=g.decode(rString,8);

            //check if rString is a binary number
            if(isBinary(rString)){
                R=g.decode(rString);
            }
            else{
                R = 0;
            }
            
            ret.push_back(R);
            fprintf(fp2, "\n%d", R);

        }

        else{
            rString = qPart + rString;
            //rString get next char
            rString += fgetc(fp);
            
             if(isBinary(rString)){
                R=g.decode(rString);
            }
            else{
                R = 0;
            }

            ret.push_back(R);
            fprintf(fp2,"\n%d",R);
        }
    
        numberOfValues++;
    }
    
    return ret;
}



//decode frame function returning a cv::Mat image
cv::Mat decodeFrame(){
    counter++;
    //create 3 vectors to store the data 1 ith lenght height*width and 2 with lenght 0.25*height*width
    vector<int> B;
    vector<int> G;
    vector<int> R;

    B=decodeChannel(imageHeight*imageWidth);
    G=decodeChannel(0.25*imageHeight*imageWidth);
    R=decodeChannel(0.25*imageHeight*imageWidth);

    

    //create cv::Mat image
    cv::Mat image = cv::Mat(imageHeight, imageWidth, CV_8UC3);

    //split image in 3 channels
    vector<cv::Mat> channels;
    cv::split(image, channels);

    //create cv::Mat for each channel
    cv::Mat BMat = cv::Mat(imageHeight, imageWidth, CV_8UC1);
    cv::Mat GMat = cv::Mat(imageHeight/2, imageWidth/2, CV_8UC1);
    cv::Mat RMat = cv::Mat(imageHeight/2, imageWidth/2, CV_8UC1);

    //fill cv::Mat with values from the vectors
    for(int i = 0; i < imageHeight; i++){
        for(int j = 0; j < imageWidth; j++){
            BMat.at<uchar>(i, j) = B[i*imageWidth + j];
        }
    }

    for(int i = 0; i < imageHeight/2; i++){
        for(int j = 0; j < imageWidth/2; j++){
            GMat.at<uchar>(i, j) = G[i*imageWidth/2 + j];
            RMat.at<uchar>(i, j) = R[i*imageWidth/2 + j];
        }
    }

    

    //merge channels
    channels[0] = BMat;
    channels[1] = GMat;
    channels[2] = RMat;


    //predictor in each channel
    predictor(channels[0]);
    predictor(channels[1]);
    predictor(channels[2]);


    //resize channels
    cv::resize(channels[1], channels[1], cv::Size(imageWidth, imageHeight));
    cv::resize(channels[2], channels[2], cv::Size(imageWidth, imageHeight));


    //merge channels
    cv::merge(channels, image);


    return image;
}



//read bitstream and decode image
int main(int argc, char *argv[]) {

    if (argc != 2){
        cout << "ERROR!\nCorrect Usage: ./videoDecoder <file.bin>\n";
        return -1;
    }
	
    //receive file name from command line
    string binFile = argv[1];
    
    
    //open file
    ifstream file(binFile, ios::binary);
    

    //store number of bits in file in charCount(its called that cuz every bit is a char)
    int charCount = 0;
    file.seekg(0, ios::end);
    charCount = file.tellg();
    file.seekg(0, ios::beg);
    charCount=charCount*8;


    //convert do .txt file using bitsream
    BitStream bs;
    bs.decode(file, charCount);

    
    //now we have a .txt file with the encoded image data, it is time to decode it
    //open file generated by decoder
    
    fp = fopen("decoded.txt", "r");
    

    //read the header of the file

    //read 16 bits for width
    string widthString = "";
    for(int i = 0; i < 16; i++){
        widthString += fgetc(fp);
    }
    imageWidth = toDec(widthString);

    //read 16 bits for height
    string heightString = "";
    for(int i = 0; i < 16; i++){
        heightString += fgetc(fp);
    }
    imageHeight = toDec(heightString);

    //read 16 bits for fps
    string fpsString = "";
    for(int i = 0; i < 16; i++){
        fpsString += fgetc(fp);
    }
    imageFps = toDec(fpsString);

    //read 32 bits for frame count
    string frameCountString = "";
    for(int i = 0; i < 32; i++){
        frameCountString += fgetc(fp);
    }
    imageFrameCount = toDec(frameCountString);


    //print image info
    cout << "Image width: " << imageWidth << endl;
    cout << "Image height: " << imageHeight << endl;
    cout << "Image fps: " << imageFps << endl;
    cout << "Image frame count: " << imageFrameCount << endl;


    //create video writer output mp4 file
    cv::VideoWriter writer;
    writer.open("output.y4m", cv::VideoWriter::fourcc('I', '4', '2', '0'), imageFps, cv::Size(imageWidth, imageHeight), true);
    
    

    //call decodeFrame frameCount times
    for (int i = 0; i < imageFrameCount; i++)
    {
        //create image 
        cv::Mat image;

        image = decodeFrame();
        
        //put image in video
        writer.write(image);
    }

    file.close();
    //Delete out.txt
    //remove("decoded.txt");
	return 0;
}
