#include <Arduino.h>
#include <main.h>
#include <md_spiffs.h>

//Aussehen
  #define BACKGROUND ILI9341_GREENYELLOW //Farbe des Rahmens
  #define TOPMARGIN 20                   //Rand oben
  #define LEFTMARGIN 12                  //Rand links und rechts
  #define xposUMNS 12                     //Anzahl der Spalten
  #define yposS 16                        //Anzahl der Zeilen
  #define BLOCKSIZE 18                   //Groesse eines Blocks in Pixel
  #define NOPIECE ILI9341_BLACK          //Farb fuer das leere Spielfeld
  #define ALLON ILI9341_DARKGREY         //Farbe fuer alle Bloecke ein
  #define BORDER ILI9341_WHITE           //Farbe fuer den Blockrand

//Unterschiedliche Pin-Belegung fuer ESP32 und D1Mini
#ifdef ESP32
    #define TFT_CS   5
    #define TFT_DC   4
    #define TFT_RST  22
    #define TFT_LED  15
    #define TOUCH_CS 14
    #define LED_ON   0
  #endif

//Instanzen der Bibliotheken
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//XPT2046_Touchscreen touch(TOUCH_CS);
md_touch touch(TOUCH_CS);
md_TouchEvent tevent(touch);
md_spiffs  conf = md_spiffs();
md_spiffs *pConf = &conf;

//Farben fuer die Bloecke
/*
  const uint16_t xposorBlock[8] =
  {
    ILI9341_BLACK, ILI9341_YELLOW, ILI9341_RED,
    ILI9341_CYAN,  ILI9341_GREEN,  ILI9341_PURPLE,
    ILI9341_BLUE,  ILI9341_ORANGE
  };
*/
/* Bitmuster fuer die Teile
   0 = Block nicht gesetzt >0 Index der Farbe f�r den Block
  */
/*
  const uint8_t piece[20][16] =
    {
      { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 0, 0,  1, 1, 0, 0 },
      { 0, 2, 0, 0,  0, 2, 0, 0,  0, 2, 0, 0,  0, 2, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  2, 2, 2, 2 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  3, 3, 0, 0,  0, 3, 3, 0 },
      { 0, 0, 0, 0,  0, 3, 0, 0,  3, 3, 0, 0,  3, 0, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  0, 4, 4, 0,  4, 4, 0, 0 },
      { 0, 0, 0, 0,  4, 0, 0, 0,  4, 4, 0, 0,  0, 4, 0, 0 },
      { 0, 0, 0, 0,  5, 0, 0, 0,  5, 0, 0, 0,  5, 5, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 5, 0,  5, 5, 5, 0 },
      { 0, 0, 0, 0,  5, 5, 0, 0,  0, 5, 0, 0,  0, 5, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  5, 5, 5, 0,  5, 0, 0, 0 },
      { 0, 0, 0, 0,  0, 6, 0, 0,  0, 6, 0, 0,  6, 6, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  6, 6, 6, 0,  0, 0, 6, 0 },
      { 0, 0, 0, 0,  6, 6, 0, 0,  6, 0, 0, 0,  6, 0, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  6, 0, 0, 0,  6, 6, 6, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  0, 7, 0, 0,  7, 7, 7, 0 },
      { 0, 0, 0, 0,  0, 7, 0, 0,  7, 7, 0, 0,  0, 7, 0, 0 },
      { 0, 0, 0, 0,  0, 0, 0, 0,  7, 7, 7, 0,  0, 7, 0, 0 },
      { 0, 0, 0, 0,  0, 7, 0, 0,  0, 7, 7, 0,  0, 7, 0, 0 }
    };
*/
/* Speicherplatz fuer das Spielfeld
   0 bedeutet Block frei >0 Index der Farbe des belegten Blocks
  */
uint8_t playGround[yposS][xposUMNS];

//Globale variablen
uint8_t curPiece;  //aktuelles Tetris Teil
int8_t curxpos;     //aktuelle Spalte
int8_t curypos;     //aktuelle Zeile
uint32_t score;    //aktueller Score
uint8_t level;     //aktueller Level
uint16_t interval; //aktuelles Zeitintervall f�r die Abw�rtsbewegung
uint32_t last;     //letzter Zeitstempel

int16_t  xpos = 1;
int16_t  ypos = 10;

uint8_t  textSize = 1;
TS_Point p;
TS_Point *pP = &p;
boolean  tch;
int16_t  wx, wy;
uint16_t ww, wh;
char     text[30];
uint8_t  rot;
int16_t  xmax, ymax;

uint16_t cxmin = 0;
uint16_t cxmax = 0;
uint16_t cymin = 0;
uint16_t cymax = 0;


void setLimits(uint8_t inRot = rot)
  {
    uint8_t i = inRot % 4;
    switch (i)
      {
        case 0: xmax = 240; ymax = 320; break;
        case 1: xmax = 320; ymax = 240; break;
        case 2: xmax = 240; ymax = 320; break;
        case 3:
        default: xmax = 320; ymax = 240; break;
      }
  }

// ----------------------------------------
void setup()
  {
    Serial.begin(115200);
    Serial.println("Start ..");
    //Hintergrundbeleuchtung einschalten
    pinMode(TFT_LED,OUTPUT);
    digitalWrite(TFT_LED, LED_ON);
    //Display initialisieren
    Serial.println(" .. tft");
    tft.begin();
    tft.setRotation(TOUCH_ROTATION);
    tft.fillScreen(BACKGROUND);
    //Touchscreen vorbereiten
    Serial.println(" .. touch");
    touch.begin();
    touch.setRotation(TOUCH_ROTATION);
    tevent.setResolution(tft.width(),tft.height());
    tevent.setDrawMode(false);
    //Callback Funktion registrieren
    tevent.registerOnTouchClick(onClick);
    //tft.fillRect(LEFTMARGIN,TOPMARGIN,xposUMNS*BLOCKSIZE,yposS*BLOCKSIZE,NOPIECE);
    Serial.println(" .. conf-flash");
    conf.init(pConf);
    Serial.println(" .. read calib");
    if (conf.exists("/conf.dat"))
      {
        //FILE *f = fopen("/conf.dat","r");
        //fscanf(f,"%d,%d,%d,%d", cxmin, cymin, cxmax, cymax);
        //fclosescanf(f,"%d", cxmin);
        char buf[50];
        uint8_t res = conf.readFile("/conf.dat", 40, buf);
        if (res == ESP_OK)
          {
            Serial.print(" .. Config found ");
            Serial.println(buf);
            int a,b,c,d;
            sscanf(buf, "%i %i %i %i", &a, &b, &c, &d);
            cxmin = (uint16_t) a;
            cymin = (uint16_t) b;
            cxmax = (uint16_t) c;
            cymax = (uint16_t) d;
          }
        Serial.print("cxmin ");
        Serial.print(cxmin); Serial.print(",");
        Serial.print(cymin); Serial.print(",");
        Serial.print(cxmax); Serial.print(",");
        Serial.println(cymax);
        tevent.calibrate(cxmin, cymin, cxmax, cymax);
      }

    //clearBoard();
    //newPiece();
    //Startzeit merken und Spielfeld loeschen
    last = millis();
    score=0;
    tft.fillScreen(ILI9341_BLACK);
    Serial.println(" .. ready");
  }

TS_Point  calP[4];
TS_Point  pRaw;
TS_Point *ppRaw = &pRaw;
TS_Point  calRaw[4];
bool      doCal     = true;
int16_t   calWin[4] = {110, 160, 70, 40};
// ----------------------------------------
// Hauptschleife

void loop()
  {
    //tft.fillScreen(ILI9341_BLACK);
    //tft.fillRect(0,0,240,20,BACKGROUND);
    //tft.fillRect(2,2,3,3,ILI9341_RED);
    tft.setTextSize(textSize);
    for (uint8_t i = 0 ; i < 4 ; i++)
      {
        setLimits(i);
        tft.setRotation(i);
        xpos = 10 + 10; ypos = 10;
        tft.drawRoundRect(10,      10, 3, 3, 1, ILI9341_YELLOW);
        sprintf(text,"r %1i", i);
        tft.setCursor(xpos,ypos);
        tft.getTextBounds(&text[0], xpos, ypos, &wx, &wy, &ww, &wh);
        tft.fillRect(wx, wy, ww, wh, ILI9341_BLACK);
        tft.print(text);
      }
    delay(3000);

    Serial.println(calWin[0]);
    Serial.println(calWin[1]);
    Serial.println(calWin[2]);
    Serial.println(calWin[3]);
    if (doCal == true)
      {
        Serial.println("draw Calib window");
        //tft.fillRoundRect(110,260,60,40,2,ILI9341_WHITE);
        tft.fillRoundRect(calWin[0], calWin[1], calWin[2], calWin[3], 2, ILI9341_GREEN);
        tft.drawRoundRect(calWin[0], calWin[1], calWin[2], calWin[3], 2, ILI9341_RED);
        tft.setCursor(calWin[0] + 2, calWin[1] + 10);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_BLUE);
        tft.print("Calib");
      }
    //get position and check if touched
    while (1)
      {
        tch = tevent.getTouchPos(pP, ppRaw);
        if (tch)
          {
            sprintf(text,"x %3i y %3i rx %5i ry %5i", p.x, p.y, pRaw.x, pRaw.y);
            xpos = 30;
            if ((pRaw.x < 1000) && (pRaw.y < 1000))
              {
                ypos = 30;
                if (doCal == true) calP[0] = pRaw;
              }
            else if ((pRaw.x < 1000) && (pRaw.y > 3000))
              {
                ypos = 90;
                if (doCal == true) calP[2] = pRaw;
              }
            else if ((pRaw.x > 3000) && (pRaw.y > 3000))
              {
                ypos = 120;
                if (doCal == true) calP[3] = pRaw;
              }
            else if ((pRaw.x > 3000) && (pRaw.y < 1000))
              {
                ypos = 60;
                if (doCal == true) calP[1] = pRaw;
              }
            else
              {
                ypos = 150;
                if (doCal == true)
                  {
                    if (   (p.x > calWin[0]) && (p.x < calWin[0] + calWin[2])
                        && (p.y > calWin[1]) && (p.y < calWin[1] + calWin[3])
                       )
                        cxmin = (calP[0].x + calP[2].x)/2;
                      {
                        cxmax = (calP[1].x + calP[3].x)/2;
                        cymin = (calP[0].y + calP[1].y)/2;
                        cymax = (calP[2].y + calP[3].y)/2;
                        int16_t dRawX = cxmax - cxmin;
                        int16_t dRawY = cymax - cymin;
                        cxmin -= dRawX/30;
                        cxmax += dRawX/30;
                        cymin -= dRawY/22;
                        cymax += dRawY/22;
                        tevent.calibrate(cxmin, cymin, cxmax, cymax);
                        Serial.print("Calib done");
                        doCal = false;
                        tft.fillRoundRect(calWin[0], calWin[1], calWin[2], calWin[3], 2, ILI9341_BLACK);

                      }
                  }
              }
            tft.setCursor(xpos,ypos);
            tft.setTextSize(1);
            tft.getTextBounds(&text[0], xpos, ypos, &wx, &wy, &ww, &wh);
            tft.fillRect(wx, wy, ww, wh, ILI9341_BLACK);
            tft.print(text);
            delay(500);
          }
        delay(500);

      }


    //tft.setTextxposor(ILI9341_GREEN);
    #ifdef XXXX
          tft.setCursor(myxpos, myypos);
          char buf[50];
          sprintf(buf,"xpos: %2i ypos: %2i",myxpos,myypos);
          tft.print(buf);
          delay (1000);
          myypos += 20;
          if (++myTextSize > 6)
            {
              myTextSize = 1;
              myypos += 20;
              tft.fillScreen(ILI9341_BLACK);
            }
      //Auf Touch Ereignisse pruefen
      tevent.pollTouchScreen();
      /* Immer wenn die Zeit intterval erreicht ist, wird das aktuelle Tetris-Teil
         um eine Zeile nach unten geschoben falls das moeglich ist.
         Kommt es dabei zu einer Kollision oder wird der untere Rand erreicht,
         so wird das Teil nicht verschoben sondern am Spielfeld verankert.
         Vollstaendige Zeilen werden entfernt und ein neues Teil am oberen
         Rand eingefuegt
       */
      if ((curPiece > 0) && ((millis()-last) > interval))
        {
          last = millis();
          if (checkPiece(curPiece,curxpos,curypos+1))
            {
              hidePiece(curPiece,curxpos,curypos);
              curypos++;
              showPiece(curPiece,curxpos,curypos);
            }
            else
            {
              putPiece(curPiece,curxpos,curypos);
              removeComplete();
              newPiece();
            }
        }

      #endif
  }

/* Funktion zeigt in der Kopfleiste den aktuellen Score und den Level an
   Abhaengig vom Score wird der Level hinaufgesetzt und das Intervall verringert
  */
void displayScore()
  {
    if      (score < 10)     {level = 1; interval = 900;}
    else if (score < 100)    {level = 2; interval = 700;}
    else if (score < 1000)   {level = 3; interval = 500;}
    else if (score < 10000)  {level = 4; interval = 300;}
    else if (score < 100000) {level = 5; interval = 100;}
    tft.fillRect(0,0,240,20,BACKGROUND);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(5,4);
    char buf[50];
    sprintf(buf,"SC: %8i LV: %i",score,level);
    tft.print(buf);
  }

/*  Funktion um ein Tetris-Teil zu drehen. Der Parameter ist die Nummer des Teils
   das gedreht werden soll. R�ckgabewert ist der Index des gedrehten Teils
  */
uint8_t rotate(uint8_t pc)
  {
    uint8_t res = 0;
    switch (pc) {
      case 1:  res = 1; break;
      case 2:  res = 3; break;
      case 3:  res = 2; break;
      case 4:  res = 5; break;
      case 5:  res = 4; break;
      case 6:  res = 7; break;
      case 7:  res = 6; break;
      case 8:  res = 9; break;
      case 9:  res = 10; break;
      case 10: res = 11; break;
      case 11: res = 8; break;
      case 12: res = 13; break;
      case 13: res = 14; break;
      case 14: res = 15; break;
      case 15: res = 12; break;
      case 16: res = 17; break;
      case 17: res = 18; break;
      case 18: res = 19; break;
      case 19: res = 16; break;
    }
    return res;
  }

//Funktion testet ob eine Zeile voll belegt ist
boolean yposComplete(int8_t rpg)
  {
    if ((rpg >= 0) && (rpg < yposS))
      {
        boolean res = true;
        uint8_t c = 0;
        //wenn ein Block nicht belegt ist (Farbe 0), ist die Zeile nicht vollstaendig
        while (res && (c < xposUMNS))
          {
            if (playGround[rpg][c] == 0) res = false;
            c++;
          }
        return res;
      }
  }

/* Funkzion prueft ob es zwischen der Zeile rpc des Tetris-Teils pc und
   der Zeile rpg des Spielfelds ab der Position cpg Kolklisionen gibt.
   Wenn eine Kollision auftritt oder die letzte Zeile des Spielfelds
   erreicht wurde wird falsch zurueckgegeben
  */
boolean checkypos(uint8_t pc, int8_t rpc, int8_t cpg, int8_t rpg)
  {
    boolean res = true;
    if (rpg >= yposS) return false;
    if (rpg < 0) return true;
    for (uint8_t i = 0; i<4; i++)
      {
        if (piece[pc][rpc*4 + i]>0)
          {
            if (((cpg+i) < 0) || ((cpg+i) >= xposUMNS))
              {
                res = false;
              }
              else
              {
                if (playGround[rpg][cpg+i] > 0) res = false;
              }
          }
      }
    return res;
  }

/* Funktion prueft ob das Tetris Teil pc am Spielfeld an der Position
   Zeile rpg Spalte cpg (linke untere Ecke des Teils) Kollisionen auftreten
  */
boolean checkPiece(uint8_t pc, int8_t cpg, int8_t rpg)
  {
    boolean res = true;
    uint8_t rpc = 0;
    while (res && (rpc < 4))
      {
        res = checkypos(pc,rpc,cpg,rpc+rpg-3);
        //Serial.printf("check %i = %i\n",rpc+rpg-3,res);
        rpc++;
      }
    return res;
  }

/* Funktion zeigt einen Block des Spielfeldes in Zeile y Spalte x mit der Farbe xposor an
   xposor ist die Farbe im 565 Format fuer das Display */
void showBlock(uint8_t x, uint8_t y, uint16_t xposor)
  {
    tft.fillRect(LEFTMARGIN+x*BLOCKSIZE+2,TOPMARGIN+y*BLOCKSIZE+2,BLOCKSIZE-4,BLOCKSIZE-4,xposor);
    tft.drawRect(LEFTMARGIN+x*BLOCKSIZE+1,TOPMARGIN+y*BLOCKSIZE+1,BLOCKSIZE-2,BLOCKSIZE-2,BORDER);
  }

// Funktion faellt einen Block des Spielfeldes in Zeile y Spalte x mit der Hintergrundfarbe
void hideBlock(uint8_t x, uint8_t y)
  {
    tft.fillRect(LEFTMARGIN+x*BLOCKSIZE,TOPMARGIN+y*BLOCKSIZE,BLOCKSIZE,BLOCKSIZE,NOPIECE);
  }

/* Funktion zeigt das Tetris-Teil pc in Zeile rpg, Spalte cpg (Linke untere Ecke) an
   Die Farbe wird der Definition des Tetris-Teils entnommen
  */
void showPiece(uint8_t pc, uint8_t cpg, uint8_t rpg)
  {
    uint8_t xposor;
    for (uint8_t r = 0; r<4; r++)
      {
        for (uint8_t c = 0; c<4; c++)
          {
            xposor = piece[pc][r*4+c];
            if ((xposor > 0) && ((3-r+rpg) >= 0))
              showBlock(cpg+c,rpg-3+r,colorBlock[xposor]);
      }
    }
  }

/* Funktion faellt die belegten Bloecke des Tetris-Teil pc in Zeile rpg,
   Spalte cpg (Linke untere Ecke) an mit Hintergrundfarbe
 */
void hidePiece(uint8_t pc, int8_t cpg, int8_t rpg)
  {
    uint8_t xposor;
    for (uint8_t r = 0; r<4; r++)
      {
        for (uint8_t c = 0; c<4; c++)
          {
            xposor = piece[pc][r*4+c];
            if ((xposor > 0) && ((3-r+rpg) >= 0)) hideBlock(cpg+c,rpg-3+r);
          }
      }
  }

/*funktion faellt die Zeile ypos des Spielfelds mit Hintergrundfarbe und
  loescht alle Eintraege fuer diese Zeile im Spielfeld-Speicher
  */
void deleteypos(int8_t ypos)
  {
    tft.fillRect(LEFTMARGIN,TOPMARGIN+ypos*BLOCKSIZE,xposUMNS * BLOCKSIZE,BLOCKSIZE,NOPIECE);
    for (uint8_t i =0; i<xposUMNS; i++) playGround[ypos][i]=0;
  }

/* Funktion kopiert die Zeile srcypos in die Zeile dstypos
   Die Anzeige der Zielzeile wird vorher geloescht. Beim
   kopieren wird die Quellzeile in der Zielzeile angezeigt
  */
void copyypos(int8_t srcypos, int8_t dstypos)
  {
    uint8_t xpos;
    deleteypos(dstypos);
    if ((srcypos < dstypos) && (srcypos >=0) && (dstypos < yposS)) {
      for (uint8_t c = 0; c < xposUMNS; c++) {
        xpos = playGround[srcypos][c];
        playGround[dstypos][c] = xpos;
        if (xpos > 0) showBlock(c,dstypos,colorBlock[xpos]);
      }
    }
  }

/* Funktion zeigt alle Bloecke des Spielfeldes mit der Farbe ALLON an.
   Nach einer Pause von 500 ms wird das Sielfeld komplett geloescht
  */
void clearBoard()
  {
    for (uint8_t x = 0; x<xposUMNS; x++) {
      for (uint8_t y = 0; y<yposS; y++) {
        showBlock(x,y,ALLON);
      }
    }
    delay(500);
    for (uint8_t i = 0; i<yposS; i++) {
      deleteypos(i);
    }
  }

/* Funktion uebertraegt das Tetris-Teil pc in den Spielfeldspeicher in der Zeile
   rpg an der Spalte cpg (linke untere Ecke)
  */
void putPiece(uint8_t pc, int8_t cpg, int8_t rpg)
  {
    uint8_t xposor;
    for (uint8_t r = 0; r<4; r++)
      {
        for (uint8_t c = 0; c<4; c++)
          {
            xposor = piece[pc][r*4+c];
            if ((xposor > 0) && ((3-r+rpg) >= 0)) playGround[rpg-3+r][cpg+c] = xposor;
          }
      }
  }

/* Ein neues Tetristeil wird am oberen Rand des Spielfeldes eingefuegt.
   Welches Teil und in welcher Spalte wird als Zufallszahl ermittelt
   Hat das neue Teil keinen Platz am Spielfeld, so ist das Spiel zu Ende
  */
boolean newPiece()
  {
    uint8_t pc = random(1,20);
    uint8_t cpg = random(0,xposUMNS-4);
    boolean res = checkPiece(pc,cpg,3);
    curPiece=0;
    if (res)
      {
        curPiece = pc;
        curxpos = cpg;
        curypos = 0;
        showPiece(pc,cpg,0);
        score += 4;
        displayScore();
      }
      else
      {
        tft.setTextSize(3);
        tft.setCursor(LEFTMARGIN+xposUMNS*BLOCKSIZE/2-79,TOPMARGIN+yposS*BLOCKSIZE/2-10);
        tft.setTextColor(ILI9341_BLACK);
        tft.print("GAME OVER");
        tft.setCursor(LEFTMARGIN+xposUMNS*BLOCKSIZE/2-81,TOPMARGIN+yposS*BLOCKSIZE/2-12);
        tft.setTextColor(ILI9341_YELLOW);
        tft.print("GAME OVER");
      }
  }

/* Funktion ermittelt komplett gef�llte Zeilen am Spielfeld und entfernt diese
   Darueberliegende Zeilen werden nach unten verschoben
 */
void removeComplete()
  {
    uint8_t s=yposS-1;
    int8_t d= yposS-1;
    while (d >= 0)
      {
        if (yposComplete(d))
          {
            s--;
            score += 10;
            copyypos(s,d);
          }
          else
          {
            if ((s < d) && (s >=0))
              {
                Serial.printf("copy %i to %i\n",s, d);
                copyypos(s,d);
              }
            s--;
            d--;
          }
      }
   displayScore();
  }

/* Funktion beginnt ein neues Spiel. Der score wird auf 0 gesetzt, das Spielfeld
   geloescht und mit einem neuen Tetris Teil gestartet
 */
void newGame()
  {
    score=0;
    displayScore();
    clearBoard();
    newPiece();
  }

/* Callbackfunktion fuer Touchscreen Ereignis Klick
   Diese Funktion wird immer dann aufgerufen, wenn der Bildschirm
   kurz beruehrt wird. p gibt die Position des Beruehrungspunktes an
 */
void onClick(TS_Point p)
  {
    if (p.y < 80)
      { //Klick im obersten Viertel des Displays
        newGame();
      }
      else if (p.y > 240)
      { //Klick im untersten Viertel
        uint8_t pc = curPiece;
        int8_t c = curxpos;
        if (p.x < 80)
          { //Klick im linken Drittel -> nach links schieben
            c--;
          }
          else if (p.x <160)
          { //Klick im mittleren Drittel -> drehen
            pc = rotate(pc);
          }
          else
          { //Klick im rechten Drittel -> nach rechts schieben
            c++;
          }
        /* nach Aenderung der Position wird auf Kollision geprueft
           nur wenn keine Kollision auftritt, wird die Bewegung
           ausgefuehrt
         */
        if (checkPiece(pc,c,curypos))
          {
            hidePiece(curPiece,curxpos,curypos);
            curPiece = pc;
            curxpos = c;
            showPiece(curPiece,curxpos,curypos);
          }
      }
  }

