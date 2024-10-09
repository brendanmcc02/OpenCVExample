/*
 * This code is provided as part of "A Practical Introduction to Computer Vision with OpenCV"
 * by Kenneth Dawson-Howe � Wiley & Sons Inc. 2014.  All rights reserved.
 */
#include "Utilities.h"
#include <iostream>
#include <fstream>
using namespace std;

#define REGENTHOUSE_IMAGE_INDEX 0
#define CAMPANILE1_IMAGE_INDEX 1
#define CAMPANILE2_IMAGE_INDEX 2
#define CREST_IMAGE_INDEX 3
#define OLDLIBRARY_IMAGE_INDEX 4
#define WINDOW1_IMAGE_INDEX 5
#define WINDOW2_IMAGE_INDEX 6
#define WINDOW1_LOCATIONS_IMAGE_INDEX 7
#define WINDOW2_LOCATIONS_IMAGE_INDEX 8
#define BIKES_IMAGE_INDEX 9
#define PEOPLE2_IMAGE_INDEX 10
#define ASTRONAUT_IMAGE_INDEX 11
#define PEOPLE1_IMAGE_INDEX 12
#define PEOPLE1_SKIN_MASK_IMAGE_INDEX 13
#define SKIN_IMAGE_INDEX 14
#define CHURCH_IMAGE_INDEX 15
#define FRUIT_IMAGE_INDEX 16
#define COATS_IMAGE_INDEX 17
#define STATIONARY_IMAGE_INDEX 18
#define PETS124_IMAGE_INDEX 19
#define PETS129_IMAGE_INDEX 20
#define PCB_IMAGE_INDEX 21
#define LICENSE_PLATE_IMAGE_INDEX 22
#define BICYCLE_BACKGROUND_IMAGE_INDEX 23
#define BICYCLE_MODEL_IMAGE_INDEX 24
#define NUMBERS_IMAGE_INDEX 25
#define GOOD_ORINGS_IMAGE_INDEX 26
#define BAD_ORINGS_IMAGE_INDEX 27
#define UNKNOWN_ORINGS_IMAGE_INDEX 28

#define SURVEILLANCE_VIDEO_INDEX 0
#define BICYCLES_VIDEO_INDEX 1
#define ABANDONMENT_VIDEO_INDEX 2

#define HAAR_FACE_CASCADE_INDEX 0



static void help()
{
	cout << "\nThis program demonstrates the famous watershed segmentation algorithm in OpenCV: watershed()\n"
		"Usage:\n"
		"./watershed [image_name -- default is ../data/fruits.jpg]\n" << endl;


	cout << "Hot keys: \n"
		"\tESC - quit the program\n"
		"\tr - restore the original image\n"
		"\tw or SPACE - run watershed segmentation algorithm\n"
		"\t\t(before running it, *roughly* mark the areas to segment on the image)\n"
		"\t  (before that, roughly outline several markers on the image)\n";
}
Mat markerMask, img, output;
Point prevPt(-1, -1);

static void onMouse(int event, int x, int y, int flags, void*)
{
	if (x < 0 || x >= img.cols || y < 0 || y >= img.rows)
		return;
	if (event == EVENT_LBUTTONUP || !(flags & EVENT_FLAG_LBUTTON))
		prevPt = Point(-1, -1);
	else if (event == EVENT_LBUTTONDOWN)
		prevPt = Point(x, y);
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		Point pt(x, y);
		if (prevPt.x < 0)
			prevPt = pt;
		line(markerMask, prevPt, pt, Scalar::all(255), 5, 8, 0);
		line(img, prevPt, pt, Scalar::all(255), 5, 8, 0);
		line(output, prevPt, pt, Scalar::all(255), 5, 8, 0);
		prevPt = pt;
		imshow("image", img);
	}
}

int watershed_main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv, "{help h | | }{ @input | ../data/fruits.jpg | }");
	if (parser.has("help"))
	{
		help();
		return 0;
	}
	string filename = parser.get<string>("@input");
	Mat img0 = imread("media/CoatHanger.jpg", 1), imgGray;

	if (img0.empty())
	{
		cout << "Couldn'g open image " << filename << ". Usage: watershed <image_name>\n";
		return 0;
	}
	help();
	namedWindow("image", 1);

	img0.copyTo(img);
	output = Mat::zeros(img.size(), CV_8UC3);
	/*
	Mat grey_img, distances_image,display;
	cvtColor(img, grey_img, COLOR_BGR2GRAY);
	blur(grey_img, grey_img, Size(7, 7));
	Mat gray_edges = grey_img.clone();
	Canny(grey_img, gray_edges, 100, 200);
	threshold(gray_edges, gray_edges, 127, 255, THRESH_BINARY_INV);
	distanceTransform(gray_edges, distances_image, CV_DIST_L2, 3);
	Mat chamfer_display_image = convert_32bit_image_for_display(distances_image);
	imshow("distances", chamfer_display_image);

	normalize(markerMask, markerMask, 255.0, 0.0, NORM_L2);
	FindLocalMaxima(distances_image, markerMask, 2.0);
	dilate(markerMask, markerMask, Mat());
	cvtColor(gray_edges, display, COLOR_GRAY2BGR);
	imshow("edges", display);
	cvtColor(markerMask, display, COLOR_GRAY2BGR);
	imshow("maxima", display);
*/
	cvtColor(img, markerMask, COLOR_BGR2GRAY);
	cvtColor(markerMask, imgGray, COLOR_GRAY2BGR);
	markerMask = Scalar::all(0);
	imshow("image", img);
	setMouseCallback("image", onMouse, 0);

	for (;;)
	{
		char c = (char)waitKey(0);

		if (c == 27)
			break;

		if (c == 'r')
		{
			markerMask = Scalar::all(0);
			img0.copyTo(img);
			imshow("image", img);
		}

		if (c == 'w' || c == ' ')
		{
			int i, j, compCount = 0;
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;

			findContours(markerMask, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

			if (contours.empty())
				continue;
			Mat markers(markerMask.size(), CV_32S);
			markers = Scalar::all(0);
			int idx = 0;
			for (; idx >= 0; idx = hierarchy[idx][0], compCount++)
				drawContours(markers, contours, idx, Scalar::all(compCount + 1), -1, 8, hierarchy, INT_MAX);

			if (compCount == 0)
				continue;

			vector<Vec3b> colorTab;
			for (i = 0; i < compCount; i++)
			{
				int b = theRNG().uniform(0, 255);
				int g = theRNG().uniform(0, 255);
				int r = theRNG().uniform(0, 255);

				colorTab.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
			}

			double t = (double)getTickCount();
			watershed(img0, markers);
			t = (double)getTickCount() - t;
			printf("execution time = %gms\n", t*1000. / getTickFrequency());

			Mat wshed(markers.size(), CV_8UC3);

			// paint the watershed image
			for (i = 0; i < markers.rows; i++)
				for (j = 0; j < markers.cols; j++)
				{
					int index = markers.at<int>(i, j);
					if (index == -1)
						wshed.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
					else if (index <= 0 || index > compCount)
						wshed.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
					else
						wshed.at<Vec3b>(i, j) = colorTab[index - 1];
				}

			wshed = wshed*0.5 + imgGray*0.5;
			Mat output1 = JoinImagesHorizontally(img0, "Original image", img, "with markers", 4);
		    Mat output2 = JoinImagesHorizontally(output1, "", wshed, "Watershed segmentation", 4);
			imwrite("watershed.bmp", output2);
			imshow("watershed transform", wshed);
		}
	}

	return 0;
}
