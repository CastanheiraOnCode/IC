//receives a .bin file and decodes it to a .ppm file

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "golomb.h"
#include "BitStream.h"
using namespace std;


//get size of airplane.ppm


//não efeciente nem permanente
//recieves string representing binary int and returns int
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

//checks if a string is a binary number
bool isBinary(string s){
    for(long unsigned int i = 0; i < s.length(); i++){
        if(s[i] != '0' && s[i] != '1'){
            return false;
        }
    }
    return true;
}






//predictor function receives a matrix and returns a vector with the values to write

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





int main(int argc, char** argv )
{   
    
   

    //receive .bin file from args
    string binFile = argv[1];

    //declare vector
    vector<int> v;


    //output ppm file name
    string ppmFile = argv[2];

    //check for correct number of args
    if(argc != 3){
        cout << "Usage: ./golombImgDecoder <.bin file> <dstFile>" << endl;
        return -1;
    }

    //open .bin file
    ifstream file(binFile, ios::binary);
    
    //n is the number of bits in the file
    int n = 0;
    file.seekg(0, ios::end);
    n = file.tellg();
    file.seekg(0, ios::beg);
    n=n*8;
    
    
    
    //print n
    cout << "n: " << n << endl;


    //create bitstream object
    BitStream bs;
    bs.decode(file,n);


    //close file
    file.close();

    //open decode.txt
    FILE *fp = fopen("decode.txt", "r");

    //m is the number of 0s at the beginning of the file
    int m = 0;
    char c;
    while((c = fgetc(fp)) != '1'){
        if(c == '0'){
            m++;
        }
        else{
            break;
        }
    }

     //create golomb object
    Golomb g = Golomb(m);
    
    //print m
    cout << "m: " << m << endl;

    int width = 0;
    int height = 0;


     //get height string being the next 16 bits
    string heightStr = "";
    for(int i = 0; i < 16; i++){
        heightStr += fgetc(fp);
    }

    //convert heightStr to int
    height = toDec(heightStr);

    //print height
    cout << "height: " << height << endl;



    //get width  string being the next 16 bits
    string widthStr = "";
    for(int i = 0; i < 16; i++){
        widthStr += fgetc(fp);
    }

    //convert widthStr to int
    width = toDec(widthStr);

    //print width
    cout << "width: " << width << endl;

   


    //move begging of file m+1 positions
    fseek(fp, m+33, SEEK_SET);

    //get value of k log2(m)
    int b = log2(m);

    //get value of t
    int t = pow(2,b+1);


    //read decode.txt until end of file
    //while not end of file
    
    while(!feof(fp)){

        
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
            
            v.push_back(R);
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
            v.push_back(R);

        }

        
    }
    
    //print position of file pointer
    cout << "position: " << ftell(fp) << endl;

    //declare img
    cv::Mat img(height, width, CV_8UC1);


    //print length of vector
    cout << "vector length: " << v.size() << endl;


    //vector from every 3rd element
    vector<int> vR;
    for(long unsigned int i = 0; i < v.size(); i+=3){
        vR.push_back(v[i]);
    }

    //vector from every 2nd element
    vector<int> vG;
    for(long unsigned int i = 1; i < v.size(); i+=3){
        vG.push_back(v[i]);
    }

    //vector from every 1st element
    vector<int> vB;
    for(long unsigned int i = 2; i < v.size(); i+=3){
        vB.push_back(v[i]);
    }


    //split outImage into 3 channels
    cv::Mat outImageR(height, width, CV_8UC1);
    cv::Mat outImageG(height, width, CV_8UC1);
    cv::Mat outImageB(height, width, CV_8UC1);

    
    //write vector to image
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            outImageR.at<uchar>(i,j) = vR[i*width + j];
            outImageG.at<uchar>(i,j) = vG[i*width + j];
            outImageB.at<uchar>(i,j) = vB[i*width + j];
        }
    }


    //run prediction on each channel
    predictor(outImageR);
    predictor(outImageG);
    predictor(outImageB);

    //merge channels
    cv::Mat outImage;
    cv::merge(std::vector<cv::Mat>{outImageR, outImageG, outImageB}, outImage);



    //write image to file
    cv::imwrite(ppmFile +".ppm", outImage);

  
    //close file
     fclose(fp);

    //delete decode.txt
    remove("decode.txt");
    
    return 0;
}









