#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <avr/pgmspace.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#include <SPI.h>
#include <SD.h>
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = PA_3; //cs PIN
File myFile;
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
const int ard = 37;
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
unsigned int co = 40;
unsigned int sep = 30;
unsigned int chc[] = {co+(sep*0),co+(sep*1),co+(sep*2),co+(sep*3),co+(sep*4),co+(sep*5),co+(sep*6),co+(sep*7)};  
//const int chipSelect = 32; //cs PIN
int Menu = 0;
int Menu1 = 0;
int Menu2 = 0;
int DibujarMenu = 0;
int DibujarPista = 0;
int Play = 0;
int PL1 = 0;
int PL2 = 0;
int pos1 = 1;
int pos2 = 1;
int cont1S = 0;
int cont2S = 0;
int y = 0;
int Enemigo1 = 3;         // posicion de enemigos
int Enemigo2 = 3;
int x1;                 //Posicion de carro de jugador
int x2;
int Tiempo = 0;           //TIempo variable de movimiento de enemigos
int Carrox1 = 82;
int Carrox2 = 222;
int carro1 = 0;
int carro2 = 0;
int Dibujar = 0;

const int Start1 = PF_4;

const int Start2 = PF_0;
byte Jugador1;
byte Jugador2;
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void ReadImagenSD(void);
int ASCII_hex(int a);
uint8_t mapear[1000];

void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);
  /*****Micro SD******/
  SPI.setModule(0);
  Serial.print("\nInitializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(PA_3, OUTPUT);     // change this to 53 on a mega
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card is inserted?");
    Serial.println("* Is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  Serial.println("Archivos Existentes en MicroSD");
  root.ls(LS_R | LS_DATE | LS_SIZE);
  
  if (!SD.begin(PA_3)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");  
  /*********************************/

  pinMode(Start1,   INPUT_PULLUP);   
  pinMode(Start2,   INPUT_PULLUP);
  pinMode(ard, OUTPUT);
  Enemigo1 = random(3);                //Generacion random de posicion de enemigos
  Enemigo2 = random(3);
  carro1 = random(7);                //Generacion random de estilo de carro de enemigo
  carro2 = random(7);
  Play = 0;
  digitalWrite(ard, HIGH);           //Comando modo menu dirijido a arduino
  //Fondo de pantalla de inicio
  
    
  
  //LCD_Bitmap(0, 0, 320, 240, uvg);
  myFile = SD.open("fondo1.txt");
  ReadImagenSD();
  delay(1000);
  LCD_Print("Fast and Furious", 30, 100, 2, 0x0000, 0xffff);     //Impresion de nombre de juego
  LCD_Print("START para empezar", 85, 130, 1, 0x0000, 0xffff);
}

//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  Jugador1 = Serial3.read();
  Jugador2 = Serial2.read();
  if (digitalRead(Start1) == LOW && Play == 0){           //Al presionar start te manda a menu para escoger carros, menu 1
    Play = 1;
    DibujarMenu = 1;
    Menu = 1;
  }
  if (digitalRead(Start2) == LOW && Play == 0){
    Play = 1;
    DibujarMenu = 1;
    Menu = 1;
    ;
  }
//**************************************************************************************************
//Menu
//***************************************************************************************************
  if (Play == 1){
    if (DibujarMenu == 1){                     //Imprime menu de eleccion de carro    
      //Pantalla de Inicio
      //LCD_Clear(0x001f);                    //Fondo azul en menu
      myFile = SD.open("fondo2.txt");
      ReadImagenSD();
      LCD_Print("CARRERA", 100, 40, 2, 0xffff, 0x0000);
      DibujarMenu = 0;
      DibujarPista = 1;
      Menu = 1;
      Menu1 = 0;
      Menu2 = 0;
      }
    if (Menu == 1){  
      LCD_Sprite( 45, 120, 18, 25, jug1,8,0,0, 0);                    //Imprime estilo de carro jugador
      LCD_Sprite( 45+30, 120, 18, 25, jug1,8,1,0, 0);                 //Imprime estilo de carro jugador
      LCD_Sprite( 45+(30*2), 120, 18, 25, jug1,8,2,0, 0);             //Imprime estilo de carro jugador
      LCD_Sprite( 45+(30*3), 120, 18, 25, jug1,8,3,0, 0);             //Imprime estilo de carro jugador
      LCD_Sprite( 45+(30*4), 120, 18, 25, jug1,8,4,0, 0);             //Imprime estilo de carro jugador
      LCD_Sprite( 45+(30*5), 120, 18, 25, jug1,8,5,0, 0);             //Imprime estilo de carro jugado
      LCD_Sprite( 45+(30*6), 120, 18, 25, jug1,8,6,0, 0);             //Imprime estilo de carro jugador
      LCD_Sprite( 45+(30*7), 120, 18, 25, jug1,8,7,0, 0);             //Imprime estilo de carro jugador
      if (Menu1  == 0){
               LCD_Print("Elije un carro", 100, 80, 1, 0xffff, 0x0000);      //Imprime indicaciones a jugador uno 
               LCD_Print("Jugador 1", 120, 160, 1, 0xffff, 0x0000);
               if (Jugador1 == 1){                  //Antirebote boton 1R  
                // Al presionar 1R aumenta PL1 para cambiar de estilo de carro
                Rect(chc[PL1],115,25,35,0x001f);                   //Borra ultima posicion de rectangulo 
                PL1 = PL1 + 1;
                if (PL1 == 8){                                     //Si contador pasa el limite, se reinicia contador
                  PL1 = 0;
                }
                
              }
              if (Jugador1 == 2){                //Antirebote de boton
                //Movimineto de cuadro a izquierda para escoger carro
                Rect(chc[PL1],115,25,35,0x001f);                     //Borra ultima posicion de rectangulo 
                PL1 = PL1 - 1;
                if (PL1 < 0){
                  PL1 = 7;                                              //Si contador pasa el limite, se reinicia contador
                }
              } 
              
              if (digitalRead(Start1) == LOW){                     //Antirebote de start
                cont1S = 1;
              }
              Rect(chc[PL1],115,25,35,0xffff);
              if (digitalRead(Start1) == HIGH && cont1S ==1){                                      //Jugador 1 listo para jugar
                  Menu1 = 1;
                  cont1S = 0;
                  Rect(chc[PL1],115,25,35,0x001f);                                                 //    Borra rectangulo de eleccion de carro de jugador 1 
                }
                
      }
      if (Menu2 == 0 && Menu1 == 1){
                   LCD_Print("Escoje un carro", 100, 80, 1, 0xffff, 0x0000);            //Imprime instrucciones de jugador 2
                   LCD_Print("Jugador 2", 120, 160, 1, 0xffff, 0x0000);
                  if (Jugador2 == 1){                                                  //Antirebote
                    //Jugador escogiendo carro derecha
                    Rect(chc[PL2],115,25,35,0x001f);
                    PL2 = PL2 + 1;
                    if (PL2 == 8){                                                                     //Si contador pasa el limite, se reinicia contador
                      PL2 = 0;
                    }
                  }
                  if (Jugador2 == 2){                                                    //Antirebote
                    //Jugador escogiendo carro izquierda
                    Rect(chc[PL2],115,25,35,0x001f);
                    PL2 = PL2 - 1;
                    if (PL2 < 0){                                            //Si contador pasa el limite, se reinicia contador
                      PL2 = 7;
                    }
                  }
                  if (digitalRead(Start2) == LOW){                                                  //Antirebote
                    cont2S = 1;
                  }
                  if (digitalRead(Start2) == HIGH && cont2S ==1){          //Jugador 2 listo para jugar
                    Menu2 = 1;
                    cont2S = 0;
                    LCD_Print("Listo", 129, 180, 1, 0xffff, 0x0000);
                    }
                    Rect(chc[PL2],115,25,35,0xffff);
      } 
      if (Menu1 == Menu2 && Menu1 == 1){                                  //  Confirmar que estan listos presionando start confirmando menu 1 y 2 que estan listos
        Menu = 0; 
        //FillRect(50,80,280,10,0x0000);                                   //Fondo azul de ultimo menu 
        LCD_Print("Presiona start para Continuar", 30, 80, 1, 0xffff, 0x0000);
      }
    }
      if (Menu == 0 && Play == 1){
      if (digitalRead(Start1) == LOW){                     //Antirebote de start
        cont1S = 1;
      }
      if (digitalRead(Start1) == HIGH && cont1S == 1){     //Start jugador 1, dirijirse a estado de play2 que es generacion de carrera
          Play = 2;
          cont1S = 0;
        }
        if (digitalRead(Start2) == LOW){                     //Antirebote de start
          cont2S = 1;
        }
        if (digitalRead(Start2) == HIGH && cont2S == 1){    //Start jugador 2, dirijirse a estado de play2 que es generacion de carrera
          Play = 2;
          cont2S = 0;
        }
      }
  } 
  if (Play == 2){
    if (DibujarPista == 1){
      DibujarPista = 0;
      //digitalWrite(ard, LOW);                           //Comienza la musica 
//**************************************************************************************************
//Generacion de pista de carrera
//***************************************************************************************************
       //Color del fondo
      FillRect(0,0,320,240,0xBEAA);
      //Asfalto y grama 
      FillRect(45, 0, 230, 240, 0x0000);
      FillRect(132, 0, 41, 240, 0xBEAA);
      //Carrileras de block
      for (int i = 0; i < 19; i++) {
           LCD_Sprite(37,(13*i),8,13,block,1,0,1,0); // blocks Riel izquierdo carril 1
           LCD_Sprite(132,(13*i),8,13,block,1,0,0,0);// blocks Riel derecho carril 1
           LCD_Sprite(173,(13*i),8,13,block,1,0,1,0); // blocks Riel izquierdo carril 2
           LCD_Sprite(272,(13*i),8,13,block,1,0,0,0);// blocks Riel derecho carril 2
       }

      //Carriles
      //Lineas cortada amarilla para dividir 3 carriles de carretera 1
      FillRect(73,0,3,12,0xffe0);
      FillRect(73,24,3,12,0xffe0);
      FillRect(73,48,3,12,0xffe0);
      FillRect(73,72,3,12,0xffe0);
      FillRect(73,96,3,12,0xffe0);
      FillRect(73,120,3,12,0xffe0);
      FillRect(73,144,3,12,0xffe0);
      FillRect(73,168,3,12,0xffe0);
      FillRect(73,192,3,12,0xffe0);
      FillRect(73,216,3,12,0xffe0);
      FillRect(104,0,3,12,0xffe0);
      FillRect(104,24,3,12,0xffe0);
      FillRect(104,48,3,12,0xffe0);
      FillRect(104,72,3,12,0xffe0);
      FillRect(104,96,3,12,0xffe0);
      FillRect(104,120,3,12,0xffe0);
      FillRect(104,144,3,12,0xffe0);
      FillRect(104,168,3,12,0xffe0);
      FillRect(104,192,3,12,0xffe0);
      FillRect(104,216,3,12,0xffe0);
      
  
      //Carriles
      //Lineas cortada amarilla para dividir 3 carriles de carretera 1
      FillRect(213,0,3,12,0xffe0);
      FillRect(213,24,3,12,0xffe0);
      FillRect(213,48,3,12,0xffe0);
      FillRect(213,72,3,12,0xffe0);
      FillRect(213,96,3,12,0xffe0);
      FillRect(213,120,3,12,0xffe0);
      FillRect(213,144,3,12,0xffe0);
      FillRect(213,168,3,12,0xffe0);
      FillRect(213,192,3,12,0xffe0);
      FillRect(213,216,3,12,0xffe0);
      FillRect(244,0,3,12,0xffe0);
      FillRect(244,24,3,12,0xffe0);
      FillRect(244,48,3,12,0xffe0);
      FillRect(244,72,3,12,0xffe0);
      FillRect(244,96,3,12,0xffe0);
      FillRect(244,120,3,12,0xffe0);
      FillRect(244,144,3,12,0xffe0);
      FillRect(244,168,3,12,0xffe0);
      FillRect(244,192,3,12,0xffe0);
      FillRect(244,216,3,12,0xffe0);
      

      y = 0;          //Condicion inicial de posicion vertical de enemigo 
      }
    y = y +1 + (Tiempo/4);    //Aumento de posicion vertical de enemigo, proporcional al tiempo(velocidad de enemigo)
  if (y > 240){
    Tiempo = Tiempo +1;        //Aumento de velocidad de enemigo
    Enemigo1 = random(3);    //Generacion de posicion horizontal aleatoria de enemigo
    Enemigo2 = random(3);
    carro1 = random(7);    //Eleccion aleatoria de apariencia de carro enemigo
    carro2 = random(7);
    y = 0;                 //Nuevo enemigo comenzando desde arriva posicion y=0
  }

  //Cordenada en x de enemigos segun posicion escogida aleatoriamente
  if (Enemigo1 == 0){
    x1 = 51;
  }
  if (Enemigo1 == 1){
    x1 = 82;
  }
  if (Enemigo1 == 2){
    x1 = 113;
  }
  if (Enemigo2 == 0){
    x2 = 191;
  }
  if (Enemigo2 == 1){
    x2 = 222;
  }
  if (Enemigo2 == 2){
    x2 = 253;
  }
//****************************************************************************************************
//Jugador 1
//****************************************************************************************************
  if (Jugador1 == 1){                                  //Antirebote
      //Go right Jugador 1 
      pos1 = pos1 + 1;
      if (pos1 == 1){
        FillRect(51, 200, 18, 26, 0x00);                             //Borra posicion anterior 
      }
      if (pos1 >= 2){
        FillRect(82, 200, 18, 26, 0x00);                            //Borra posicion anterior 
      }
      if (pos1>2){                                                  //Limite de contador de variable
        pos1 = 2;
      }
    }
    if (Jugador1 == 2){                                //Antirebote
      //GO left 
      pos1 = pos1 - 1;
      if (pos1<0){
        pos1 = 0;
      }
      FillRect(51, 200, 18, 26, 0x00);
      FillRect(82, 200, 18, 26, 0x00);                            //Borra 3 posiciones antes de imprimir nueva posicion
      FillRect(113, 200, 18, 26, 0x00);
    }
    //Cordenada en x segun carril actual
  if (pos1 == 0){                                               //Mapeo carril a cordenada en x para imprimir carro
    Carrox1 = 51;
   }
   if (pos1 == 1){
    Carrox1 = 82;
   }
   if (pos1 == 2){ 
    Carrox1 = 113;
   }
   LCD_Sprite(Carrox1, 200, 18, 25, jug1,8,PL1,0, 0);                 //Imprime carro jugador 1 
//****************************************************************************************************
//Jugador 2
//****************************************************************************************************
  if (Jugador2 == 1){
      pos2 = pos2 + 1;
      if (pos2 == 1){
        FillRect(191, 200, 18, 26, 0x00);
      }
      if (pos2 == 2){
        FillRect(222, 200, 18, 26, 0x00);
      }
      if (pos2>2){
        pos2 = 2;
      }
    }
    if (Jugador2 == 2){
      pos2 = pos2 - 1;
      if (pos2 == 1){
        FillRect(253, 200, 18, 26, 0x00);
      }
      if (pos2 <= 0){
        FillRect(222, 200, 18, 26, 0x00);
      }
      if (pos2<0){
        pos2 = 0;
      }
    }

    
  if (pos2 == 0){
    Carrox2 = 191;
   }
   if (pos2 == 1){
    Carrox2 = 222;
   }
   if (pos2 == 2){
    Carrox2 = 253;
   }
   LCD_Sprite(Carrox2, 200, 18, 25, jug1,8, PL2,0, 0);
//****************************************************************************************************
//Enemigos
//****************************************************************************************************
    //Dependiendo la velocidad de movimiento de carros enemigos, se tiene que borrar el rastro de su paso
    //Entre mas rapido van mas lineas de borrado de rastro hay que crear
    if ((Tiempo/4) == 0){                                                              //velocidad lentra
      LCD_Sprite(x1, y, 18, 26, cars,8,carro1,0, 0);                                 //Impresion de carro enemigo segun posicion random obtenida y estilo de carro random obtenida
      H_line( x1, y-1, 18, 0x00);                                                    //Se borra una linea despues de carro
      LCD_Sprite(x2, y, 18, 26, cars,8,carro2,0, 0);                                 //Impresion de carro enemigo segun posicion random obtenida y estilo de carro random obtenida
      H_line( x2, y-1, 18, 0x00);
    }
    if ((Tiempo/4) == 1){                                                             //mas velocidad 
      LCD_Sprite(x1, y, 18, 26, cars,8,carro1,0, 0);                                //Impresion de carro enemigo segun posicion random obtenida y estilo de carro random obtenida
      H_line( x1, y-1, 18, 0x00);                                                   //por mas velocidad se borran dos lineas de rastro de carro
      H_line( x1, y-2, 18, 0x00);  
      LCD_Sprite(x2, y, 18, 26, cars,8,carro2,0, 0);                                //Impresion de carro enemigo segun posicion random obtenida y estilo de carro random obtenida
      H_line( x2, y-1, 18, 0x00);                                                   //por mas velocidad se borran dos lineas de rastro de carro
      H_line( x2, y-2, 18, 0x00);
    }
    if ((Tiempo/4) == 2){                                                              
      LCD_Sprite(x1, y, 18, 26, cars,8,carro1,0, 0);
      H_line( x1, y-1, 18, 0x00);
      H_line( x1, y-2, 18, 0x00);
      H_line( x1, y-3, 18, 0x00);     
      LCD_Sprite(x2, y, 18, 26, cars,8,carro2,0, 0);
      H_line( x2, y-1, 18, 0x00);
      H_line( x2, y-2, 18, 0x00);
      H_line( x2, y-3, 18, 0x00);
    }
    if ((Tiempo/4) == 3){
      LCD_Sprite(x1, y, 18, 26, cars,8,carro1,0, 0);
      H_line( x1, y-1, 18, 0x00);
      H_line( x1, y-2, 18, 0x00);
      H_line( x1, y-3, 18, 0x00);
      H_line( x1, y-4, 18, 0x00);
      LCD_Sprite(x2, y, 18, 26, cars,8,carro2,0, 0);
      H_line( x2, y-1, 18, 0x00);
      H_line( x2, y-2, 18, 0x00);
      H_line( x2, y-3, 18, 0x00);
      H_line( x2, y-4, 18, 0x00);
    }
    if ((Tiempo/4) == 3){
      LCD_Sprite(x1, y, 18, 26, cars,8,carro1,0, 0);
      H_line( x1, y-1, 18, 0x00);
      H_line( x1, y-2, 18, 0x00);
      H_line( x1, y-3, 18, 0x00);
      H_line( x1, y-4, 18, 0x00);
      H_line( x1, y-5, 18, 0x00);
      LCD_Sprite(x2, y, 18, 26, cars,8,carro2,0, 0);
      H_line( x2, y-1, 18, 0x00);
      H_line( x2, y-2, 18, 0x00);
      H_line( x2, y-3, 18, 0x00);
      H_line( x2, y-4, 18, 0x00);
      H_line( x2, y-5, 18, 0x00);
    }
    if ((Tiempo/4) == 4){
      LCD_Sprite(x1, y, 18, 26, cars,8,carro1,0, 0);
      H_line( x1, y-1, 18, 0x00);
      H_line( x1, y-2, 18, 0x00);
      H_line( x1, y-3, 18, 0x00);
      H_line( x1, y-4, 18, 0x00);
      H_line( x1, y-5, 18, 0x00);
      H_line( x1, y-6, 18, 0x00);
      LCD_Sprite(x2, y, 18, 26, cars,8,carro2,0, 0);
      H_line( x2, y-1, 18, 0x00);
      H_line( x2, y-2, 18, 0x00);
      H_line( x2, y-3, 18, 0x00);
      H_line( x2, y-4, 18, 0x00);
      H_line( x2, y-5, 18, 0x00);
      H_line( x2, y-6, 18, 0x00);
    }
    if ((Tiempo/4) > 4){
      LCD_Sprite(x1, y, 18, 26, cars,8,carro1,0, 0);
      H_line( x1, y-1, 18, 0x00);
      H_line( x1, y-2, 18, 0x00);
      H_line( x1, y-3, 18, 0x00);
      H_line( x1, y-4, 18, 0x00);
      H_line( x1, y-5, 18, 0x00);
      H_line( x1, y-6, 18, 0x00);
      H_line( x1, y-7, 18, 0x00);
      LCD_Sprite(x2, y, 18, 26, cars,8,carro2,0, 0);
      H_line( x2, y-1, 18, 0x00);
      H_line( x2, y-2, 18, 0x00);
      H_line( x2, y-3, 18, 0x00);
      H_line( x2, y-4, 18, 0x00);
      H_line( x2, y-5, 18, 0x00);
      H_line( x2, y-6, 18, 0x00);
      H_line( x2, y-7, 18, 0x00);
    }
//***************************************************************************************************
//Ganador
//***************************************************************************************************
  if (y < 227 && y > 173){
        if (Carrox2 == x2 && Carrox1 == x1){                                             //Condicion de empate, si los dos chocan al mismo tiempo
      LCD_Sprite(x2, 200, 18, 26, jug1,8, 7,0, 0);
      LCD_Sprite(x1, 200, 18, 26, jug1,8, 7,0, 0);
      
      myFile = SD.open("Choque.txt");
      ReadImagenSD();
      LCD_Print("Empate", 120, 100, 2, 0xffff, 0x0000);
      LCD_Print("Presiona start", 50, 130, 2, 0xffff, 0x0000);  
      Play = 4;
      Dibujar = 1;
      digitalWrite(ard, HIGH);
    }
    if (Carrox1 == x1 && Carrox2 != x1 && Dibujar == 0){                                   //Condicion, si no hay empate y carro 1 pierde. Carro  conincide en x con enemigo
      LCD_Sprite(x1, 200, 18, 26, cars,8, 7,0, 0);
      myFile = SD.open("Choque.txt");
      ReadImagenSD();    
      LCD_Print("Jugador 2 gana", 50, 100, 2, 0xffff, 0x0000);                      //Carro 2 gana
      LCD_Print("Presiona start", 50, 130, 2, 0xffff, 0x0000);
      Play = 4;
      digitalWrite(ard, HIGH);                                                     //Termina musica
    }
    if (Carrox2 == x2 && Carrox1 != x1 && Dibujar == 0){                                  //Si no hay empate y carro 2 pierde. Carro 2 coincide con enemigo en cordenada x
      LCD_Sprite(x2, 200, 18, 26, jug1,8, 7,0, 0);
      myFile = SD.open("Choque.txt");
      ReadImagenSD();  
      LCD_Print("Jugador 1 Gana", 50, 100, 2, 0xffff, 0x0000);
      LCD_Print("Presione start", 50, 130, 2, 0xffff, 0x0000);  
      Play = 4;
      digitalWrite(ard, HIGH);                                                     //Termina musica
    }

  }
 }
  if ((digitalRead(Start1) == LOW && Play == 4) || (digitalRead(Start2) == LOW && Play == 4)){   //Al enfrentar derrota oprime start para volver a escoger carros
      Play = 1;
      Tiempo = 0;
      DibujarMenu = 1;
      Dibujar = 0;
  }
//*******************************************************************************************************
//END
//******************************************************************************************************* 
}
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  //Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    //Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2 =   y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}
void ReadImagenSD(void)
{
  //myFile = SD.open("Zebra.txt");
  int hex1 = 0;
  int val1 = 0; 
  int val2 = 0;
  int mapar = 0;
  int vert = 0;
  if(myFile)
  {
    Serial.println("Leyendo el archivo...");
      while(myFile.available())
      {
        mapar=0;
        while(mapar<640)
        {
          hex1 = myFile.read();
          if(hex1 == 120)
          {
            val1 = myFile.read();
            val2 = myFile.read();
            val1 = ASCII_Hex(val1);
            val2 = ASCII_Hex(val2);
            mapear[mapar] = val1*16+val2;
            mapar++;
          }
        }
        LCD_Bitmap(0, vert, 320, 1, mapear);
        vert++;
      }
      myFile.close();
  }
  else{
    Serial.println("No se pudo leer el archivo");
    myFile.close();
  }
}
int ASCII_Hex(int a)
{
  switch(a){
    case 48:
      return 0;
    case 49:
      return 1;
    case 50:
      return 2;
    case 51:
      return 3;
    case 52:
      return 4;
    case 53:
      return 5;
    case 54:
      return 6;
    case 55:
      return 7;
    case 56:
      return 8;
    case 57:
      return 9;
    case 97:
      return 10;
    case 98:
      return 11;
    case 99:
      return 12;
    case 100:
      return 13;
    case 101:
      return 14;
    case 102:
      return 15;
  }
}
