#include <opencv2/opencv.hpp>
#include <stdio.h>
using namespace std;

int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("usage: ppm_copy srcFile outFile\n");
        return -1;
    }

    char* srcFile = argv[argc-2];
    char* outFile = argv[argc-1];
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
    //criar vetor com tamanho pré escolhido
    cv::Mat outImage = srcImage.clone();

    for(int i = 0; i < srcImage.rows ;i++){
        for(int j = 0; j< srcImage.cols; j++){
            outImage.at<uchar>(i,j) = srcImage.at<uchar>(i,j);
        }
    }
    cv::imwrite(outFile,outImage);
    cv::namedWindow("srcImage", cv::WINDOW_AUTOSIZE);
    cv::imshow("srcImage", srcImage);
    cv::namedWindow("outImage",cv::WINDOW_AUTOSIZE);
    cv::imshow("outImage", outImage);
    cv::waitKey(0);
    return 0;
}