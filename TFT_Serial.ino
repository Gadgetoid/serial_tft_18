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
#define MAX_COLS 24

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

unsigned int cols[MAX_COLS] = {
  0x0000, //black   0
  0x001F, //blue    1
  0xF800, //red     2
  0x07E0, //green   3
  0x07FF, //cyan    4
  0xF81F, //magenta 5
  0xFFE0, //yellow  6
  0xFFFF, //white   7
  
  // Solarized
  0x1AB,  //black   8 
  0x245A, //blue    9
  0xD985, //red     10
  0x84C0, //green   11
  0x2D13, //cyan    12
  0xD1B0, //magenta 13
  0xB440, //yellow  14
  0xEF5A, //white   15
  
  // Solarized
  0x1AB,  //black   8 
  0x245A, //blue    9
  0xD985, //red     10
  0x84C0, //green   11
  0x2D13, //cyan    12
  0xD1B0, //magenta 13
  0xB440, //yellow  14
  0xEF5A  //white   15
};

unsigned char foreground  = 7; //col_white;
unsigned char background  = 0; //col_black;

unsigned int col_foreground = cols[foreground];
unsigned int col_background = cols[background];
  
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

  Serial.begin(9600);
}

void loop() {
  
  // Nothing
}

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
          case 16:
            tft_draw_pixel();
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
  tft.fillScreen(cols[background]);
  tft.setTextSize(2);
  
  tft.setCursor(x_pos, y_pos);
  tft.setTextColor(cols[foreground], cols[background]);
}

void tft_clear()
{
  tft.fillScreen(cols[background]);
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

/*
  We can afford slowdown here,
  because this command
  should not be called often!
*/
void tft_set_color()
{
    unsigned int new_color = (unsigned char)inputString[3] | ((unsigned char)inputString[2] << 8);

    cols[inputString[1]] = new_color;

    col_background = cols[background];
    col_foreground = cols[foreground];
}

void tft_set_fg_color()
{
  if(inputString[1]<MAX_COLS)
  {
    foreground = inputString[1];
    col_foreground = cols[foreground];
  }
  tft.setTextColor(col_foreground, col_background); 
}

void tft_set_bg_color()
{
  if(inputString[1]<MAX_COLS)
  {
    background = inputString[1];
    col_background = cols[background];
  }  
  tft.setTextColor(col_foreground, col_background); 
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

void tft_draw_pixel()
{
  unsigned int color = col_foreground;
  if (inputStringIndex == 4)
  {
    color = cols[inputString[3]];
  }

  // Draw Pixel, at X1,Y1
  tft.drawPixel((int16_t)inputString[1], (int16_t)inputString[2], color); 
}

void tft_draw_line()
{
  unsigned int color = col_foreground;
  if (inputStringIndex == 6)
  {
    color = cols[inputString[5]];
  }

  // Draw Line, from X1,Y1 to X2,Y2
  tft.drawLine((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], (int16_t)inputString[4], color);
} 

void tft_draw_box()
{
  unsigned int color = col_foreground;
  if (inputStringIndex == 6)
  {
    color = cols[inputString[5]];
  }

  // Draw Box, from X1,Y1 to X2,Y2
  tft.drawRect((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], (int16_t)inputString[4], color);
}

void tft_fill_box()
{
  unsigned int color = col_foreground;
  if (inputStringIndex == 6)
  {
    color = cols[inputString[5]];
  }

  // Draw Box, from X1,Y1 to X2,Y2 and fill it with colour
  tft.fillRect((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], (int16_t)(inputString[4]), color); 
}

void tft_draw_circle()
{
  unsigned int color = col_foreground;
  if (inputStringIndex == 5)
  {
    color = cols[inputString[4]];
  }

  // Draw circle at x, y, radius
  tft.drawCircle((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], color);
}

void tft_fill_circle()
{
  unsigned int color = col_foreground;
  if (inputStringIndex == 5)
  {
    color = cols[inputString[4]];
  }

  // Draw circle at x, y, radius and fill
  tft.fillCircle((int16_t)inputString[1], (int16_t)inputString[2], (int16_t)inputString[3], color);
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
