#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

cv::Mat img_origin, img_gray,img_blur, img_canny, img_thr, img_dil, img_warp;




cv::Mat pre_processing(cv::Mat img)
{
	cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(img_gray, img_blur, cv::Size(3, 3), 3, 0);
	cv::Canny(img_blur, img_canny, 25, 75);

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::dilate(img_canny, img_dil, kernel);

	return img_dil;
}

std::vector<cv::Point> get_contours(cv::Mat img)
{

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	std::vector<std::vector<cv::Point>> control(contours.size());
	std::vector<cv::Rect> boundRect(contours.size());

	std::vector<cv::Point> biggest;
	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = cv::contourArea(contours[i]);
		std::cout << area << std::endl;

		if (area > 1000)
		{
			double peri = cv::arcLength(contours[i], true);
			cv::approxPolyDP(contours[i], control[i], 0.02 * peri, true);

			if (area > maxArea && control[i].size() == 4)
			{
				cv::drawContours(img, control, i, cv::Scalar(255, 0, 255), 5);
				biggest = { control[i][0], control[i][1], control[i][2], control[i][3] };
				maxArea = area;
			}

		}

	}
	return biggest;
}

void draw_points(std::vector<cv::Point> points, cv::Scalar color)
{

	for (int i = 0; i < points.size(); i++)
	{
		cv::circle(img_origin, points[i], 10, color, cv::FILLED);
		cv::putText(img_origin, std::to_string(i), points[i], cv::FONT_HERSHEY_PLAIN, 4, color, 4);
	}

}

std::vector<cv::Point> reorder(std::vector<cv::Point> points)
{
	if (points.size() == 4)
	{

		std::vector<cv::Point> new_points;
		std::vector<int> sum_points, sub_points;

		for (int i = 0; i < 4; i++)
		{
			sum_points.push_back(points[i].x + points[i].y);
			sub_points.push_back(points[i].x - points[i].y);
		}

		new_points.push_back(points[std::min_element(sum_points.begin(), sum_points.end()) - sum_points.begin()]);
		new_points.push_back(points[std::max_element(sub_points.begin(), sub_points.end()) - sub_points.begin()]);
		new_points.push_back(points[std::min_element(sub_points.begin(), sub_points.end()) - sub_points.begin()]);
		new_points.push_back(points[std::max_element(sum_points.begin(), sum_points.end()) - sum_points.begin()]);

		return new_points;
	}
	else return points;
}

cv::Mat getWarp(cv::Mat img, std::vector<cv::Point> points, float w, float h)
{

	if (points.size() == 4)
	{
		cv::Point2f src[4] = { points[0],points[1], points[2], points[3] };
		cv::Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

		cv::Mat matrix = cv::getPerspectiveTransform(src, dst);
		cv::warpPerspective(img, img_warp, matrix, cv::Point(w, h));

		return img_warp;
	}
	else return img_origin;
}

void main()
{

	
	cv::VideoCapture cap(0);
	
	while (true)
	{

		std::vector<cv::Point> initial_p, final_p;

		cap.read(img_origin);

		//Preprocessing

		img_thr = pre_processing(img_origin);

		//Get Contour

		initial_p = get_contours(img_thr);

		//drawPoints(initialPoints, cv::Scalar(0, 0, 255));

		final_p = reorder(initial_p);

		draw_points(final_p, cv::Scalar(0, 255, 0));

		//Warp

		img_warp = getWarp(img_origin, final_p, 420, 596);

		//Edit

		/*int cropValue = 7;
		cv::Rect roi(cropValue, cropValue, 420 - (2 * cropValue), 596 - (2 * cropValue));
		imgWarp = imgWarp(roi);*/

		cv::imshow("1", img_origin);
		cv::imshow("2", img_thr);
		cv::imshow("3", img_warp);
		cv::waitKey(1);
	}
	
	
	
}