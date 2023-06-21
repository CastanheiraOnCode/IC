#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
using namespace std;

cv::Mat outImage;

void negativePixel(cv::Mat srcImage, int i, int j){
    
    int blue = srcImage.at<cv::Vec3b>(i,j)[0];
    int green = srcImage.at<cv::Vec3b>(i,j)[1];
    int red = srcImage.at<cv::Vec3b>(i,j)[2];
    blue = 255 - blue;
    green = 255 - green;
    red = 255 - red;
    outImage.at<cv::Vec3b>(i,j)[0] = blue;
    outImage.at<cv::Vec3b>(i,j)[1] = green;
    outImage.at<cv::Vec3b>(i,j)[2] = red;

}

void mirrorX(cv::Mat srcImage, int i, int j){

    outImage.at<cv::Vec3b>(i,outImage.cols-1-j)[0] = srcImage.at<cv::Vec3b>(i,j)[0];
    outImage.at<cv::Vec3b>(i,outImage.cols-1-j)[1] = srcImage.at<cv::Vec3b>(i,j)[1];
    outImage.at<cv::Vec3b>(i,outImage.cols-1-j)[2] = srcImage.at<cv::Vec3b>(i,j)[2];
}


void mirrorY(cv::Mat srcImage, int i, int j){

    outImage.at<cv::Vec3b>(outImage.rows-1-i,j)[0] = srcImage.at<cv::Vec3b>(i,j)[0];
    outImage.at<cv::Vec3b>(outImage.rows-1-i,j)[1] = srcImage.at<cv::Vec3b>(i,j)[1];
    outImage.at<cv::Vec3b>(outImage.rows-1-i,j)[2] = srcImage.at<cv::Vec3b>(i,j)[2];
}

//rotate image by k*90 degrees
//iteration over columns and lines had to be done here because of the diferent aspect ratio of the image
void rotate90(cv::Mat srcImage, int k){
    int i,j;
    int x,y;
    int width = srcImage.cols;
    int height = srcImage.rows;
    int newWidth = width;
    int newHeight = height;
    if(k%2 == 1){
        newWidth = height;
        newHeight = width;
    }
    outImage = cv::Mat(newHeight,newWidth,CV_8UC3);
    for(i=0;i<height;i++){
        for(j=0;j<width;j++){
            if(k%4 == 0){
                x = i;
                y = j;
            }
            else if(k%4 == 1){
                x = j;
                y = height-1-i;
            }
            else if(k%4 == 2){
                x = height-1-i;
                y = width-1-j;
            }
            else if(k%4 == 3){
                x = width-1-j;
                y = i;
            }
            outImage.at<cv::Vec3b>(x,y)[0] = srcImage.at<cv::Vec3b>(i,j)[0];
            outImage.at<cv::Vec3b>(x,y)[1] = srcImage.at<cv::Vec3b>(i,j)[1];
            outImage.at<cv::Vec3b>(x,y)[2] = srcImage.at<cv::Vec3b>(i,j)[2];
        }
    }
}



void adjustIntensity(cv::Mat srcImage, int i, int j, int intensity){
    
    int blue = srcImage.at<cv::Vec3b>(i,j)[0];
    int green = srcImage.at<cv::Vec3b>(i,j)[1];
    int red = srcImage.at<cv::Vec3b>(i,j)[2];
    blue = blue + intensity;
    green = green + intensity;
    red = red + intensity;
    if(blue > 255) blue = 255;
    if(blue < 0) blue = 0;
    if(green > 255) green = 255;
    if(green < 0) green = 0;
    if(red > 255) red = 255;
    if(red < 0) red = 0;
    outImage.at<cv::Vec3b>(i,j)[0] = blue;
    outImage.at<cv::Vec3b>(i,j)[1] = green;
    outImage.at<cv::Vec3b>(i,j)[2] = red;
}




int main(int argc, char** argv)
{
    if (argc != 4) {
        printf("usage: ppm_effects srcFile outFile effectName\n");
        return -1;
    }

    char* srcFile = argv[argc-3];
    char* outFile = argv[argc-2];
    //criação do array do tipo Mat que vai guardar a imagem original
    cv::Mat srcImage; 
    //Leitura para srcImage da imagem passada na linha de argumentos
    //o segundo argumento converte a imagem para BGR 
    srcImage = cv::imread(srcFile, 1);
    //Verificação do ficheiro escolhido
    if (!srcImage.data) {
        printf("Please choose a valid image \n");
        return -1;

    }

    std::vector<std::string> effectList {"negative","mirrorX", "mirrorY", "rotate90", "changeLightIntensity" };
    string effect = argv[argc-1];
    int opt;
    bool found = false;
    
    for(uint32_t i = 0; i<effectList.size(); i++){
		if (effect == effectList[i]){
			opt = i;
            found = true;
			break;
		}
	}	

    if(!found){
        cerr << "Error: effect: " << effect << " not found\n";
        return 1;
    }


 
    //criar vetor com tamanho pré escolhido
    outImage = srcImage.clone();
    cout << opt << "\n";    
    
    switch (opt)
    {
    case 0:
        for(int i = 0; i < srcImage.rows ;i++){
            for(int j = 0; j< srcImage.cols; j++){  
                negativePixel(srcImage,i,j);
            }
        }
        break;

    case 2:
        for(int i = 0; i < srcImage.rows ;i++){
                for(int j = 0; j< srcImage.cols; j++){
                        mirrorX(srcImage,i,j);
                }
            }
        break;

    case 1:
        for(int i = 0; i < srcImage.rows ;i++){
                for(int j = 0; j< srcImage.cols; j++){
                        mirrorY(srcImage,i,j);
                }
            }
        break;

    case 3:
        //ask for number of rotations
        int k;
        cout << "Number of rotations: ";
        cin >> k;
        rotate90(srcImage,k);
        break;
    
    case 4:
        //ask for intensity
        int intensity;
        cout << "Intensity: ";
        cin >> intensity;
        for(int i = 0; i < srcImage.rows ;i++){
            for(int j = 0; j< srcImage.cols; j++){  
                adjustIntensity(srcImage,i,j,intensity);
            }
        }
        break;

    default:
        
        break;
    }
    cv::imwrite(outFile,outImage);
    cv::namedWindow("srcImage", cv::WINDOW_AUTOSIZE);
    cv::imshow("srcImage", srcImage);
    cv::namedWindow("outImage",cv::WINDOW_AUTOSIZE);
    cv::imshow("outImage", outImage);
    cv::waitKey(0);
    return 0;
}

//01234567
//

//200,200,200,400,400,400,600,600,600
