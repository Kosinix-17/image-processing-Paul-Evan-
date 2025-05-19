#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"

t_bmp8* bmp8_loadImage(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Erreur: Impossible d'ouvrir le fichier\n");
        return NULL;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    if (header[0] != 'B' || header[1] != 'M') {
        printf("Erreur: Le fichier n'est pas au format BMP\n");
        fclose(file);
        return NULL;
    }

    unsigned int width = *(unsigned int*)&header[18];
    unsigned int height = *(unsigned int*)&header[22];
    unsigned short colorDepth = *(unsigned short*)&header[28];

    if (colorDepth != 8) {
        printf("Erreur: L'image n'est pas en 8 bits\n");
        fclose(file);
        return NULL;
    }

    t_bmp8* img = (t_bmp8*)malloc(sizeof(t_bmp8));
    if (img == NULL) {
        printf("Erreur: Allocation mémoire échouée\n");
        fclose(file);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;
    img->dataSize = width * height;

    fread(img->palette, sizeof(unsigned char), 1024, file);

    img->pixels = (unsigned char*)malloc(img->dataSize);
    if (img->pixels == NULL) {
        printf("Erreur: Allocation mémoire échouée pour les pixels\n");
        free(img);
        fclose(file);
        return NULL;
    }

    fread(img->pixels, sizeof(unsigned char), img->dataSize, file);
    fclose(file);
    return img;
}

void bmp8_saveImage(const char* filename, t_bmp8* img) {
    if (img == NULL) {
        printf("Erreur: Image invalide\n");
        return;
    }

    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Erreur: Impossible de créer le fichier\n");
        return;
    }

    unsigned char header[54] = {0};
    header[0] = 'B';
    header[1] = 'M';

    unsigned int fileSize = 54 + 1024 + img->dataSize;
    unsigned int offset = 54 + 1024;

    *(unsigned int*)&header[2] = fileSize;
    *(unsigned int*)&header[10] = offset;
    *(unsigned int*)&header[14] = 40;
    *(unsigned int*)&header[18] = img->width;
    *(unsigned int*)&header[22] = img->height;
    *(unsigned short*)&header[26] = 1;
    *(unsigned short*)&header[28] = img->colorDepth;
    *(unsigned int*)&header[34] = img->dataSize;

    fwrite(header, sizeof(unsigned char), 54, file);
    fwrite(img->palette, sizeof(unsigned char), 1024, file);
    fwrite(img->pixels, sizeof(unsigned char), img->dataSize, file);

    fclose(file);
}

void bmp8_free(t_bmp8* img) {
    if (img != NULL) {
        if (img->pixels != NULL) {
            free(img->pixels);
        }
        free(img);
    }
}

void bmp8_printInfo(t_bmp8* img) {
    if (img != NULL) {
        printf("Image Info:\n");
        printf("Width: %u\n", img->width);
        printf("Height: %u\n", img->height);
        printf("Color Depth: %u\n", img->colorDepth);
        printf("Data Size: %u\n", img->dataSize);
    }
}

// Fonction pour inverser les couleurs de l'image
void bmp8_negative(t_bmp8* img) {
    // Vérification que l'image et ses pixels existent
    if (img == NULL || img->pixels == NULL) {
        return;
    }

    // Parcours de tous les pixels et inversion des valeurs
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->pixels[i] = 255 - img->pixels[i];
    }
}

// Fonction pour modifier la luminosité de l'image
void bmp8_brightness(t_bmp8* img, int value) {
    // Vérification que l'image et ses pixels existent
    if (img == NULL || img->pixels == NULL) {
        return;
    }

    // Parcours de tous les pixels
    for (unsigned int i = 0; i < img->dataSize; i++) {
        int newValue = img->pixels[i] + value;

        // Limitation des valeurs entre 0 et 255
        if (newValue > 255) {
            newValue = 255;
        }
        if (newValue < 0) {
            newValue = 0;
        }

        img->pixels[i] = (unsigned char)newValue;
    }
}

// Fonction pour appliquer un seuil à l'image
void bmp8_threshold(t_bmp8* img, int threshold) {
    // Vérification que l'image et ses pixels existent
    if (img == NULL || img->pixels == NULL) {
        return;
    }

    // Parcours de tous les pixels
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->pixels[i] = (img->pixels[i] >= threshold) ? 255 : 0;
    }
}

// Fonction pour appliquer un filtre à l'image
void bmp8_applyFilter(t_bmp8* img, float** kernel, int kernelSize) {
    // Vérifications de base
    if (img == NULL || img->pixels == NULL || kernel == NULL) {
        return;
    }

    // Vérification que la taille du noyau est impaire
    if (kernelSize % 2 == 0) {
        return;
    }

    int n = kernelSize / 2;

    // Création d'une copie temporaire de l'image
    unsigned char* tempPixels = (unsigned char*)malloc(img->dataSize);
    if (tempPixels == NULL) {
        return;
    }

    // Copie des pixels originaux
    for (unsigned int i = 0; i < img->dataSize; i++) {
        tempPixels[i] = img->pixels[i];
    }

    // Application du filtre
    for (unsigned int y = n; y < img->height - n; y++) {
        for (unsigned int x = n; x < img->width - n; x++) {
            float sum = 0.0f;

            // Application du noyau de convolution
            for (int i = -n; i <= n; i++) {
                for (int j = -n; j <= n; j++) {
                    // Calcul de la position du pixel dans l'image
                    unsigned int pixelPos = (y + i) * img->width + (x + j);
                    // Valeur du kernel à la position correspondante
                    float kernelValue = kernel[i + n][j + n];
                    // Accumulation de la somme
                    sum += tempPixels[pixelPos] * kernelValue;
                }
            }

            // Limitation des valeurs entre 0 et 255
            if (sum > 255) sum = 255;
            if (sum < 0) sum = 0;

            // Mise à jour du pixel dans l'image originale
            img->pixels[y * img->width + x] = (unsigned char)sum;
        }
    }

    // Libération de la mémoire temporaire
    free(tempPixels);
}