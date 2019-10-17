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

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;
using std::vector;

Canvas * canvas;

void InitCanvas(){
    int argc;
    char *argv[];
    RGBMatrix::Options defaults;
    defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
    defaults.rows = 64;
    defaults.chain_length = 2;
    defaults.parallel = 1;
    defaults.show_refresh_rate = true;
    canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);

//si fichier !exist -> creer fichier
}


//Dessine la matrice
static void DrawOnCanvas(Canvas *canvas) {
int i, j;  
//lecture fichier et ecriture matrice
for(i=0; i<64; i++){
    for(j=0; j<64; j++){
    canvas->SetPixel(i, j, matrix[i][j].r, matrix[i][j].g, matrix[i][j].b);
    }
}
}

    
void DoStart(char* filename,int frames){
    InitCanvas();
    if (canvas == NULL){
        printf("Canvas NULL\n");
        return 1;
    }
    if(fork()){
        FILE * fichier_pid = fopen("pid_file", "w");
        fprints(ficher_pid,itoa(getpid()));
        system("chmod 000 pid_file");
        //callback
    }
}

void DoStop(){
    //verifier si le ficher pid existe
    //sudo ouvre le ficher du pid 
    // kill-7 le pid
    //sudo supprime le ficher en question
    
    // Animation finished. Shut down the RGB matrix.
    canvas->Clear();
    delete canvas;
}


int main(int argc, char *argv[]) {
if(strcomp(argv[1],"start")){
    int fps = 30;
    if(argc = 4){
        fps = atoi(argv[3]);
    }
    DoStart(argv[2],fps);
}
if(strcomp(argv[1],"stop")){
    DoStop();
}

return 0;
}
