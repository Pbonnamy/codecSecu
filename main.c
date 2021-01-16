//
//  main.c
//  Decriptage
//
//  Created by William Lin and Pierre Bonnamy on 12/12/2020.
//  Copyright © 2020 Groupe7. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define NOIR al_map_rgb( 0,0,0)
#define ROUGE al_map_rgb( 255,0,0)
#define BLANC al_map_rgb( 255,255,255)
#define VERT al_map_rgb( 0,255,0)

ALLEGRO_DISPLAY *display;
ALLEGRO_BITMAP*image;
ALLEGRO_BITMAP*coursImg;
ALLEGRO_FONT*arial;
ALLEGRO_FONT*arial2;
ALLEGRO_EVENT_QUEUE *queue;
ALLEGRO_EVENT event;
ALLEGRO_FILECHOOSER *filechooser;

void erreur(const char*txt){
    FILE *fp;
    fp = fopen("error.txt","r+");
    if (fp!=NULL) {
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "%s",txt);
    }
    fclose(fp);
}

int g4cVerification(const char pathname[],char matrix[]){
    //printf("\n%s",pathname);
    FILE *file = fopen(pathname, "r");
       int n = 0;
       int error = 0;
       char element;
       char ref[41]={'G','4','C','=','[','-','-','-','-','-','-','-','-',' ','-','-','-','-','-','-','-','-',' ','-','-','-','-','-','-','-','-',' ','-','-','-','-','-','-','-','-',']'};

       if (file == NULL)
           erreur("erreur ouverture fichier");

       fseek(file, 0, SEEK_END);
       long fSize = ftell(file);
       fseek(file, 0, SEEK_SET);

       if (fSize != 41){
           error = 1;
       }else{
           do{
               element = fgetc(file);

               if(ref[n] != '-'){
                   if (ref[n] != element){
                       error = 1;
                   }
               }
               else if (ref[n] == '-'){
                   if(element != '0' && element != '1'){
                       error = 1;
                   }
               }

               n+=1;
           }while (feof(file) == 0 && n!=41);
       }

       if(error == 0){
           fseek(file, 0, SEEK_SET);
           for(n=0;n<41;n++){
               matrix[n]=fgetc(file);
           }
           return 1;
       }else if (error == 1){
           return 0;
       }
       fclose(file);
    return 0;
}


int *MatriceIdentiter(char ref[], int Midentity[]){
    int i,j=0;
    int counter=1;
    char ligne1='1';
    char ligne2='0';
    char ligne3='0';
    char ligne4='0';
    while(Midentity[3]==0){
        for(i=5;i<13;++i){
            if(ref[i]==ligne1 && ref[i+9]==ligne2 && ref[i+18]==ligne3 && ref[i+27]==ligne4 ){
                Midentity[j]=counter;
                counter = 1;
                break;
            }
            counter++;
        }
        ++j;
        switch (j) {
            case 1:
                ligne1='0';
                ligne2='1';
                ligne3='0';
                ligne4='0';
                break;
            case 2:
                ligne1='0';
                ligne2='0';
                ligne3='1';
                ligne4='0';
                break;
            case 3:
                ligne1='0';
                ligne2='0';
                ligne3='0';
                ligne4='1';
                break;
        }
    }
    return Midentity;
}

char *convertBinary(int Midentity[], char octet, char tmpBinary[]){
    int i,j;
    int a = 0x100;
    char tmpHexa[10];
    long hexadecimal;

    sprintf(tmpHexa,"%2.2hhx",octet);//pour avoir l octer en hexa dans le tableau tmpHexa

    hexadecimal = strtol(tmpHexa,NULL,16);

    for(i=0;i<4;++i){
        a=0x100;
        for(j=0;j<Midentity[i];++j){
            a=a/2;
        }
        if(hexadecimal & a){
            tmpBinary[i]='1';
        }else{
            tmpBinary[i]='0';
        }
    }
    return tmpBinary;
}

int deciffer(char matrix[],const char pathFile[]){
    int i=0;
    unsigned char tmp=1;
    char octet;
    char tmpBinary[4];
    char tmpOctet[8]={-1,-1,-1,-1,-1,-1,-1,-1};
    char buffer=0;
    int Midentity[4]={0};
    char filename[255];
    MatriceIdentiter(matrix, Midentity);

    FILE *fp;

    fp = fopen(pathFile, "rb");
    char *newPathFile = strrchr(pathFile, 'c');
    strcpy(newPathFile, "");
    newPathFile = strrchr(pathFile,'\\');
    strcpy(filename, newPathFile+1);
    if (fp != NULL) {
        FILE *newFile;
        newFile = fopen(filename,"w+b");
        if (newFile == NULL) {
            erreur("\nErreur Sur la creation du fichier");
            return 1;
        }else{
             do {
                octet = fgetc(fp);
                convertBinary(Midentity, octet, tmpBinary);
                //permet de prendre l'octet et de retourner les 4 bits de la matrice identite

                if(tmpOctet[0]==-1){
                    for(i=0;i<4;++i){
                        tmpOctet[i]=tmpBinary[i]; //remplie les 4 premiere case du tableau
                    }
                }else{
                    for(i=4;i<8;++i){
                        tmpOctet[i]=tmpBinary[i-4];//remplie les 4 derniere case du tableau
                    }
                    //decalage de 00000001 de tmp pour match tmpOctet
                    for(i=7;i>=0;--i){
                        tmp=1;
                        if(tmpOctet[i]=='1'){
                            tmp <<= (7-i);
                            buffer |= tmp; //fait en sorte que buffer prend a chaque fois la nouvelle valeur de tmp
                        }
                    }
                    fputc(buffer, newFile);

                    buffer=0;
                    tmpOctet[0]=-1;
                }

            } while (feof(fp)==0);
        }
        fclose(newFile);
    } else {
        erreur("erreur d'acces au fichier\n") ;
        return 1;
    }
    fclose(fp);
    return 0;
}

void returnBinary(char octet,char tmpBinary[]){
    int i;
    int a = 0x100;
    char tmpHexa[10];
    long hexadecimal;

    sprintf(tmpHexa,"%2.2hhx",octet);//pour avoir l octer en hexa dans le tableau tmpHexa
    hexadecimal = strtol(tmpHexa,NULL,16);
    for(i=0;i<8;++i){
        a=a/2;
        if(hexadecimal & a){
            tmpBinary[i]='1';
        }else{
            tmpBinary[i]='0';
        }
    }
}

void fillXn(char Xn[][8],char array[][4],char matrix[]){
    int i,j;
    int a,b,c,d;
    int number[8]={0};

    int Midentity[4]={0};
    MatriceIdentiter(matrix, Midentity);

    for(i=0;i<4;++i){
        number[(Midentity[i])-1]=1;
    }
    for(i=0;i<4;++i){
        for(j=1;j<16;++j){
            Xn[j][(Midentity[i])-1]=array[j][i];
        }
    }
    for (i=0;i<8;++i) {
        if (number[i]==0) {
            for(j=1;j<16;++j){
                a = (array[j][0]-'0') * (matrix[i+5]-'0');
                b = (array[j][1]-'0') * (matrix[i+5+9]-'0');
                c = (array[j][2]-'0') * (matrix[i+5+18]-'0');
                d = (array[j][3]-'0') * (matrix[i+5+27]-'0');
                if(a+b == 2){
                    a = 0;
                }else{
                    a += b;
                }
                if(a+c == 2){
                    a = 0;
                }else{
                    a += c;
                }
                if(a+d == 2){
                    a = 0;
                }else{
                    a += d;
                }
                if(a==1){
                    Xn[j][i]='1';
                }else{
                    Xn[j][i]='0';
                }
            }
        }
    }
}


int encrypts(char matrix[],const char pathFile[]){
    int i,j,k;
    int counter=0;
    unsigned char tmp=1;
    char Xn[16][8]={0};
    char array[16][4]={ '0','0','0','0',//0
                        '0','0','0','1',//1
                        '0','0','1','0',//2
                        '0','1','0','0',//3
                        '1','0','0','0',//4
                        '0','0','1','1',//5
                        '0','1','1','0',//6
                        '1','1','0','0',//7
                        '0','1','0','1',//8
                        '1','0','1','0',//9
                        '1','0','0','1',//10
                        '0','1','1','1',//11
                        '1','1','1','0',//12
                        '1','1','0','1',//13
                        '1','0','1','1',//14
                        '1','1','1','1'};//15
    char octet;
    char tmpBinary[8];
    char binaryEncrypted[8];
    char binaryEncrypted2[8];
    char buffer=0;
    long sizeOfFile;
    unsigned long long finish=0;
    char filename[255];
    for(i=0;i<8;++i){
        Xn[0][i]='0';
    }
    fillXn(Xn, array,matrix);

    FILE *fp ;
    fp = fopen(pathFile, "rb");
    char * lastPosition = strrchr(pathFile, '\\');
    strcpy(filename, lastPosition+1);
    strcat(filename, "c");
    if (fp != NULL) {
        FILE *newFile;
        newFile = fopen(filename,"w+b");
        if (newFile == NULL) {
            erreur("\nErreur Sur la creation du fichier a chiffrer");
            return 1;
        }else{
            fseek(fp, 0, SEEK_END);
            sizeOfFile = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            do{
                octet = getc(fp);
                returnBinary(octet, tmpBinary);

                //Boucle pour chiffrer les 4 premier bits de l'octet
                for(j=0;j<16;++j){
                    counter=0;
                    for(k=0;k<4;++k) {
                        if(array[j][k] == tmpBinary[k]){
                            counter++;
                        }
                    }
                    if (counter == 4) {
                        for (k=0;k<8;++k) {
                            binaryEncrypted[k]=Xn[j][k];
                        }
                        j=16;
                    }
                }

                //Boucle pour chiffrer les 4 dernier bits de l'octet
                for(j=0;j<16;++j){
                    counter=0;
                    for(k=0;k<4;++k) {
                        if(array[j][k] == tmpBinary[k+4]){
                            counter++;
                        }
                    }
                    if (counter == 4) {
                        for (k=0;k<8;++k) {
                            binaryEncrypted2[k]=Xn[j][k];
                        }
                        j=16;
                    }
                }

                buffer=0;
                for(i=7;i>=0;--i){
                    tmp=1;
                    if(binaryEncrypted[i]=='1'){
                        tmp <<= (7-i);
                        buffer |= tmp; //fait en sorte que buffer prend a chaque fois la nouvelle valeur de tmp
                    }
                }
                fputc(buffer, newFile);

                buffer=0;
                for(i=7;i>=0;--i){
                    tmp=1;
                    if(binaryEncrypted2[i]=='1'){
                        tmp <<= (7-i);
                        buffer |= tmp; //fait en sorte que buffer prend a chaque fois la nouvelle valeur de tmp
                    }
                }
                fputc(buffer, newFile);
                finish++;
            } while (feof(fp)==0 && finish != sizeOfFile);

        }
        fclose(newFile);

    } else {
        erreur("erreur d'acces au fichier\n") ;
        return 1;
    }
    fclose(fp);
    return 0;
}

int main(int argc, char **argv) {

    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    display=al_create_display(800,400);

    char matrix[41];

    int tx, ty, screenx, screeny;

    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    screenx = al_get_display_width(display);
    screeny = al_get_display_height(display);

    arial = al_load_font("asset/arial.ttf", 25, 0);
    image = al_load_bitmap("asset/test.bmp");
    if(!image)
        erreur("background error");

    tx = al_get_bitmap_width(image);
    ty = al_get_bitmap_height(image);

    al_set_window_title( display, "CODEC");

    int end = 0;
    const char* pathFile="";
    const char* pathG4C="";
    //int counter=0;
    int displayFinish=0;
    int displayForG4C=0;
    int displayForFile=0;
    int booleanForG4C=0;
    int booleanForFile=0;
    while(!end){

        al_draw_scaled_bitmap(image, 0, 0, tx, ty, 0, 0, screenx, screeny, 0);

        al_draw_rectangle(screenx-75,25,screenx-25,50,ROUGE,2);
        al_draw_text(arial,ROUGE,screenx-50,25,ALLEGRO_ALIGN_CENTRE,"X");
        al_draw_text(arial,NOIR,screenx-(screenx/2),25,ALLEGRO_ALIGN_CENTRE,"CODEC");

        al_draw_rectangle(screenx-(screenx/2)-25,screeny-25,25,screeny-(screeny/2),NOIR,2);
        al_draw_text(arial,NOIR,screenx-((screenx/4)*3),(screeny-(screeny/4)-25),ALLEGRO_ALIGN_CENTRE,"MATRICE G4C");

        al_draw_rectangle(screenx-(screenx/2)-25,60,25,screeny-(screeny/2)-25,NOIR,2);
        al_draw_text(arial,NOIR,screenx-((screenx/4)*3),(screeny-((screeny/4)*3)),ALLEGRO_ALIGN_CENTRE,"CHIFFRER");
        al_draw_rectangle(screenx-25,screeny-25,screenx-(screenx/2)+25,screeny-(screeny/2),NOIR,2);
        al_draw_text(arial,NOIR,screenx-(screenx/4),(screeny-(screeny/4)-25),ALLEGRO_ALIGN_CENTRE,"FICHIER");

        al_draw_rectangle(screenx-25,60,screenx-(screenx/2)+25,screeny-(screeny/2)-25,NOIR,2);
        al_draw_text(arial,NOIR,screenx-(screenx/4),(screeny-((screeny/4)*3)),ALLEGRO_ALIGN_CENTRE,"DECHIFFRER");

        if(displayForG4C == 1){
            al_draw_text(arial,ROUGE,screenx-((screenx/4)*3),(screeny-(screeny/4)+10),ALLEGRO_ALIGN_CENTRE,"INVALIDE");
        }
        if(displayForG4C == 2){
            al_draw_text(arial,VERT,screenx-((screenx/4)*3),(screeny-(screeny/4)+10),ALLEGRO_ALIGN_CENTRE,"VALIDE");
        }
        if(displayForFile==1){
            al_draw_text(arial,VERT,screenx-(screenx/4),(screeny-(screeny/4)+10),ALLEGRO_ALIGN_CENTRE,"VALIDE");
        }
        if(displayForFile==2){
            al_draw_text(arial,ROUGE,screenx-(screenx/4),(screeny-(screeny/4)+10),ALLEGRO_ALIGN_CENTRE,"INVALIDE");
        }
        if(displayFinish == 1){
            al_draw_filled_rectangle(screenx,55,0,400,BLANC);
            al_draw_text(arial,NOIR,screenx-(screenx/2),200,ALLEGRO_ALIGN_CENTRE,"CHIFFRAGE TERMINÉ");
            al_draw_rectangle(screenx-(screenx/2)-100,screeny-(screeny/4)+25,screenx-(screenx/2)+100,screeny-(screeny/4)-25,NOIR,2);
            al_draw_text(arial,NOIR,screenx-(screenx/2),screeny-(screeny/4)-12,ALLEGRO_ALIGN_CENTRE,"RETOUR");
        }
        if (displayFinish == 2) {
            al_draw_filled_rectangle(screenx,55,0,400,BLANC);
            al_draw_text(arial,NOIR,screenx-(screenx/2),200,ALLEGRO_ALIGN_CENTRE,"DECHIFFRAGE TERMINÉ");
            al_draw_rectangle(screenx-(screenx/2)-100,screeny-(screeny/4)+25,screenx-(screenx/2)+100,screeny-(screeny/4)-25,NOIR,2);
            al_draw_text(arial,NOIR,screenx-(screenx/2),screeny-(screeny/4)-12,ALLEGRO_ALIGN_CENTRE,"RETOUR");
        }
        if (displayFinish == 3) {
            al_draw_filled_rectangle(screenx,55,0,400,BLANC);
            al_draw_text(arial,NOIR,screenx-(screenx/2),200,ALLEGRO_ALIGN_CENTRE,"FAILED");
            al_draw_rectangle(screenx-(screenx/2)-50,screeny-(screeny/2)+25,screenx-(screenx/2)+50,screeny-(screeny/2)-25,NOIR,2);
            al_draw_text(arial,NOIR,screenx-(screenx/2),screeny-(screeny/4)-12,ALLEGRO_ALIGN_CENTRE,"RETOUR");
        }
        al_flip_display();

        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
            if (event.mouse.x >= (screenx-75) && event.mouse.x < (screenx-25) &&
                event.mouse.y >= 25 && event.mouse.y < 50)
            {
                end= 1;
            }
            if(displayFinish >=1){
                if( event.mouse.x >= (screenx-(screenx/2)-100) && event.mouse.x < (screenx-(screenx/2)+100) &&
                event.mouse.y >= (screeny-(screeny/4)-25) && event.mouse.y < (screeny-(screeny/4)+25) ){
                    displayForFile = 0;
                    displayForG4C = 0;
                    displayFinish = 0;
                    //counter = 0;
                    booleanForFile=0;
                    booleanForG4C=0;
                    pathFile = "";
                    pathG4C = "";
                }
            }else{
                if (event.mouse.x >= 25 && event.mouse.x < (screenx-(screenx/2)-25) &&
                    event.mouse.y < (screeny-25) && event.mouse.y >= (screeny-(screeny/2)) )
                {
                    //CHOISIR SA MATRICE G4C
                    filechooser = al_create_native_file_dialog("/","Choose a G4C matrix","*.*",ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
                    al_show_native_file_dialog(display, filechooser);
                    if (strcmp(pathG4C, "")!=0) {
                        //counter--;
                        pathG4C = "";
                        booleanForG4C=0;
                    }
                    if(al_get_native_file_dialog_path(filechooser, 0)!=NULL){
                        pathG4C = al_get_native_file_dialog_path(filechooser, 0);

                         if(g4cVerification(pathG4C,matrix)==0){
                             displayForG4C=1;
                             booleanForG4C=0;
                         }else{
                             displayForG4C=2;
                             //counter++;
                             booleanForG4C=1;
                         }
                    }else{
                        displayForG4C=1;
                    }

                }
                else if (event.mouse.x >= 25 && event.mouse.x < (screenx-(screenx/2)-25) &&
                         event.mouse.y < (screeny-(screeny/2)-25) && event.mouse.y >= 25)
                {
                    //CHIFFRER
                    if(booleanForFile==1 &&booleanForG4C==1){
                        if(encrypts(matrix,pathFile)==0){
                            displayFinish=1;
                        }else{
                            displayFinish=3;
                        }
                        //counter=0;
                    }
                }else if(event.mouse.x >= (screenx-(screenx/2)+25) && event.mouse.x < (screenx-25) &&
                         event.mouse.y < (screeny-25) && event.mouse.y >= (screeny-(screeny/2)) ){
                    //CHOISIR SON FICHIER A CHIFFRER OU A DECHIFFRER
                    if (strcmp(pathFile, "")!=0) {
                        //counter--;
                        pathFile = "";
                        booleanForFile=0;
                    }
                    filechooser = al_create_native_file_dialog("/","Choose a file to encrypt or deciffer","*.*",ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
                    al_show_native_file_dialog(display, filechooser);
                    if(al_get_native_file_dialog_path(filechooser, 0)!=NULL){
                        pathFile = al_get_native_file_dialog_path(filechooser, 0);
                        if (strcmp(pathFile,"")!=0 ) {
                            displayForFile=1;
                            //counter++;
                            booleanForFile=1;
                        }
                    }else{
                        displayForFile = 2;
                    }
                }else if(event.mouse.x >= (screenx-(screenx/2)+25) && event.mouse.x < (screenx-25) &&
                         event.mouse.y < (screeny-(screeny/2)-25) && event.mouse.y >= 25){
                    //DECHIFFRER
                    if(booleanForFile==1 && booleanForG4C==1 && pathFile[strlen(pathFile)-1] == 'c'){
                        if(deciffer(matrix,pathFile)==0){
                            displayFinish=2;
                        }else{
                            displayFinish=3;
                        }
                        //counter=0;
                    }
                }
            }
        }
    }
    al_destroy_display(display);
    al_destroy_bitmap(image);
    al_destroy_event_queue(queue);
    al_destroy_native_file_dialog(filechooser);
    return 0;
}
