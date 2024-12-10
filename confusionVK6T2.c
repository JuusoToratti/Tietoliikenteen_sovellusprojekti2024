#include <zephyr/kernel.h>
#include <math.h>
#include "confusion.h"
#include "adc.h"

#include "neuroverkonKertoimet.h"
#include <stdio.h>
#include <zephyr/logging/log.h>

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

LOG_MODULE_REGISTER(my_module, LOG_LEVEL_DBG);

// Z-score normalisointi
void normalize_input(float* input, int size) {
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += input[i];
    }
    mean /= size;

    float variance = 0.0f;
    for (int i = 0; i < size; i++) {
        variance += (input[i] - mean) * (input[i] - mean);
    }
    variance /= size;
    float stddev = sqrtf(variance);

    for (int i = 0; i < size; i++) {
        input[i] = (input[i] - mean) / stddev;
    }
}

// Normalisoidaan painot ja biasit
void normalize_weights_and_biases() {
    normalize_input(&weights_0[0][0], 3 * 64);  // Painot 0
    normalize_input(biases_1, 64);  // Biasit 1
    normalize_input(&weights_2[0][0], 64 * 32);  // Painot 2
    normalize_input(biases_3, 32);  // Biasit 3
    normalize_input(&weights_4[0][0], 32 * 6);  // Painot 4
    normalize_input(biases_5, 6);  // Biasit 5
}

// Parannettu ReLU
float relu(float x, float alpha) {
    return (x > 0) ? x : alpha * x;
}

// Parannettu softmax
void softmax(float* z, int size, float* output) {
    float max_z = z[0];
    for (int i = 1; i < size; i++) {
        if (z[i] > max_z) {
            max_z = z[i];
        }
    }

    // Vähennetään suurin arvo estääksemme ylivuodon
    for (int i = 0; i < size; i++) {
        z[i] -= max_z;
    }

    float sum = 0.0;
    for (int i = 0; i < size; i++) {
        z[i] = expf(z[i]);
        sum += z[i];
    }

    if (sum > 0) {
        for (int i = 0; i < size; i++) {
            output[i] = z[i] / sum;
        }
    }
}

// Forward-propagation
void forward_propagation(float* input, float* output) {
    float layer1[64] = {0};
    float layer2[32] = {0};
    float layer3[6] = {0};

    // Piilokerros 1
    for (int j = 0; j < 64; j++) {
        for (int i = 0; i < 3; i++) {
            layer1[j] += input[i] * weights_0[i][j];
        }
        layer1[j] += biases_1[j];
        layer1[j] = relu(layer1[j], 0.1f);  // Leaky ReLU with alpha 0.1
    }

    // Piilokerros 2
    for (int j = 0; j < 32; j++) {
        for (int i = 0; i < 64; i++) {
            layer2[j] += layer1[i] * weights_2[i][j];
        }
        layer2[j] += biases_3[j];
        layer2[j] = relu(layer2[j], 0.1f);
    }

    // Ulostulokerros
    for (int j = 0; j < 6; j++) {
        for (int i = 0; i < 32; i++) {
            layer3[j] += layer2[i] * weights_4[i][j];
        }
        layer3[j] += biases_5[j];
    }

    // Lisää tarkistus ennen softmaxia
    for (int i = 0; i < 6; i++) {
        printk("Layer3[%d] = %f\n", i, layer3[i]);
    }
    softmax(layer3, 6, output);
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

void makeOneClassificationAndUpdateConfusionMatrix(int direction, int confusion_matrix[6][6])
{
  
   struct Measurement m = readADCValue();
   printk("x = %d, y = %d, z = %d\n", m.x, m.y, m.z);
  
  // Syöte forward propagation -funktiolle
    float input[3] = {m.x, m.y, m.z};
    float output[6];
    // Normalisoi painot ja biasit
    normalize_weights_and_biases();
    forward_propagation(input, output);

    // Tulosta output-arvot
    printk("Output values: ");
    for (int i = 0; i < 6; i++) {
        printk("output[%d] = %.6f ", i, output[i]);
    }
    printk("\n");

    // Ennustettu luokka
    int predicted_class = 0;
    for (int k = 1; k < 6; k++) {
        if (output[k] > output[predicted_class]) {
            predicted_class = k;
        }
    }

    printk("Predicted class: %d\n", predicted_class);

    // Päivitä virhematriisi
    CM[direction][predicted_class]++;
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
