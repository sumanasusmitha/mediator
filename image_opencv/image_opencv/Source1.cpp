#include <stdio.h>
#include <stdlib.h>
#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"
#include "cxmisc.h"
#include "ml.h"
#include <math.h>
#include "tools_camera.h"
#include <fstream>

int g_nFingers = 0; //so ngon tay trong lan detect truoc

int main(int argc, char ** argv)

{

using namespace std;

int end = 0; int key = 0; 
int minus2,minus1,present,final = 0;
CvCapture* capture = cvCaptureFromCAM(0);
if ( !capture )
     {
   	fprintf( stderr, "ERROR: capture is NULL \n" );
    	getchar();
    	return -1;
     }

while(!end)

{
cvGrabFrame (capture); 
IplImage* in0 = cvRetrieveFrame (capture);
if ( !in0 ) 
     {
       fprintf( stderr, "ERROR: frame is null...\n" );
       getchar();
       break;
     }
cvNamedWindow("init",CV_WINDOW_AUTOSIZE );


/*==================init phase=====================*/
if (key == 0)
	{
	IplImage* init = cvCloneImage(in0);
	//cvNamedWindow("init",CV_WINDOW_AUTOSIZE );
	cvShowImage("init",init);
	if ( (cvWaitKey(10) & 255) == 10 ) // enter key to escape init and start recognition phase
		{
		key = 1;
		//cvDestroyWindow( "init" );
		}
	}

/*==================recognition phase=====================*/

if (key == 1)
{

//convert to gray image
IplImage* in = cvCreateImage( cvSize( 640, 480 ), IPL_DEPTH_8U, 1 );
cvCvtColor( in0, in, CV_RGB2GRAY ); 

int nFingers = 0; 

	// tach 3 plane, tao ảnh gray
	/*IplImage* hsv = cvCreateImage( cvGetSize(in), 8, 3 );
	cvCvtColor( in, hsv, CV_BGR2HSV );
	IplImage* h_plane = cvCreateImage( cvGetSize(in), 8, 1 );
	IplImage* s_plane = cvCreateImage( cvGetSize(in), 8, 1 );
	IplImage* v_plane = cvCreateImage( cvGetSize(in), 8, 1 );
	IplImage* planes[] = { h_plane, s_plane };
	cvCvtPixToPlane( hsv, h_plane, s_plane, v_plane, 0 );*/
	
	

	// tach duong bao
	IplImage* smooth = doSmoothGaussian(in, 31,31);
	IplImage* thres2 = doThresBinary(smooth,30);
	IplImage* contour = cvCloneImage(thres2);
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* hand_contour = 0;
	cvFindContours (contour,storage,&hand_contour,sizeof(CvContour),CV_RETR_TREE, CV_CHAIN_APPROX_NONE,cvPoint(0,0));
	if (hand_contour)
	{
	cvDrawContours( in, hand_contour, CV_RGB(255,0,0), CV_RGB(0,255,0), 1, 2, 8, cvPoint(0,0));
	
		int i,h=0;
		int r = 100; 	//khoang cach giua diem trung tam va 2 lan can
		int step = 16; 	//buoc nhay diem chay tren contour
		for( i = 0; i < (hand_contour ? hand_contour->total : 0); i+= step )
		{
		
		CvPoint* r0 = (CvPoint*)cvGetSeqElem( hand_contour, i );
		CvPoint n0 = cvPoint (r0->x,r0->y);

		double cos0 = curve (hand_contour, i, r);			//tinh he so cos tai r0
		signed int z = zCrossProduct (hand_contour, i, r);		//tinh z tich chap 2 vecto 

			if (cos0 > 0.5)
				{
				double cos1 = curve (hand_contour, i - step, r); 	// tinh he so cos tai lan can 1
				double cos2 = curve (hand_contour, i + step, r); 	// tinh he so cos tai lan can 2
				double max1 = max_3 (cos0, cos1, cos2);			// tinh max 3 he so goc
				bool equal = IsEqual (max1 , cos0);
				signed int z = zCrossProduct (hand_contour, i, r);	// tinh z tich chap 2 vecto 
					if (equal == 1 && z<0)
						{
						nFingers += 1;
						CvPoint* r0 = (CvPoint*)cvGetSeqElem( hand_contour, i );
						CvPoint n0 = cvPoint (r0->x,r0->y);
						cvCircle( in0,n0 , 8, CV_RGB(0,255,0), 3, 8, 0 );
						}
				}
		
		minus2=minus1;
		minus1=present;
		present=nFingers;

		if (minus2 == nFingers)
		final = nFingers;
		else
		final = minus2;
		}

	}



//cvNamedWindow("in",CV_WINDOW_AUTOSIZE );
cvShowImage("init",in0);

//cvNamedWindow("v-plane",CV_WINDOW_AUTOSIZE );
//cvShowImage("v-plane",v_plane);

//cvNamedWindow("thres2",CV_WINDOW_AUTOSIZE );
// cvShowImage("thres2",thres2);

//cout << hand_contour->total << endl;

//fingers quantity stabilizer


if (g_nFingers != final )
{
g_nFingers = final;
cout << g_nFingers << " fingers" << endl;
}

if ( (cvWaitKey(10) & 255) == 27 ) //wait for esc key to stop
end = 1;


cvReleaseMemStorage( &storage );
cvReleaseImage(&in);
cvReleaseImage(&thres2);
cvReleaseImage(&smooth);
/*cvReleaseImage(&v_plane);
cvReleaseImage(&s_plane);
cvReleaseImage(&h_plane);*/
}


//cvDestroyWindow( "thres2" );


}

cvDestroyWindow( "in" );

cvReleaseCapture(&capture);
return 0;
}
