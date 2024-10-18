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

float color_distance(const Scalar& s1, const Scalar& s2) {
    float dist = 0;

    for (int i = 0; i < 3; ++i) {
        dist += (s1[i] - s2[i]) * (s1[i] - s2[i]);
    }

    return sqrt(dist);
}

void MyApplication() {
	
	// TODO make these safer and increase boundaries?
	const int NUM_MEDIAN_BLUR_ITERATIONS = 1;  // tested for optimal value
	const int MEDIAN_BLUR_FILTER_SIZE = 3;  // tested for optimal value
	const int BINARY_THRESHOLD_VALUE = 100;
	const int BINARY_MAX_THRESHOLD = 255;
	const int CLOSING_KERNEL_SIZE = 2;  // tested for optimal value
	const int CANNY_MIN_THRESHOLD = 150;  // tested for optimal value
	const int CANNY_MAX_THRESHOLD = 255;  // tested for optimal value
	const int CONTOUR_SIZE_THRESHOLD = 90;  // tested for optimal value
	const int MIN_CONTOUR_AREA_THRESHOLD = 50;  // not super optimal, lots of trade-offs with 100 so I set to a safe 50 for now.
	const int MAX_CONTOUR_AREA_THRESHOLD = 3000;  // tested for optimal value
	const int MIN_HULL_AREA_THRESHOLD = 400;  // tested for optimal value
	const int MAX_HULL_AREA_THRESHOLD = 3500;  // tested for optimal value
	const float RECTANGULARITY_THRESHOLD = 0.6;  // tested for optimal value
	const float HULL_DISTANCE_THRESHOLD = 200.0;
	const float COLOR_DISTANCE_THRESHOLD = 150.0;  // tested for optimal value

	// 	get the image
	char* file_location = "../media/";
	for (int image_index = 10; image_index <= 19; image_index++) {
		// get the original image
		char filename[200];
		sprintf(filename, "PC%d.jpg", image_index);
		string file(file_location);
		file.append(filename);
		Mat original_image;
		original_image = imread(file, -1);
		Mat ground_truth = get_ground_truth(image_index, original_image);
		imshow("Original", original_image);

		// Iterative Median smoothing
		Mat smoothed_image;
		Mat* median_images = new Mat[NUM_MEDIAN_BLUR_ITERATIONS+1];
		median_images[0] = original_image;

		for (int i = 0; i < NUM_MEDIAN_BLUR_ITERATIONS; i++) {
			medianBlur(median_images[i], median_images[i+1], MEDIAN_BLUR_FILTER_SIZE);
		}

		smoothed_image = median_images[NUM_MEDIAN_BLUR_ITERATIONS];
		imshow("Median Smoothing", smoothed_image);

		// Canny Edge Detection
		vector<Mat> input_planes(3);
		Mat processed_image = smoothed_image.clone();
		vector<Mat> output_planes;
		split(processed_image, output_planes);
		split(smoothed_image, input_planes);
		
		for (int plane = 0; plane < smoothed_image.channels(); plane++) {
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
		imshow("Otsu Threshold", otsu_image);

		// Closing Operation to fill small edge gaps, better results for CCA
		Mat closing_image;
		Mat kernel(CLOSING_KERNEL_SIZE, CLOSING_KERNEL_SIZE, CV_8U, Scalar(1));
		dilate(otsu_image, closing_image, kernel);
		erode(closing_image, closing_image, kernel);
		imshow("Closing", closing_image);

		// CCA & Shape Analysis
		Mat overlay_image = original_image.clone();
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		Mat binary_image_copy = closing_image.clone();
		findContours(binary_image_copy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
		int contours_length = contours.size();
		vector<vector<Point>> hulls_unfiltered(contours_length);
		vector<vector<Point>> hulls_filtered(0);
		Mat contours_image = Mat::zeros(closing_image.size(), CV_8UC3);
		Mat hull_image = Mat::zeros(closing_image.size(), CV_8UC3);
		Mat min_bound_rectangle_image = Mat::zeros(closing_image.size(), CV_8UC3);
		vector<RotatedRect> min_bounding_rectangle(contours_length);

		for (int c = 0; c < contours_length; c++) {
			// filter by contour perimeter and area
			int contour_area = contourArea(contours[c]);
			if (contours[c].size() >= CONTOUR_SIZE_THRESHOLD && contour_area >= MIN_CONTOUR_AREA_THRESHOLD
				&& contour_area <= MAX_CONTOUR_AREA_THRESHOLD) {
				// CCA
				drawContours(contours_image, contours, c, Scalar(255, 255, 255), FILLED, 8, hierarchy);

				// draw Hulls around edges
				convexHull(contours[c], hulls_unfiltered[c]);
				int hull_area = contourArea(hulls_unfiltered[c]);

				// filter by hull area
				if (hull_area >= MIN_HULL_AREA_THRESHOLD && hull_area <= MAX_HULL_AREA_THRESHOLD) {
					min_bounding_rectangle[c] = minAreaRect(contours[c]);
					int min_bound_rect_area = min_bounding_rectangle[c].size.width * min_bounding_rectangle[c].size.height;
					float rectangularity = ((float)hull_area / (float)min_bound_rect_area);
					
					// filter by rectangularity
					if (rectangularity >= RECTANGULARITY_THRESHOLD) {
						// Draw the convex hull
						drawContours(hull_image, hulls_unfiltered, c, Scalar(255, 0, 0));
						drawContours(overlay_image, hulls_unfiltered, c, Scalar(255, 0, 0), 2);
						// add to filtered hull list
						hulls_filtered.push_back(contours[c]);
					}
				}
			}
		}

		imshow("CCA", contours_image);
		// imshow("Hull", hull_image);
		imshow("Overlay", overlay_image);

		// Filter hulls that are isolated
		vector<vector<Point>> close_hulls(0);
		vector<Point> hull_centers(0);
		Mat close_hulls_image = original_image.clone();
		int hull_filtered_length = hulls_filtered.size();
		// get center point of each hull
		for (int c = 0; c < hull_filtered_length; c++) {
			Moments m = moments(hulls_filtered[c]);

			if (m.m00 != 0) {
				int center_x = m.m10 / m.m00;
            	int center_y = m.m01 / m.m00;
				hull_centers.push_back(Point(center_x, center_y));
			}
		}

		// for each hull, calculate its distance to other hulls
		for (int i = 0; i < hull_filtered_length; i++) {
			bool isClose = false;
			for (int j = 0; j < hull_filtered_length && !isClose && j != i; j++) {
				float dist = norm(hull_centers[i] - hull_centers[j]);
				if (dist <= HULL_DISTANCE_THRESHOLD) {
					isClose = true;
				}
			}

			if (isClose) {
				close_hulls.push_back(hulls_filtered[i]);
			}
		}

		// draw the close hulls
		int close_hulls_length = close_hulls.size();
		for (int c = 0; c < close_hulls_length; c++) {
			drawContours(close_hulls_image, close_hulls, c, Scalar(255, 0, 0), 2);
		}

		imshow("Close Hulls", close_hulls_image);

		// // todo change variables after close_hulls
		// // Check if region is white-ish
		// vector<vector<Point>> potential_pedestrian_crossings(0);
		// Mat potential_image = original_image.clone();
		// int hull_filtered_length = hulls_filtered.size();
		// for (int c = 0; c < hull_filtered_length; c++) {
		// 	// get the average colour of the region
		// 	Mat mask = Mat::zeros(original_image.size(), CV_8UC1);
		// 	fillConvexPoly(mask, hulls_filtered[c], Scalar(255));
		// 	Scalar meanColor = mean(original_image, mask);

		// 	// compare it with white
		// 	Scalar white = Scalar(255, 255, 255);
		// 	float color_dist = color_distance(white, meanColor);
		// 	if (color_dist <= COLOR_DISTANCE_THRESHOLD) {
		// 		potential_pedestrian_crossings.push_back(hulls_filtered[c]);
		// 		drawContours(potential_image, hulls_filtered, c, Scalar(255, 0, 0), 2);
		// 	}
		// }
		
		// imshow("Potential Crossings", potential_image);

		// go to next image
		char c = cv::waitKey();
		cv::destroyAllWindows();
		
	}
}
