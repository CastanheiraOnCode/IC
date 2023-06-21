#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

double getPSNR(const Mat& I1, const Mat& I2)
{
     Mat s1;
    absdiff(I1, I2, s1);       // |I1 - I2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |I1 - I2|^2

    Scalar s = sum(s1);        // sum elements per channel

    double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

    if (sse <= 1e-10) // for small values return zero
        return 0;
    else
    {
        double mse = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }
}

int main(int argc, char** argv)
{
// Check if the required number of arguments is provided
if (argc != 3)
{
cout << "Error: invalid number of arguments" << endl;
cout << "Usage: " << argv[0] << " <video_file1> <video_file2>" << endl;
return -1;
}

// Open the video files
VideoCapture cap1(argv[1]);
VideoCapture cap2(argv[2]);

// Check if the video files are opened successfully
if (!cap1.isOpened() || !cap2.isOpened())
{
    cout << "Error: Unable to open video files" << endl;
    return -1;
}

// Get the frame rate and total number of frames in each video
double fps1 = cap1.get(CAP_PROP_FPS);
double fps2 = cap2.get(CAP_PROP_FPS);
int total_frames1 = cap1.get(CAP_PROP_FRAME_COUNT);
int total_frames2 = cap2.get(CAP_PROP_FRAME_COUNT);

// Check if the frame rate and number of frames are valid
if (fps1 <= 0 || fps2 <= 0 || total_frames1 <= 0 || total_frames2 <= 0)
{
    cout << "Error: Invalid frame rate or number of frames" << endl;
    return -1;
}



// Set the total number of frames to the minimum of the two videos
int total_frames = min(total_frames1, total_frames2);

double psnr_sum = 0;
double psnr_avg = 0;

// Loop through all the frames in the videos
for (int i = 0; i < total_frames; i++)
{
// Read the frames from the videos
Mat frame1, frame2;
cap1 >> frame1;
cap2 >> frame2;

// Check if the frames are empty
if (frame1.empty() || frame2.empty())
{
    cout << "Error: empty frame" << endl;
    return -1;
}

// Calculate the PSNR for the current frame
double psnr = getPSNR(frame1, frame2);

// Add the PSNR value to the sum
psnr_sum += psnr;

}

// Calculate the average PSNR for all the frames
psnr_avg = psnr_sum / total_frames;

// Print the average PSNR
cout << "Average PSNR: " << psnr_avg << endl;

return 0;
}