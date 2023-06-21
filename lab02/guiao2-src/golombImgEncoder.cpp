#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include "golomb.h"
#include "BitStream.h"
using namespace std;

cv::Mat outImage;
cv::Mat outImage2;
int charCount = 0;







//encode function implementing jpeg 2000 golomb-rice algorithm

void encode(cv::Mat image){


    FILE *fp;
    fp = fopen("out.txt", "a");

    cv::Mat imageChannels[3];

    //split image to channels
    cv::split(image,imageChannels);
    

    //calculate optimal m for golomb-rice algorithm for each channel
    //this is a very lazy way of doing it, but it works (most of the time)
    int m = 0;
    int mx = 0;
    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){
            if(image.at<uchar>(i,j) > mx){
                mx = image.at<uchar>(i,j);
            }
        }
    }
    m = (int)ceil(log2(mx+1));
    cout << "m: " << m << endl;

    //add 0 equal to m at the beggining of out.txt ith 1 at the end
    for(int i = 0; i < m; i++){
        fprintf(fp, "0");
    }
    fprintf(fp, "1");

    //add binary representation of image.rows in out.txt
    string rows = toBin(image.rows, 16);
    fprintf(fp, "%s", rows.c_str());
    string cols = toBin(image.cols, 16);
    fprintf(fp, "%s", cols.c_str());

    //print rows and cols string to console
    cout << "rows: " << rows << endl;
    cout << "cols: " << cols << endl;
    

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
            for(int k =0;k<3;k++){
                image= imageChannels[k];
                //check if pixel is on edge

            if (i == 0 || j == 0)
            {
                int R = image.at<uchar>(i, j);
                
                        
                //create golomb object
                Golomb g = Golomb(m);

                string encodedResidual = g.encode(R);

                //get number of characters in encoded residual
                int n = encodedResidual.length();
                charCount += n;


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

                
                //write R to test.txt

                
                
                
                //create golomb object
                Golomb g = Golomb(m);

                string encodedResidual = g.encode(R);

                //get number of characters in encoded residual
                int n = encodedResidual.length();
                charCount += n;


                //write to file
                
                fprintf(fp, "%s", encodedResidual.c_str());
                
                }

            }

        }
    }

    fclose(fp);
    //Create bitstream object
    ifstream fileOut("out.txt", ios::binary);
    //add number of 0 equal to m at beginning of file out.txt
    
    BitStream bs;
    bs.encode(fileOut, charCount);
    fileOut.close();
    //Delete out.txt
    remove("out.txt");

    

}


int main(int argc, char** argv )
{   


    //input image is argv[2]
    cv::Mat image = cv::imread(argv[1],1);

    //output image is argv[3]
    string dstFile = argv[2];

    if (image.empty()) {
        printf("No image data \n");
        return -1;
    }

    
    //check right number of arguments
    if (argc != 3)
    {
        printf("usage: ./golombImgEncode <SrcImage> <DstFileName>");
        return -1;
    }
    

    //if first argument is "Encode" call function to encode the image
    
        
    //channels matrixes
    

    //Encode each channel
    encode(image);

    //rename out.bin to dstFile
    dstFile = dstFile + ".bin";
    rename("encode.bin", dstFile.c_str());


   

return 0;
}
