#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src, src_gray;
int thresh = 100;
int max_thresh = 255;

void reconocer_elipses(int, void*);

/** @function main */
int main( int argc, char** argv )
{


	namedWindow( "OpenCV Video", CV_WINDOW_AUTOSIZE);

	// cargar el archivo de video especificado
	VideoCapture vc("d://casa//PadronAnillos_02.avi");

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
		if ((c==1)||(c==240)||(c==480)||(c==720)||(c==960)||(c==1300)||(c==1420)||(c==1600))
		{
			cout<<endl<<"RESULTADOS DEL FRAME NRO:  "<<c<<endl<<endl;

			src = frame.clone();

			/// Convertir imagen a escala de grises y luego suavizar
			cvtColor( src, src_gray, CV_BGR2GRAY );
			blur( src_gray, src_gray, Size(3,3) );

			imshow( "source", src );

			//crear la barra de segmentacion
			createTrackbar( " Umbral de segmemtación:", "source", &thresh, max_thresh, reconocer_elipses );
			reconocer_elipses( 0, 0 );

			waitKey(0);
		}


		if (c>1640)
		{
			return 0;
		}

		
	}
	
}

/** @function reconocer_elipses */
void reconocer_elipses(int, void* )
{
  Mat threshold_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detectar contornos usando Threshold, osea segmentado
  threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
  imshow( "Imagen segmentada", threshold_output );

  /// Encontrar contornos
  findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Encontrar rectangulos rotados y elipses por cada contorno
  vector<RotatedRect> minEllipse( contours.size() );

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

	   if ((dif_centros_ejex_sig<=2 && dif_centros_ejey_sig<=2) || (dif_centros_ejex_ant<=2 && dif_centros_ejey_ant<=2))
	   {
		   if (centrox!=0)
		   {
		   ellipse( drawing, minEllipse[i], color, 1,255 );
		   cout<<"Centro (X,Y) anillo "<<cont<< " :  ( " <<centrox<<" , "<<centroy<<" ) "  << endl;
		   tempx=centrox;
		   tempy=centroy;

		   if (estado==-1)
			   {
				cout<<"diferencia de centros X , Y : "<<centrox-tempx<<", "<<centroy-tempy<<endl;
				cout<<endl;
			   }
		   estado=estado*-1;
		   cont=cont+1;
		   }
	   }

     }

  /// Mostrar resultados
  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );
}
