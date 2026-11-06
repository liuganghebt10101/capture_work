#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

int main(){
    std::string path ="/home/liu/桌面/homework/Seminar/capture2/picture/apple.jpg";
    cv::Mat apple =cv::imread(path);
    if(apple.empty()){
        std::cerr << "无法打开照片!!" <<std::endl;
        exit(-1);
    }
    //cv::imshow("apple",apple);
    //cv::waitKey();
    cv::Mat hsv;
    cv::cvtColor(apple, hsv, cv::COLOR_BGR2HSV);
    cv::imshow("hsv", hsv);
    cv::waitKey();
    cv::Scalar lower_red = cv::Scalar(0, 50, 50);
    cv::Scalar upper_red = cv::Scalar(10, 255, 255);
    cv::Mat mask;
    cv::inRange(hsv, lower_red, upper_red, mask);
    cv::imshow("mask", mask);
    cv::waitKey();

    return 0;
}