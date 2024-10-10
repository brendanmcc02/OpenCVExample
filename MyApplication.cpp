#include "Utilities.h"

// Ground truth for pedestrian crossings.  Each row contains
// 1. the image number (PC?.jpg)
// 2. the coordinates of the line at the top of the pedestrian crossing (left column, left row, right column, right row)
// 3. the coordinates of the line at the bottom of the pedestrian crossing (left column, left row, right column, right row)
int pedestrian_crossing_ground_truth[][9] = {
	{ 10,0,132,503,113,0,177,503,148},
	{ 11,0,131,503,144,0,168,503,177},
	{ 12,0,154,503,164,0,206,503,213},
	{ 13,0,110,503,110,0,156,503,144},
	{ 14,0,95,503,104,0,124,503,128},
	{ 15,0,85,503,91,0,113,503,128},
	{ 16,0,65,503,173,0,79,503,215},
	{ 17,0,43,503,93,0,89,503,146},
	{ 18,0,122,503,117,0,169,503,176}
};

void get_ground_truth(int image_index, Mat original_image) {
	Mat ground_truth_image = original_image.clone();
	Point * points = new Point[4];
	
	for (int i = 1; i < 5; i++) {
		points[i-1] = Point(pedestrian_crossing_ground_truth[image_index-10][(i*2)-1], pedestrian_crossing_ground_truth[image_index-10][i*2]);
	}

	cv::line(ground_truth_image, points[0], points[1], Scalar(0, 255, 0), 2);
	cv::line(ground_truth_image, points[0], points[2], Scalar(0, 255, 0), 2);
	cv::line(ground_truth_image, points[1], points[3], Scalar(0, 255, 0), 2);
	cv::line(ground_truth_image, points[2], points[3], Scalar(0, 255, 0), 2);
	imshow("Ground Truth", ground_truth_image);
}

void MyApplication() {
	// 	get the image
	char* file_location = "../media/";
	const int num_median_iter = 5;


	for (int image_index = 10; image_index <= 19; image_index++) {
		char filename[200];
		sprintf(filename, "PC%d.jpg", image_index);
		string file(file_location);
		file.append(filename);
		Mat original_image;
		original_image = imread(file, -1);
		// imshow(filename, original_image);

		Mat output_image;

		// Iterative Median smoothing
		Mat* median_images = new Mat[num_median_iter+1];
		median_images[0] = original_image;

		for (int i = 0; i < num_median_iter; i++) {
			medianBlur(median_images[i], median_images[i+1] ,3);
		}

		/*
		// MSS
		Mat mss_image;
		pyrMeanShiftFiltering(median_images[num_median_iter], mss_image, 3, 40, 1);
		floodFillPostprocess(mss_image, Scalar::all(2));  // colour the MSS
		*/

		// Canny Edge Detection
		vector<Mat> input_planes(3);
		Mat processed_image = median_images[num_median_iter].clone();
		vector<Mat> output_planes;
		split(processed_image,output_planes);
		split(median_images[num_median_iter], input_planes);
		for (int plane=0; plane < median_images[num_median_iter].channels(); plane++) {
			Canny(input_planes[plane],output_planes[plane],100,200);
		}
			
		Mat multispectral_edges;
		merge(output_planes, multispectral_edges);

		// show output
		imshow("Output", multispectral_edges);

		get_ground_truth(image_index, original_image);
		
		// go to next image
		char c = cv::waitKey();
		cv::destroyAllWindows();
	}
}
