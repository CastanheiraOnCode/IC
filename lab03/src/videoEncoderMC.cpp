#include <iostream>
#include <math.h>
#include <numeric>
#include <sndfile.hh>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include "utils/golomb.h"
#include "utils/BitStream.h"

using namespace std;
using namespace cv;

//global variables

cv::Mat lastKeyFrame;
cv::Mat outImage;
cv::Mat outImage2;
int charCount = 0;
int FrameNumber=0;
int counter = 0;
int m=8;
FILE *fp;

bool isBinary(string s){
    for(long unsigned int i = 0; i < s.length(); i++){
        if(s[i] != '0' && s[i] != '1'){
            return false;
        }
    }
    return true;
}

//this function will now encode a block in IntraMode and return a string with the encoed residual
string encodeIntra(cv::Mat image){
    //create golomb object
    Golomb g = Golomb(m);


    //check if image is emty
    if (image.empty())
    {
        printf("Error: Image is empty");
    }

    //split image into channels (BGR)
    vector<cv::Mat> channels;
    cv::split(image, channels);

    //channel 1 and 2 are 1/4 size of channel 0
    cv::resize(channels[1], channels[1], cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
    cv::resize(channels[2], channels[2], cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);

    //get image dimensions
    int width = image.cols;
    int height = image.rows;
    int i,j;
    string encodedResidual = "";
    //apply LOCO-I/JPEG-LS non linear prediction

    //for each channel in the image
    for (int k=0;k<3;k++){
        int numOfValues = 0;
        image = channels[k];

        if (k>0){
            
            width = image.cols;
            height = image.rows;
        }


        for (i = 0; i < height; i++)
        {
            
            for (j = 0; j < width; j++)
            {
                //C is upper left pixel
                //B is upper pixel
                //A is left pixel
                
                //cicle for running 3 times
            
                if (i == 0 || j == 0)
                {
                int R = image.at<uchar>(i, j);
                encodedResidual += g.encode(R);
                numOfValues++;
                }

                else
                {
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
                    int R = image.at<uchar>(i, j) - P;

                    
                    //create golomb object
                    Golomb g = Golomb(m);

                    encodedResidual += g.encode(R);
                    numOfValues++;
                    
                    
                }

             }

        }


    }



   

    //return 'error'
    return '0' + encodedResidual;
        
}

//this function will now encode a block in InterMode and return a string
string encodeInter(const cv::Mat& block, int searchWindow, int left, int top)
{
    // Create a cv::Mat to store the best matching block from the previous keyframe
    cv::Mat bestMatch;

    int MVx = 0;
    int MVy = 0;

    int minSAD = INT_MAX;



    // Iterate over the search window in the previous keyframe to find the best matching block
    for (int y = top - searchWindow; y <= top + searchWindow; y++)
    {
        for (int x = left - searchWindow; x <= left + searchWindow; x++)
        {   
            
            // Check if the search window is out of bounds
            if (x < 0 || y < 0 || x + block.cols > lastKeyFrame.cols || y + block.rows > lastKeyFrame.rows)
            {
                // if it is out of bounds, skip the current block
                continue;

            }

            cv::Mat currentBlock;

            // Create a cv::Mat to store the current block from the previous keyframe 
            currentBlock = lastKeyFrame(cv::Rect(x, y, block.cols, block.rows));

            // Compute the sum of absolute differences (SAD) between the input block and the current block
            double sad = cv::sum(cv::abs(block - currentBlock))[0];
            
            // If the SAD value is smaller than the current minimum SAD value,
            // update the minimum SAD value and the best matching block
            if (sad < minSAD)
            {
                minSAD = sad;
                bestMatch = currentBlock;
                //calculate the motion vector residuals
                MVx = x - left;
                MVy = y - top;
            }
         }
    }

    //create golomb object
    Golomb g = Golomb(m);

    //encode the motion vector residuals
    string encodedMVx = g.encode(MVx);
    string encodedMVy = g.encode(MVy);

    

    //split block and best match into channels (BGR)
    vector<cv::Mat> channelsBlock;
    vector<cv::Mat> channelsBestMatch;
    cv::split(block, channelsBlock);
    cv::split(bestMatch, channelsBestMatch);


    int numberOfPixels = 0;

    string diffPixel = "";
    
    //iterare over block and best match cheking witch pixels are different
    for (int i = 0; i < block.rows; i++)
    {
        for (int j = 0; j < block.cols; j++)
        {
            if (block.at<uchar>(i, j) != bestMatch.at<uchar>(i, j))
            {
                //change the char at the position of the different pixel to 1
                numberOfPixels++;

                //calculate the difference between the pixels channel by channel store int diffB, diffG, diffR
                int diffB = channelsBlock[0].at<uchar>(i, j) - channelsBestMatch[0].at<uchar>(i, j);           
                int diffG = channelsBlock[1].at<uchar>(i, j) - channelsBestMatch[1].at<uchar>(i, j);
                int diffR = channelsBlock[2].at<uchar>(i, j) - channelsBestMatch[2].at<uchar>(i, j);


                //encode the difference
                string encodedDiff = g.encode(j)+ g.encode(i) + g.encode(diffB) + g.encode(diffG) + g.encode(diffR);

                //add the encoded difference to the encoded motion vector residuals
                diffPixel += encodedDiff;
            }
        }
    }

    // Calculate if the block is intra or inter
    // If the SAD value is smaller than the threshold, the block is intra
    // Otherwise, the block is inter
    
    // If the block is intra, return the encoded motion vector residuals
    return '1' + g.encode(numberOfPixels) + encodedMVx + encodedMVy + diffPixel;
    
}





//main encode function receives the  channel of frame and divides it into blocks of blocksize x blocksize
string encode(cv::Mat image, int blocksize,bool keyFrame, int searchWindow){
    
    //check if image is emty
    if (image.empty())
    {
        printf("Error: Image is empty");
    }

    //initialize variables
    string Inter;
    string Intra;


    //get image dimensions

    int width = image.cols;
    int height = image.rows;
    int i,j;

    cv::Mat block;

    //string to store the total encoded frame
    string encodedFrame;

    //divide image into blocks of blocksize x blocksize

    int blockCount = 0;

   

    
    for (i = 0; i < height; i+=blocksize)
    {
        for (j = 0; j < width; j+=blocksize)
        {
            
            //get block
            block = image(cv::Rect(j,i,blocksize,blocksize));

           

            blockCount++;
                

                if (keyFrame)
                {
                    //encode block
                    Intra=encodeIntra(block);
                    encodedFrame += Intra;
                    
                    
                    
                }

                else

                {
                    //encode block
                    Inter = encodeInter(block,searchWindow,j,i);
                    Intra = encodeIntra(block);

                    //compare sizes and choose the best one
                    if ( (Inter.size() < Intra.size()) && isBinary(Inter))
                    {
                        //store the encoded block
                        encodedFrame += Inter;
                    }
                    else
                    {
                        //store the encoded block
                        encodedFrame += Intra;
                    }

            }


        }
                        
    }
    
    cout<<blockCount<<endl;
    //return the encoded frame
    return encodedFrame;
}

//Main argument function receives <video file> <search window> <block size> <period of key frames> 

int main(int argc, char** argv){

    FrameNumber=0;
    fp = fopen("out.txt", "a");
    
   

	VideoCapture cap(argv[1]); // open the default camera
	if (!cap.isOpened()) return -1; 
	Mat frame;


    //read search window and block size from command line
    int searchWindow = atoi(argv[2]);
    int blockSize = atoi(argv[3]);
    int period = atoi(argv[4]);


    //Values to put in the header
    int width = cap.get(CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CAP_PROP_FRAME_HEIGHT);
    int fps = cap.get(CAP_PROP_FPS);
    int32_t frameCount = cap.get(CAP_PROP_FRAME_COUNT);

    

    //safety checks go here

    

    //Header values to binary
    string widthBin = toBin(width,16);
    string heightBin = toBin(height,16);
    string fpsBin = toBin(fps,16);
    string frameCountBin = toBin(frameCount,32);
    string blockSizeBin = toBin(blockSize,8);
    string searchWindowBin = toBin(searchWindow,8);
    string periodBin = toBin(period,8);

    //print header values
    cout << "Width: " << width << endl;
    cout << "Height: " << height << endl;
    cout << "FPS: " << fps << endl;
    cout << "Frame Count: " << frameCount << endl;
    cout << "Block Size: " << blockSize << endl;
    cout << "Search Window: " << searchWindow << endl;
    cout << "Period: " << period << endl;



   
    
    //Write header to out.txt
    fprintf(fp, "%s", widthBin.c_str());
    fprintf(fp, "%s", heightBin.c_str());
    fprintf(fp, "%s", fpsBin.c_str());
    fprintf(fp, "%s", frameCountBin.c_str());
    fprintf(fp, "%s", blockSizeBin.c_str());
    fprintf(fp, "%s", searchWindowBin.c_str());
    fprintf(fp, "%s", periodBin.c_str());

    //iterate through frames of video
    bool end = false;


    //check if image is divisible by block size
    if (width % blockSize != 0 || height % blockSize != 0)
    {
        printf("Error: Image is not divisible by block size");
    }

    //declare image

	while(!end){
        
       
		cap >> frame;
        if (frame.empty())	break;

        
        if(FrameNumber==1){

            lastKeyFrame = frame.clone();    
            
        }


        // cap reads frames in BGR format <--IMPORTANT 
        //store frame in an image
        cv::Mat image = frame.clone();
        

        bool isKeyFrame = (FrameNumber % period == 0);
        cout<<"frame number:" << FrameNumber<<endl;

        if (isKeyFrame)
        {
            lastKeyFrame = image.clone();
        }
    
        //encode key frame and store in frameCode string the numbers are identifiers for the channels
        
        string frameCode =  encode(image,blockSize,isKeyFrame,searchWindow);

        charCount += frameCode.size();
        
 
        //write frameCode to file
        fprintf(fp, "%s", frameCode.c_str());
        FrameNumber++;
  
	}

	 
    
    
    //create bitstream object
    ifstream fileIn("out.txt");
    BitStream bs;
    bs.encode(fileIn, charCount);
    
    //Delete out.txt
    remove("out.txt");

	cap.release();
	destroyAllWindows();

	return 0;
}
