#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



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
vector<RotatedRect> sepEllipse(22);
vector<RotatedRect> resEllipse(22);

int es_colineal(int a,int b, int c)
{
	if (a!=b && a!=c)
	{
		float dac=sqrt(pow(sepEllipse[a].center.x - sepEllipse[c].center.x,2)+pow(sepEllipse[a].center.y-sepEllipse[c].center.y,2));
		float dab=sqrt(pow(sepEllipse[a].center.x - sepEllipse[b].center.x,2)+pow(sepEllipse[a].center.y-sepEllipse[b].center.y,2));
		float dbc=sqrt(pow(sepEllipse[b].center.x - sepEllipse[c].center.x,2)+pow(sepEllipse[b].center.y-sepEllipse[c].center.y,2));
		float dabc=dab+dbc;
		if ((dac>=(dabc-1)) && (dac<=(dabc+1)))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}


int buscar_mascercano(int clave_busqueda)
{
float menordist=10000,menordist1=10000;
float dist=0;
float cenx=sepEllipse[clave_busqueda].center.x;
float ceny=sepEllipse[clave_busqueda].center.y;
int resultado=0;
for(int x=1;x<=20;x++)
	{
		dist=sqrt(pow(sepEllipse[x].center.x-cenx,2)+pow(sepEllipse[x].center.y-ceny,2));

		if (dist<menordist && sepEllipse[x].center.x<cenx && clave_busqueda!=x && sepEllipse[x].center.y<ceny)
		{
			menordist=dist;
			resultado=x;
		}
	}
	return resultado;
}


int Ibuscar_mascercano(int clave_busqueda)
{
float menordist=10000,menordist1=10000;
float dist=0;
float cenx=sepEllipse[clave_busqueda].center.x;
float ceny=sepEllipse[clave_busqueda].center.y;
int resultado=0;
for(int x=1;x<=20;x++)
	{
		dist=sqrt(pow(sepEllipse[x].center.x-cenx,2)+pow(sepEllipse[x].center.y-ceny,2));

		if (dist<menordist && sepEllipse[x].center.x>cenx && clave_busqueda!=x && sepEllipse[x].center.y<ceny)
		{
			menordist=dist;
			resultado=x;
		}
	}
	return resultado;
}

int buscar_sigmascercano(int clave_busqueda, int no_evaluar)
{
float menordist=10000,menordist1=10000;
float dist=0;
float cenx=sepEllipse[clave_busqueda].center.x;
float ceny=sepEllipse[clave_busqueda].center.y;
int resultado=0;
for(int x=1;x<=20;x++)
	{
		dist=sqrt(pow(sepEllipse[x].center.x-cenx,2)+pow(sepEllipse[x].center.y-ceny,2));

		if ((dist<menordist1) && (x!=no_evaluar) && clave_busqueda!=x && sepEllipse[x].center.y<ceny && sepEllipse[x].center.x>cenx)
		{
			menordist1=dist;
			resultado=x;
		}
	}
	return resultado;
}


int Ibuscar_sigmascercano(int clave_busqueda, int no_evaluar)
{
float menordist=10000,menordist1=10000;
float dist=0;
float cenx=sepEllipse[clave_busqueda].center.x;
float ceny=sepEllipse[clave_busqueda].center.y;
int resultado=0;
for(int x=1;x<=20;x++)
	{
		dist=sqrt(pow(sepEllipse[x].center.x-cenx,2)+pow(sepEllipse[x].center.y-ceny,2));

		if ((dist<menordist1) && (x!=no_evaluar) && clave_busqueda!=x && sepEllipse[x].center.y<ceny && sepEllipse[x].center.x<cenx)
		{
			menordist1=dist;
			resultado=x;
		}
	}
	return resultado;
}

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
  float tempx=0;
  float tempy=0;
  int estado=1;
  for( int i = 1; i< contours.size()-1; i++ )
     {
       Scalar color = Scalar( 255, 255,255 );
       
       float centrox;
	   float centroy;
	   float dif_centros_ejex_sig = abs(minEllipse[i].center.x - minEllipse[i+1].center.x);
	   float dif_centros_ejey_sig = abs(minEllipse[i].center.y - minEllipse[i+1].center.y);
	   float dif_centros_ejex_ant = abs(minEllipse[i].center.x - minEllipse[i-1].center.x);
	   float dif_centros_ejey_ant = abs(minEllipse[i].center.y - minEllipse[i-1].center.y);
	   float proporcion=minEllipse[i].size.width/minEllipse[i].size.height;

	   if (((dif_centros_ejex_sig<=1 && dif_centros_ejey_sig<=1) || (dif_centros_ejex_ant<=1 && dif_centros_ejey_ant<=1))&&(proporcion<=1.6&&proporcion>=0.5))
	   {
		  if (centrox=minEllipse[i].center.x!=0)
		   {
		   centrox=minEllipse[i].center.x;
		   centroy=minEllipse[i].center.y;
		   //ellipse( drawing, minEllipse[i], color, 1,255 );

		   
		   
		   //cout<<"Centro (X,Y) anillo "<<cont<< " :  ( " <<centrox<<" , "<<centroy<<" ) "  << endl;

		   if (estado==-1)
			   {
				//cout<<"diferencia de centros X , Y : "<<centrox-tempx<<", "<<centroy-tempy<<endl;		
				// line(drawing, Point(20,20),Point(200,200),color);
				float prom_x=(centrox+tempx)/2;
				float prom_y=(centroy+tempy)/2;
				sepEllipse[cont]=minEllipse[i];
				sepEllipse[cont].center.x=prom_x;
				sepEllipse[cont].center.y=prom_y;
				ellipse( drawing, sepEllipse[cont], color, 1,255 );
				cont=cont+1;
			   }
		   tempx=centrox;
		   tempy=centroy;
		   estado=estado*-1;

		   }
	   }

     }
  if (cont<=41)
  {
	  frames_correctos=frames_correctos+1;
	  
  }

////INDEXACION SPACIAL DE PUNTOS
    
	int cercano,sigcercano;
	ostringstream convert;
	float cenx=sepEllipse[1].center.x;
	float ceny=sepEllipse[1].center.y;
	float cen_ant=sepEllipse[3].center.x;

			
	if (cen_ant<=cenx)  //si esta inclinado hacia la derecha
	{
	
	//encontrar puntos claves 
	int key20=1;
	int key19=0;
	int key15=0;
	int key14=0;
	int key10=0;
	int key9=0;
	int key5=0;
	int key4=0;

	resEllipse[20]=sepEllipse[key20];
	key19=buscar_mascercano(key20);
	resEllipse[19]=sepEllipse[key19];

	key15=buscar_sigmascercano(key20,key19);
	resEllipse[15]=sepEllipse[key15];
	key14=buscar_mascercano(key15);
	resEllipse[14]=sepEllipse[key14];

	key10=buscar_sigmascercano(key15,key14);
	resEllipse[10]=sepEllipse[key10];
	key9=buscar_mascercano(key10);
	resEllipse[9]=sepEllipse[key9];

	key5=buscar_sigmascercano(key10,key9);
	resEllipse[5]=sepEllipse[key5];
	key4=buscar_mascercano(key5);
	resEllipse[4]=sepEllipse[key4];


	////INDEXACION CON PUNTOS CLAVE Y COLINEARIDAD

	//int c1=18;
	//int c2=13;
	//int c3=8;
	//int c4=3;
	//for(int j=1;j<=20;j++)
	//{
	//	if (j!=key20 && j!=key19 && j!=key15 &&  j!=key14 &&  j!=key10 &&  j!=key9 &&  j!=key5 &&  j!=key4)
	//	{
	//		if (es_colineal(j,key19,key20)==1)
	//		{
	//			resEllipse[c1]=sepEllipse[j];
	//			c1=c1-1;
	//		}

	//		if (es_colineal(j,key14,key15)==1)
	//		{
	//			resEllipse[c2]=sepEllipse[j];
	//			c2=c2-1;
	//		}

	//		if (es_colineal(j,key9,key10)==1)
	//		{
	//			resEllipse[c3]=sepEllipse[j];
	//			c3=c3-1;
	//		}

	//		if (es_colineal(j,key4,key5)==1)
	//		{

	//			resEllipse[c4]=sepEllipse[j];
	//			c4=c4-1;
	//		}
	//	}
	//}

	////FIN INDEXACION CON PUNTOS CLAVE Y COLINEARIDAD





	///INDEXACION CON PUNTOS CLAVE Y VECINOS MAS CERCANOS

	//ubicar ultima fila del 16 al 20
	cercano=key20;
	resEllipse[20]=sepEllipse[cercano];
	for(int i=20;i>=17;i--)
	{
		cercano=buscar_mascercano(cercano);
		resEllipse[i-1]=sepEllipse[cercano];
	}

	//ubicar fila del 11 al 15
	cercano=key15;
	resEllipse[15]=sepEllipse[cercano];
	for(int i=15;i>=12;i--)
	{
		cercano=buscar_mascercano(cercano);
		resEllipse[i-1]=sepEllipse[cercano];
	}

	//ubicar fila del 06 al 10
	cercano=key10;
	resEllipse[10]=sepEllipse[cercano];
	for(int i=10;i>=7;i--)
	{
		cercano=buscar_mascercano(cercano);
		resEllipse[i-1]=sepEllipse[cercano];
	}

	//ubicar fila del 01 al 5
	cercano=key5;
	resEllipse[5]=sepEllipse[cercano];
	for(int i=5;i>=2;i--)
	{
		cercano=buscar_mascercano(cercano);
		resEllipse[i-1]=sepEllipse[cercano];
	}

	/// FIN INDEXACION CON PUNTOS CLAVE Y VECINOS MAS CERCANOS





	////INDEXACION CON ESQUINAS DEL PATRON

	//float menory=10000;
	//float mayory=0;
	//float menorx=10000;
	//float mayorx=0;
	//float menx,meny,mayx,mayy;

	//for(int i=1;i<=20;i++)
	//{
	//	if (sepEllipse[i].center.y<menory)
	//	{
	//		menory=sepEllipse[i].center.y;
	//		meny=i;
	//	}
	//	if (sepEllipse[i].center.y>mayory)
	//	{
	//		mayory=sepEllipse[i].center.y;
	//		mayy=i;
	//	}
	//	if (sepEllipse[i].center.x<menorx)
	//	{
	//		menorx=sepEllipse[i].center.x;
	//		menx=i;
	//	}
	//	if (sepEllipse[i].center.x>mayorx)
	//	{
	//		mayorx=sepEllipse[i].center.x;
	//		mayx=i;
	//	}
	//}

	////if (sepEllipse[mayx].center.y < sepEllipse[menx].center.y)
	//if (sepEllipse[meny].center.x < sepEllipse[mayy].center.x)
	//{
	//	resEllipse[1]=sepEllipse[meny];
	//	resEllipse[5]=sepEllipse[mayx];
	//	resEllipse[16]=sepEllipse[menx];
	//	resEllipse[20]=sepEllipse[mayy];
	//}
	////if (sepEllipse[mayx].center.y < sepEllipse[menx].center.y)
	//if (sepEllipse[meny].center.x > sepEllipse[mayy].center.x)
	//{
	//	resEllipse[1]=sepEllipse[menx];
	//	resEllipse[5]=sepEllipse[meny];
	//	resEllipse[16]=sepEllipse[mayy];
	//	resEllipse[20]=sepEllipse[mayx];
	//}

	//// FIN INDEXACION CON ESQUINAS DEL PATRON




	}




	if (cen_ant>cenx)  //si esta inclinado hacia la izquierda
	
	{

	int key16=1;
	int key17=0;
	int key11=0;
	int key12=0;
	int key6=1;
	int key7=0;
	int key1=0;
	int key2=0;


	resEllipse[16]=sepEllipse[key16];
	key17=Ibuscar_mascercano(key16);
	resEllipse[17]=sepEllipse[key17];

	key11=Ibuscar_sigmascercano(key16,key17);
	resEllipse[11]=sepEllipse[key11];
	key12=Ibuscar_mascercano(key11);
	resEllipse[12]=sepEllipse[key12];

	key6=Ibuscar_sigmascercano(key11,key12);
	resEllipse[6]=sepEllipse[key6];
	key7=Ibuscar_mascercano(key6);
	resEllipse[7]=sepEllipse[key7];

	key1=Ibuscar_sigmascercano(key6,key7);
	resEllipse[1]=sepEllipse[key1];
	key2=Ibuscar_mascercano(key1);
	resEllipse[2]=sepEllipse[key2];

	//ubicar ultima fila del 16 al 20
	cercano=key16;
	resEllipse[16]=sepEllipse[cercano];
	for(int i=16;i<=19;i++)
	{
		cercano=Ibuscar_mascercano(cercano);
		resEllipse[i+1]=sepEllipse[cercano];
	}

	//ubicar fila del 11 al 15
	cercano=key11;
	resEllipse[11]=sepEllipse[cercano];
	for(int i=11;i<=14;i++)
	{
		cercano=Ibuscar_mascercano(cercano);
		resEllipse[i+1]=sepEllipse[cercano];
	}

	//ubicar fila del 06 al 10
	cercano=key6;
	resEllipse[6]=sepEllipse[cercano];
	for(int i=6;i<=9;i++)
	{
		cercano=Ibuscar_mascercano(cercano);
		resEllipse[i+1]=sepEllipse[cercano];
	}

	//ubicar fila del 01 al 5
	cercano=key1;
	resEllipse[1]=sepEllipse[cercano];
	for(int i=1;i<=4;i++)
	{
		cercano=Ibuscar_mascercano(cercano);
		resEllipse[i+1]=sepEllipse[cercano];
	}
	

	
  }
	//			
				
//// fin de ver colineariedad



  /// Mostrar resultados
  //line(drawing, resEllipse[1].center,resEllipse[10].center,Scalar( 0, 255,0 ));
  //circle(drawing, sepEllipse[1].center, 1, Scalar( 255, 0,0 ), 20);
  //circle(drawing, sepEllipse[3].center, 1, Scalar( 0, 255,0 ), 20);
  //circle(drawing, resEllipse[10].center, 1, Scalar( 0, 255,0 ), 20);
  //circle(drawing, resEllipse[31].center, 1, Scalar( 0,0,255 ), 20);
  //circle(drawing, resEllipse[40].center, 1, Scalar( 255,0,255 ), 20);
  //circle(drawing, Point(10,100), 1, Scalar( 255, 255,0 ), 20);

 // putText(drawing, "1", sepEllipse[1].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
 //putText(drawing, "2", sepEllipse[2].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
 // putText(drawing, "3", sepEllipse[3].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
 // putText(drawing, "4", sepEllipse[4].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
 // putText(drawing, "5", sepEllipse[5].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "20", resEllipse[20].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "19", resEllipse[19].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "15", resEllipse[15].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "14", resEllipse[14].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "10", resEllipse[10].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "9", resEllipse[9].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "5", resEllipse[5].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	//putText(drawing, "4", resEllipse[4].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);

	//MOstrar etiquetas ordenadas
	for(int i=16;i<=16;i++)
	{
		ostringstream convert; 
		convert<<i;
		putText(drawing, convert.str(), resEllipse[i].center, FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
	}

	//mostrar lineas y puntos pequeños
	//for(int i=1;i<=4;i++)
	//{
	//	if (resEllipse[i].center.x!=0){
	//	line(drawing, resEllipse[i].center,resEllipse[i+1].center,Scalar( 0, 255,0 ));
	//	circle(drawing, resEllipse[i].center, 1, Scalar( 0, 255,0 ), 2);
	//	if (i==4){
	//		circle(drawing, resEllipse[5].center, 1, Scalar( 0, 255,0 ), 2);}}
	//}

	//if (resEllipse[5].center.x!=0 && resEllipse[6].center.x!=0){
	//	line(drawing, resEllipse[5].center,resEllipse[6].center,Scalar( 255, 255,255 ));}
	//
	//for(int i=6;i<=9;i++)
	//{
	//	if (resEllipse[i].center.x!=0){
	//	line(drawing, resEllipse[i].center,resEllipse[i+1].center,Scalar( 255, 0,0 ));
	//	circle(drawing, resEllipse[i].center, 1, Scalar( 255, 0,0 ), 2);
	//	if (i==9){
	//		circle(drawing, resEllipse[10].center, 1, Scalar( 255, 0,0 ), 2);}}
	//}

	//if (resEllipse[10].center.x!=0 && resEllipse[11].center.x!=0){
	//	line(drawing, resEllipse[10].center,resEllipse[11].center,Scalar( 255, 255,255 ));}

	//for(int i=11;i<=14;i++)
	//{
	//	if (resEllipse[i].center.x!=0){
	//	line(drawing, resEllipse[i].center,resEllipse[i+1].center,Scalar( 0, 0,255 ));
	//	circle(drawing, resEllipse[i].center, 1, Scalar( 0, 0,255 ), 2);
	//	if (i==14){
	//		circle(drawing, resEllipse[15].center, 1, Scalar( 0, 0,255 ), 2);}}
	//}

	//	if (resEllipse[15].center.x!=0 && resEllipse[16].center.x!=0){
	//		line(drawing, resEllipse[15].center,resEllipse[16].center,Scalar( 255, 255,255 ));}

	//for(int i=16;i<=19;i++)
	//{
	//	if (resEllipse[i].center.x!=0){
	//	line(drawing, resEllipse[i].center,resEllipse[i+1].center,Scalar( 0, 255,255 ));
	//	circle(drawing, resEllipse[i].center, 1, Scalar( 0,255, 255 ), 2);
	//	if (i==19){
	//		circle(drawing, resEllipse[20].center, 1, Scalar( 0, 255, 255 ), 2);}}
	//}

  namedWindow( "Reconocimiento", CV_WINDOW_AUTOSIZE );
  imshow( "Reconocimiento", drawing );
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
	//double fps = vc.get(CV_CAP_PROP_FPS);

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


			imshow( "source", src_gray );

			//crear la barra de segmentacion
			//createTrackbar( " Umbral de segmemtación:", "source", &thresh, max_thresh, reconocer_elipses );
			reconocer_elipses(0,0);

	

			waitKey();
		}


		if (c==3090)
		{
			cout<<"Frames Correctos : "<<frames_correctos<<" de "<<c<<endl;	
			waitKey();
			return 0;
		}

	  
	}
	
}






