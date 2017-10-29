//���л��� VS2012+opencv3.0
#include "opencv2/opencv.hpp"
#include <fstream>
using namespace std;
using namespace cv;

int main()
{
    ofstream fout("caliberation_result.txt");  /**    ���涨�������ļ�     **/

    /************************************************************************  
           ��ȡÿһ��ͼ�񣬴�����ȡ���ǵ㣬Ȼ��Խǵ���������ؾ�ȷ��  
    *************************************************************************/   
    cout<<"CORNER......."<<endl; 
    int image_count=  24;                    /****    ͼ������     ****/    
    Size board_size = Size(7,5);            /****    �������ÿ�С��еĽǵ���       ****/  
    vector<Point2f> corners;                  /****    ����ÿ��ͼ���ϼ�⵽�Ľǵ�       ****/
    vector<vector<Point2f> >  corners_Seq;    /****  �����⵽�����нǵ�       ****/   
    vector<Mat>  image_Seq;
	int successImageNum = 0;				/****	�ɹ���ȡ�ǵ������ͼ����	****/

    int count = 0;
    for( int i = 0;  i != image_count ; i++)
    {
        cout<<"Frame #"<<i+1<<"..."<<endl;
        string imageFileName;
        std::stringstream StrStm;
        StrStm<<i+1;
        StrStm>>imageFileName;
        imageFileName += ".jpg";
        cv::Mat image = imread("img"+imageFileName); 
        /* ��ȡ�ǵ� */   
        Mat imageGray;
        cvtColor(image, imageGray , CV_RGB2GRAY);
        bool patternfound = findChessboardCorners(image, board_size, corners,CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE+ 
            CALIB_CB_FAST_CHECK );
        if (!patternfound)   
        {   
            cout<<"can not find chessboard corners!\n";  
            continue;
            exit(1);   
        } 
        else
        {   
            /* �����ؾ�ȷ�� */
            cornerSubPix(imageGray, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            /* ���Ƽ�⵽�Ľǵ㲢���� */
            Mat imageTemp = image.clone();
            for (int j = 0; j < corners.size(); j++)
            {
                circle( imageTemp, corners[j], 10, Scalar(0,0,255), 2, 8, 0);
            }
            string imageFileName;
            std::stringstream StrStm;
            StrStm<<i+1;
            StrStm>>imageFileName;
            imageFileName += "_corner.jpg";
            imwrite(imageFileName,imageTemp);
            cout<<"Frame corner#"<<i+1<<"...end"<<endl;

            count = count + corners.size();
			successImageNum = successImageNum + 1;
            corners_Seq.push_back(corners);
        }   
        image_Seq.push_back(image);
    }   
    cout<<"CORNER SUCCESS!!!\n"; 
    /************************************************************************  
           ���������  
    *************************************************************************/   
    cout<<"STEP1....."<<endl;  
	Size square_size = Size(20,20);     
	vector<vector<Point3f> >  object_Points;        /****  ���涨����Ͻǵ����ά����   ****/

    Mat image_points = Mat(1, count, CV_32FC2, Scalar::all(0));  /*****   ������ȡ�����нǵ�   *****/
    vector<int>  point_counts;                                                         
    /* ��ʼ��������Ͻǵ����ά���� */
	for (int t = 0; t<successImageNum; t++)
    {
        vector<Point3f> tempPointSet;
        for (int i = 0; i<board_size.height; i++)
        {
            for (int j = 0; j<board_size.width; j++)
            {
                /* ���趨��������������ϵ��z=0��ƽ���� */
                Point3f tempPoint;
                tempPoint.x = i*square_size.width;
                tempPoint.y = j*square_size.height;
                tempPoint.z = 0;
                tempPointSet.push_back(tempPoint);
            }
        }
        object_Points.push_back(tempPointSet);
    }
	for (int i = 0; i< successImageNum; i++)
    {
        point_counts.push_back(board_size.width*board_size.height);
    }
    /* ��ʼ���� */
    Size image_size = image_Seq[0].size();
    cv::Matx33d intrinsic_matrix;    /*****    ������ڲ�������    ****/
    cv::Vec4d distortion_coeffs;     /* �������4������ϵ����k1,k2,k3,k4*/
    std::vector<cv::Vec3d> rotation_vectors;                           /* ÿ��ͼ�����ת���� */
    std::vector<cv::Vec3d> translation_vectors;                        /* ÿ��ͼ���ƽ������ */
    int flags = 0;
    flags |= cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC;
    flags |= cv::fisheye::CALIB_CHECK_COND;
    flags |= cv::fisheye::CALIB_FIX_SKEW;
    fisheye::calibrate(object_Points, corners_Seq, image_size, intrinsic_matrix, distortion_coeffs, rotation_vectors, translation_vectors, flags, cv::TermCriteria(3, 20, 1e-6));
    cout<<"STEP1 SUCCESS\n";   

    /************************************************************************  
           �Զ�������������  
    *************************************************************************/   
    cout<<"JUDGE THE RESULT...."<<endl;   
    double total_err = 0.0;                   /* ����ͼ���ƽ�������ܺ� */   
    double err = 0.0;                        /* ÿ��ͼ���ƽ����� */   
    vector<Point2f>  image_points2;             /****   �������¼���õ���ͶӰ��    ****/   

    cout<<"ERROR1:"<<endl;   
    cout<<"ERROR2"<<endl<<endl;   
    for (int i=0;  i<image_count;  i++) 
    {
        vector<Point3f> tempPointSet = object_Points[i];
        /****    ͨ���õ������������������Կռ����ά���������ͶӰ���㣬�õ��µ�ͶӰ��     ****/
		fisheye::projectPoints(tempPointSet, image_points2, rotation_vectors[i], translation_vectors[i], intrinsic_matrix, distortion_coeffs);
        /* �����µ�ͶӰ��;ɵ�ͶӰ��֮������*/  
        vector<Point2f> tempImagePoint = corners_Seq[i];
        Mat tempImagePointMat = Mat(1,tempImagePoint.size(),CV_32FC2);
        Mat image_points2Mat = Mat(1,image_points2.size(), CV_32FC2);
        for (size_t i = 0 ; i != tempImagePoint.size(); i++)
        {
            image_points2Mat.at<Vec2f>(0,i) = Vec2f(image_points2[i].x, image_points2[i].y);
            tempImagePointMat.at<Vec2f>(0,i) = Vec2f(tempImagePoint[i].x, tempImagePoint[i].y);
        }
        err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
        total_err += err/=  point_counts[i];   
        cout<<"NO."<<i+1<<"AVERAGE ERROR"<<err<<"����"<<endl;   
        fout<<"NO."<<i+1<<"AVERAGE ERROR"<<err<<"����"<<endl;   
    }   
    cout<<"TOTAL AVERAGE ERROR:"<<total_err/image_count<<"����"<<endl;   
    fout<<"TOTAL AVERAGE ERROR:"<<total_err/image_count<<"����"<<endl<<endl;   
    cout<<"JUDGE DONE!"<<endl;   

    /************************************************************************  
           ���涨����  
    *************************************************************************/   
    cout<<"BEGIN SAVE THE RESULT"<<endl;       
    Mat rotation_matrix = Mat(3,3,CV_32FC1, Scalar::all(0)); /* ����ÿ��ͼ�����ת���� */   

    fout<<"����ڲ�������"<<endl;   
    fout<<intrinsic_matrix<<endl;   
    fout<<"����ϵ����\n";   
    fout<<distortion_coeffs<<endl;   
    for (int i=0; i<image_count; i++) 
    { 
        fout<<"��"<<i+1<<"��ͼ�����ת������"<<endl;   
        fout<<rotation_vectors[i]<<endl;   

        /* ����ת����ת��Ϊ���Ӧ����ת���� */   
        Rodrigues(rotation_vectors[i],rotation_matrix);   
        fout<<"��"<<i+1<<"��ͼ�����ת����"<<endl;   
        fout<<rotation_matrix<<endl;   
        fout<<"��"<<i+1<<"��ͼ���ƽ��������"<<endl;   
        fout<<translation_vectors[i]<<endl;   
    }   
    cout<<"��ɱ���"<<endl; 
    fout<<endl;


    /************************************************************************  
           ��ʾ������  
    *************************************************************************/
    Mat mapx = Mat(image_size,CV_32FC1);
    Mat mapy = Mat(image_size,CV_32FC1);
    Mat R = Mat::eye(3,3,CV_32F);
    cout<<"�������ͼ��"<<endl;
    for (int i = 0 ; i != image_count ; i++)
    {
        cout<<"Frame #"<<i+1<<"..."<<endl;
        Mat newCameraMatrix = Mat(3,3,CV_32FC1,Scalar::all(0));
		fisheye::initUndistortRectifyMap(intrinsic_matrix,distortion_coeffs,R,intrinsic_matrix,image_size,CV_32FC1,mapx,mapy);
        Mat t = image_Seq[i].clone();
        cv::remap(image_Seq[i],t,mapx, mapy, INTER_LINEAR);
        string imageFileName;
        std::stringstream StrStm;
        StrStm<<i+1;
        StrStm>>imageFileName;
        imageFileName += "_d.jpg";
        imwrite(imageFileName,t);
    }
    cout<<"�������"<<endl;


    /************************************************************************  
           ����һ��ͼƬ  
    *************************************************************************/
	Mat test_sample = imread("img1.jpg");
	Size t_image_size=test_sample.size();
	Mat t_mapx=Mat(t_image_size,CV_32FC1);
	Mat t_mapy=Mat(t_image_size,CV_32FC1);
	for(int i=0;i<6;i++)
    {
		string TestFilename;
		std::stringstream strstm;
		strstm<<i;
		strstm>>TestFilename;
		TestFilename += ".png";
		cv::Mat testImage = imread(TestFilename);
        cout<<"TestImage ..."<<i+1<<endl;
        Mat newCameraMatrix = Mat(3,3,CV_32FC1,Scalar::all(0));
        fisheye::initUndistortRectifyMap(intrinsic_matrix,distortion_coeffs,R,intrinsic_matrix,t_image_size,CV_32FC1,t_mapx,t_mapy);
        Mat t = testImage.clone();
      	cv::remap(testImage,t,t_mapx, t_mapy, INTER_NEAREST);

        imwrite("o_"+TestFilename,t);
        cout<<"�������"<<endl;
    }


    return 0;
}
