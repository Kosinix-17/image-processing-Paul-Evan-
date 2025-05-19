//
// Created by arket on 26/04/2025.
//

#ifndef BMP8_H
#define BMP8_H

typedef struct {
    // Pointeur vers les données de l'image
    unsigned char* pixels;

    // Palette de couleurs (1024 octets = 256 couleurs × 4 octets)
    unsigned char palette[1024];

    // Largeur de l'image en pixels
    unsigned int width;

    // Hauteur de l'image en pixels
    unsigned int height;

    // Profondeur de couleur (8 bits pour niveaux de gris)
    unsigned short colorDepth;

    // Taille totale des données de l'image
    unsigned int dataSize;
} t_bmp8;

// Fonctions de base pour la manipulation d'images
t_bmp8* bmp8_loadImage(const char* filename);
void bmp8_saveImage(const char* filename, t_bmp8* img);
void bmp8_free(t_bmp8* img);
void bmp8_printInfo(t_bmp8* img);

// Nouvelles fonctions de traitement d'image
void bmp8_negative(t_bmp8* img);
void bmp8_brightness(t_bmp8* img, int value);
void bmp8_threshold(t_bmp8* img, int threshold);
void bmp8_applyFilter(t_bmp8* img, float** kernel, int kernelSize);

#endif
