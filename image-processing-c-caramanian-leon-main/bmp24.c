#include <stdlib.h>
#include <stdio.h>
#include "bmp24.h"

void file_rawRead(uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}

void file_rawWrite(uint32_t position, void * buffer, uint32_t size, size_t n, FILE * file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}

t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = malloc(height * sizeof(t_pixel *));
    if (pixels == NULL) {
        fprintf(stderr, "Erreur : allocation mémoire échouée pour les lignes.\n");
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(t_pixel));
        if (pixels[i] == NULL) {
            fprintf(stderr, "Erreur : allocation mémoire échouée pour la ligne %d.\n", i);
            // Libérer ce qui a été alloué avant
            for (int j = 0; j < i; j++) {
                free(pixels[j]);
            }
            free(pixels);
            return NULL;
        }
    }
    return pixels;
}

void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    if (pixels == NULL) return;
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = malloc(sizeof(t_bmp24));
    if (img == NULL) {
        fprintf(stderr, "Erreur : allocation mémoire échouée pour l'image.\n");
        return NULL;
    }

    img->data = bmp24_allocateDataPixels(width, height);
    if (img->data == NULL) {
        free(img);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;

    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (img == NULL) return;
    bmp24_freeDataPixels(img->data, img->height);
    free(img);
}

void bmp24_readPixelValue(t_bmp24 *image, int x, int y, FILE *file) {
    uint8_t colors[3]; // Buffer pour B, G, R

    // Lire 3 octets à la position actuelle
    fread(colors, sizeof(uint8_t), 3, file);

    // Attention, dans le fichier : BGR -> en mémoire : RGB
    image->data[y][x].blue  = colors[0];
    image->data[y][x].green = colors[1];
    image->data[y][x].red   = colors[2];
}

void bmp24_writePixelValue(t_bmp24 *image, int x, int y, FILE *file) {
    uint8_t colors[3];

    // Stocker en BGR pour écrire dans le fichier
    colors[0] = image->data[y][x].blue;
    colors[1] = image->data[y][x].green;
    colors[2] = image->data[y][x].red;

    // Écrire 3 octets
    fwrite(colors, sizeof(uint8_t), 3, file);
}

void bmp24_readPixelData(t_bmp24 *image, FILE *file) {
    // Aller à l'offset où commencent les pixels
    fseek(file, BITMAP_OFFSET, SEEK_SET);

    // Parcourir l'image de bas en haut
    for (int y = image->height - 1; y >= 0; y--) {
        for (int x = 0; x < image->width; x++) {
            bmp24_readPixelValue(image, x, y, file);
        }
    }
}

void bmp24_writePixelData(t_bmp24 *image, FILE *file) {
    // Aller à l'offset où commencent les pixels
    fseek(file, BITMAP_OFFSET, SEEK_SET);

    // Parcourir l'image de bas en haut
    for (int y = image->height - 1; y >= 0; y--) {
        for (int x = 0; x < image->width; x++) {
            if (fwrite(&image->data[y][x], sizeof(t_pixel), 1, file) != 1) {
                printf("Erreur d'écriture des pixels dans le fichier.\n");
                return;
            }
        }
    }
}

t_bmp24 *bmp24_loadImage(const char * filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }

    int width, height;
    uint16_t depth;

    file_rawRead(BITMAP_WIDTH, &width, sizeof(int32_t), 1, file);
    file_rawRead(BITMAP_HEIGHT, &height, sizeof(int32_t), 1, file);
    file_rawRead(BITMAP_DEPTH, &depth, sizeof(uint16_t), 1, file);

    t_bmp24 *img = bmp24_allocate(width, height, depth);
    if (!img) {
        fclose(file);
        return NULL;
    }

    file_rawRead(BITMAP_MAGIC, &(img->header), sizeof(t_bmp_header), 1, file);
    file_rawRead(HEADER_SIZE, &(img->header_info), sizeof(t_bmp_info), 1, file);

    bmp24_readPixelData(img, file);

    fclose(file);
    return img;
}

void bmp24_saveImage(t_bmp24 *img, const char * filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erreur : Impossible de creer le fichier %s\n", filename);
        return;
    }

    // Calculer la taille du fichier complet (en-tête + données)
    img->header.size = sizeof(t_bmp_header) + sizeof(t_bmp_info) + img->height * img->width * 3;
    img->header_info.imagesize = img->height * img->width * 3;

    // Ecrire l'en-tête BMP
    file_rawWrite(BITMAP_MAGIC, &(img->header), sizeof(t_bmp_header), 1, file);
    file_rawWrite(HEADER_SIZE, &(img->header_info), sizeof(t_bmp_info), 1, file);

    // Ecrire la largeur, la hauteur et la profondeur de couleur
    file_rawWrite(BITMAP_WIDTH, &(img->width), sizeof(int32_t), 1, file);
    file_rawWrite(BITMAP_HEIGHT, &(img->height), sizeof(int32_t), 1, file);
    file_rawWrite(BITMAP_DEPTH, &(img->colorDepth), sizeof(uint16_t), 1, file);

    // Ecrire les données des pixels
    bmp24_writePixelData(img, file);

    fclose(file);
    printf("Image sauvegardée avec succès !\n");
}1
void bmp24_negative(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red   = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue  = 255 - img->data[y][x].blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t gray = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            img->data[y][x].red   = gray;
            img->data[y][x].green = gray;
            img->data[y][x].blue  = gray;
        }
    }
}

void bmp24_brightness(t_bmp24 *img, int value) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int r = img->data[y][x].red + value;
            int g = img->data[y][x].green + value;
            int b = img->data[y][x].blue + value;

            // Clamp entre 0 et 255
            if (r > 255) r = 255;
            if (r < 0)   r = 0;
            if (g > 255) g = 255;
            if (g < 0)   g = 0;
            if (b > 255) b = 255;
            if (b < 0)   b = 0;

            img->data[y][x].red   = (uint8_t)r;
            img->data[y][x].green = (uint8_t)g;
            img->data[y][x].blue  = (uint8_t)b;
        }
    }
}
