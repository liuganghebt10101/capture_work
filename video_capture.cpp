#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

int main(){
    cv::VideoCapture cap;
    cap.open(0);
    if (!cap.isOpened()){
        std::cerr << "Error: Could not open camara." << std::endl;
        return -1;
    }
    cv::Mat frame;
    cv::namedWindow("摄像头", cv::WINDOW_NORMAL);
    while (true){
        cap >> frame;
        if (frame.empty()){
        std::cerr <<"Error: Could not read frame."<< std::endl;
        break;
        }
        cv::imshow("摄像头", frame);
        if (27 == cv::waitKey(1)){
        break;
        }
    }
    cap.release();
    cv::destroyAllWindows();
    return 0;
}