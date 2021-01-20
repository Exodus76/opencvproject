#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <string>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <regex>

using namespace cv;
namespace fs = std::filesystem;

class rgbpro {
private:
    Mat frame, red_blur, res;
    std::vector<Mat> spl;
    int channel = 2; //2 = R, 1 = B, 0 = G
    std::string username = "midoru", password = "12345";

public:
    bool loginCheck()
    {
        std::string u, pass;
        int tries = 0;
        while (tries < 3) //keep asking for login until out of tries
        {
            std::cout << "enter username: ";
            std::cin >> u;
            std::cout << "enter pass: ";
            std::cin >> pass;       
            if (u == username && pass == password)
            {
                std::cout << "login successful" << std::endl;
                return true;
            }
            else {
                tries++;
            }
        }
        return false;
    };

    //searching for the filename to check if it already exists if it does give it a new name
    std::string setFilename(std::string filename) 
    {
        int count = 0;
        for (auto& p : fs::directory_iterator(fs::current_path()))
        {
            std::string f = p.path().filename().string();
            
            if (f.find(filename + std::to_string(count)) == 0)
            {
                return filename + std::to_string(count + 1);
            }
        };
        return filename + std::to_string(count);
    }

    //function to display all files
    void displayFiles()
    {
        std::smatch s;
        for (auto& p : fs::directory_iterator(fs::current_path()))
        {
            std::string f = p.path().filename().string();
            if (std::regex_search(f, s, std::regex("[0-9]*\\.mp4"))) //regex to search if the filename witht the following pattern is in the folder the program is run
            {
                std::cout << f << std::endl;
            }
        };
    }

    void VideoRecord()
    {
        VideoCapture cap(0);
        if (!cap.isOpened())
        {
            std::cerr << "error cant access camera\n";
            return;
        }

        cap >> frame; //getting a frame to get the frame size type

        if (frame.empty())
        {
            std::cerr << "no frame grabbed\n";
            return;
        }

        Size S1 = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
            (int)cap.get(CAP_PROP_FRAME_HEIGHT));

        std::cout << "Input frame resolution: Width=" << S1.width << "  Height=" << S1.height
            << " of nr#: " << cap.get(CAP_PROP_FRAME_COUNT) << std::endl;

        VideoWriter outputVideo;
        outputVideo.open("./tmp.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), 25.0, S1, true); //set video format to mp4 and fps to 25

        if (!outputVideo.isOpened())
        {
            std::cerr << "cant open the video file to write\n";
            return;
        }

        std::chrono::duration<float> duration;
        auto start = std::chrono::high_resolution_clock::now(); //to check to video recording duration
        for (;;)
        {
            cap.read(frame);
            duration = std::chrono::high_resolution_clock::now() - start; //checking for the time passed with every iteration
            std::cout << duration.count() << std::endl;
            if ((duration.count() > 10.0) || (waitKey(1) >= 0))
                break;

            outputVideo.write(frame);
            imshow("Recording", frame);
        }
        destroyWindow("Recording");
    };

    void Filters(std::string filename)
    {
        VideoCapture inputVideo("./tmp.mp4");
        if (!inputVideo.isOpened())
        {
            std::cout << "cant open input video: " << "./tmp.mp4" << std::endl;
            return;
        }

        Size S2 = Size((int)inputVideo.get(CAP_PROP_FRAME_WIDTH),
            (int)inputVideo.get(CAP_PROP_FRAME_HEIGHT));

        VideoWriter outputVideoFinal;
        outputVideoFinal.open("./" + filename +".mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), 25.0, S2, true);

        if (!outputVideoFinal.isOpened())
        {
            std::cerr << "cant open the video file to write\n";
            return;
        }

        for (;;)
        {
            inputVideo.read(frame);
            if (frame.empty()) break;

            split(frame, spl);
            for (int i = 0; i < 3; ++i)
                if (i != channel)
                    spl[i] = Mat::zeros(S2, spl[0].type());
            merge(spl, res);

            GaussianBlur(res, red_blur, Size(21, 21), 0, 0, BORDER_DEFAULT);    

            outputVideoFinal.write(red_blur);
        }
    }
};

int main()
{
    std::string filename;
    rgbpro r;

    bool logcheck = r.loginCheck();

    if (logcheck == false) //if logging failed exit program
    {
        std::cout << "login failed" << std::endl;
        return 0;
    }

    std::cout << "Enter the desired filename: ";
    std::cin >> filename;

    auto f = r.setFilename(filename);

    r.VideoRecord();
    r.Filters(f);
    std::remove("./tmp.mp4"); //removing the temporary recorded video with no filters

    std::cout << "listing all files coverted by this program in the current folder" << std::endl;
    r.displayFiles();

    return 0;

}
