/* 
 Hobbytronics Arduino TFT Serial Driver
 
 Requires the Adafruit GFX and ST7735 Libraries, please see product page for download links
 
 Hobbytronics.co.uk 2013

 Version History
 ---------------
 1.00    17 Apr 2013   Initial Release 
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>

#define TFT_CS  10          // Chip select line for TFT display
#define TFT_DC   8          // Data/command line for TFT
#define TFT_RST  NULL       // Reset line for TFT (or connect to +5V)
#define lcdBacklight  7     // 7 on Serial board, 9 on TFT Shield
#define SD_CS    4          // Chip select line for SD card

#define MODE_COMMAND 1
#define MODE_TEXT    0
#define COMMAND_START 0x1B
#define COMMAND_END 0xFF

unsigned char sd_card=0;       // SD Card inserted?
unsigned char x_pos=0;
unsigned char y_pos=0;
unsigned char text_size=2;
unsigned char screen_width=160;
unsigned char mode=MODE_TEXT;

unsigned char inputString[40];         // a string to hold incoming data
int inputStringIndex = 0;

// Bring colour constants into variables so we can redefine at runtime

// Color definitions
  /*
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0  
#define ST7735_WHITE   0xFFFF
*/


unsigned int col_black   = 0x0000;
unsigned int col_blue    = 0x001F;
unsigned int col_red     = 0xF800;
unsigned int col_green   = 0x07E0;
unsigned int col_cyan    = 0x07FF;
unsigned int col_magenta = 0xF81F;
unsigned int col_yellow  = 0xFFE0;
unsigned int col_white   = 0xFFFF;

unsigned int foreground  = col_white;
unsigned int background  = col_black;

  
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup(void) {
  
  // Set TFT and SD Chip Select pins as output
  pinMode(10, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  
  // If your TFT's plastic wrap has a Red Tab, use the following:
  tft.initR(INITR_REDTAB);   // initialize a ST7735R chip, red tab
  // If your TFT's plastic wrap has a Green Tab, use the following:
  //tft.initR(INITR_GREENTAB); // initialize a ST7735R chip, green tab
  // If your TFT's plastic wrap has a Black Tab, use the following:
  //tft.initR(INITR_BLACKTAB);   // initialize a ST7735R chip, black tab  
  
  tft.setRotation(3);               // Set to landscape mode
  //analogWrite(lcdBacklight, 255);   // Turn Backlight on full
  digitalWrite(lcdBacklight, HIGH);
  
  // Check for SD Card
  if (!SD.begin(SD_CS)) 
  {
    // No SD card, or failed to initialise card0
    sd_card=0;
    // Arduino SD library does something to SPI speed when it fails
    // So need to reset otherwise screen is slow.
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 16/4 MHz
  }  
  else sd_card=1;

  tftInit();

/*
  if(sd_card==1)
  
  {
    tft.print("SD!"); 
File config = SD.open("config.txt");

if(config){ 

  col_black = config.parseInt();
  col_red   = config.parseInt();
  col_green = config.parseInt();
  col_yellow = config.parseInt();
  col_blue = config.parseInt();
  col_magenta = config.parseInt();
  col_cyan = config.parseInt();
  col_white = config.parseInt();
  
  config.close();

}
  }
*/


/*
  col_black = 0x1A8;
  col_red   = 0xD985;
  col_green = 0x84C0;
  col_yellow = 0xB440;
  col_blue = 0x245A;
  col_magenta = 0xD1B0;
  col_cyan = 0x2D13;
  col_white = 0xEF5A;

  foreground = col_white;
  background = col_black;

  tft_test();
*/
/*
  foreground = col_black;
  background = col_white;

  tft_test();
  */
  Serial.begin(9600);
}

void loop() {
  
  // Nothing
}

/*
void tft_test()
{

  tft.setTextSize(3);

  tft.print("Hello World");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(col_red, background);
  tft.print("red");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(col_green, background);
  tft.print("green");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(col_yellow, background);
  tft.print("yellow");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(col_blue, background);
  tft.print("blue");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(col_magenta, background);
  tft.print("magenta");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(col_cyan, background);
  tft.print("cyan");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(col_white, background);
  tft.print("white");
  delay(500);
  tft.fillScreen(background);

  tft.setCursor(x_pos, y_pos);
  delay(500);

  tft.setTextColor(foreground,background);
  tft.fillScreen(background);
}
*/

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 
 NOTE: Arduino UNO buffer size is 64 bytes
 */
void serialEvent() {
  while (Serial.available()) 
  {
    // get the new byte:
    int inChar = Serial.read(); 
    if(mode==MODE_TEXT)
    {    
      if (inChar == COMMAND_START) 
      {
        //COMMAND char - command mode until COMMAND_END
        mode=MODE_COMMAND;
      }
      else if (inChar == '\r') 
      {
        x_pos=0;
        y_pos+=(text_size*8);
        tft.setCursor(x_pos, y_pos);      
      }    
      else if (inChar == '\n') 
      {
        //ignore
      }     
      else
      {
        tft.print((char)inChar);
        x_pos+=(text_size*6);
      } 
    }  
    else
    {
      // in COMMAND MODE
      if (
        (inChar == COMMAND_END && inputString[0] != 15) // Natural command end
        || (inputString[0] == 15 && inputStringIndex == 4) // Specific command length
        )
      {
        // End of command received - validate and run command
        
        inputString[inputStringIndex] = '\0';
        
        switch(inputString[0])
        {
          case 0: 
            tft_clear();          // Clear screen
            break;
          case 1:
            tft_set_fg_color();   // Set Foreground colour
            break;            
          case 2:
            tft_set_bg_color();   // Set Background colour
            break;     
          case 3: 
            tft_rotation();       // Draw filled circle
            break;              
          case 4: 
            tft_fontsize();       // Set fontsize
            break;          
          case 5: 
            tft_bol();            // Goto beginning of line
            break;            
          case 6: 
            tft_text_goto();      // Goto text xy - depends on text size currently set
            break; 
          case 7: 
            tft_pix_goto();      // Goto pixel xy
            break;             
          case 8: 
            tft_draw_line();      // Draw line
            break;     
          case 9: 
            tft_draw_box();       // Draw box
            break;      
          case 10: 
            tft_fill_box();       // Draw filled box
            break;   
          case 11: 
            tft_draw_circle();    // Draw circle
            break;
          case 12: 
            tft_fill_circle();    // Draw filled circle
            break;   
          case 13: 
            tft_bitmap();         // Draw bitmap
            break;  
          case 14: 
            tft_backlight();      // Backlight
            break;  
          case 15:
            tft_set_color();
            break;
          
        }
        inputString[0] = '\0';
        inputStringIndex = 0;
        mode=MODE_TEXT;    // Back to Text mode   
      }    
      else
      {
        // Command mode -accumulate command
        inputString[inputStringIndex] = inChar;
        inputStringIndex++;
      }      
    }  
     
    if(x_pos>=(screen_width-(text_size*6)))
    {
      //can't fit next char on screen - wrap
      x_pos=0;
      y_pos+=(text_size*8);
      tft.setCursor(x_pos, y_pos);
    }      
  }
  
}

void tftInit()
{
  // Clear screen
  char i;
  
  tft.setTextWrap(false);
  tft.fillScreen(background);
  tft.setTextSize(2);
  
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(foreground, background);
}

void tft_clear()
{
  tft.fillScreen(background);
  x_pos=0;
  y_pos=0;
  tft.setCursor(x_pos, y_pos);
}    

void tft_fontsize()
{
  if((inputString[1] >=1) & (inputString[1] <=3))
  {
    text_size=inputString[1];
    tft.setTextSize(text_size);
  }  
}

void tft_set_color()
{
    unsigned int new_color = (unsigned char)inputString[3] | ((unsigned char)inputString[2] << 8);

    switch(inputString[1])
    {
      case 0: 
        col_black = new_color;
        break;
      case 1: 
        col_blue = new_color;
        break;
      case 2: 
        col_red = new_color;
        break;
      case 3: 
        col_green = new_color;
        break;
      case 4: 
        col_cyan = new_color;
        break;
      case 5: 
        col_magenta = new_color;
        break;
      case 6: 
        col_yellow = new_color;
        break;
      case 7: 
        col_white = new_color;
        break;
    } 
}

unsigned int get_color()
{
    switch(inputString[1])
    {
      case 0: 
        return col_black;
        break;
      case 1: 
        return col_blue;
        break;
      case 2: 
        return col_red;
        break;
      case 3: 
        return col_green;
        break;
      case 4: 
        return col_cyan;
        break;
      case 5: 
        return col_magenta;
        break;
      case 6: 
        return col_yellow;
        break;
      case 7: 
        return col_white;
        break;
    }  
}

void tft_set_fg_color()
{
  if(inputString[1]<=7)
  {
    foreground = get_color();
  }
  tft.setTextColor(foreground, background);  
}

void tft_set_bg_color()
{
  if(inputString[1]<=7)
  {
    background = get_color();
  }  
  tft.setTextColor(foreground, background);
}

void tft_bol()
{
  // Goto beginning of line
  x_pos=0;
  tft.setCursor(x_pos, y_pos);
} 

void tft_text_goto()
{
  // Goto text X,Y
  x_pos=(text_size*6)*(inputString[1]);
  y_pos=(text_size*8)*(inputString[2]);
  tft.setCursor(x_pos, y_pos);
} 

void tft_pix_goto()
{
  // Goto pixel position
  if((inputString[1]<160) & (inputString[2]<127))
  {
    x_pos=inputString[1];
    y_pos=inputString[2];  
    tft.setCursor(x_pos, y_pos);
  }  
}

void tft_draw_line()
{
  // Draw Line, from X1,Y1 to X2,Y2
  tft.drawLine((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], (int16_t)inputString[4], foreground);
} 

void tft_draw_box()
{
  // Draw Box, from X1,Y1 to X2,Y2
  tft.drawRect((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3] - (int16_t)inputString[1], (int16_t)inputString[4] - (int16_t)inputString[2], foreground);
}

void tft_fill_box()
{
  // Draw Box, from X1,Y1 to X2,Y2 and fill it with colour
  tft.fillRect((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3] - (int16_t)inputString[1], (int16_t)inputString[4] - (int16_t)inputString[2], foreground); 
}

void tft_draw_circle()
{
  // Draw circle at x, y, radius
  tft.drawCircle((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], foreground);
}

void tft_fill_circle()
{
  // Draw circle at x, y, radius and fill
  tft.fillCircle((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], foreground);
}

void tft_rotation()
{
    tft.setRotation((int16_t)inputString[1]); 
}

void tft_backlight()
{
  // Set backlight
  if(inputString[1] >= 100) digitalWrite(lcdBacklight, HIGH); // Turn Backlight on full
  else analogWrite(lcdBacklight, inputString[1]*2.5 );   
} 

void tft_bitmap(void)
{
  // display bitmap
  char bmp_filename[13];
  char i;
  for(i=3;i<=15;i++)
  {
    // Extract Filename 8.3 dos format (max 12 chars) - no directory (root dir)
    if(inputString[i]!=0x00) 
    {
      bmp_filename[i-3]=inputString[i];
      bmp_filename[i-2]=0x00;      
    }
    else i=99;  // end loop
  }  

  if(sd_card==1)
  {
     bmpDraw(bmp_filename, (int16_t)inputString[1], (int16_t)inputString[2]);
  }
}

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 10

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;

  if((x >= tft.width()) || (y >= tft.height())) return;

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    tft.print("File not found (");
    tft.print(filename);
    tft.println(")");   
    return;
  }
  
  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    (void)read32(bmpFile); // Read and ignore file size
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    // Read DIB header
    (void)read32(bmpFile); // Ignore header
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) tft.println("BMP error.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
