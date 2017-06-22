//=====================================
// 画像ファイルを線画に変換する処理
//=====================================
#include"stdafx.h"

#include <opencv2/imgproc/imgproc.hpp>

#include"ConvertImage2LineImage.h"


/** 画像ファイルを線画に変換する */
cv::Mat ConvertImage2LineImage(cv::Mat& image)
{
	cv::Mat image_gray;
	if(image.channels() > 1)
	{
		// グレースケールに変換する
		cvtColor(image, image_gray,CV_RGB2GRAY);
	}
	else
	{
		image_gray = image;
	}

	// 白い部分を膨張させる
	cv::Mat element(5, 5, CV_8S, cv::Scalar(255));
	cv::Mat image_gray_dilate;
	cv::dilate(image_gray, image_gray_dilate, element);

	// 差を取る
	cv::Mat image_diff;
	cv::absdiff(image_gray, image_gray_dilate, image_diff);

	// 白黒反転
	cv::Mat image_diff_rev = 255-image_diff;

	//// シグモイド関数を通して濃淡の極端化
	//IplImage image = image_diff_rev;
	//double threshold = 210;
	//for(unsigned int y=0; y<image.height; y++)
	//{
	//	for(unsigned int x=0; x<image.width; x++)
	//	{
	//		double preValue = (unsigned char)*(image.imageData + y*image.widthStep + x*image.nChannels);

	//		double tmpValue = (preValue - threshold);
	//		if(tmpValue < 0)
	//			tmpValue = tmpValue / threshold;
	//		else
	//			tmpValue = tmpValue / (0xFF - threshold);
	//		double result = 1.0 / (1.0 + exp(-5.0*tmpValue));

	//		*(image.imageData + y*image.widthStep + x*image.nChannels) = (unsigned char)(result * 0xFF);
	//	}
	//}

	return image_diff_rev;
}

/** 画像ファイルを読み込んで線画に変換する */
cv::Mat ConvertImage2LineImageFromFile(const boost::filesystem::path& filePath)
{
	// 画像ファイルを読み込む
	cv::Mat image_gray = cv::imread(filePath.string(), CV_LOAD_IMAGE_GRAYSCALE);
	if(image_gray.data == NULL)
			return cv::Mat();

	return ConvertImage2LineImage(image_gray);
}