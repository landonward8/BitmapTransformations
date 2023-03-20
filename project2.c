#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

// Struct for an image, containing its dimensions and pixel data
struct bitmap
{
        int *pixels;
	int width;
	int height;
};

const int DIB_HEADER_SIZE = 14;
const int BMP_HEADER_SIZE = 40;

// Make "byte" mean "unsigned char"
typedef unsigned char byte;

// Calculates the stride of a .bmp file.
// (The stride is how many bytes of memory a single row of
// the image requires.)
inline int bmp_file_stride(struct bitmap *bmp);

// Calculates the total size that a .bmp file for this bitmap
// would need (in bytes)
inline int bmp_file_size(struct bitmap *bmp);

// Opens the file with the given name and maps it into memory
// so that we can access its contents through pointers.
void *map_file_for_reading(char *filename);

// Opens (and creates if necessary) the file with the given name
// and maps it into memory so that we can access its contents
// through pointers. 
void *map_file_for_writing(char *filename, int file_size);

// Takes the contents of a bitmap file (bmp_file) and reads
// its data, filling in the struct bitmap pointed to by bmp.
// Returns 0 if everything worked, -1 if the file data isn't
// valid.
int read_bitmap(void *bmp_file, struct bitmap *bmp);

void write_bitmap(void *bmp_file, struct bitmap *bmp);

// Converts between a packed pixel (0xRRGGBB) and its components.
void rgb_to_pixel(int *p, int r, int g, int b);
void pixel_to_rgb(int p, int *r, int *g, int *b);

void bitmap_to_grayscale(struct bitmap *bmp);

void bitmap_posterize(struct bitmap *bmp);

void bitmap_squash(struct bitmap *bmp);


/* Please note: if your program has a main() function, then
 * the test programs given to you will not run (your main()
 * will override the test program's). When running a test,
 * make sure to either comment out or rename your main()
 * function!
 */

int main(int argc, char *argv[])
 {
        
         printf("Menu\n");
         printf("   G) Make grayscale\n");
         printf("   P) Posterize\n");
         printf("   U) Squash\n");
         printf("   S) Save\n");
         printf("   Q) Quit\n");
         printf("USE tiv test3.bmp TO SEE IMAGE\n");
         printf("What would you like to do?\n");

        char *read = map_file_for_reading(argv[1]);
        struct bitmap select;
        read_bitmap(read, &select);
     char enter;
     scanf("%c", &enter);
        switch(enter) {
           case 'S':
              printf("Saved");
              printf("If you would like to do anything else to the image, type: !. ");
              break;
           case 'G':
              printf("Grayscale selected");
              bitmap_to_grayscale(&select);
              printf("\n");
              printf("If you would like to do anything else to the image, type: !. ");
              break;
           case 'P':
              printf("Posterize Selected");
              bitmap_posterize(&select);
              printf("\n");
              printf("If you would like to do anything else to the image, type: !. ");
              break;
           case 'U':
              printf("Squash Selected");
              bitmap_squash(&select); 
              printf("\n");
              printf("If you would like to do anything else to the image, type: !. ");
              break;
           case 'Q':
              printf("Quit Selected");
              break;
           default:
              printf("Error");
           }
      char *write = map_file_for_writing("test3.bmp", bmp_file_size(&select));
      write_bitmap(write, &select);
      return 0;
}

int bmp_file_stride(struct bitmap *bmp)
{
    return (24 * bmp->width + 31) / 32 * 4;
}

int bmp_file_size(struct bitmap *bmp)
{
    int stride = bmp_file_stride(bmp);
    return DIB_HEADER_SIZE
        + BMP_HEADER_SIZE
        + stride * bmp->height;
}

void *map_file_for_reading(char *filename)
{
    // A) Use open() to open the file for reading.
    int opener = open(filename, O_RDONLY);
    struct stat statbuf;
    if(opener == -1) {
        perror(NULL);
        return NULL;
     }
    // B) Use fstat() to determine the size of the file.
    int statsize = fstat(opener, &statbuf);
        if(statsize == -1) {
        perror(NULL);
        return NULL;
    }
    long file_size = statbuf.st_size;
    // C) Call mmap() to map the file into memory.
      char*  map = mmap(NULL, file_size, PROT_READ, MAP_SHARED, opener, 0);
      if (map == MAP_FAILED) {
          perror(NULL);
          return NULL;
    }
    // D) Close the file using close().
     close(opener);

    // E) Return the pointer returned from mmap().


    // Default implementation: just returns NULL.
    return map;
}

void *map_file_for_writing(char *filename, int file_size)
{
    // A) Use open() to open the file for writing.
    int opener = open(filename, O_RDWR | O_CREAT, 0644);
    if(opener == -1) 
    {
        perror(NULL);
        return (NULL);
    }
    // B) Use ftruncate() to set the size of the file.
    int setsize = ftruncate(opener, file_size);
    if (setsize == -1) 
    {
       perror(NULL);
       return (NULL);
    }

    // C) Call mmap() to map the file into memory.
    int *p = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, opener, 0);
    if(p == MAP_FAILED) 
    {
      perror(NULL);
      return (NULL);
    }

    // D) Close the file using close().
    close(opener);

    // E) Return the pointer returned from mmap().
    return p;

    // Default implementation: just returns NULL.
   //  return NULL;

}

int read_bitmap(void *bmp_file, struct bitmap *bmp)
{
    // Cast bmp_file to a byte * so we can access it
    // byte by byte.
    byte *file = (byte *) bmp_file;

    // Check the magic: it should start with "BM"
    if (file[0] != 'B' || file[1] != 'M') {
       printf("ERROR");
       return -1;
       }
   bmp ->width = *((int *)(file + 18));
   bmp ->height = *((int *)(file + 22));
   // Data offset
   int data = *((int *)(file + 10));
   // Color Depth
    short color = *((int *)(file + 28));
    if(color != 24) 
    {
       printf("Color depth is not 24");
        return -1;   
    } 
    int *compression = (int *)(file + 30);
    if(*compression != 0)
    {
        printf("The compression is not 0");
        return -1;
    } 
   //Stride
   int stride = bmp_file_stride(bmp);
   
   //Size of picture
   int picture_size = bmp->height * bmp->width;

   //Malloc for pixel data
   bmp->pixels = (int*) malloc(picture_size * sizeof(int));
   int pixel;
   for(int y =0; y < bmp->height; y++)
   { 
   int begins = 0;
      for (int x = 0; x < bmp->width; x++)
      {
         int b = *(file + data++);
         int g = *(file + data++);
         int r = *(file + data++);
         rgb_to_pixel(&pixel, r,g,b);
         bmp->pixels[picture_size-(bmp->width * (y + 1)) + begins] = pixel;
         begins++;
      }
      data = data + stride %3;
    }
    // Default implementation: just returns 0.
    return 0;
}

void write_bitmap(void *bmp_file, struct bitmap *bmp)
{
    
    // Cast bmp_file to a byte * so we can access it
    // byte by byte.
    byte *file = (byte *) bmp_file;

    *((int*)(file+18)) = bmp ->width;
    *((int*)(file+22)) = bmp ->height;

    // Magic 
    file[0] = 'B';
    file[1] = 'M';
    // File size
    *((int *)(file + 2)) = bmp_file_size(bmp);
    // Pixel data offset
    int data = 54;
    file[10] = data;
    // Header Size
    file[14] = 40;
    // Color Planes
    file[26] = 1;  
    // Color Depth
    file[28] = 24; 
    // Compression method
    file[30] = 0;   
    // Stride
    int stride = bmp_file_stride(bmp);
    // Image size
    int image_size = bmp->height * bmp_file_stride(bmp);
    *((int *)(file +34)) = image_size; 
    // Palette size 
    file[46] = 0;

    int b;
    int g;
    int r;
    int begins = 0;

    for(int y = bmp->height - 1; y >= 0; y--)
    {
        for(int x = 0; x < bmp-> width; x++)
        {
           int pixel = bmp->pixels[y*bmp->width + x];
           pixel_to_rgb(pixel, &r, &g, &b);
           file[data + begins++] = b;
           file[data + begins++] = g;
           file[data + begins++] = r;
        }
        begins  = begins + stride %3;
    }       

}

void rgb_to_pixel(int *p, int r, int g, int b)
{

    // Pack r, g, and b into an int value and save
    // into what p points to
    *p = ((r<<16)|(g << 8)|(b));
}

void pixel_to_rgb(int p, int *r, int *g, int *b)
{
    // Separate the pixel p into its components
    // and save in the pointers
    *r = ((p & 0xff0000) >>  16);
    *g = ((p & 0xff00) >> 8);
    *b = ((p & 0xff));
}
 void bitmap_to_grayscale(struct bitmap *bmp) 
{
     int total_pixels = bmp->width *  bmp->height;
     int p, r,g,b, gray;
     for(int i = 0; i < total_pixels; i++)
     {
        p = bmp->pixels[i];
        pixel_to_rgb(p, &r, &g, &b);
        gray = (r + b + g)/3;
        r = g = b = gray;
        rgb_to_pixel(&p, r, g, b);
        bmp->pixels[i] = p;
     }
 }
 void bitmap_posterize(struct bitmap *bmp)
{
  int total_pixels = bmp->height * bmp -> width;
  int p,r,g,b; 
  int *value[3] = {&r, &g, &b};
  for (int i = 0; i < total_pixels; i++) 
  {
     p = bmp->pixels[i];
     pixel_to_rgb(p,&r,&g,&b);
     for(int j = 0; j < 3; j++) 
     {
        if(*value[j] < 32)
        {
            *value[j] = 0;
        }
        else if(*value[j] <= 95) 
        { 
            *value[j] = 64;
        }
        else if(*value[j] <= 159)
        {
            *value[j] = 128;
        }
        else if(*value[j] <= 223)
        {
            *value[j] = 192;
        }
        else if(*value[j] >= 224)
        {
            *value[j] = 225;
        }   
     }     
     rgb_to_pixel(&p,r,g,b);
     bmp->pixels[i] = p;
     }
  }  
 void bitmap_squash(struct bitmap *bmp) 
 {
 
 int temp_width = bmp->width/2;
 int new_height = bmp->height;
 int *total_pixels = (int*) malloc(temp_width * bmp->height * sizeof(int));
 int p;
 int r1;
 int r;
 int g1; 
 int g; 
 int b1; 
 int b;
 
 for(int y = 0; y < new_height; y++)
 {
    for(int x = 0; x < temp_width; x++)
    {
       int first = bmp->pixels[y*bmp->width + (2 * x)];
       pixel_to_rgb(first, &r1, &g1, &b1);
       int second = bmp->pixels[y*bmp->width + ((2 * x) + 1)];
       pixel_to_rgb(second, &r, &g, &b);
       int red = (r1 + r)/2;
       int green = (g1 + g)/2;
       int blue = (b1 + b)/2;
       rgb_to_pixel(&p, red, green, blue);
       total_pixels[y*temp_width + x] = p;
    }
    }
    free(bmp->pixels);
    bmp->width = temp_width;
    bmp->pixels = total_pixels;
 }

  
    
