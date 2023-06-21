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


cv::Mat lastKeyFrame;


int counter = 0;
int charCount = 0;
int imageHeight=0;
int imageWidth=0;
int imageFrameCount=0;
int imageFps=0;
int blockSize = 8;
int m=8;
int b = log2(m);
int t = pow(2,b+1);
int frameCounter = 0;
int period=0;




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

//readValue function this will read the next value from the file and return it as an int
int readValue(){
    //get q  counting the number of 1s
        //create golomb object
        Golomb g = Golomb(m);
        int numberOfValues = 0;

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
            
            return R;
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

            return R;
        }
    
        numberOfValues++;
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
    



//---------------------------decode functions-------------------------------------------
cv::Mat decodeInter(int left, int top )
{
   
    //read blocksize*blocksize number of values from the file store in a string
    string diferentPixels = "";
    
    
    //read the number of different pixels
    int numberOfDiferentPixels = readValue();
    

    //decode the motion vector residuals from the encoded string
    int MVx = readValue();
    int MVy = readValue();


    //create a vector to store the different pixels
    vector<int> pixelResidualsVector;

    //recover the motion vector
    int x = left + MVx;
    int y = top + MVy;

    //check if x and y are within the bounds of the image
    if(x < 0){
        x = 0;
    }
    if(y < 0){
        y = 0;
    }

    if(x > imageWidth - blockSize){
        x = imageWidth - blockSize;
    }

    if(y > imageHeight - blockSize){
        y = imageHeight - blockSize;
    }

        
    // Create a cv::Mat to store the best matching block from the previous keyframe
    cv::Mat bestMatch;

    //extract block from lastKeyFrame and store in bestMatch
    bestMatch = lastKeyFrame(cv::Rect(x, y, blockSize, blockSize));


    
    //use the different pixels to recover the original block

    //split bestMatch into 3 channels
    vector<cv::Mat> channels;
    cv::split(bestMatch, channels);
    

    //iterate over the number of different pixels for each pixel shall be read 5 values coordinates + remainders for each channel
    for(int i = 0; i < numberOfDiferentPixels; i++){
        //read the coordinates of the pixel
        int x = readValue();
        int y = readValue();

        //read the residuals for each channel
        int R1 = readValue();
        int R2 = readValue();
        int R3 = readValue();

        //add the residuals to the bestMatch block
        channels[0].at<uchar>(y, x) += R1;
        channels[1].at<uchar>(y/2, x/2) += R2;
        channels[2].at<uchar>(y/2, x/2) += R3;
    }
    

    //return the decoded block
    return bestMatch;
    
}


//decodes an intra encoded block and returns the decoded block as image 
cv::Mat decodeIntra(int blockSize){


    vector<int> ret;
    //create image with 3 channels
    cv::Mat image = cv::Mat::zeros(blockSize, blockSize, CV_8UC3);

    //split into channels
    vector<cv::Mat> channels;
    cv::split(image, channels);
    channels[0] = cv::Mat::zeros(blockSize, blockSize, CV_8UC1);
    channels[1] = cv::Mat::zeros(blockSize/2, blockSize/2, CV_8UC1);
    channels[2] = cv::Mat::zeros(blockSize/2, blockSize/2, CV_8UC1);


   

    for(int channel = 0; channel < 3; channel++){
        //read height*width values from the file and store them in Y
        image=channels[channel];
        int i =0;
        int j=0;


        if (channel>0){
            blockSize=blockSize/2;
        }

        
        for(i = 0; i < blockSize*blockSize; i++){
            //print i
            ret.push_back(readValue());
        }

    


        //copy values from Y to image
        for(i = 0; i < blockSize; i++){
            for(j = 0; j < blockSize; j++){
                image.at<uchar>(i,j) = ret[i*blockSize + j];
            }
        }


        
    

        //apply predictor
        for (i = 0; i < blockSize; i++)
        {
                for (j = 0; j < blockSize; j++)
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

        

        if (channel>0){
            blockSize=blockSize*2;
            //image back to original size
            cv::resize(image, image, cv::Size(), 2, 2, cv::INTER_NEAREST);
            
            //print image to file check for errors
            cv::imwrite("image.jpg", image);
            
        }

        //copy image to channel
        channels[channel] = image;

       //clear ret
        ret.clear();
    }

   
    cv::merge(channels, image);

    return image;

}


   

//decode frame function returning a cv::Mat image
cv::Mat decodeFrame(int imageWidth, int imageHeight){
    counter++;


    //create a cv::Mat to store the decoded frame
    cv::Mat decodedFrame = cv::Mat::zeros(imageHeight, imageWidth, CV_8UC3);
    cv::Mat decodedBlock = cv::Mat::zeros(blockSize, blockSize, CV_8UC3);

    //set decodedBLock channels to 3
    decodedBlock = cv::Mat::zeros(blockSize, blockSize, CV_8UC3);

    int numRows = imageHeight/blockSize;
    int numCols = imageWidth/blockSize;

    int collumnCounter = 0;
    int rowCounter = 0;

    int frameSize = imageHeight*imageWidth;
    
    
    int numberOfBlocks = frameSize/(blockSize*blockSize);

   

    //Array of MATs to store the decoded blocks
    cv::Mat decodedBlockArray[numberOfBlocks];



    for(int blockCounter = 0; blockCounter < numberOfBlocks; blockCounter++){
        //read one char from the file
        char c; 
        c=fgetc(fp);

        
        //if the char is 0, the block is intra encoded
        if((c=='0') ){
            //decode the block

            decodedBlock = decodeIntra(blockSize);
            //append the block to the decodedBlockArray
            collumnCounter++;
            decodedBlockArray[blockCounter] = decodedBlock;

        }


        //if the char is 1, the block is inter encoded
        else{
            //if the blockNumber is more than number of block in a row, change the row
            if(collumnCounter>numCols-1){
                rowCounter++;
                collumnCounter = 0;
            }


            //decode the block
            decodedBlock = decodeInter(collumnCounter*blockSize,rowCounter*blockSize);
            collumnCounter++;
            //append the block to the decodedBlockArray
            decodedBlockArray[blockCounter] = decodedBlock;
        
                        
        }

    }

    //copy the decoded blocks to the decoded channel left to right, top to bottom
    for(int i = 0; i < numRows; i++){
        for(int j = 0; j < numCols; j++){

            decodedBlockArray[i*numCols + j].copyTo(decodedFrame(cv::Rect(j*blockSize, i*blockSize, blockSize, blockSize)));
        }
    }
    
    if (counter==1){
        cv::imwrite("decodedFrame.jpg", decodedFrame);
    }
    
    return decodedFrame;
}



//read bitstream and decode image
int main(int argc, char *argv[]) {
	
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

    //read 8 bit for block size
    string blockSizeString = "";
    for(int i = 0; i < 8; i++){
        blockSizeString += fgetc(fp);
    }
    blockSize = toDec(blockSizeString);



    //read 8 bit for search window size
    string searchWindowSizeString = "";
    for(int i = 0; i < 8; i++){
        searchWindowSizeString += fgetc(fp);
    }
    int searchWindowSize = toDec(searchWindowSizeString);

    //read 8 bit for period
    string periodString = "";
    for(int i = 0; i < 8; i++){
        periodString += fgetc(fp);
    }
    period = toDec(periodString);



    //print image info
    cout << "Image width: " << imageWidth << endl;
    cout << "Image height: " << imageHeight << endl;
    cout << "Image fps: " << imageFps << endl;
    cout << "Image frame count: " << imageFrameCount << endl;
    cout << "Block size: " << blockSize << endl;
    cout << "Search window size: " << searchWindowSize << endl;
    cout << "Period: " << period << endl;


    //create video writer output mp4 file
    cv::VideoWriter writer;
    writer.open("output.y4m", cv::VideoWriter::fourcc('I', '4', '2', '0'), imageFps, cv::Size(imageWidth, imageHeight), true);
    
    

    //call decodeFrame frameCount times
    for (int i = 0; i < imageFrameCount; i++)
    {
        //create image 
        cv::Mat image;

        image = decodeFrame(imageWidth, imageHeight);

        if(frameCounter%period==0){
            lastKeyFrame = image;
        }

        //put image in video
        writer.write(image);
        //write image to file
        frameCounter++;

        
    }

    file.close();
    //Delete out.txt
    //remove("decoded.txt");
	return 0;
}
