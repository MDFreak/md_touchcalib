#ifndef _MAIN_H_
  #define _MAIN_H_

  #include <Arduino.h>
  #include <wire.h>
  #include <Adafruit_GFX.h> //Grafik Bibliothek
  #include <Adafruit_ILI9341.h> // Display Treiber
  #include <XPT2046_Touchscreen.h> //Touchscreen Treiber
//  #include <md_touch.h>
  #include <md_TouchEvent.h> //Auswertung von Touchscreen Ereignissen

  //Aussehen
  #define BACKGROUND ILI9341_GREENYELLOW //Farbe des Rahmens
  #define TOPMARGIN 20                   //Rand oben
  #define LEFTMARGIN 12                  //Rand links und rechts
  #define COLUMNS 12                     //Anzahl der Spalten
  #define ROWS 16                        //Anzahl der Zeilen
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

  #define TOUCHSIZE    24

  #if (TOUCHSIZE == 24)
    #define TOUCH_ROTATION 3 //muss fuer 2.4 Zoll Display 1 und fuer 2.8 Zoll Display 3 sein
  #else
    #define TOUCH_ROTATION 3 //muss fuer 2.4 Zoll Display 1 und fuer 2.8 Zoll Display 3 sein
  #endif

  //Farben fuer die Bloecke
  const uint16_t colorBlock[8] =
    {
      ILI9341_BLACK, ILI9341_YELLOW, ILI9341_RED,
      ILI9341_CYAN,  ILI9341_GREEN,  ILI9341_PURPLE,
      ILI9341_BLUE,  ILI9341_ORANGE
    };

  /* Bitmuster fuer die Teile
     0 = Block nicht gesetzt >0 Index der Farbe f�r den Block
    */
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

  // ----------------------------------------
  /* Funktion zeigt in der Kopfleiste den aktuellen Score und den Level an
     Abhaengig vom Score wird der Level hinaufgesetzt und das Intervall verringert
    */
  void displayScore();

  /*  Funktion um ein Tetris-Teil zu drehen. Der Parameter ist die Nummer des Teils
     das gedreht werden soll. R�ckgabewert ist der Index des gedrehten Teils
    */
  uint8_t rotate(uint8_t pc);

  //Funktion testet ob eine Zeile voll belegt ist
  boolean rowComplete(int8_t rpg);

  /* Funkzion prueft ob es zwischen der Zeile rpc des Tetris-Teils pc und
     der Zeile rpg des Spielfelds ab der Position cpg Kolklisionen gibt.
     Wenn eine Kollision auftritt oder die letzte Zeile des Spielfelds
     erreicht wurde wird falsch zurueckgegeben
    */
  boolean checkRow(uint8_t pc, int8_t rpc, int8_t cpg, int8_t rpg);

  /* Funktion prueft ob das Tetris Teil pc am Spielfeld an der Position
     Zeile rpg Spalte cpg (linke untere Ecke des Teils) Kollisionen auftreten
    */
  boolean checkPiece(uint8_t pc, int8_t cpg, int8_t rpg);

  /* Funktion zeigt einen Block des Spielfeldes in Zeile y Spalte x mit der Farbe color an
     color ist die Farbe im 565 Format fuer das Display */
  void showBlock(uint8_t x, uint8_t y, uint16_t color);

  // Funktion faellt einen Block des Spielfeldes in Zeile y Spalte x mit der Hintergrundfarbe
  void hideBlock(uint8_t x, uint8_t y);

  /* Funktion zeigt das Tetris-Teil pc in Zeile rpg, Spalte cpg (Linke untere Ecke) an
     Die Farbe wird der Definition des Tetris-Teils entnommen
    */
  void showPiece(uint8_t pc, uint8_t cpg, uint8_t rpg);

  /* Funktion faellt die belegten Bloecke des Tetris-Teil pc in Zeile rpg,
     Spalte cpg (Linke untere Ecke) an mit Hintergrundfarbe
   */
  void hidePiece(uint8_t pc, int8_t cpg, int8_t rpg);

  /*funktion faellt die Zeile row des Spielfelds mit Hintergrundfarbe und
    loescht alle Eintraege fuer diese Zeile im Spielfeld-Speicher
    */
  void deleteRow(int8_t row);

  /* Funktion kopiert die Zeile srcrow in die Zeile dstrow
     Die Anzeige der Zielzeile wird vorher geloescht. Beim
     kopieren wird die Quellzeile in der Zielzeile angezeigt
    */
  void copyRow(int8_t srcrow, int8_t dstrow);

  /* Funktion zeigt alle Bloecke des Spielfeldes mit der Farbe ALLON an.
     Nach einer Pause von 500 ms wird das Sielfeld komplett geloescht
    */
  void clearBoard();

  /* Funktion uebertraegt das Tetris-Teil pc in den Spielfeldspeicher in der Zeile
     rpg an der Spalte cpg (linke untere Ecke)
    */
  void putPiece(uint8_t pc, int8_t cpg, int8_t rpg);

  /* Ein neues Tetristeil wird am oberen Rand des Spielfeldes eingefuegt.
     Welches Teil und in welcher Spalte wird als Zufallszahl ermittelt
     Hat das neue Teil keinen Platz am Spielfeld, so ist das Spiel zu Ende
    */
  boolean newPiece();

  /* Funktion ermittelt komplett gef�llte Zeilen am Spielfeld und entfernt diese
     Darueberliegende Zeilen werden nach unten verschoben
   */
  void removeComplete();

  /* Funktion beginnt ein neues Spiel. Der score wird auf 0 gesetzt, das Spielfeld
     geloescht und mit einem neuen Tetris Teil gestartet
   */
  void newGame();

  /* Callbackfunktion fuer Touchscreen Ereignis Klick
     Diese Funktion wird immer dann aufgerufen, wenn der Bildschirm
     kurz beruehrt wird. p gibt die Position des Beruehrungspunktes an
   */
  void onClick(TS_Point p);

#endif // _MAIN_H_
