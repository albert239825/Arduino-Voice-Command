// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#include <MOVIShield.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_PIC32)
#include <SoftwareSerial.h> // This is nice and flexible but only supported on AVR and PIC32 architecture, other boards need to use Serial1
#endif

MOVI recognizer(true); // Get a MOVI object, true enables serial monitor interface, rx and tx can be passed as parameters for alternate communication pins on AVR architecture

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN 6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 93

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait)
{
    for (int i = 0; i < strip.numPixels(); i++)
    {                                  // For each pixel in strip...
        strip.setPixelColor(i, color); //  Set pixel's color (in RAM)
        strip.show();                  //  Update strip to match
        delay(wait);                   //  Pause for a moment
    }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait)
{
    for (int a = 0; a < 10; a++)
    { // Repeat 10 times...
        for (int b = 0; b < 3; b++)
        {                  //  'b' counts from 0 to 2...
            strip.clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in steps of 3...
            for (int c = b; c < strip.numPixels(); c += 3)
            {
                strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
            }
            strip.show(); // Update strip with new contents
            delay(wait);  // Pause for a moment
        }
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait)
{
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this loop:
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
    {
        // strip.rainbow() can take a single argument (first pixel hue) or
        // optionally a few extras: number of rainbow repetitions (default 1),
        // saturation and value (brightness) (both 0-255, similar to the
        // ColorHSV() function, default 255), and a true/false flag for whether
        // to apply gamma correction to provide 'truer' colors (default true).
        strip.rainbow(firstPixelHue);
        // Above line is equivalent to:
        // strip.rainbow(firstPixelHue, 1, 255, 255, true);
        strip.show(); // Update strip with new contents
        delay(wait);  // Pause for a moment
    }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait)
{
    int firstPixelHue = 0; // First pixel starts at red (hue 0)
    for (int a = 0; a < 30; a++)
    { // Repeat 30 times...
        for (int b = 0; b < 3; b++)
        {                  //  'b' counts from 0 to 2...
            strip.clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in increments of 3...
            for (int c = b; c < strip.numPixels(); c += 3)
            {
                // hue of pixel 'c' is offset by an amount to make one full
                // revolution of the color wheel (range 65536) along the length
                // of the strip (strip.numPixels() steps):
                int hue = firstPixelHue + c * 65536L / strip.numPixels();
                uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
                strip.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
            }
            strip.show();                // Update strip with new contents
            delay(wait);                 // Pause for a moment
            firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
        }
    }
}

//Definition of global variable -------------------------------------------

//stores current state of led
int state;

//timer - counts down time until return to default state
int timer;

// setup() function -- runs once at startup --------------------------------

void setup()
{
    //Begin serial monitor
    Serial.begin(9600);

    //NeoPixel Setup
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

    strip.fill(strip.Color(255, 255, 255), 0); //Setting default color as white
    strip.show();

    //Movi Setup
    recognizer.init(); // Initialize MOVI (waits for it to boot)

    // Note: training can only be performed in setup().
    // The training functions are "lazy" and only do something if there are changes.
    // They can be commented out to save memory and startup time once training has been performed.
    recognizer.callSign("gallery");       // Train callsign Arduino (may take 20 seconds)
    recognizer.addSentence("turn red");   // Add sentence 1
    recognizer.addSentence("turn green"); // Add sentence 2
    recognizer.addSentence("turn blue");  // Add sentence 3
    recognizer.addSentence("troll me");   // Add sentence 4 (Troll me baby)
    recognizer.addSentence("rainbow");    //Add sentence 5 (Rainbow)
    recognizer.train();                   // Train (may take 20seconds)
                                          //*/

    recognizer.setThreshold(20); // uncomment and set to a higher value (valid range 2-95) if you have a problems due to a noisy environment.

    //set state to default state
    state = 0;

    //set timer to -1 - inactive
    timer = -1;
}

// loop() function -- runs repeatedly as long as board is on ---------------

void loop()
{
    //Reset state to -1
    state = -1;
    //get respose from movi
    //----------------------------------------------------------------------------------------
    //res is the reponse from the voice recognition
    signed int res = recognizer.poll();

    //red
    if (res == 1)
    {
        state = 1;
    }
    //green
    if (res == 2)
    {
        state = 2;
    }
    //blue
    if (res == 3)
    {
        state = 3;
    }

    //troll me
    if (res == 4)
    {
        state = 4;
    }

    if (res == 5)
    {
        state = 0;
    }

    Serial.print("state is: ");
    Serial.print(state);
    Serial.print(" response is: ");
    Serial.print(res);
    Serial.print("\n");

    //-------------------------------------------------------------------------------
    //Change behavior based on state

    //default state - rainbow
    if (state == 0)
    {
        strip.rainbow(random(65536));
        strip.show();
    }

    //red
    else if (state == 1)
    {
        strip.fill(strip.Color(255, 0, 0), 0);
        strip.show();
    }
    //green
    else if (state == 2)
    {
        strip.fill(strip.Color(0, 255, 0), 0);
        strip.show();
    }
    //blue
    else if (state == 3)
    {
        strip.fill(strip.Color(0, 0, 255), 0);
        strip.show();
    }

    else if (state == 4)
    {
        recognizer.play("troll.wav");
    }

    // // Fill along the length of the strip in various colors...
    // colorWipe(strip.Color(255, 0, 0), 50); // Red
    // colorWipe(strip.Color(0, 255, 0), 50); // Green
    // colorWipe(strip.Color(0, 0, 255), 50); // Blue

    // // Do a theater marquee effect in various colors...
    // theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
    // theaterChase(strip.Color(127, 0, 0), 50);     // Red, half brightness
    // theaterChase(strip.Color(0, 0, 127), 50);     // Blue, half brightness

    // rainbow(10);             // Flowing rainbow cycle along the whole strip
    // theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
}