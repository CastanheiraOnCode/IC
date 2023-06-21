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

cv::Mat outImage;
cv::Mat outImage2;
int charCount = 0;
int FrameNumber=0;
int m=8;
FILE *fp;



//encode function implementing jpeg 2000 golomb-rice algorithm

void encode(cv::Mat image){
    

    FrameNumber++;
    //check if image is emty
    if (image.empty())
    {
        printf("Error: Image is empty");
    }

    //get image dimensions

    int width = image.cols;
    int height = image.rows;
    int i,j;

    //apply LOCO-I/JPEG-LS non linear prediction

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
                        
                //create golomb object
                Golomb g = Golomb(m);

                string encodedResidual = g.encode(R);


                //write to file
                
                fprintf(fp, "%s", encodedResidual.c_str());


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

                string encodedResidual = g.encode(R);

                //write to file
                
                fprintf(fp, "%s", encodedResidual.c_str());
                
                }

            }

        }
        
}

int main(int argc, char *argv[]) {

    if (argc != 2){
        cout << "ERROR!\nCorrect Usage: ./videoEncoder <video.y4m>\n";
        return -1;
    }
    
    fp = fopen("out.txt", "a");

	VideoCapture cap(argv[argc-1]);

	if (!cap.isOpened()) return -1;

	Mat frame;


    //Values to put in the header
    int width = cap.get(CAP_PROP_FRAME_WIDTH);
    int height = cap.get(CAP_PROP_FRAME_HEIGHT);
    int fps = cap.get(CAP_PROP_FPS);
    int32_t frameCount = cap.get(CAP_PROP_FRAME_COUNT);

    

    //Header values to binary
    string widthBin = toBin(width,16);
    string heightBin = toBin(height,16);
    string fpsBin = toBin(fps,16);
    string frameCountBin = toBin(frameCount,32);

    //print header values
    cout << "Width: " << width << endl;
    cout << "Height: " << height << endl;
    cout << "FPS: " << fps << endl;
    cout << "Frame Count: " << frameCount << endl;

    //print header values in binary
    cout << "Width: " << widthBin << endl;
    cout << "Height: " << heightBin << endl;
    cout << "FPS: " << fpsBin << endl;
    cout << "Frame Count: " << frameCountBin << endl;
    
    //Write header to out.txt
    fprintf(fp, "%s", widthBin.c_str());
    fprintf(fp, "%s", heightBin.c_str());
    fprintf(fp, "%s", fpsBin.c_str());
    fprintf(fp, "%s", frameCountBin.c_str());

    //iterate through frames of video
    bool end = false;

    //declare image

	while(!end){
        
		cap >> frame;
        if (frame.empty())	break;


        // cap reads frames in BGR format <--IMPORTANT 
        //store frame in an image
        cv::Mat image = frame.clone();
        

        //split image into 3 channels
        std::vector<cv::Mat> imageChannels(3);
        cv::split(image, imageChannels);
        


        //reduce chroma channels to 1/4 of original size
        cv::resize(imageChannels[1], imageChannels[1], cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
        cv::resize(imageChannels[2], imageChannels[2], cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);


		encode(imageChannels[0]);
        encode(imageChannels[1]);
        encode(imageChannels[2]);

        
	}

	 //Create bitstream object
    
    
    
    
    //count number of chars in file
    fseek(fp, 0, SEEK_END);
    charCount = ftell(fp);
    fseek(fp, 0, SEEK_SET);



    
    
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
