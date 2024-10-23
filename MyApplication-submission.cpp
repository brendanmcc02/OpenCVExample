/*
#include "Utilities.h"
#include <list>
#define DEBUG false
#define TEST false
// Ground truth for pedestrian crossings.  Each row contains
// 1. the image number (PC?.jpg)
// 2. the coordinates of the line at the top of the pedestrian crossing (left column, left row, right column, right row)
// 3. the coordinates of the line at the bottom of the pedestrian crossing (left column, left row, right column, right row)
int pedestrianCrossingGroundTruth[][9] = {
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

const Scalar WHITE = Scalar(255, 255, 255);
const Scalar BLUE = Scalar(255, 0, 0);
const Scalar GREEN  = Scalar(0, 255, 0);
const int NUM_MEDIAN_BLUR_ITERATIONS = 1;  // tested for optimal value
const int MEDIAN_BLUR_FILTER_SIZE = 3;  // tested for optimal value
const int CLOSING_KERNEL_SIZE = 2;  // tested for optimal value
const int CANNY_MIN_THRESHOLD = 150;  // tested for optimal value
const int CANNY_MAX_THRESHOLD = 255;  // tested for optimal value
const int CONTOUR_SIZE_THRESHOLD = 90;  // tested for optimal value
const int MIN_CONTOUR_AREA_THRESHOLD = 50;  // 50 is pretty loose/safe
const int MAX_CONTOUR_AREA_THRESHOLD = 3000;  // tested for optimal value
const int MIN_HULL_AREA_THRESHOLD = 500;  // train: 400
const int MAX_HULL_AREA_THRESHOLD = 3500;  // tested for optimal value
const float RECTANGULARITY_THRESHOLD = 0.5;  // train: 0.6
const float MIN_HULL_DISTANCE_THRESHOLD = 10.0;  // train: 5.0
const float MAX_HULL_DISTANCE_THRESHOLD = 175.0;  // train: 150
const float WHITE_COLOR_DISTANCE_THRESHOLD = 150.0;  // tested for optimal value
const float LINE_DEGREES_THRESHOLD = 8.0;  // train: 5.0
const float HORIZONTAL_ANGLE_THRESHOLD = 30.0;  // tested for optimal value

Mat getGroundTruth(int imageIndex, Mat& originalImage) {
	Mat groundTruthImage = originalImage.clone();
#if !TEST
	Point * points = new Point[4];
	
	// init points
	for (int i = 1; i < 5; i++) {
		points[i-1] = Point(pedestrianCrossingGroundTruth[imageIndex-10][(i*2)-1], 
							pedestrianCrossingGroundTruth[imageIndex-10][i*2]);
	}

	// Draw lines
	line(groundTruthImage, points[0], points[1], GREEN, 2);
	line(groundTruthImage, points[0], points[2], GREEN, 2);
	line(groundTruthImage, points[1], points[3], GREEN, 2);
	line(groundTruthImage, points[2], points[3], GREEN, 2);
#endif
	return groundTruthImage;
}

float getColorDistance(const Scalar& s1, const Scalar& s2) {
    float dist = 0;

    for (int i = 0; i < 3; ++i) {
        dist += (s1[i] - s2[i]) * (s1[i] - s2[i]);
    }

    return sqrt(dist);
}

double angleBetweenLines(Point2f p1, Point2f p2, Point2f p3, Point2f p4) {
    Point2f v1 = p2 - p1;
    Point2f v2 = p4 - p3;
    
    double dotProduct = v1.x * v2.x + v1.y * v2.y;
    double magV1 = sqrt(v1.x * v1.x + v1.y * v1.y);
    double magV2 = sqrt(v2.x * v2.x + v2.y * v2.y);
    double cosTheta = dotProduct / (magV1 * magV2);
    
    // avoid floating-point precision errors
    if (cosTheta > 1.0) {
		cosTheta = 1.0;
	} else if (cosTheta < -1.0) {
		cosTheta = -1.0;
	}
    
    // Calculate the angle in radians and then convert to degrees
    double angleInRadians = acos(cosTheta);
    double angleInDegrees = angleInRadians * 180.0 / CV_PI;
    
    return angleInDegrees;
}

Mat smoothing(Mat& originalImage) {
	Mat smoothedImage;
	Mat* medianImages = new Mat[NUM_MEDIAN_BLUR_ITERATIONS+1];
	medianImages[0] = originalImage;

	for (int i = 0; i < NUM_MEDIAN_BLUR_ITERATIONS; i++) {
		medianBlur(medianImages[i], medianImages[i+1], MEDIAN_BLUR_FILTER_SIZE);
	}

	smoothedImage = medianImages[NUM_MEDIAN_BLUR_ITERATIONS];
	return smoothedImage;
}

Mat edgeDetection(Mat& image) {
	vector<Mat> inputPlanes(3);
	Mat processedImage = image.clone();
	vector<Mat> outputPlanes;
	split(processedImage, outputPlanes);
	split(image, inputPlanes);
	
	for (int plane = 0; plane < image.channels(); plane++) {
		Canny(inputPlanes[plane], outputPlanes[plane], CANNY_MIN_THRESHOLD, CANNY_MAX_THRESHOLD);
	}
		
	Mat multispectralEdges;
	merge(outputPlanes, multispectralEdges);
	return multispectralEdges;
}

Mat binaryThreshold(Mat& image) {
	Mat grayscaleImage, otsuImage;
	cvtColor(image, grayscaleImage, COLOR_BGR2GRAY);
	threshold(grayscaleImage, otsuImage, 100, 255, THRESH_BINARY | THRESH_OTSU);
	return otsuImage;
}

Mat closing(Mat& image) {
	Mat closingImage;
	Mat kernel(CLOSING_KERNEL_SIZE, CLOSING_KERNEL_SIZE, CV_8U, Scalar(1));
	dilate(image, closingImage, kernel);
	erode(closingImage, closingImage, kernel);
	return closingImage;
}

void MyApplication() {
	// 	get the image
	char* fileLocation = "../media/";
#if TEST
	for (int imageIndex = 1; imageIndex <= 11; imageIndex++) {
		// Get the original image
		char filename[200];
		sprintf_s(filename, "test-%d.jpg", imageIndex);
#else
	for (int imageIndex = 10; imageIndex <= 19; imageIndex++) {
		// Get the original image
		char filename[200];
		sprintf_s(filename, "PC%d.jpg", imageIndex);
#endif
		string file(fileLocation);
		file.append(filename);
		Mat originalImage;
		originalImage = imread(file, -1);
		const Size originalImageSize = originalImage.size();
		const Point HORIZONTAL_LINE_1 = Point(0.0, originalImageSize.height/2.0);
		const Point HORIZONTAL_LINE_2 = Point(originalImageSize.width, originalImageSize.height/2.0);
		Mat groundTruthImage = getGroundTruth(imageIndex, originalImage);

		// Iterative Median smoothing
		Mat smoothedImage = smoothing(originalImage);
		Mat output1 = JoinImagesHorizontally(originalImage, "Original", smoothedImage, "Median Smoothing");

		// Canny Edge Detection
		Mat multispectralEdges = edgeDetection(smoothedImage);
		Mat output2 = JoinImagesHorizontally(output1, "", multispectralEdges, "Canny Edge Detection");
		imshow("Stage 1", output2);

		// Otsu Thresholding
		Mat otsuImage = binaryThreshold(multispectralEdges);

		// Closing Operation: to fill small edge gaps, better results for CCA
		Mat closingImage = closing(otsuImage);

		Mat otsuImageDisplay, closingImageDisplay;
		cvtColor(otsuImage, otsuImageDisplay, COLOR_GRAY2BGR);
		cvtColor(closingImage, closingImageDisplay, COLOR_GRAY2BGR);
		Mat output3 = JoinImagesHorizontally(otsuImageDisplay, "Otsu Thresholding", closingImageDisplay, "Closing");

		// CCA & Shape Analysis
		Mat overlayImage = originalImage.clone();
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		Mat binaryImageCopy = closingImage.clone();
		findContours(binaryImageCopy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
		int contoursLength = contours.size();
		vector<vector<Point>> convexHullsUnfiltered(contoursLength);
		vector<int> convexHullsFiltered(0);
		Mat contoursImage = Mat::zeros(closingImage.size(), CV_8UC3);
		Mat minBoundRectangleImage = Mat::zeros(closingImage.size(), CV_8UC3);
		vector<RotatedRect> minBoundingRectangle(contoursLength);

		for (int i = 0; i < contoursLength; i++) {
			// filter by contour perimeter and area
			int contour_area = contourArea(contours[i]);
			if (contours[i].size() >= CONTOUR_SIZE_THRESHOLD && contour_area >= MIN_CONTOUR_AREA_THRESHOLD
				&& contour_area <= MAX_CONTOUR_AREA_THRESHOLD) {
				// CCA
				drawContours(contoursImage, contours, i, WHITE, FILLED, 8, hierarchy);

				// draw convex hulls around edges
				convexHull(contours[i], convexHullsUnfiltered[i]);
				int convexHullArea = contourArea(convexHullsUnfiltered[i]);

				// filter by convex hull area
				if (convexHullArea >= MIN_HULL_AREA_THRESHOLD && convexHullArea <= MAX_HULL_AREA_THRESHOLD) {
					minBoundingRectangle[i] = minAreaRect(contours[i]);
					int minBoundRectArea = minBoundingRectangle[i].size.width * minBoundingRectangle[i].size.height;
					float rectangularity = ((float)convexHullArea / (float)minBoundRectArea);
					
					// filter by rectangularity
					if (rectangularity >= RECTANGULARITY_THRESHOLD) {
						// Draw the convex hull on 2 different images
						drawContours(overlayImage, convexHullsUnfiltered, i, BLUE, 2);
						// add to filtered hull index list
						convexHullsFiltered.push_back(i);
					}
				}
			}
		}

		Mat output4 = JoinImagesHorizontally(output3, "", contoursImage, "CCA");
		imshow("Stage 2", output4);

		// Check if region is white-ish
		vector<int> whiteRegions(0);
		Mat whiteRegionsImage = originalImage.clone();
		int convexHullsFilteredLength = convexHullsFiltered.size();
		for (int i = 0; i < convexHullsFilteredLength; i++) {
			// get the average colour of the region
			Mat mask = Mat::zeros(originalImageSize, CV_8UC1);
			fillConvexPoly(mask, convexHullsUnfiltered[convexHullsFiltered[i]], Scalar(255));
			Scalar meanColor = mean(originalImage, mask);

			// compare it with white
			float colorDist = getColorDistance(WHITE, meanColor);
			if (colorDist <= WHITE_COLOR_DISTANCE_THRESHOLD) {
				whiteRegions.push_back(convexHullsFiltered[i]);
				drawContours(whiteRegionsImage, convexHullsUnfiltered, convexHullsFiltered[i], BLUE, 2);
			}
		}
		Mat output5 = JoinImagesHorizontally(overlayImage, "Convex Hulls", whiteRegionsImage, "White regions");

		// Filter isolated convex hulls
		vector<Point> convexHullCenters(0);
		vector<int> nonOverlappingConvexHulls(0);
		Mat closeImage = originalImage.clone();
		int whiteRegionsLength = whiteRegions.size();
		int prevX = -1.0;
		int prevY = -1.0;
		
		// get center point of each hull
		for (int i = 0; i < whiteRegionsLength; i++) {
			Moments m = moments(convexHullsUnfiltered[whiteRegions[i]]);
			int centerX = m.m10 / m.m00;
			int centerY = m.m01 / m.m00;

			// if it's the first hull we spot
			if (prevX == -1.0 && prevY == -1.0) {
				convexHullCenters.push_back(Point(centerX, centerY));
				nonOverlappingConvexHulls.push_back(whiteRegions[i]);
				prevX = centerX;
				prevY = centerY;
			}
			// else, check if the current hull isn't overlapping with the previous one
			else if (norm(Point(centerX, centerY) - Point(prevX, prevY)) >= MIN_HULL_DISTANCE_THRESHOLD) {
				convexHullCenters.push_back(Point(centerX, centerY));
				nonOverlappingConvexHulls.push_back(whiteRegions[i]);
				prevX = centerX;
				prevY = centerY;
			}
		}

		// for each hull, calculate its distance to other convex hulls
		vector<int> closeConvexHulls(0);
		int nonOverlappingConvexHullsLength = nonOverlappingConvexHulls.size();
		for (int i = 0; i < nonOverlappingConvexHullsLength; i++) {
			for (int j = 0; j < nonOverlappingConvexHullsLength; j++) {
				if (j != i) {
					float dist = norm(convexHullCenters[i] - convexHullCenters[j]);
					// if there is a close hull
					if (dist <= MAX_HULL_DISTANCE_THRESHOLD) {
						closeConvexHulls.push_back(nonOverlappingConvexHulls[i]);
						j = nonOverlappingConvexHullsLength; // end inner for loop
					}
				}
			}
		}

		// draw the close convexHulls
		Mat potentialCrossingsImage = Mat::zeros(closingImage.size(), CV_8UC3);
		int closeConvexHullsLength = closeConvexHulls.size();
		for (int i = 0; i < closeConvexHullsLength; i++) {
			drawContours(closeImage, convexHullsUnfiltered, closeConvexHulls[i], BLUE, 2);
			drawContours(potentialCrossingsImage, convexHullsUnfiltered, closeConvexHulls[i], BLUE, 2);
		}

		Mat output6 = JoinImagesHorizontally(output5, "", closeImage, "Close Hulls");
		imshow("Stage 3", output6);

		// get center point of each potential crossing
		vector<Point> potentialCrossingsCenters;
		for (int i = 0; i < closeConvexHullsLength; i++) {
			Moments m = moments(convexHullsUnfiltered[closeConvexHulls[i]]);
			int centerX = m.m10 / m.m00;
			int centerY = m.m01 / m.m00;
			potentialCrossingsCenters.push_back(Point(centerX, centerY));
		}

		// Find the longest linear sequence amongst the potential pedestrian crossings
		int maxCount = 2;
		float minAngleSum = 181.0;
#if DEBUG
		Mat debugImage;
#endif
		vector<int> maxPotentialCrossings(0);
		for (int i = 0; i < closeConvexHullsLength - 2; i++) {
			for (int j = i+1; j < closeConvexHullsLength - 1; j++) {
#if DEBUG
				debugImage = originalImage.clone();
				circle(debugImage, potentialCrossingsCenters[i], 3, BLUE, -1);
				circle(debugImage, potentialCrossingsCenters[j], 3, BLUE, -1);
#endif
				int count = 2;
				float angleSum = 0.0;
				vector<int> potentialCrossings = {closeConvexHulls[i], closeConvexHulls[j]};

				for (int k = j+1; k < closeConvexHullsLength; k++) {
					float angle = angleBetweenLines(potentialCrossingsCenters[i], potentialCrossingsCenters[j],
													potentialCrossingsCenters[i], potentialCrossingsCenters[k]);
					// calculate the angle between a horizontal line, and a line going through the centers of the crossing
					float horizontalAngle = angleBetweenLines(potentialCrossingsCenters[i], potentialCrossingsCenters[k], 
														  HORIZONTAL_LINE_1, HORIZONTAL_LINE_2);
#if DEBUG
					waitKey();
					circle(debugImage, potentialCrossingsCenters[k], 3, Scalar(0, 0, 255), -1);
					imshow("Debug", debugImage);
#endif
					if (angle > 90.0) {
						angle = 180.0 - angle;
					}

					if (horizontalAngle > 90.0) {
						horizontalAngle = 180.0 - horizontalAngle;
					}

					if (angle <= LINE_DEGREES_THRESHOLD && horizontalAngle <= HORIZONTAL_ANGLE_THRESHOLD) {
#if DEBUG
						waitKey();
						circle(debugImage, potentialCrossingsCenters[k], 3, BLUE, -1);
						imshow("Debug", debugImage);
#endif
						count++;
						angleSum += angle;
						potentialCrossings.push_back(closeConvexHulls[k]);
					}
				}
				
				// if we actually found a linear sequence with length > 2
				if (count > 2) {
					if (count > maxCount) {
						maxCount = count;
						minAngleSum = angleSum;
						maxPotentialCrossings = potentialCrossings;
						// if we've found a sequence that's already the max possible length, end the loop early
						if (maxCount == closeConvexHullsLength) {
							i = closeConvexHullsLength;
							j = closeConvexHullsLength;
						}	
					}
					// if there's a tie for the longest sequence
					else if (count == maxCount) {
						// choose the sequence that is 'straighter'
						if (minAngleSum > angleSum) {
							minAngleSum = angleSum;
							maxPotentialCrossings = potentialCrossings;
						}
					}
				}
			}
		}

		// draw the potential pedestrian crossings
		int maxPotentialCrossingLength = maxPotentialCrossings.size();
		Mat pedestrianCrossingImage = Mat::zeros(originalImageSize, CV_8UC3);
		for (int i = 0; i < maxPotentialCrossingLength; i++) {
			drawContours(pedestrianCrossingImage, convexHullsUnfiltered, maxPotentialCrossings[i], GREEN, 2);
		}

		imshow("Stage 4", pedestrianCrossingImage);

		// find the min & max y-coordinate out of all the pedestrian crossings
		int minY = originalImageSize.height;
		int minX = 0;
		int maxY = 0;
		int maxX = 0;
		for (int i = 0; i < maxPotentialCrossingLength; i++) {
			for (Point point : convexHullsUnfiltered[maxPotentialCrossings[i]]) {
				
				int y = point.y;
				if (y < minY) {
					minY = y;
					minX = point.x;
				}

				if (y > maxY) {
					maxY = y;
					maxX = point.x;
				}
			}
		}

		// get the average slope of the lines going through the centers of the pedestrian crossings
		float slopeTotal = 0.0;
		int slopeCount = 0;
		for (int i = 0; i < maxPotentialCrossingLength - 1; i++) {
			for (int j = i + 1; j < maxPotentialCrossingLength; j++) {
				Moments m1 = moments(convexHullsUnfiltered[maxPotentialCrossings[i]]);
				int centerX_1 = m1.m10 / m1.m00;
				int centerY_1 = m1.m01 / m1.m00;
				Moments m2 = moments(convexHullsUnfiltered[maxPotentialCrossings[j]]);
				int centerX_2 = m2.m10 / m2.m00;
				int centerY_2 = m2.m01 / m2.m00;
				slopeTotal += (float)(centerY_2 - centerY_1) / (float)(centerX_2 - centerX_1);
				slopeCount++;
			}
		}

		float meanSlope = slopeTotal / slopeCount;

		// draw the enclosing box
		Mat predictedImage = originalImage.clone();
		int width = originalImageSize.width - 1;
		float intercept = maxY - (meanSlope * maxX);
		Point bottomLeft = Point(0, intercept);
		Point bottomRight = Point(width, (meanSlope * width) + intercept);
		intercept = minY - (meanSlope * minX);
		Point topLeft = Point(0, intercept);
		Point topRight = Point(width, (meanSlope * width) + intercept);
		line(predictedImage, bottomLeft, bottomRight, BLUE, 2);
		line(predictedImage, topLeft, topRight, BLUE, 2);
		line(predictedImage, topLeft, bottomLeft, BLUE, 2);
		line(predictedImage, topRight, bottomRight, BLUE, 2);

		Mat output = JoinImagesHorizontally(groundTruthImage, "Original", predictedImage, "Predicted");
		imshow("Final Output", output);

		// go to next image
		waitKey();
		destroyAllWindows();
	}
}
*/
