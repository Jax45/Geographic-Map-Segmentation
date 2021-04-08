#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
//Jackson Hoenig
//Computer Vision 6420
//Programming Assignment 4
//Date: 3/27/2021
//Description: This program is made to segment out a specific state from the FullColorUSA.png image of the United States Of America.
//	The program first takes in the input image as a command line argument, reads it, and displays it to the user.
//	Next the user is able to click a state they want segmented. That click then extracts the color of the state and removes all
//  other colors from a copy of the input image. Then that image expands a Region Of Interest(ROI) from that point until the next layer
//  of expanding ROI contains no new pixels that are not black. That ROI is then doubled in pixels and superimposed on top of the same
//  spot as the original state was in the input image. Note: If the resized state will go out of bounds of the input image it will be pushed away
//  from the edge and a console message will detail that.
//
//	UPDATE: Added the k-means algorithm as an option that will then instead of removing all other state colors will find the segmented regions
//			in a vector of images and then the program works the same as before by picking the closed region the user chose and ignoring
//			the other states of the same color. This does not work nearly as well as the other method because of the alpha channel in the
//			borders of the states.

//Known bugs for non-k-means:
//When clicking a state near an edge of the screen the roi will increase until it hits that edge of the screen and then stop prematurely.
//
//sometimes if you click a state like michigan too close to another state of the same color, it will increase the roi until it also hits the other state.
// this is needed though to pick up possible islands or disconnected parts of the state. it can possibly be refined to stop at a certain distance from the central region.

//Known bugs for k-means:
// states have extra regions in their cluster that are close to the same color because of the messed up alpha channel in the image when read
// as BGR.

cv::Point userPt = cv::Point(-5, -5);
cv::Mat imgIn;
void getStateZoom(int event, int x, int y, int flags, void* userdata) {
	if (event == cv::EVENT_LBUTTONDOWN) {
		if (imgIn.at<cv::Vec3b>(y, x) == cv::Vec3b(0, 0, 0)) {
			std::cout << "You did not click a state please click a state." << std::endl;
		}
		else {
			userPt = cv::Point(x, y);
			std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
			cv::destroyAllWindows();
		}
	}
}

int main(int argc, const char** argv)
{
	const cv::String keys =
		"{help h usage ?  |                   | print this message }"
		"{k kmeans        |       true        | Use K-Means Clustering Boolean [Default: true] }"
		"{n numclusters   |       49          | The number of clustering regions in the K-means algorithm. }"
		"{@input_image    |                   | Name of file containing image}";
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("Application name v1.0.0");
	//int K = 49;

	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	if (!parser.check())
	{
		parser.printErrors();
		return 0;
	}
	//get the args
	bool kmeans = parser.get<bool>("k");
	int K = parser.get<int>("n");
	cv::String inputFile = parser.get<cv::String>("@input_image");
	try
	{
		//2. Read the input image
		imgIn = cv::imread(inputFile);

		//resize for easier viewing on my laptop.
	/*	cv::Mat imgOut;
		imgIn.copyTo(imgOut);
		cv::resize(imgOut, imgOut, cv::Size(), 0.5, 0.5);
		cv::imwrite("FullColorUSA.0.5.png", imgOut);*/


		//If using k-means, cluster the image first and get the possible regions.
		cv::Mat ocv;
		cv::Mat data;
		//cv::Mat data2;
		std::vector<cv::Mat> clusterImages;
		cv::Mat labels, centers;
		if(kmeans){
			
			imgIn.copyTo(ocv);

			// convert to float & reshape to a [3 x W*H] Mat 
			//  (so every pixel is on a row of it's own)
			ocv.convertTo(data, CV_32F);
			data = data.reshape(1, data.total());

			// do kmeans
			
			cv::kmeans(data, K, labels, cv::TermCriteria(cv::TermCriteria::COUNT, 10, 1.0), 3,
				cv::KMEANS_PP_CENTERS, centers);

			// reshape both to a single row of Vec3f pixels:
			centers = centers.reshape(3, centers.rows);
			data = data.reshape(3, data.rows);

			for (int j = 0; j < K; j++) {
				cv::Mat data2 = data.clone();
				//get vector of clusters
				cv::Vec3f* ptr = data2.ptr<cv::Vec3f>();
				for (size_t i = 0; i < data2.rows; i++) {
					int center_id = labels.at<int>(i);
					if (center_id == j) {
						ptr[i] = centers.at<cv::Vec3f>(center_id);
					}
					else {
						ptr[i] = centers.at<cv::Vec3f>(0);
					}
				}
				cv::Mat o = ocv.clone();
				cv::Mat data3 = data2.clone();
				//data2.copyTo(data3);
				//ocv.copyTo(o);
				o = data3.reshape(3, o.rows);
				o.convertTo(o, CV_8U);
				clusterImages.push_back(o);
			}


			

			// replace pixel values with their center value:
			cv::Vec3f* p = data.ptr<cv::Vec3f>();
			for (size_t i = 0; i < data.rows; i++) {
				int center_id = labels.at<int>(i);
				p[i] = centers.at<cv::Vec3f>(center_id);
			}

			//back to 2d and uchar for cluster images
			/*for (int i = 0; i < clusterImages.size(); i++) {
				clusterImages[i] = clusterImages[i].reshape(3, ocv.rows);
				clusterImages[i].convertTo(clusterImages[i], CV_8U);
			}*/

			// back to 2d, and uchar:
			ocv = data.reshape(3, ocv.rows);
			ocv.convertTo(ocv, CV_8U);
			//end kmeans
		}

		for (int i = 0; i < clusterImages.size(); i++) {
			cv::imshow("Cluster", clusterImages[i]);
			cv::waitKey();
		}
		cv::Mat test = clusterImages[1].clone();
		//cv::imshow("data2", ocv);
		/*3.a Display the image and receive the input(left mouse button click) from the user.*/
		cv::namedWindow("input image", 1);

		cv::setMouseCallback("input image", getStateZoom, NULL);
		cv::imshow("input image", imgIn);
		while (userPt.x < 0 && userPt.y < 0) {
			cv::waitKey();
		}
		//we now have our point.
		/*3.b Extract the state selected*/
		cv::Mat imgInCpy;
		cv::Vec3b color;
		if (!kmeans) {
			//threshold image to color given
			imgIn.copyTo(imgInCpy);

			//get color of state
			color = imgIn.at<cv::Vec3b>(userPt.y, userPt.x);

			//remove all other colors
			for (int i = 0; i < imgInCpy.rows; i++) {
				for (int j = 0; j < imgInCpy.cols; j++) {
					if (imgInCpy.at<cv::Vec3b>(i, j) != color) {
						imgInCpy.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
					}
				}
			}
		}
		else {
			for (int i = 0; i < clusterImages.size(); i++) {
				if (clusterImages[i].at<cv::Vec3b>(userPt.y, userPt.x) != cv::Vec3b(0,0,0)) {
					//this is the correct cluster image
					imgInCpy = clusterImages[i].clone();
					color = clusterImages[i].at<cv::Vec3b>(userPt.y, userPt.x);
					break;
				}
			}
		}

		//remove the other same color states
		//NOTE: HERE WE MAY WANT TO TRY TO KEEP ISLANDS
		bool allFilled = false;
		int distance = 3;
		cv::Point statePt = cv::Point(0, 0);
		int x1 = userPt.x + 1;
		int y1 = userPt.y + 1;
		cv::Mat state;
		while (!allFilled) {
			allFilled = true;
			//create roi
			--x1;
			--y1;
			if (x1 < 0 || y1 < 0 || x1 + distance > imgInCpy.cols || y1 + distance > imgInCpy.rows) {
				//too big roi save previous roi
				x1++;
				y1++;
				distance -= 2;
				cv::Rect roi(x1, y1, distance, distance);
				cv::Mat submat = imgInCpy(roi);
				submat.copyTo(state);
				break;
			}
			cv::Rect roi(x1, y1, distance, distance);
			cv::Mat submat = imgInCpy(roi);

			cv::Vec3b* currentRow;

			for (int j = 0; j < submat.rows; ++j)
			{
				currentRow = submat.ptr<cv::Vec3b>(j);
				for (int i = 0; i < submat.cols; ++i)
				{
					//Skip all non outer layer pixels.
					//allow j=0 any i, j= rows-1 any i, same viceversa
					if ((j != 0 && j != submat.rows - 1) && (i != 0 && i != submat.cols - 1)) {
						continue;
					}
					if (submat.at<cv::Vec3b>(j, i) != cv::Vec3b(0,0,0)) {
						allFilled = false;
					}

				}
			}
			if (allFilled) {
				//save roi
				cv::Rect roi(x1, y1, distance, distance);
				cv::Mat submat = imgInCpy(roi);
				submat.copyTo(state);
				break;
			}
			distance += 2;
			statePt.x++;
			statePt.y++;
		}

		//crop the extra black from the sides or top

		//check left side
		int leftPadding = 0;
		bool stopFlag = false;
		for (int i = 0; i < state.cols; i++) {
			//if any of these pixels is not black stop
			if (stopFlag) {
				break;
			}
			for (int j = 0; j < state.rows; j++) {
				if (state.at<cv::Vec3b>(j, i) != cv::Vec3b(0, 0, 0)) {
					stopFlag = true;
				}
			}
			leftPadding = i;
		}

		//check right side
		int rightPadding = 0;
		stopFlag = false;
		for (int i = state.rows - 1; i > 0; i--) {
			//if any of these pixels is not black stop
			if (stopFlag) {
				break;
			}
			for (int j = 0; j < state.cols; j++) {
				if (state.at<cv::Vec3b>(j, i) != cv::Vec3b(0, 0, 0)) {
					stopFlag = true;
				}
			}
			rightPadding = state.rows - i;
		}
		//check bottom side
		int botPadding = 0;
		stopFlag = false;
		for (int i = state.rows - 1; i > 0; i--) {
			//if any of these pixels is not black stop
			if (stopFlag) {
				break;
			}
			for (int j = 0; j < state.cols; j++) {
				if (state.at<cv::Vec3b>(i, j) != cv::Vec3b(0, 0, 0)) {
					stopFlag = true;
				}
			}
			botPadding = state.rows - i;
		}
		//check top side
		int topPadding = 0;
		stopFlag = false;
		for (int i = 0; i < state.rows; i++) {
			//if any of these pixels is not black stop
			if (stopFlag) {
				break;
			}
			for (int j = 0; j < state.cols; j++) {
				if (state.at<cv::Vec3b>(i, j) != cv::Vec3b(0, 0, 0)) {
					stopFlag = true;
				}
			}
			topPadding = i;
		}

		//update roi with the padding
		cv::Rect roi(leftPadding, topPadding, distance - leftPadding - rightPadding, distance - topPadding - botPadding);
		cv::Mat submat = state(roi);
		submat.copyTo(state);

		/*zoom it to twice the size(preferably by using affine transform) */
		
		float aspectRatio = state.rows / state.cols;

		cv::Mat resizedState;
		cv::resize(state, resizedState, cv::Size((int)(state.cols* 2), (state.rows * 2)));

		/*and display it superimposed on the map with the centroid of the
			zoomed state coincident with the centroid of original state.
			Make sure that the zoomed state stays completely within the original image.*/
		//we need to keep track of the centroid of the state image before and after resizing.
		statePt.x *= 2;
		statePt.y *= 2;

		//from the top left of the resized state to the end
		//We need to check to see if we will over flow the original image. if we are going to we need to adjust enough so that it does not overflow.
		int maxHeight = (resizedState.rows + y1 + topPadding - (resizedState.rows / 4));
		int minHeight = (y1 + topPadding - (resizedState.rows / 4));
		int maxWidth = resizedState.cols + x1 + leftPadding - (resizedState.cols / 4);
		int minWidth = x1 + leftPadding - (resizedState.cols / 4);
		int heightOffset = 0;
		int widthOffset = 0;

		if (minHeight < 0) {
			heightOffset = -1 * minHeight;
			std::cout << "Resized state was too tall and had to be pushed down " << heightOffset << "pixels from the top." << std::endl;
		}
		else if (maxHeight > imgIn.rows) {
			heightOffset = imgIn.rows - maxHeight;
			std::cout << "Resized state was too tall and had to be pushed up " << -1*heightOffset << "pixels from the bottom." << std::endl;
		}

		if (minWidth < 0) {
			widthOffset = -1 * minWidth;
			std::cout << "Resized state was too wide and had to be pushed right " << widthOffset << "pixels from the left." << std::endl;
		}
		else if (maxWidth > imgIn.cols) {
			widthOffset = imgIn.cols - maxWidth;
			std::cout << "Resized state was too wide and had to be pushed left " << widthOffset << "pixels from the right." << std::endl;
		}
		for (int i = 0; i < resizedState.rows; i++) {
			for (int j = 0; j < resizedState.cols; j++) {
				if (resizedState.at<cv::Vec3b>(i, j) != cv::Vec3b(0, 0, 0)) {
					//draw the pixel
					imgIn.at<cv::Vec3b>(i + y1 + topPadding - (resizedState.rows / 4) + heightOffset, j + x1 + leftPadding - (resizedState.cols / 4) + widthOffset) = resizedState.at<cv::Vec3b>(i, j);
				}
			}
		}
		cv::imshow("New Image", imgIn);
		cv::imshow("state", resizedState);
		/*4. Terminate the program when the user hits the character q*/


		/*cv::Mat imgOut;
		state.copyTo(imgOut);
		cv::resize(imgOut, imgOut, cv::Size(), 0.5, 0.5);
		cv::imwrite("MichiganROI.png", imgOut);
		imgIn.copyTo(imgOut);
		cv::resize(imgOut, imgOut, cv::Size(), 0.5, 0.5);
		cv::imwrite("MichiganImgOut.png", imgOut);*/

		while(cv::waitKey() != 'q');

		
	}
	catch (std::string& str)
	{
		std::cerr << "Error: " << argv[0] << ": " << str << std::endl;
		return (1);
	}
	catch (cv::Exception& e)
	{
		std::cerr << "Error: " << argv[0] << ": " << e.msg << std::endl;
		return (1);
	}
	return 0;

}