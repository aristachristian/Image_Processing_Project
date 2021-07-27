//g++ main.cpp `pkg-config --cflags --libs opencv4`
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/ximgproc.hpp>
#include <iostream>
#include <string>
#include "Node.h"
#include "Postfix.h"
#include "Stack.h"

using namespace cv;
using namespace std;
using namespace ximgproc;

void createEndPointKernel(Mat *);
void createBranchPointKernel(Mat *);
void getImage(Mat &, Mat &);
void getFilteredImg(Mat &);
char getOp(const Mat &);
int countWhite(const Mat &);
int countEdge(const Mat &);
void endPoint(const Mat &, Mat &);
void branchPoint(const Mat &, Mat &);
void spur(Mat &, Mat &, const int = 1);
int getY(const Mat &);
int getX(const Mat &);

int main()
{
    Mat imgRgb, imgBw;

    VideoCapture cam(0);
    cam >> imgRgb;
    Rect rec1(imgRgb.cols / 2 - 100, imgRgb.rows / 2 - 100 , 200, 200);
    cam.release();

    char c, op;
    string infix, postfix;
    while (op != '=')
    {
        getImage(imgRgb, imgBw);
        imshow("RGB", imgRgb(rec1));
        getFilteredImg(imgBw);

        waitKey(1000);

        if ((op = getOp(imgBw)))
        {
            cout << "Operand / Operator : " << op << endl;
            
            do
            {
                cout << "Press Y to store Operand / Operator or N to delete Operand / Operator! ";
                cin >> c;
            }while(c != 'Y' && c != 'y' && c != 'N' && c != 'n');

            if (c == 'Y' || c == 'y')
            {
                if (isdigit(op))
                    infix += op;
                else
                    infix = infix + ' ' + op + ' ';
            }else
                op = 0;

            cout << "Equation : " << infix << "\n\n";
        }else
            cout << "Your Image Is Not Recognized\n\n";
    }

    postfix = convertToPostFix(infix);
  
    cout << infix << evaluatePostfixExpression(postfix) << endl;

    return 0;
}

void createEndPointKernel(Mat *kernel)
{
    kernel[0] = (Mat_<int>(3, 3) <<
        0, -1, -1,
        1,  1, -1,
        0, -1, -1);
    for (size_t i = 1; i < 4; ++i)
    {
        rotate(kernel[i - 1], kernel[i], ROTATE_90_CLOCKWISE);
    }

    kernel[4] = (Mat_<int>(3, 3) <<
         1, -1, -1,
        -1,  1, -1,
        -1, -1, -1);
    for (size_t i = 5; i < 8; ++i)
    {
        rotate(kernel[i - 1], kernel[i], ROTATE_90_CLOCKWISE);
    }    
}

void createBranchPointKernel(Mat *kernel)
{
    kernel[0] = (Mat_<int>(3, 3) <<
         0,  1,  0,
        -1,  1,  1,
         0,  1,  0); 
    for (size_t i = 1; i < 4; ++i)
    {
        rotate(kernel[i - 1], kernel[i], ROTATE_90_CLOCKWISE);
    }

    kernel[4] = (Mat_<int>(3, 3) <<
         1,  0,  0,
        -1,  1,  1,
         0,  1,  0); 
    for (size_t i = 5; i < 8; ++i)
    {
        rotate(kernel[i - 1], kernel[i], ROTATE_90_CLOCKWISE);
    }

    kernel[8] = (Mat_<int>(3, 3) <<
         0,  1,  0,
        -1,  1,  1,
         1,  0,  0); 
    for (size_t i = 9; i < 12; ++i)
    {
        rotate(kernel[i - 1], kernel[i], ROTATE_90_CLOCKWISE);
    }

    kernel[12] = (Mat_<int>(3, 3) <<
         1,  0,  0,
        -1,  1,  1,
         1,  0,  0); 
    for (size_t i = 13; i < 16; ++i)
    {
        rotate(kernel[i - 1], kernel[i], ROTATE_90_CLOCKWISE);
    }
}

int countWhite(const Mat &img)
{
    int count = 0;

    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j)
            if (img.at <uint8_t> (i, j))
                ++count;

    return count;
}

void getImage(Mat &imgRgb, Mat &imgBw)
{
    VideoCapture camera(0);
    camera >> imgRgb;

    Rect rec1(imgRgb.cols / 2 - 100, imgRgb.rows / 2 - 100 , 200, 200);
    Rect rec2(imgRgb.cols / 2 - 105, imgRgb.rows / 2 - 105, 210, 210);

    int i = 0;
    for (;;)
    {
        camera >> imgRgb;
        rectangle(imgRgb, rec2, {255, 0, 0}, 5);

        inRange(imgRgb, Scalar(115, 115, 115), Scalar(255, 255, 255), imgBw);
        imgBw = imgBw(rec1);

        imshow("Camera", imgRgb);

        if (countEdge(imgBw) >= 0.9 * 2*(imgBw.rows + imgBw.cols))
        {
            if (i++ >= 20)
                break;
        }else
            i = 0;

        waitKey(25);
    }

    camera.release();
    destroyAllWindows();
}

void getFilteredImg(Mat &img)
{
    Mat kernelSqr;
    kernelSqr = getStructuringElement(MORPH_RECT, Size(3, 3));


    bitwise_not(img, img);

    morphologyEx(img, img, MORPH_OPEN, kernelSqr, Point(-1, -1), 5);
    morphologyEx(img, img, MORPH_CLOSE, kernelSqr, Point(-1, -1), 8);

    thinning(img, img);

    spur(img, img, 30);
}

char getOp(const Mat &img)
{
    Mat imgTemp;
    endPoint(img, imgTemp);
    int ep = countWhite(imgTemp);

    switch (ep)
    { 
        case 0 :
        {
            imgTemp = img.clone();
            branchPoint(imgTemp, imgTemp);
            int bp = countWhite(imgTemp);
            if (bp == 0)
                return '0';
            else if (bp == 2)
                return '8';
            else
                return 0;
        }

        case 1 :
        {
            //6, 9
            imgTemp = img.clone();
            Mat imgTemp1, imgTemp2;
            endPoint(imgTemp, imgTemp1);
            branchPoint(imgTemp, imgTemp2);
            
            if (getY(imgTemp1) > getY(imgTemp2))
                return '9';
            else
                return '6';
        }

        case 2 :
        {
            //1, 2, 5, 7, -
            imgTemp = img.clone();

            Mat imgTemp1, imgTemp2;
            Mat kernelV = (Mat_<int> (3, 3) <<
                -1,  1, -1,
                -1,  1, -1,
                -1,  1, -1);
                
            Mat kernelH = (Mat_<int> (3, 3) <<
                -1, -1, -1,
                 1,  1,  1,
                -1, -1, -1);
                
            morphologyEx(imgTemp, imgTemp1, MORPH_HITMISS, kernelV);
            int vLine = countWhite(imgTemp1);

            morphologyEx(imgTemp, imgTemp2, MORPH_HITMISS, kernelH);
            int hLine = countWhite(imgTemp2);

            if (vLine < 15 && hLine < 15)
                return '/';
            else if (vLine >= 15 && hLine < 15)
                return '1';
            else if (vLine < 15 && hLine >= 15)
                return '-';
            else
            {
                Mat kernelSqr;
                kernelSqr = getStructuringElement(MORPH_RECT, Size(3, 3));
                
                morphologyEx(imgTemp1, imgTemp1, MORPH_DILATE, kernelSqr, Point(-1, -1), 5);
                thinning(imgTemp1, imgTemp1);
                
                endPoint(imgTemp1, imgTemp1);
                imshow("End Point Vertical Line", imgTemp1);
                
                int ep = countWhite(imgTemp1);

                if (ep == 2)
                    return '7';
                else if (ep == 4)
                {
                    endPoint(imgTemp, imgTemp);
                    
                    if (getX(imgTemp) < getX(imgTemp1))
                        return '2';
                    else
                        return '5';
                }else
                    return 0;
            }
        }

        case 3 :
        {
            //3, 4
            imgTemp = img.clone();
            
            Mat kernelH = (Mat_<int> (3, 3) <<
                -1, -1, -1,
                 1,  1,  1,
                -1, -1, -1);

            morphologyEx(imgTemp, imgTemp, MORPH_HITMISS, kernelH);

            Mat kernelSqr;
            kernelSqr = getStructuringElement(MORPH_RECT, Size(3, 3));
            morphologyEx(imgTemp, imgTemp, MORPH_DILATE, kernelSqr, Point(-1, -1), 5);
            thinning(imgTemp, imgTemp);

            endPoint(imgTemp, imgTemp);
            
            waitKey(1000);
            int ep = countWhite(imgTemp);
            
            if (ep == 2)
                return '4';
            else if (ep == 6)
                return '3';
            else
                return 0;
        }

        case 4 :
        {
            //+, x
            imgTemp = img.clone();
            imshow("Number", imgTemp);
            Mat imgTemp1, imgTemp2;
            Mat kernelV = (Mat_<int> (3, 3) <<
                -1,  1, -1,
                -1,  1, -1,
                -1,  1, -1);
                
            Mat kernelH = (Mat_<int> (3, 3) <<
                -1, -1, -1,
                 1,  1,  1,
                -1, -1, -1);
                
            morphologyEx(imgTemp, imgTemp1, MORPH_HITMISS, kernelV);
            int vLine = countWhite(imgTemp1);
            
            morphologyEx(imgTemp, imgTemp2, MORPH_HITMISS, kernelH);
            int hLine = countWhite(imgTemp2);

            if (vLine >= 15 && hLine >= 15)
                return '+';
            else if (vLine < 15 && hLine >= 15)
                return '=';
            else if (vLine < 15 && hLine < 15)
                return '*';
            else
                return 0;
        }

        default :
        {
            //Unrecognized Operator / Operand
            return 0;
        }
    }
}

int countEdge(const Mat &img)
{
    int count = 0;

    for (int i = 0; i < img.rows; ++i)
    {
        if (img.at <uint8_t> (Point(0, i)))
            ++count;

        if(img.at <uint8_t> (Point(img.cols - 1, i)))
            ++count;
    }

    for (int j = 0; j < img.cols; ++j)
    {
        if (img.at <uint8_t> (Point(j, 0)))
            ++count;

        if(img.at <uint8_t> (Point(j, img.rows - 1)))
            ++count;
    }

    return count;
}

void endPoint(const Mat &imgSrc, Mat &imgDst)
{
    Mat kernel[8];
    createEndPointKernel(kernel);

    Mat imgTemp[8];

    for (size_t i = 0; i < 8; ++i)
    {
        morphologyEx(imgSrc, imgTemp[i], MORPH_HITMISS, kernel[i]);
    }

    imgDst = imgTemp[0];
    for (size_t i = 1; i < 8; ++i)
    {
        bitwise_or(imgDst, imgTemp[i], imgDst);
    }
}

void branchPoint(const Mat &imgSrc, Mat &imgDst)
{
    Mat kernel[16];
    createBranchPointKernel(kernel);

    Mat imgTemp[16];

    for (size_t i = 0; i < 16; ++i)
    {
        morphologyEx(imgSrc, imgTemp[i], MORPH_HITMISS, kernel[i]);
    }

    imgDst = imgTemp[0];
    for (size_t i = 1; i < 16; ++i)
    {
        bitwise_or(imgDst, imgTemp[i], imgDst);
    }
}

void spur(Mat &imgSrc, Mat &imgDst, const int iteration)
{
    Mat kernel[8];
    createEndPointKernel(kernel);

    Mat imgEndPoint;

    for (int i = 0; i < iteration; ++i)
    {
        endPoint(imgSrc, imgEndPoint);
        
        for (int j = 0; j < imgEndPoint.rows; ++j)
        {
            for (int k = 0; k < imgEndPoint.cols; ++k)
            {
                if (imgEndPoint.at <uint8_t> (Point(k, j)))
                    imgDst.at <uint8_t> (Point(k, j)) = 0;
            }
        }
    }
}

int getY(const Mat &img)
{
    for (int i = 0; i < img.cols; ++i)
    {
        for (int j = 0; j < img.rows; ++j)
        {
            if (img.at <uint8_t> (Point(i, j)))
                return j;
        }
    }

    return -1;
}

int getX(const Mat &img)
{
    for (int i = 0; i < img.rows; ++i)
    {
        for (int j = 0; j < img.cols; ++j)
        {
            if (img.at <uint8_t> (Point(j, i)))
                return j;
        }
    }

    return -1;
}
