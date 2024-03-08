#include "ClientVideoStreamer.h"

void VideoStreamer::CheckVisionEmergency(Mat& frame)
{
    ///////////// juno add emergency code /////////////
    //
    //
    //
    //
    Mat hsv;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    Scalar lower_red = Scalar(0, 100, 100);
    Scalar upper_red = Scalar(10, 255, 255);
    Scalar lower_red2 = Scalar(160, 100, 100);
    Scalar upper_red2 = Scalar(179, 255, 255);

    Mat mask1, mask2, red_mask;
    inRange(hsv, lower_red, upper_red, mask1);
    inRange(hsv, lower_red2, upper_red2, mask2);
    red_mask = mask1 | mask2;

    int red_pixel_count = countNonZero(red_mask);
    int total_pixel_count = frame.rows * frame.cols;
    double red_pixel_ratio = (double)red_pixel_count / total_pixel_count;

    if (red_pixel_ratio > emergency_red_pixel_threshold_ratio_) {
      cout << "Fireeeeeeeeeeeeeeeeee" << endl;
      is_emergency_ = "ok";
    }
    else is_emergency_ = "no";
    //
    //
    //
    //
    ///////////// juno add emergency code /////////////
}