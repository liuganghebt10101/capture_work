#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

struct ColorInfo {
    std::string name;
    cv::Scalar lower1;
    cv::Scalar upper1;
    cv::Scalar lower2;
    cv::Scalar upper2;
    bool isRed;
    cv::Scalar bgrColor;
};

struct DetectedBlock {
    cv::Rect rect;
    std::string colorName;
    cv::Scalar drawColor;
};

int getDistance(cv::Rect a, cv::Rect b) {
    cv::Point ca = (a.tl() + a.br()) / 2;
    cv::Point cb = (b.tl() + b.br()) / 2;
    return std::abs(ca.x - cb.x) + std::abs(ca.y - cb.y);
}

int main() {
    std::string videoPath = "/home/liu/桌面/homework/Seminar/capture3/video/cube.mp4"; 
    cv::VideoCapture cap(videoPath);

    if (!cap.isOpened()) {
        std::cerr << "无法打开视频文件！" << std::endl;
        return -1;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps == 0) fps = 30.0;

    cv::Mat frame, hsv, mask;
    
    std::vector<ColorInfo> colors = {
        {"Red",    cv::Scalar(0, 150, 100),   cv::Scalar(10, 255, 255),  cv::Scalar(165, 150, 100), cv::Scalar(180, 255, 255), true,  cv::Scalar(0, 0, 255)},
        {"Orange", cv::Scalar(12, 160, 150),  cv::Scalar(24, 255, 255),  cv::Scalar(0, 0, 0),       cv::Scalar(0, 0, 0),       false, cv::Scalar(0, 165, 255)},
        {"Yellow", cv::Scalar(25, 100, 150),  cv::Scalar(35, 255, 255),  cv::Scalar(0, 0, 0),       cv::Scalar(0, 0, 0),       false, cv::Scalar(0, 255, 255)},
        {"Green",  cv::Scalar(35, 80, 100),   cv::Scalar(85, 255, 255),  cv::Scalar(0, 0, 0),       cv::Scalar(0, 0, 0),       false, cv::Scalar(0, 255, 0)},
        {"Blue",   cv::Scalar(86, 80, 100),   cv::Scalar(130, 255, 255), cv::Scalar(0, 0, 0),       cv::Scalar(0, 0, 0),       false, cv::Scalar(255, 0, 0)},
        {"White",  cv::Scalar(0, 0, 180),     cv::Scalar(180, 50, 255),  cv::Scalar(0, 0, 0),       cv::Scalar(0, 0, 0),       false, cv::Scalar(255, 255, 255)}
    };

    cv::namedWindow("Cube Detection", cv::WINDOW_NORMAL);
    cv::resizeWindow("Cube Detection", 800, 600);

    std::map<std::string, long long> total_color_counts;
    for (auto& c : colors) total_color_counts[c.name] = 0;

    long long frame_count = 0;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        double current_time_sec = frame_count / fps;
        frame_count++;

        if (current_time_sec < 3.0 || current_time_sec > 27.0) {
            cv::putText(frame, "Waiting (3s - 27s)...", cv::Point(30, 50), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 255), 2);
            cv::imshow("Cube Detection", frame);
            if (cv::waitKey(30) == 27) break;
            continue;
        }

        cv::Mat lab;
        cv::cvtColor(frame, lab, cv::COLOR_BGR2Lab);
        std::vector<cv::Mat> lab_planes(3);
        cv::split(lab, lab_planes);
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        clahe->apply(lab_planes[0], lab_planes[0]);
        cv::merge(lab_planes, lab);
        cv::cvtColor(lab, frame, cv::COLOR_Lab2BGR);

        cv::GaussianBlur(frame, frame, cv::Size(3, 3), 0);
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

        std::map<std::string, int> current_frame_counts;
        std::vector<DetectedBlock> rawBlocks;
        for (auto& color : colors) {
            if (color.isRed) {
                cv::Mat mask1, mask2;
                cv::inRange(hsv, color.lower1, color.upper1, mask1);
                cv::inRange(hsv, color.lower2, color.upper2, mask2);
                cv::bitwise_or(mask1, mask2, mask);
            } else {
                cv::inRange(hsv, color.lower1, color.upper1, mask);
            }

            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
            cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
            cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            for (size_t i = 0; i < contours.size(); i++) {
                double area = cv::contourArea(contours[i]);
                
                if (area > 300 && area < 6000) {
                    cv::Rect rect = cv::boundingRect(contours[i]);
                    float aspect_ratio = (float)rect.width / rect.height;
                    
                    if (aspect_ratio > 0.6 && aspect_ratio < 1.6) {
                        std::vector<cv::Point> approx;
                        double peri = cv::arcLength(contours[i], true);
                        cv::approxPolyDP(contours[i], approx, 0.02 * peri, true);
                        
                        if (approx.size() >= 4 && approx.size() <= 6) {
                            rawBlocks.push_back({rect, color.name, color.bgrColor});
                        }
                    }
                }
            }
        }

        std::vector<DetectedBlock> validBlocks;
        for (size_t i = 0; i < rawBlocks.size(); i++) {
            int neighborCount = 0;
            for (size_t j = 0; j < rawBlocks.size(); j++) {
                if (i == j) continue;
                
                int dist = getDistance(rawBlocks[i].rect, rawBlocks[j].rect);
                int threshold = (rawBlocks[i].rect.width + rawBlocks[j].rect.width) * 1.8;
                
                if (dist < threshold) neighborCount++;
            }
            
            if (neighborCount >= 1) {
                validBlocks.push_back(rawBlocks[i]);
                current_frame_counts[rawBlocks[i].colorName]++;
            }
        }

        bool is_cube_present = (validBlocks.size() >= 4);

        if (is_cube_present) {
            std::vector<cv::Point> all_pts;
            for (const auto& block : validBlocks) {
                all_pts.push_back(block.rect.tl());
                all_pts.push_back(block.rect.br());
            }
            cv::Rect total_rect = cv::boundingRect(all_pts);
            
            cv::rectangle(frame, total_rect, cv::Scalar(255, 255, 255), 2);

            for (const auto& block : validBlocks) {
                cv::rectangle(frame, block.rect, block.drawColor, 2);
                cv::putText(frame, block.colorName, cv::Point(block.rect.x, block.rect.y - 5), 
                            cv::FONT_HERSHEY_SIMPLEX, 0.4, block.drawColor, 1);
            }

            for (auto const& [name, count] : current_frame_counts) {
                total_color_counts[name] += count;
            }
        } else {
            current_frame_counts.clear();
        }

        cv::Mat overlay = frame.clone();
        cv::rectangle(overlay, cv::Rect(0, 0, 200, 260), cv::Scalar(0, 0, 0), -1);
        cv::addWeighted(overlay, 0.6, frame, 0.4, 0, frame);

        int y_offset = 30;
        std::string time_text = "Time: " + std::to_string((int)current_time_sec) + "s";
        cv::putText(frame, time_text, cv::Point(10, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
        y_offset += 30;
        
        std::string status = is_cube_present ? "Status: Found" : "Status: Searching";
        cv::Scalar statusColor = is_cube_present ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        cv::putText(frame, status, cv::Point(10, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.6, statusColor, 2);
        y_offset += 30;
        
        for (auto const& [name, count] : current_frame_counts) {
            std::string text = name + ": " + std::to_string(count);
            cv::putText(frame, text, cv::Point(15, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 200, 200), 1);
            y_offset += 25;
        }

        y_offset += 5;
        cv::putText(frame, "-- Total --", cv::Point(15, y_offset),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
        y_offset += 22;
        for (auto& c : colors) {
            std::string text = c.name + ": " + std::to_string(total_color_counts[c.name]);
            cv::putText(frame, text, cv::Point(15, y_offset),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, c.bgrColor, 1);
            y_offset += 22;
        }

        cv::imshow("Cube Detection", frame);

        if (cv::waitKey(30) == 27) break;
    }

    std::cout << "\n========== 最终统计（视频 3s~27s 内累计）==========\n";
    for (auto& c : colors) {
        std::cout << c.name << ": " << total_color_counts[c.name] << " 次\n";
    }
    std::cout << "===================================================\n";

    cv::Mat resultImg(400, 600, CV_8UC3, cv::Scalar(30, 30, 30));
    cv::putText(resultImg, "Final Color Counts (3s-27s)",
                cv::Point(30, 50), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 255, 255), 2);
    int y = 110;
    for (auto& c : colors) {
        std::string text = c.name + ": " + std::to_string(total_color_counts[c.name]);
        cv::putText(resultImg, text, cv::Point(60, y),
                    cv::FONT_HERSHEY_SIMPLEX, 0.9, c.bgrColor, 2);
        y += 45;
    }
    cv::imshow("Final Result", resultImg);
    cv::waitKey(0);

    cap.release();
    cv::destroyAllWindows();
    return 0;
}