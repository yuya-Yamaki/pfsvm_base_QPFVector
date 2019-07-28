#include <stdio.h>
#include <stdlib.h>

typedef unsigned char img_t;
typedef struct {
  int height;
  int width;
  int maxval;
  img_t **val;
} IMAGE;

FILE *fileopen(char *filename, const char *mode)
{
    FILE *fp;
    fp = fopen(filename, mode);
    if (fp == NULL) {
        fprintf(stderr, "Can\'t open %s!\n", filename);
        exit(1);
    }
    return (fp);
}

void *alloc_mem(size_t size)
{
    void *ptr;
    if ((ptr = (void *)malloc(size)) == NULL) {
        fprintf(stderr, "Can\'t allocate memory (size = %d)!\n", (int)size);
        exit(1);
    }
    return (ptr);
}

void **alloc_2d_array(int height, int width, int size)
{
    void **mat;
    char *ptr;
    int k;

    mat = (void **)alloc_mem(sizeof(void *) * height + height * width * size);
    ptr = (char *)(mat + height);
    for (k = 0; k < height; k++) {
        mat[k] =  ptr;
        ptr += width * size;
    }
    return (mat);
}

IMAGE *alloc_image(int width, int height, int maxval)
{
    IMAGE *img;
    img = (IMAGE *)alloc_mem(sizeof(IMAGE));
    img->width = width;
    img->height = height;
    img->maxval = maxval;
    img->val = (img_t **)alloc_2d_array(img->height, img->width,
                                        sizeof(img_t));
    return (img);
}

void free_image(IMAGE *img)
{
    if (img != NULL && img->val != NULL) {
        free(img->val);
        free(img);
    } else {
        fprintf(stderr, "! error in free_image()\n");
        exit(1);
    }
}

IMAGE *read_pgm(char *filename)
{
    int i, j, width, height, maxval;
    char tmp[256];
    IMAGE *img;
    FILE *fp;

    fp = fileopen(filename, "rb");
    fgets(tmp, 256, fp);
    if (tmp[0] != 'P' || tmp[1] != '5') {
        fprintf(stderr, "Not a PGM file!\n");
        exit(1);
    }
    while (*(fgets(tmp, 256, fp)) == '#');
    sscanf(tmp, "%d %d", &width, &height);
    while (*(fgets(tmp, 256, fp)) == '#');
    sscanf(tmp, "%d", &maxval);
    img = alloc_image(width, height, maxval);
    for (i = 0; i < img->height; i++) {
	for (j = 0; j < img->width; j++) {
            img->val[i][j] = (img_t)fgetc(fp);
	}
    }
    fclose(fp);
    return (img);
}

int main(int argc, char **argv)
{
    int i, j, v;
    double y, cb, cr;
    IMAGE *img1, *img2;
    FILE *fp;

    img1 = read_pgm(argv[1]);
    img2 = read_pgm(argv[2]);

    fp = fopen("diff.ppm", "wb");
    fprintf(fp, "P6\n%d %d\n255\n", img1->width, img1->height);
    for (i = 0; i < img1->height; i++) {
	for (j = 0; j < img1->width; j++) {
	    y = img1->val[i][j];
	    cb = 0;
	    cr = (img2->val[i][j] - img1->val[i][j]) * 50;
	    v = y + 1.5748 * cr + 0.5;
	    if (v < 0) v = 0;
	    else if (v > 255) v = 255;
	    fputc(v, fp);
	    v = y - 0.187324 * cb - 0.468124 * cr + 0.5;
	    if (v < 0) v = 0;
	    else if (v > 255) v = 255;
	    fputc(v, fp);
	    v = y + 1.8556 * cb + 0.5;
	    if (v < 0) v = 0;
	    else if (v > 255) v = 255;
	    fputc(v, fp);
	}
    }
    fclose(fp);
    return (0);
}
