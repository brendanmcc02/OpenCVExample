#include "Utilities.h"

void MyApplication(VideoCapture& roadVideo) {
	if (roadVideo.isOpened()) {
		// image variables
		Mat currentFrameOriginal, currentFrameGray, manualOutput, otsuImageBinary, otsuOutput, adaptiveOutput;

		// constants
		const int numberOfFrames = roadVideo.get(CAP_PROP_FRAME_COUNT);
		const int maxThreshold = 255;
		const int thresholdValue = 100;

		while (cv::waitKey(50) != 'q') {
			// move to next frame	
			roadVideo >> currentFrameOriginal;

			// loop video
			if (currentFrameOriginal.empty()) {
				roadVideo.set(CAP_PROP_POS_FRAMES, 0);
				continue; // skip to next iteration
			}

			// convert current frame to grayscale
			cvtColor(currentFrameOriginal, currentFrameGray, COLOR_BGR2GRAY);

			// otsu thresholding
			threshold(currentFrameGray, otsuImageBinary, thresholdValue, maxThreshold, THRESH_BINARY | THRESH_OTSU);
			cvtColor(otsuImageBinary, otsuOutput, COLOR_GRAY2BGR);

			// manual thresholding
			threshold(currentFrameGray, manualOutput, thresholdValue, maxThreshold, THRESH_BINARY);

			// display videos
			imshow("Input Video", currentFrameOriginal);
			imshow("Otsu Thresholding", otsuOutput);
			imshow("Manual", manualOutput);
		}

		cv::destroyAllWindows();
	} else {
		std::cerr << "Error opening video file" << std::endl;
	}
}
