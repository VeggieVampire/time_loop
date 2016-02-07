#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 15
#define LEDS 32


// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t milli_color  = strip.Color ( 20,  0,  0);
uint32_t second_color = strip.Color (  0,  0, 20);
uint32_t hour_color   = strip.Color (  0, 20,  0);
uint32_t minute_color = strip.Color ( 15, 10, 10);
uint32_t off_color    = strip.Color (  0,  0,  0);


/* TODO 0.0 to 1.0 percent between current and next value. for color fading */
/* or event based lerping? */

/* TODO gamut values from goggles */
/* TODO smooth sub pixel rendering of bar/pixel ends (careful to avoid 50% average brightness in middle for a pixel */


/* CLOCK */
class ClockPositions
{
 public:
  uint8_t milli;
  uint8_t second;
  uint8_t minute;
  uint8_t hour;

  ClockPositions ();
  void update    ();
};


ClockPositions::ClockPositions()
{
  milli = second = minute = hour = 0;
  //DateTime(__DATE__, __TIME__);
}


void ClockPositions::update()
{
  second = map ((millis() % 60000), 0, 60000, 0, (LEDS-1));
  milli  = map ((millis() %  1000), 0,  1000, 0, LEDS);
  hour   = map (10 % 12, 0,  12, 0, LEDS);
  minute = map (31 % 60, 0,  60, 0, LEDS);
}



/* CLOCK VIEW */

class ClockSegments
{
 public:
  ClockPositions    &positions;
  Adafruit_NeoPixel &strip;

  ClockSegments (Adafruit_NeoPixel&, ClockPositions&);
  void draw  ();
  void clear ();
  void add_color (uint8_t position, uint32_t color);
  uint32_t blend (uint32_t color1, uint32_t color2);
};


ClockSegments::ClockSegments (Adafruit_NeoPixel& n_strip, ClockPositions& n_positions): strip (n_strip), positions (n_positions)
{
}


void ClockSegments::draw()
{
  clear();

  add_color (positions.minute   % LEDS,  minute_color);
  add_color (positions.hour     % LEDS,  hour_color  );
  add_color ((positions.hour+1) % LEDS,  hour_color  );

  add_color (positions.second     % LEDS, second_color);
  add_color ((positions.second+1) % LEDS, second_color);
  add_color ((positions.second+2) % LEDS, second_color);

  add_color (positions.milli     % LEDS,  milli_color);
  add_color ((positions.milli+1) % LEDS,  milli_color);
  add_color ((positions.milli+2) % LEDS,  milli_color);

  strip.show ();
}


void ClockSegments::add_color (uint8_t position, uint32_t color)
{
  uint32_t blended_color = blend (strip.getPixelColor (position), color);

  /* Gamma mapping */
  uint8_t r,b,g;

  r = (uint8_t)(blended_color >> (LEDS)),
  g = (uint8_t)(blended_color >>  (LEDS/2)),
  b = (uint8_t)(blended_color >>  (LEDS/LEDS));

  strip.setPixelColor (position, blended_color);
}


uint32_t ClockSegments::blend (uint32_t color1, uint32_t color2)
{
  uint8_t r1,g1,b1;
  uint8_t r2,g2,b2;
  uint8_t r3,g3,b3;

  r1 = (uint8_t)(color1 >> (LEDS)),
  g1 = (uint8_t)(color1 >>  (LEDS/2)),
  b1 = (uint8_t)(color1 >>  (LEDS/LEDS));

  r2 = (uint8_t)(color2 >> (LEDS)),
  g2 = (uint8_t)(color2 >>  (LEDS/2)),
  b2 = (uint8_t)(color2 >>  (LEDS/LEDS));


  return strip.Color (constrain (r1+r2, 0, 255), constrain (g1+g2, 0, 255), constrain (b1+b2, 0, 255));
}


void ClockSegments::clear ()
{
  for(uint16_t i=0; i<strip.numPixels (); i++) {
      strip.setPixelColor (i, off_color);
  }
}



/* SIMPLE MIXER */
// add rgb and clamp





/* APP */
ClockPositions positions;
ClockSegments  segments(strip, positions);

void setup ()
{
  strip.begin ();
  strip.show (); // Initialize all pixels to 'off'
}


void loop ()
{
  positions.update ();
  segments.draw ();
}




// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint32_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
