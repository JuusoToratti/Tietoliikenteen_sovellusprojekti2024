#include <zephyr/kernel.h>
#include <math.h>
#include "confusion.h"
#include "adc.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
/* 
  K-means algorithm should provide 6 center points with
  3 values x,y,z. Let's test measurement system with known
  center points. I.e. x,y,z are supposed to have only values
  1 = down and 2 = up
  
  CP matrix is thus the 6 center points got from K-means algoritm
  teaching process. This should actually come from include file like
  #include "KmeansCenterPoints.h"
  
  And measurements matrix is just fake matrix for testing purpose
  actual measurements are taken from ADC when accelerator is connected.
*/ 

 int CP[6][3] = {
    {1208, 1501, 1514}, // CP[0]
    {1802, 1487, 1505}, // CP[1]
    {1502, 1210, 1423}, // CP[2]
    {1523, 1774, 1618}, // CP[3]
    {1507, 1585, 1245}, // CP[4]
    {1508, 1392, 1806}, // CP[5]
};

int measurements[6][3]={
	                     {1,0,0},
						 {2,0,0},
						 {0,1,0},
						 {0,2,0},
						 {0,0,1},
						 {0,0,2}
};

int CM[6][6]= {0};


void printConfusionMatrix(void)
{
	printk("Confusion matrix = \n");
	printk("   cp1 cp2 cp3 cp4 cp5 cp6\n");
	for(int i = 0;i<6;i++)
	{
		printk("cp%d %d   %d   %d   %d   %d   %d\n",i+1,CM[i][0],CM[i][1],CM[i][2],CM[i][3],CM[i][4],CM[i][5]);
	}
}

// Simuloitu satunnaisgeneraattori
int simple_random(void) {
    return rand();  // Palauttaa satunnaisen kokonaisluvun
}

void makeHundredFakeClassifications(void)
{
   /*******************************************
   Jos ja toivottavasti kun teet toteutuksen paloissa eli varmistat ensin,
   että etäisyyden laskenta 6 keskipisteeseen toimii ja osaat valita 6 etäisyydestä
   voittajaksi sen lyhyimmän etäisyyden, niin silloin voit käyttää tätä aliohjelmaa
   varmistaaksesi, että etäisuuden laskenta ja luokittelu toimii varmasti tunnetulla
   itse keksimälläsi sensoridatalla ja itse keksimilläsi keskipisteillä.
   *******************************************/
  
    for (int i = 0; i < 100; i++) 
    {
        // Luo satunnaisia koordinaatteja (x, y, z), simuloi kiihtyvyysanturin mittauksia
        int x = 1201 + (simple_random() % (1810 - 1201 + 1));  // x välillä 1201-1810
        int y = 1196 + (simple_random() % (1785 - 1196 + 1));  // y välillä 1196-1785
        int z = 1230 + (simple_random() % (1814 - 1230 + 1));  // z välillä 1230-1814

        // Käytetään calculateDistanceToAllCentrePointsAndSelectWinner -funktiota luokitteluun
        int winner_index = calculateDistanceToAllCentrePointsAndSelectWinner(x, y, z);

        // Päivitetään sekaannusmatriisi
        CM[winner_index][winner_index]++;
        // Tulostetaan tulokset, debug tulostuu ensin calculateDistanceToAllCentrePointsAndSelectWinner -funktiosta
        printk("Test %d: Sensor data = (%d, %d, %d), Closest CP[%d]\n", i + 1, x, y, z, winner_index);
    }
}

void makeOneClassificationAndUpdateConfusionMatrix(int direction)
{
   /**************************************
   Tee toteutus tälle ja voit tietysti muuttaa tämän aliohjelman sellaiseksi,
   että se tekee esim 100 kpl mittauksia tai sitten niin, että tätä funktiota
   kutsutaan 100 kertaa yhden mittauksen ja sen luokittelun tekemiseksi.
   **************************************/
   printk("Make your own implementation for this function if you need this\n");
}

int calculateDistanceToAllCentrePointsAndSelectWinner(int x,int y,int z)
{
   /***************************************
   Tämän aliohjelma ottaa yhden kiihtyvyysanturin mittauksen x,y,z,
   laskee etäisyyden kaikkiin 6 K-means keskipisteisiin ja valitsee
   sen keskipisteen, jonka etäisyys mittaustulokseen on lyhyin.
   ***************************************/
     int min_distance = INT_MAX;
     int winner_index = -1;

    for (int i = 0; i < 6; i++) 

    {
        int dx = x - CP[i][0];
        int dy = y - CP[i][1];
        int dz = z - CP[i][2];

        int distance_squared = dx * dx + dy * dy + dz * dz; // Etäisyyden neliö
        printk("Debug: CP[%d] = {%d, %d, %d}, distance^2 = %d\n", i, CP[i][0], CP[i][1], CP[i][2], distance_squared);
        

        // Päivitetään lähin etäisyys
       if (distance_squared < min_distance) 
       {
           min_distance = distance_squared;  // Pieni etäisyys on uusi minimi
           winner_index = i;  // Päivitetään voittajan indeksi
       }
   }

    printk("Debug: Min distance^2 = %d, Winner index = %d\n", min_distance, winner_index);
    //printk("Closest center to point (%d, %d, %d) is CP[%d] with distance = %.2f\n", 
    //   x, y, z, winner_index, sqrt(min_distance));
    return winner_index; // Palauttaa lähimmän keskipisteen indeksin
}

void resetConfusionMatrix(void)
{
	for(int i=0;i<6;i++)
	{ 
		for(int j = 0;j<6;j++)
		{
			CM[i][j]=0;
		}
	}
}

