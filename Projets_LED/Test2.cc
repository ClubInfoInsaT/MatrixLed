// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
#include "led-matrix.h"

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/shm.h>

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>

static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;
using std::vector;

bool interrupt_received;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

typedef struct _coul {
  int r;
  int g;
  int b;
} coul;

enum Direction {l,u,r,d}; //l -> left u -> up r -> right d -> down


typedef struct _corps { //Contain the coordonates of the parts of the snake
  unsigned int x;
  unsigned int y;
  struct _corps* next;
} corps;

 typedef struct _Snake {
   enum Direction dir; //Direction of snake. 1 left 2 up 3 right 4 bot
   int length; //Length of the snake, optionnal ?
   corps * head;
   corps * tail;
 } Snake;
 
 
 
coul matrix[64][64];
Canvas * canvas;

void init(int argc, char *argv[]){

  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 64;
  defaults.chain_length = 2;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;
  canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
  
  printf("Canvas OK\n");
  canvas->Fill(255,255,255);
  int i, j ;
    for(i=0; i<64; i++){
      for(j=0; j<64; j++){
        matrix[i][j].r=0;
        matrix[i][j].g=0;
        matrix[i][j].b=0;
      }
    }

}


//Dessine la matrice
static void DrawOnCanvas(Canvas *canvas) {
  int i, j;  
  for(i=0; i<64; i++){
    for(j=0; j<64; j++){
      canvas->SetPixel(i, j, matrix[i][j].r, matrix[i][j].g, matrix[i][j].b);
    }
  }
}

    
    

void AutoMove(Snake * snake){ //Fonction qui fait avancer le snake selon sa direction
  corps * aux = NULL;
  switch (snake->dir){
    case (l) :
    snake->head->next = (corps *)malloc(sizeof(corps)); //On crée un nouvelle tête
    snake->head->next->x = (snake->head->x - 1)%64; //On initialise les coordonées de la nouvelle tête
    snake->head->next->y = (snake->head->y)%64;
    snake->head->next->next =NULL; //On fait pointer le next de la nouvelle tête vers NULL
    snake->head = snake->head->next; //La nouvelle tête est rattachée à la case Sbake du début
    aux = snake->tail->next; //On avance la tail du snake en supprimant la la queue et en la réalouant à la case d'après
    free(snake->tail);
    snake->tail = aux;
    break;
    case (u) :
    snake->head->next = (corps *)malloc(sizeof(corps));
    snake->head->next->x = (snake->head->x)%64;
    snake->head->next->y = (snake->head->y -1)%64;
    snake->head->next->next =NULL;
    snake->head = snake->head->next;
    aux = snake->tail->next;
    free(snake->tail);
    snake->tail = aux;
    break;
    case (r) :
    snake->head->next = (corps *)malloc(sizeof(corps));
    snake->head->next->x = (snake->head->x + 1)%64;
    snake->head->next->y = (snake->head->y)%64;
    snake->head->next->next =NULL;
    snake->head = snake->head->next;
    aux = snake->tail->next;
    free(snake->tail);
    snake->tail = aux;
    break;
    case (d) :
    snake->head->next = (corps *)malloc(sizeof(corps));
    snake->head->next->x = (snake->head->x)%64;
    snake->head->next->y = (snake->head->y +1)%64;
    snake->head->next->next =NULL;
    snake->head = snake->head->next;
    aux = snake->tail->next;
    free(snake->tail);
    snake->tail = aux;
    break;
    default : 
    break;
  }
}

void PixelOff(Snake * snake){
  corps * aux = (snake->tail);
  for (int i = 0; i<(snake->length);i++){
    printf("Fonction PixelOn\n");
    printf("x = %d\ni = %d\nadresse de aux = %x\n",aux->x,i,aux);
    matrix[aux->x][aux->y].r = 0;
    matrix[aux->x][aux->y].g = 0;
    matrix[aux->x][aux->y].b = 0;
    aux = aux->next;
  }

}

  
void PixelOn(Snake * snake){
  corps * aux = (snake->tail);
  for (int i = 0; i<(snake->length);i++){
    printf("Fonction PixelOn\n");
    printf("x = %d\ni = %d\nadresse de aux = %x\n",aux->x,i,aux);
    matrix[aux->x][aux->y].r = 255;
    matrix[aux->x][aux->y].g = 255;
    matrix[aux->x][aux->y].b = 255;
    aux = aux->next;
  }
}


    

int main(int argc, char *argv[]) {
  /* _______________Gestion Clavier__________________________*/
  const char *dev = "/dev/input/by-path/platform-3f980000.usb-usb-0:1.2:1.0-event-kbd";
    struct input_event ev;
    ssize_t n;
    int fd;

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        return EXIT_FAILURE;
    }
   
  /*_____________CODE_____________________________*/
  
  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  init(argc,argv);
  
  if (canvas == NULL){
    printf("Canvas NULL\n");
    return 1;
  }
  
   unsigned int x =31,y=31,red=255,green=255,blue=255;
   
   Snake * snake = NULL; snake = (Snake *)malloc(sizeof(Snake));
   corps * case1 = NULL; case1 = (corps *)malloc(sizeof(corps));
   corps * case2 = NULL; case2 = (corps *)malloc(sizeof(corps));
   corps * case3 = NULL; case3 = (corps *)malloc(sizeof(corps));
   printf("Snake : %p\n1 : %x\n2 : %x\n3: %x\n",snake,case1,case2,case3);
   snake->dir = r;
   snake->length = 3;
   snake->head = case1;
   snake->tail = case3;
   case3->x = 31+2;
   case3->y = 31;
   case3->next = case2; 
   case2->x = 31+1;
   case2->y = 31;
   case2->next = case1;
   case1->x = 31;
   case1->y = 31;
   case1->next = NULL;

  
  while(!interrupt_received){
    usleep(9000);
    PixelOff(snake);
    AutoMove(snake);
    PixelOn(snake);
    DrawOnCanvas(canvas);
    //matrix[x][y].r = 0;
    //matrix[x][y].g = 0;
    //matrix[x][y].b = 0;
    
    n = read(fd, &ev, sizeof ev);
        if (n == (ssize_t)-1) {
            if (errno == EINTR)
                continue;
            else
                break;
        } else
        if (n != sizeof ev) {
            errno = EIO;
            break;
		}
    if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2){
     //printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);
     if(ev.value == 1 || ev.value == 2){
        switch(ev.code){
          case(103): //up
            //x=(x+1)%64;
            snake->dir = u;
            break;
          case(108): //down
            //x=(x-1)%64;
            snake->dir = d;
            break;
          case(106): // right
            //y=(y+1)%64;
            snake->dir = r;
            break;
          case(105): //left
            //y=(y-1)%64;
            snake->dir = l;
            break;
          case(75): //touche 4 aug red
            red=(red+8)%256;
            break;
          case(76): //touche 5 aug green
            green=(green+8)%256;
            break;
          case(77): //touche 6 aug blue
            blue=(blue+8)%256;
            break;
          case(79): //touche 1 dim red
            red=(red-8)%256;
            break;
          case(80): //touche 2 dim green
            green=(green-8)%256;
            break;
          case(81): //touche 3 dim blue
            blue=(blue-8)%256;
            break;
          default :
            break;
        }
      }
    }
    //matrix[x][y].r = red;
    //matrix[x][y].g = green;
    //matrix[x][y].b = blue;
    //system("clear");
  }
   fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
  
  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
