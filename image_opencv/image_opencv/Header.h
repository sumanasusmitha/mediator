/*
* tools.h
*
*  Created on: Jul 12, 2011
*      Author: Ho Dac Loc
*/

#include <stdio.h>
#include <stdlib.h>
#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"
#include "cxmisc.h"
#include "ml.h"
#include <math.h>

#define PI 3.1415926535898
double rads(double degs)
{
	return (PI / 180 * degs);
}

CvHistogram* doCalHistHS(IplImage* src, int h_bins, int s_bins)
{

	IplImage* hsv = cvCreateImage(cvGetSize(src), 8, 3);
	cvCvtColor(src, hsv, CV_BGR2HSV);
	IplImage* h_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* s_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* v_plane = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage* planes[] = { h_plane, s_plane };
	cvCvtPixToPlane(hsv, h_plane, s_plane, v_plane, 0);
	// Build the histogram and compute its contents.
	//

	CvHistogram* hist;
	{
		int hist_size[] = { h_bins, s_bins };
		float h_ranges[] = { 0, 180 }; // hue is [0,180]
		float s_ranges[] = { 0, 255 };
		float* ranges[] = { h_ranges, s_ranges };
		hist = cvCreateHist(
			2,
			hist_size,
			CV_HIST_ARRAY,
			ranges,
			1
			);
	}

	cvCalcHist(planes, hist, 0, 0); //Compute histogram
	cvNormalizeHist(hist, 20 * 255); //Normalize it
	return (hist);
}

IplImage* doDrawHist(CvHistogram* hist, int h_bins, int s_bins)
{

	int scale = 10;
	IplImage* hist_img = cvCreateImage(
		cvSize(h_bins * scale, s_bins * scale),
		8,
		3
		);
	cvZero(hist_img);
	// populate our visualization with little gray squares.
	//
	float max_value = 0;
	int h = 0;
	int s = 0;
	cvGetMinMaxHistValue(hist, 0, &max_value, 0, 0);
	for (h = 0; h < h_bins; h++) {
		for (s = 0; s < s_bins; s++) {
			float bin_val = cvQueryHistValue_2D(hist, h, s);
			int intensity = cvRound(bin_val * 255 / max_value);
			cvRectangle(
				hist_img,
				cvPoint(h*scale, s*scale),
				cvPoint((h + 1)*scale - 1, (s + 1)*scale - 1),
				CV_RGB(intensity, intensity, intensity),
				CV_FILLED
				);
		}
	}
	return (hist_img);

}

IplImage* doSmoothGaussian(IplImage* in, int par1, int par2)
{
	IplImage* out = cvCreateImage(cvGetSize(in), in->depth, in->nChannels);
	cvSmooth(in, out, CV_GAUSSIAN, par1, par2);
	return (out);
}

IplImage* doEqualHist(IplImage* in)
{
	IplImage* out = cvCreateImage(cvGetSize(in), in->depth, in->nChannels);
	cvEqualizeHist(in, out);
	return (out);
}

IplImage* doSmoothMedian(IplImage* in, int par1, int par2)
{
	IplImage* out = cvCreateImage(cvGetSize(in), in->depth, in->nChannels);
	cvSmooth(in, out, CV_BLUR, par1, par2);
	return (out);
}

IplImage* doThresBinary(IplImage* in, int thres)
{
	IplImage* out = cvCreateImage(cvGetSize(in), in->depth, in->nChannels);
	cvThreshold(in, out, thres, 255, CV_THRESH_BINARY);
	return (out);
}

IplImage* doAdapThres(IplImage* in)
{
	IplImage* out = cvCreateImage(cvGetSize(in), in->depth, in->nChannels);
	cvAdaptiveThreshold(in, out, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 71, 15);
	return (out);
}

IplImage* doDilate(IplImage* in)
{
	IplImage* out = cvCreateImage(cvGetSize(in), in->depth, in->nChannels);
	cvDilate(in, out, NULL, 1);
	return (out);
}



double cos_goc(CvPoint* pt1, CvPoint* pt2, CvPoint* pt3, CvPoint* pt4)
{
	double vx1 = pt1->x - pt2->x;
	double vy1 = pt1->y - pt2->y;
	double vx2 = pt3->x - pt4->x;
	double vy2 = pt3->y - pt4->y;
	return (vx1*vx2 + vy1*vy2) / sqrt((vx1*vx1 + vy1*vy1)*(vx2*vx2 + vy2*vy2));
}

double cos_vector(CvPoint vt1, CvPoint vt2)
{
	double vx1 = vt1.x;
	double vy1 = vt1.y;
	double vx2 = vt2.x;
	double vy2 = vt2.y;
	return (vx1*vx2 + vy1*vy2) / sqrt((vx1*vx1 + vy1*vy1)*(vx2*vx2 + vy2*vy2));

}

signed int zCrossProduct(CvSeq* contour, int pt, int r)
{
	CvPoint* r0 = (CvPoint*)cvGetSeqElem(contour, pt);
	CvPoint* r1 = (CvPoint*)cvGetSeqElem(contour, pt + r);
	CvPoint* r2 = (CvPoint*)cvGetSeqElem(contour, pt - r);
	if ((r0) && (r1) && (r2))
	{
		double vx1 = r0->x - r1->x;
		double vy1 = r0->y - r1->y;
		double vx2 = r0->x - r2->x;
		double vy2 = r0->y - r2->y;
		return (vx1*vy2 - vx2*vy1);
	}
	else
		return 10000;
}

double max(double a, double b)
{
	if (a>b)
		return a;
	else
		return b;
}

double max_3(double a, double b, double c)
{
	double m1 = max(a, b);
	double m2 = max(m1, c);
	return m2;
}

bool IsEqual(double dX, double dY)

{
	const double dEpsilon = 0.000001; // or some other small number
	return fabs(dX - dY) <= dEpsilon * fabs(dX);
}


double curve(CvSeq* contour, int pt, int r)

{

	CvPoint* r0 = (CvPoint*)cvGetSeqElem(contour, pt);
	CvPoint* r1 = (CvPoint*)cvGetSeqElem(contour, pt + r);
	CvPoint* r2 = (CvPoint*)cvGetSeqElem(contour, pt - r);
	if ((r0) && (r1) && (r2))
	{
		double vx1 = r0->x - r1->x;
		double vy1 = r0->y - r1->y;
		double vx2 = r0->x - r2->x;
		double vy2 = r0->y - r2->y;
		return (vx1*vx2 + vy1*vy2) / sqrt((vx1*vx1 + vy1*vy1)*(vx2*vx2 + vy2*vy2));
	}
	else
		return 5;
}

