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
    {1802, 1495, 1498}, // CP[0] x high // suunta 4
    {1207, 1489, 1536}, // CP[1] x low // suunta 2
    {1516, 1771, 1627}, // CP[2] y high // suunta 6
    {1497, 1214, 1408}, // CP[3] y low // suunta 5
    {1511, 1387, 1804}, // CP[4] z high // suunta 1
    {1500, 1589, 1242}, // CP[5] z low // suunta 3
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
	printk("   cp0 cp1 cp2 cp3 cp4 cp5\n");
	for(int i = 0;i<6;i++)
	{
		printk("cp%d %d   %d   %d   %d   %d   %d\n",i,CM[i][0],CM[i][1],CM[i][2],CM[i][3],CM[i][4],CM[i][5]);
	}
}

// Simuloidaan satunnaisgeneraattori
int simple_random(void) {
    return rand();  // Palautetaan satunnaisen kokonaisluvun
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
  
    for (int i = 0; i < 1; i++) 
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

    struct Measurement m;
    int closestCP = -1;
    int minDistance = INT_MAX;
    
    // Toistetaan 100 kertaa mittaus ja luokittelu
     for (int j = 0; j < 100; j++) {
        m = readADCValue();  // Luetaan ADC-arvot
        printk("x = %d,  y = %d,  z = %d\n", m.x, m.y, m.z);
    

        for (int i = 0; i < 6; i++) {
            // Lasketaan Euklidinen etäisyys keskipisteeseen CP[i]
            int dx = m.x - CP[i][0];
            int dy = m.y - CP[i][1];
            int dz = m.z - CP[i][2];
            int distance = dx * dx + dy * dy + dz * dz; // Neliöetäisyys riittää vertailuun

            if (distance < minDistance) {
                minDistance = distance;
                closestCP = i;
            }
        }

        // Debug-tuloste
        printk("Direction: %d -> Closest CP[%d], Min Distance: %d\n", direction, closestCP, minDistance);

        // Päivitetään sekaannusmatriisi
        CM[closestCP][direction]++;

        // Pieni viive testaukseen
        //k_sleep(K_MSEC(500)); 
    }   
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
