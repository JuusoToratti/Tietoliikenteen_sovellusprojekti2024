#include <zephyr/kernel.h>
#include <math.h>
#include "confusion.h"
#include "adc.h"

#include "neuroverkonKertoimet.h"
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

int CP[6][3]={
	                     {1,0,0},
						 {2,0,0},
						 {0,1,0},
						 {0,2,0},
						 {0,0,1},
						 {0,0,2}
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

// Aktivointifunktio ReLU, jossa kerrotaan miljoonalla
int relu(int x) {
    return (x < 0) ? 0 : x;
}

// Aktivointifunktio Softmax, jossa käsitellään int-tyyppejä
void softmax(int* z, int size, int* output) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        z[i] = (int)(expf(z[i] / 1000000.0f) * 1000000); // Muutetaan takaisin floatiksi
        sum += z[i];
    }
    for (int i = 0; i < size; i++) {
        output[i] = z[i] * 1000000 / sum;
    }
}

// Eteenpäin propagointifunktio, jossa käytetään int-tyyppejä
void eteenpainpropagointi(int input_data[]) {
    // Piilokerros 1
    int z0[64];
    for (int i = 0; i < 64; i++) {
        z0[i] = 0;
        for (int j = 0; j < 3; j++) {
            z0[i] += input_data[j] * weights_0[j][i];
        }
        z0[i] += biases_1[i];  // Lisätään bias
    }

    // Piilokerros 2
    int z1[32];
    for (int i = 0; i < 32; i++) {
        z1[i] = 0;
        for (int j = 0; j < 64; j++) {
            z1[i] += z0[j] * weights_2[j][i];
        }
        z1[i] += biases_3[i];  // Lisätään bias
    }

    // Ulostulokerros
    int z2[6];
    for (int i = 0; i < 6; i++) {
        z2[i] = 0;
        for (int j = 0; j < 32; j++) {
            z2[i] += z1[j] * weights_4[j][i];
        }
        z2[i] += biases_5[i];  // Lisätään bias
    }

    // Softmax aktivoiminen ja tulostaminen
    int output[6];
    softmax(z2, 6, output);
    for (int i = 0; i < 6; i++) {
        printk("Ulostulo %d: %d\n", i, output[i]);
    }
}


void printConfusionMatrix(void)
{
	printk("Confusion matrix = \n");
	printk("   cp1 cp2 cp3 cp4 cp5 cp6\n");
	for(int i = 0;i<6;i++)
	{
		printk("cp%d %d   %d   %d   %d   %d   %d\n",i+1,CM[i][0],CM[i][1],CM[i][2],CM[i][3],CM[i][4],CM[i][5]);
	}
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
   printk("Make your own implementation for this function if you need this\n");
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
   
   printk("Make your own implementation for this function if you need this\n");
   return 0;
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

