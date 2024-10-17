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

Mat get_ground_truth(int image_index, Mat original_image) {
	Mat ground_truth_image = original_image.clone();
	Point * points = new Point[4];
	
	// init points
	for (int i = 1; i < 5; i++) {
		points[i-1] = Point(pedestrian_crossing_ground_truth[image_index-10][(i*2)-1], 
							pedestrian_crossing_ground_truth[image_index-10][i*2]);
	}

	// Draw lines
	cv::line(ground_truth_image, points[0], points[1], Scalar(0, 255, 0), 2);
	cv::line(ground_truth_image, points[0], points[2], Scalar(0, 255, 0), 2);
	cv::line(ground_truth_image, points[1], points[3], Scalar(0, 255, 0), 2);
	cv::line(ground_truth_image, points[2], points[3], Scalar(0, 255, 0), 2);
	return ground_truth_image;
}

void MyApplication() {
	// 	get the image
	char* file_location = "../media/";
	const int NUM_MEDIAN_BLUR_ITERATIONS = 10;
	const int MEDIAN_BLUR_FILTER_SIZE = 3;  // tested for optimal value
	const int BINARY_THRESHOLD_VALUE = 100;
	const int BINARY_MAX_THRESHOLD = 255;
	const int CANNY_MIN_THRESHOLD = 150;  // tested for optimal value
	const int CANNY_MAX_THRESHOLD = 255;  // tested for optimal value
	const int CONTOUR_SIZE_THRESHOLD = 90;  // tested for optimal value
	const int EDGE_SIZE_THRESHOLD = 150;  // tested for optimal value

	for (int image_index = 10; image_index <= 19; image_index++) {
		// get the original image
		char filename[200];
		sprintf(filename, "PC%d.jpg", image_index);
		string file(file_location);
		file.append(filename);
		Mat original_image;
		original_image = imread(file, -1);
		Mat ground_truth = get_ground_truth(image_index, original_image);

		// HLS
		Mat hls_image;
		cvtColor(original_image, hls_image, COLOR_BGR2HLS);
		vector<Mat> hls_channels(3);
		split(hls_image, hls_channels);
		equalizeHist(hls_channels[1], hls_channels[1]);
		imshow("Equalized Luminance", hls_channels[1]);

		/*
		// Iterative Median smoothing
		Mat* median_images = new Mat[NUM_MEDIAN_BLUR_ITERATIONS+1];
		median_images[0] = original_image;

		for (int i = 0; i < NUM_MEDIAN_BLUR_ITERATIONS; i++) {
			medianBlur(median_images[i], median_images[i+1], MEDIAN_BLUR_FILTER_SIZE);
		}

		imshow("Median Smoothing", median_images[NUM_MEDIAN_BLUR_ITERATIONS]);

		// Canny Edge Detection
		vector<Mat> input_planes(3);
		Mat processed_image = median_images[NUM_MEDIAN_BLUR_ITERATIONS].clone();
		vector<Mat> output_planes;
		split(processed_image, output_planes);
		split(median_images[NUM_MEDIAN_BLUR_ITERATIONS], input_planes);
		
		for (int plane = 0; plane < median_images[NUM_MEDIAN_BLUR_ITERATIONS].channels(); plane++) {
			Canny(input_planes[plane], output_planes[plane], CANNY_MIN_THRESHOLD, CANNY_MAX_THRESHOLD);
		}
			
		Mat multispectral_edges;
		merge(output_planes, multispectral_edges);
		imshow("Canny Edge Detection", multispectral_edges);

		// Binary Threshold - Otsu
		Mat grayscale_image, otsu_image, otsu_output;
		cvtColor(multispectral_edges, grayscale_image, COLOR_BGR2GRAY);
		threshold(grayscale_image, otsu_image, BINARY_THRESHOLD_VALUE, 
				  BINARY_MAX_THRESHOLD, THRESH_BINARY | THRESH_OTSU);
		imshow("2 - Otsu Threshold", otsu_image);

		// CCA
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		Mat binary_image_copy = otsu_image.clone();
		findContours(binary_image_copy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
		Mat contours_image = Mat::zeros(otsu_image.size(), CV_8UC3);
		Mat min_bound_rectangle_image = Mat::zeros(otsu_image.size(), CV_8UC3);
		vector<RotatedRect> min_bounding_rectangle(contours.size());

		// Shape analysis
		for (int c = 0; c < contours.size(); c++) {
			// filter by contour size
			if (contours[c].size() >= CONTOUR_SIZE_THRESHOLD) {
				drawContours(contours_image, contours, c, Scalar(255, 255, 255), FILLED, 8, hierarchy);
				// Determine the minimum bounding rectangle
				min_bounding_rectangle[c] = minAreaRect(contours[c]);
				Point2f bounding_rect_points[4];
				min_bounding_rectangle[c].points(bounding_rect_points);
				int length = norm(bounding_rect_points[0] - bounding_rect_points[1]);
				int width = norm(bounding_rect_points[1] - bounding_rect_points[2]);
				// filter by edge (length or width) size
				if (length <= EDGE_SIZE_THRESHOLD && width <= EDGE_SIZE_THRESHOLD) {
					line(min_bound_rectangle_image, bounding_rect_points[0], bounding_rect_points[1], Scalar(255, 0, 0));
					line(min_bound_rectangle_image, bounding_rect_points[1], bounding_rect_points[2], Scalar(255, 0, 0));
					line(min_bound_rectangle_image, bounding_rect_points[2], bounding_rect_points[3], Scalar(255, 0, 0));
					line(min_bound_rectangle_image, bounding_rect_points[3], bounding_rect_points[0], Scalar(255, 0, 0));
				}
			}
		}

		imshow("CCA", contours_image);
		imshow("Shape Analysis", min_bound_rectangle_image);

		// show output
		Mat output = min_bound_rectangle_image;
		Mat split_screen = JoinImagesHorizontally(ground_truth, "Original", output, "Output", 4);
		imshow("Original vs Output", split_screen);
		*/

		// go to next image
		char c = cv::waitKey();
		cv::destroyAllWindows();
	}
}

/*

// MSS
Mat mss_image;
pyrMeanShiftFiltering(median_images[NUM_MEDIAN_BLUR_ITERATIONS], mss_image, 3, 40, 1);
floodFillPostprocess(mss_image, Scalar::all(2));  // colour the MSS

*/
