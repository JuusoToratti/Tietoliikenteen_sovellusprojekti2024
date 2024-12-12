#include <zephyr/kernel.h>
#include <math.h>
#include "confusion.h"
#include "adc.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "../neuroverkonKertoimet.h"

/* 
  K-means -algoritmin tulisi tuottaa 6 keskipistettä, joissa on
  kolme arvoa: x, y, z. Testaamme mittausjärjestelmää tunnetuilla 
  keskipisteillä, eli x, y, z voivat saada vain arvot 1 = alas ja 2 = ylös.

  CP-matriisi on näin ollen 6 keskipistettä, jotka saatiin K-means-algoritmin 
  opetusprosessissa. Tämä pitäisi oikeasti tulla include-tiedostosta kuten 
  #include "KmeansCenterPoints.h".
  
  Mittausmatriisi on vain testausdataksi luotu "feikkimittaus", joka simuloi 
  kiihtyvyysanturin mittauksia. Oikeat mittaukset saadaan ADC:ltä, kun kiihtyvyysanturi on kytketty.
*/ 

// Määritellään 6 keskipistettä (CP), joissa kussakin on kolme koordinaattia x, y, z
int CP[6][3] = {
    {1802, 1495, 1498}, // CP[0] x korkea // suunta 4
    {1207, 1489, 1536}, // CP[1] x matala // suunta 2
    {1516, 1771, 1627}, // CP[2] y korkea // suunta 6
    {1497, 1214, 1408}, // CP[3] y matala // suunta 5
    {1511, 1387, 1804}, // CP[4] z korkea // suunta 1
    {1500, 1589, 1242}, // CP[5] z matala // suunta 3
};

// Testimittauksia varten määritellään feikkimittausmatriisi
int measurements[6][3] = {
    {1, 0, 0}, // suunta 1
    {2, 0, 0}, // suunta 2
    {0, 1, 0}, // suunta 3
    {0, 2, 0}, // suunta 4
    {0, 0, 1}, // suunta 5
    {0, 0, 2}, // suunta 6
};

// Seuraavassa taulukossa tallennetaan sekaannusmatriisin arvot
int CM[6][6] = {0};

// Funktio, joka tulostaa sekaannusmatriisin
void printConfusionMatrix(void) {
    printk("Confusion matrix = \n");
    printk("   cp0 cp1 cp2 cp3 cp4 cp5\n");
    for (int i = 0; i < 6; i++) {
        printk("cp%d %d   %d   %d   %d   %d   %d\n", i, CM[i][0], CM[i][1], CM[i][2], CM[i][3], CM[i][4], CM[i][5]);
    }
}

// Simuloidaan satunnaislukugeneraattoria
int simple_random(void) {
    return rand();  // Palautetaan satunnainen kokonaisluku
}

// Funktio, joka luo 100 satunnaista luokitusta testiksi
void makeHundredFakeClassifications(void)
{
    // Tämä funktio on suunniteltu testiin, jossa luodaan 100 satunnaista mittausta
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
        // Tulostetaan tulokset
        printk("Test %d: Sensor data = (%d, %d, %d), Closest CP[%d]\n", i + 1, x, y, z, winner_index);
    }
}
// Funktio, joka tekee yhden luokituksen ja päivittää sekaannusmatriisin
void makeOneClassificationAndUpdateConfusionMatrix2(int direction)
{
    // Tämä funktio suorittaa mittauksen 100 kertaa ja luokittelee sen
    struct Measurement m;
    int closestCP = -1;
    int minDistance = INT_MAX;
    
    // Suoritetaan 100 mittausta ja luokitusta
    for (int j = 0; j < 100; j++) {
        m = readADCValue();  // Luetaan ADC-arvot
        printk("x = %d,  y = %d,  z = %d\n", m.x, m.y, m.z);
    
        // Lasketaan etäisyys kullekin keskipisteelle ja etsitään lähin
       closestCP=classifyWithNeuralNetwork(m.x,m.y,m.z);
       
        // Debug-tuloste
        printk("Direction: %d -> Closest CP[%d], Min Distance: %d\n", direction, closestCP, minDistance);

        // Päivitetään sekaannusmatriisi
        CM[closestCP][direction]++;
    }   
}
// Funktio, joka tekee yhden luokituksen ja päivittää sekaannusmatriisin
void makeOneClassificationAndUpdateConfusionMatrix(int direction)
{
    // Tämä funktio suorittaa mittauksen 100 kertaa ja luokittelee sen
    struct Measurement m;
    int closestCP = -1;
    int minDistance = INT_MAX;
    
    // Suoritetaan 100 mittausta ja luokitusta
    for (int j = 0; j < 100; j++) {
        m = readADCValue();  // Luetaan ADC-arvot
        printk("x = %d,  y = %d,  z = %d\n", m.x, m.y, m.z);
    
        // Lasketaan etäisyys kullekin keskipisteelle ja etsitään lähin
        for (int i = 0; i < 6; i++) {
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
    }   
}

// ReLU-aktivaatioprofunkti
float relu(float x) {
    return x > 0 ? x : 0;
}

// Neuroverkolla luokittelu (kolme syötettä, kuusi luokkaa)
int classifyWithNeuralNetwork(int x, int y, int z) {
    float input[3] = {x, y, z};
    float output[6] = {0};

    // Matriisikertolasku: input * weights_0 + biases_1
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            output[i] += input[j] * weights_0[j][i];
        }
        output[i] += biases_1[i];
        output[i] = relu(output[i]); // Aktivoi tulos
    }

    // Etsi suurimman lähtöarvon indeksi
    int winner_index = 0;
    float max_value = output[0];
    for (int i = 1; i < 6; i++) {
        if (output[i] > max_value) {
            max_value = output[i];
            winner_index = i;
        }
    }

    return winner_index;
}

// Etäisyyksien laskeminen keskipisteistä ja voittajan valinta neuroverkon avulla
int calculateDistanceToAllCentrePointsAndSelectWinner(int x, int y, int z) {
    return classifyWithNeuralNetwork(x, y, z); // Käytetään neuroverkkoa
}

// Funktio, joka nollaa sekaannusmatriisin
void resetConfusionMatrix(void)
{
    for (int i = 0; i < 6; i++) { 
        for (int j = 0; j < 6; j++) {
            CM[i][j] = 0;
        }
    }
}
