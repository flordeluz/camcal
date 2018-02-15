#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

#define IMAGE_WIDTH  640
#define IMAGE_HEIGHT 480
#define S (IMAGE_WIDTH/8)
#define T (0.15f)
IplImage* cvFrame;
IplImage* binImg;
int key;

Mat src, src_gray;
int thresh = 140;
int max_thresh = 255;
int frames_correctos=0;


void adaptiveThreshold(unsigned char* input, unsigned char* bin)
{
	unsigned long* integralImg = 0;
	int i, j;
	long sum=0;
	int count=0;
	int index;
	int x1, y1, x2, y2;
	int s2 = S/2;

	// create the integral image
	integralImg = (unsigned long*)malloc(IMAGE_WIDTH*IMAGE_HEIGHT*sizeof(unsigned long*));  //imagen integral en blanco

	for (i=0; i<IMAGE_WIDTH; i++)
	{
		// reset this column sum
		sum = 0;

		for (j=0; j<IMAGE_HEIGHT; j++)
		{
			index = j*IMAGE_WIDTH+i;

			sum += input[index];
			if (i==0)
				integralImg[index] = sum;
			else
				integralImg[index] = integralImg[index-1] + sum;
		}
	}

	// perform thresholding
	for (i=0; i<IMAGE_WIDTH; i++)
	{
		for (j=0; j<IMAGE_HEIGHT; j++)
		{
			index = j*IMAGE_WIDTH+i;

			// set the SxS region
			x1=i-s2; x2=i+s2;
			y1=j-s2; y2=j+s2;

			// check the border
			if (x1 < 0) x1 = 0;
			if (x2 >= IMAGE_WIDTH) x2 = IMAGE_WIDTH-1;
			if (y1 < 0) y1 = 0;
			if (y2 >= IMAGE_HEIGHT) y2 = IMAGE_HEIGHT-1;
			
			count = (x2-x1)*(y2-y1);

			// I(x,y)=s(x2,y2)-s(x1,y2)-s(x2,y1)+s(x1,x1)
			sum = integralImg[y2*IMAGE_WIDTH+x2] -
				  integralImg[y1*IMAGE_WIDTH+x2] -
				  integralImg[y2*IMAGE_WIDTH+x1] +
				  integralImg[y1*IMAGE_WIDTH+x1];

			if ((long)(input[index]*count) < (long)(sum*(1.0-T)))
				bin[index] = 0;
			else
				bin[index] = 255;
		}
	}

	free (integralImg);
}



/** @function reconocer_elipses */
void reconocer_elipses(int, void* )
{
  Mat threshold_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detectar contornos usando Threshold, osea segmentado
  //threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );

  //Declaración de los parámetros(imagenes iplimage) para la funcion adaptativeThreshold

  binImg = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);  //en blanco
  cvFrame = (IplImage*)(&IplImage(src_gray)); // convierte la matriz imagen src_gray a tipo IplImage

  adaptiveThreshold((unsigned char*)cvFrame->imageData, (unsigned char*)binImg->imageData);

  threshold_output = cvarrToMat(binImg);

  imshow( "Imagen segmentada", threshold_output );

  /// Encontrar contornos
  findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0) );

  /// Encontrar rectangulos rotados y elipses por cada contorno
  vector<RotatedRect> minEllipse(contours.size() );

  for( int i = 0; i < contours.size(); i++ )
     {
       if( (contours[i].size() > 5) && (contours[i].size() < 100) )
         {
			 minEllipse[i] = fitEllipse( Mat(contours[i]) ); 
	     }
     }


  /// Crear imagen para mostrar elipses
  Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
  int cont=1;
  int tempx=0;
  int tempy=0;
  int estado=1;
  for( int i = 1; i< contours.size()-1; i++ )
     {
       Scalar color = Scalar( 0, 255,0 );
       
       float centrox=minEllipse[i].center.x;
	   float centroy=minEllipse[i].center.y;
	   float dif_centros_ejex_sig = abs(minEllipse[i].center.x - minEllipse[i+1].center.x);
	   float dif_centros_ejey_sig = abs(minEllipse[i].center.y - minEllipse[i+1].center.y);
	   float dif_centros_ejex_ant = abs(minEllipse[i].center.x - minEllipse[i-1].center.x);
	   float dif_centros_ejey_ant = abs(minEllipse[i].center.y - minEllipse[i-1].center.y);
	   float proporcion=minEllipse[i].size.width/minEllipse[i].size.height;

	   if (((dif_centros_ejex_sig<=1 && dif_centros_ejey_sig<=1) || (dif_centros_ejex_ant<=1 && dif_centros_ejey_ant<=1))&&(proporcion<=1.4&&proporcion>=0.7))
	   {
		  if (centrox!=0)
		   {
		   ellipse( drawing, minEllipse[i], color, 1,255 );
		   //cout<<"Centro (X,Y) anillo "<<cont<< " :  ( " <<centrox<<" , "<<centroy<<" ) "  << endl;
		   tempx=centrox;
		   tempy=centroy;

		   if (estado==-1)
			   {
				//cout<<"diferencia de centros X , Y : "<<centrox-tempx<<", "<<centroy-tempy<<endl;
				//cout<<endl;
			   }
		   estado=estado*-1;
		   cont=cont+1;
		   }
	   }

     }
  if (cont<=61)
  {
	  frames_correctos=frames_correctos+1;
	  
  }

  /// Mostrar resultados

  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );
}



/** @function main */
int main( int argc, char** argv )
{


	namedWindow( "OpenCV Video", CV_WINDOW_AUTOSIZE);

	// cargar el archivo de video especificado
	VideoCapture vc("d://casa//calibration_ps3eyecam.avi");

	// verificar si se ha podio cargar el video
	if(!vc.isOpened()) return -1;
	// obtener los cuadros por segundo
	double fps = vc.get(CV_CAP_PROP_FPS);

	//cout<<"velocidad es: "<<fps<<endl;

	int c=0;

	while (true)
	{

		Mat frame;
		vc >> frame;
		c=c+1;
		//if ((c==1)||(c==240)||(c==480)||(c==720)||(c==960)||(c==1300)||(c==1420)||(c==1600))
		{
			cout<<endl<<"PROCESAMIENTO DE FRAME NRO :  "<<c<<endl<<endl;

			src = frame.clone();

			/// Convertir imagen a escala de grises y luego suavizar
			cvtColor( src, src_gray, CV_BGR2GRAY );
			blur( src_gray, src_gray, Size(3,3) );

			imshow( "source", src );

			//crear la barra de segmentacion
			//createTrackbar( " Umbral de segmemtación:", "source", &thresh, max_thresh, reconocer_elipses );
			reconocer_elipses(0,0);

			//waitKey();
		}


		if (c==3090)
		{
			cout<<"Frames Correctos : "<<frames_correctos<<" de "<<c<<endl;	
			waitKey();
			return 0;
		}

	  
	}
	
}






