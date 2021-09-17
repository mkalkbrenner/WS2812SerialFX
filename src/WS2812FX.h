/*
  WS2812FX.h - Library for WS2812 LED effects.

  Harm Aldick - 2016
  www.aldick.org
  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit NeoPixel Library
  NOTES
    * Uses the Adafruit NeoPixel library. Get it here:
      https://github.com/adafruit/Adafruit_NeoPixel
  LICENSE
  The MIT License (MIT)
  Copyright (c) 2016  Harm Aldick
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  CHANGELOG
  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
  2017-02-02   added external trigger functionality (e.g. for sound-to-light)
  2021-02-25   experimental port from Adafruit_NeoPixel to WS2812Serial by Markus Kalkbrenner
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#define FSH(x) (__FlashStringHelper*)(x)
#define MAX_MILLIS (0UL - 1UL) /* ULONG_MAX */

#include <WS2812Serial.h>

#define DEFAULT_BRIGHTNESS (uint8_t)50
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint16_t)1000
#define DEFAULT_COLOR      (uint32_t)0xFF0000
#define DEFAULT_COLORS     { RED, GREEN, BLUE }
#define COLORS(...)        (const uint32_t[]){__VA_ARGS__}

#if defined(ESP8266) || defined(ESP32)
  //#pragma message("Compiling for ESP")
  #define SPEED_MIN (uint16_t)2
#else
  //#pragma message("Compiling for Arduino")
  #define SPEED_MIN (uint16_t)10
#endif
#define SPEED_MAX (uint16_t)65535

#define BRIGHTNESS_MIN (uint8_t)0
#define BRIGHTNESS_MAX (uint8_t)255

/* each segment uses 36 bytes of SRAM memory, so if you're compile fails
  because of insufficient flash memory, decreasing MAX_NUM_SEGMENTS may help */
#define MAX_NUM_SEGMENTS         10
#define MAX_NUM_ACTIVE_SEGMENTS  10
#define INACTIVE_SEGMENT        255 /* max uint_8 */
#define MAX_NUM_COLORS            3 /* number of colors per segment */
#define MAX_CUSTOM_MODES          8

// some common colors
#define RED        (uint32_t)0xFF0000
#define GREEN      (uint32_t)0x00FF00
#define BLUE       (uint32_t)0x0000FF
#define WHITE      (uint32_t)0xFFFFFF
#define BLACK      (uint32_t)0x000000
#define YELLOW     (uint32_t)0xFFFF00
#define CYAN       (uint32_t)0x00FFFF
#define MAGENTA    (uint32_t)0xFF00FF
#define PURPLE     (uint32_t)0x400080
#define ORANGE     (uint32_t)0xFF3000
#define PINK       (uint32_t)0xFF1493
#define GRAY       (uint32_t)0x101010
#define ULTRAWHITE (uint32_t)0xFFFFFFFF
#define DIM(c)     (uint32_t)((c >> 2) & 0x3f3f3f3f) // color at 25% intensity
#define DARK(c)    (uint32_t)((c >> 4) & 0x0f0f0f0f) // color at  6% intensity


// segment options
// bit    7: reverse animation
// bits 4-6: fade rate (0-7)
// bit    3: gamma correction
// bits 1-2: size
// bits   0: TBD
#define NO_OPTIONS   (uint8_t)B00000000
#define REVERSE      (uint8_t)B10000000
#define IS_REVERSE   ((_seg->options & REVERSE) == REVERSE)
#define FADE_XFAST   (uint8_t)B00010000
#define FADE_FAST    (uint8_t)B00100000
#define FADE_MEDIUM  (uint8_t)B00110000
#define FADE_SLOW    (uint8_t)B01000000
#define FADE_XSLOW   (uint8_t)B01010000
#define FADE_XXSLOW  (uint8_t)B01100000
#define FADE_GLACIAL (uint8_t)B01110000
#define FADE_RATE    ((_seg->options >> 4) & 7)
#define GAMMA        (uint8_t)B00001000
#define IS_GAMMA     ((_seg->options & GAMMA) == GAMMA)
#define SIZE_SMALL   (uint8_t)B00000000
#define SIZE_MEDIUM  (uint8_t)B00000010
#define SIZE_LARGE   (uint8_t)B00000100
#define SIZE_XLARGE  (uint8_t)B00000110
#define SIZE_OPTION  ((_seg->options >> 1) & 3)

// segment runtime options (aux_param2)
#define FRAME           (uint8_t)B10000000
#define SET_FRAME       (_seg_rt->aux_param2 |=  FRAME)
#define CLR_FRAME       (_seg_rt->aux_param2 &= ~FRAME)
#define CYCLE           (uint8_t)B01000000
#define SET_CYCLE       (_seg_rt->aux_param2 |=  CYCLE)
#define CLR_CYCLE       (_seg_rt->aux_param2 &= ~CYCLE)
#define CLR_FRAME_CYCLE (_seg_rt->aux_param2 &= ~(FRAME | CYCLE))

#define MODE_COUNT (sizeof(_names)/sizeof(_names[0]))

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_INV           4 
#define FX_MODE_COLOR_WIPE_REV           5
#define FX_MODE_COLOR_WIPE_REV_INV       6
#define FX_MODE_COLOR_WIPE_RANDOM        7
#define FX_MODE_RANDOM_COLOR             8
#define FX_MODE_SINGLE_DYNAMIC           9
#define FX_MODE_MULTI_DYNAMIC           10
#define FX_MODE_RAINBOW                 11
#define FX_MODE_RAINBOW_CYCLE           12
#define FX_MODE_SCAN                    13
#define FX_MODE_DUAL_SCAN               14
#define FX_MODE_FADE                    15
#define FX_MODE_THEATER_CHASE           16
#define FX_MODE_THEATER_CHASE_RAINBOW   17
#define FX_MODE_RUNNING_LIGHTS          18
#define FX_MODE_TWINKLE                 19
#define FX_MODE_TWINKLE_RANDOM          20
#define FX_MODE_TWINKLE_FADE            21
#define FX_MODE_TWINKLE_FADE_RANDOM     22
#define FX_MODE_SPARKLE                 23
#define FX_MODE_FLASH_SPARKLE           24
#define FX_MODE_HYPER_SPARKLE           25
#define FX_MODE_STROBE                  26
#define FX_MODE_STROBE_RAINBOW          27
#define FX_MODE_MULTI_STROBE            28
#define FX_MODE_BLINK_RAINBOW           29
#define FX_MODE_CHASE_WHITE             30
#define FX_MODE_CHASE_COLOR             31
#define FX_MODE_CHASE_RANDOM            32
#define FX_MODE_CHASE_RAINBOW           33
#define FX_MODE_CHASE_FLASH             34
#define FX_MODE_CHASE_FLASH_RANDOM      35
#define FX_MODE_CHASE_RAINBOW_WHITE     36
#define FX_MODE_CHASE_BLACKOUT          37
#define FX_MODE_CHASE_BLACKOUT_RAINBOW  38
#define FX_MODE_COLOR_SWEEP_RANDOM      39
#define FX_MODE_RUNNING_COLOR           40
#define FX_MODE_RUNNING_RED_BLUE        41
#define FX_MODE_RUNNING_RANDOM          42
#define FX_MODE_LARSON_SCANNER          43
#define FX_MODE_COMET                   44
#define FX_MODE_FIREWORKS               45
#define FX_MODE_FIREWORKS_RANDOM        46
#define FX_MODE_MERRY_CHRISTMAS         47
#define FX_MODE_FIRE_FLICKER            48
#define FX_MODE_FIRE_FLICKER_SOFT       49
#define FX_MODE_FIRE_FLICKER_INTENSE    50
#define FX_MODE_CIRCUS_COMBUSTUS        51
#define FX_MODE_HALLOWEEN               52
#define FX_MODE_BICOLOR_CHASE           53
#define FX_MODE_TRICOLOR_CHASE          54
#define FX_MODE_TWINKLEFOX              55
#define FX_MODE_CUSTOM                  56  // keep this for backward compatiblity
#define FX_MODE_CUSTOM_0                56  // custom modes need to go at the end
#define FX_MODE_CUSTOM_1                57
#define FX_MODE_CUSTOM_2                58
#define FX_MODE_CUSTOM_3                59
#define FX_MODE_CUSTOM_4                60
#define FX_MODE_CUSTOM_5                61
#define FX_MODE_CUSTOM_6                62
#define FX_MODE_CUSTOM_7                63

// create GLOBAL names to allow WS2812FX to compile with sketches and other libs
// that store strings in PROGMEM (get rid of the "section type conflict with __c"
// errors once and for all. Amen.)
const char name_0[] PROGMEM = "Static";
const char name_1[] PROGMEM = "Blink";
const char name_2[] PROGMEM = "Breath";
const char name_3[] PROGMEM = "Color Wipe";
const char name_4[] PROGMEM = "Color Wipe Inverse";
const char name_5[] PROGMEM = "Color Wipe Reverse";
const char name_6[] PROGMEM = "Color Wipe Reverse Inverse";
const char name_7[] PROGMEM = "Color Wipe Random";
const char name_8[] PROGMEM = "Random Color";
const char name_9[] PROGMEM = "Single Dynamic";
const char name_10[] PROGMEM = "Multi Dynamic";
const char name_11[] PROGMEM = "Rainbow";
const char name_12[] PROGMEM = "Rainbow Cycle";
const char name_13[] PROGMEM = "Scan";
const char name_14[] PROGMEM = "Dual Scan";
const char name_15[] PROGMEM = "Fade";
const char name_16[] PROGMEM = "Theater Chase";
const char name_17[] PROGMEM = "Theater Chase Rainbow";
const char name_18[] PROGMEM = "Running Lights";
const char name_19[] PROGMEM = "Twinkle";
const char name_20[] PROGMEM = "Twinkle Random";
const char name_21[] PROGMEM = "Twinkle Fade";
const char name_22[] PROGMEM = "Twinkle Fade Random";
const char name_23[] PROGMEM = "Sparkle";
const char name_24[] PROGMEM = "Flash Sparkle";
const char name_25[] PROGMEM = "Hyper Sparkle";
const char name_26[] PROGMEM = "Strobe";
const char name_27[] PROGMEM = "Strobe Rainbow";
const char name_28[] PROGMEM = "Multi Strobe";
const char name_29[] PROGMEM = "Blink Rainbow";
const char name_30[] PROGMEM = "Chase White";
const char name_31[] PROGMEM = "Chase Color";
const char name_32[] PROGMEM = "Chase Random";
const char name_33[] PROGMEM = "Chase Rainbow";
const char name_34[] PROGMEM = "Chase Flash";
const char name_35[] PROGMEM = "Chase Flash Random";
const char name_36[] PROGMEM = "Chase Rainbow White";
const char name_37[] PROGMEM = "Chase Blackout";
const char name_38[] PROGMEM = "Chase Blackout Rainbow";
const char name_39[] PROGMEM = "Color Sweep Random";
const char name_40[] PROGMEM = "Running Color";
const char name_41[] PROGMEM = "Running Red Blue";
const char name_42[] PROGMEM = "Running Random";
const char name_43[] PROGMEM = "Larson Scanner";
const char name_44[] PROGMEM = "Comet";
const char name_45[] PROGMEM = "Fireworks";
const char name_46[] PROGMEM = "Fireworks Random";
const char name_47[] PROGMEM = "Merry Christmas";
const char name_48[] PROGMEM = "Fire Flicker";
const char name_49[] PROGMEM = "Fire Flicker (soft)";
const char name_50[] PROGMEM = "Fire Flicker (intense)";
const char name_51[] PROGMEM = "Circus Combustus";
const char name_52[] PROGMEM = "Halloween";
const char name_53[] PROGMEM = "Bicolor Chase";
const char name_54[] PROGMEM = "Tricolor Chase";
const char name_55[] PROGMEM = "TwinkleFOX";
const char name_56[] PROGMEM = "Custom 0"; // custom modes need to go at the end
const char name_57[] PROGMEM = "Custom 1";
const char name_58[] PROGMEM = "Custom 2";
const char name_59[] PROGMEM = "Custom 3";
const char name_60[] PROGMEM = "Custom 4";
const char name_61[] PROGMEM = "Custom 5";
const char name_62[] PROGMEM = "Custom 6";
const char name_63[] PROGMEM = "Custom 7";

static const __FlashStringHelper* _names[] = {
  FSH(name_0),
  FSH(name_1),
  FSH(name_2),
  FSH(name_3),
  FSH(name_4),
  FSH(name_5),
  FSH(name_6),
  FSH(name_7),
  FSH(name_8),
  FSH(name_9),
  FSH(name_10),
  FSH(name_11),
  FSH(name_12),
  FSH(name_13),
  FSH(name_14),
  FSH(name_15),
  FSH(name_16),
  FSH(name_17),
  FSH(name_18),
  FSH(name_19),
  FSH(name_20),
  FSH(name_21),
  FSH(name_22),
  FSH(name_23),
  FSH(name_24),
  FSH(name_25),
  FSH(name_26),
  FSH(name_27),
  FSH(name_28),
  FSH(name_29),
  FSH(name_30),
  FSH(name_31),
  FSH(name_32),
  FSH(name_33),
  FSH(name_34),
  FSH(name_35),
  FSH(name_36),
  FSH(name_37),
  FSH(name_38),
  FSH(name_39),
  FSH(name_40),
  FSH(name_41),
  FSH(name_42),
  FSH(name_43),
  FSH(name_44),
  FSH(name_45),
  FSH(name_46),
  FSH(name_47),
  FSH(name_48),
  FSH(name_49),
  FSH(name_50),
  FSH(name_51),
  FSH(name_52),
  FSH(name_53),
  FSH(name_54),
  FSH(name_55),
  FSH(name_56),
  FSH(name_57),
  FSH(name_58),
  FSH(name_59),
  FSH(name_60),
  FSH(name_61),
  FSH(name_62),
  FSH(name_63)
};

/* A PROGMEM (flash mem) table containing 8-bit unsigned sine wave (0-255).
   Copy & paste this snippet into a Python REPL to regenerate:
import math
for x in range(256):
    print("{:3},".format(int((math.sin(x/128.0*math.pi)+1.0)*127.5+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM _NeoPixelSineTable[256] = {
128,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,
176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,
218,220,222,224,226,228,230,232,234,235,237,238,240,241,243,244,
245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,
255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,
245,244,243,241,240,238,237,235,234,232,230,228,226,224,222,220,
218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,
176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,
128,124,121,118,115,112,109,106,103,100, 97, 93, 90, 88, 85, 82,
79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40,
37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11,
10,  9,  7,  6,  5,  5,  4,  3,  2,  2,  1,  1,  1,  0,  0,  0,
0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  5,  5,  6,  7,  9,
10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76,
79, 82, 85, 88, 90, 93, 97,100,103,106,109,112,115,118,121,124};

/* Similar to above, but for an 8-bit gamma-correction table.
   Copy & paste this snippet into a Python REPL to regenerate:
import math
gamma=2.6
for x in range(256):
    print("{:3},".format(int(math.pow((x)/255.0,gamma)*255.0+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM _NeoPixelGammaTable[256] = {
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,
7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12,
13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20,
20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29,
30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42,
42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75,
76, 77, 78, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
97, 99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,
122,124,125,127,129,130,132,134,136,137,139,141,143,145,146,148,
150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,
182,184,186,188,191,193,195,197,199,202,204,206,209,211,213,215,
218,220,223,225,227,230,232,235,237,240,242,245,247,250,252,255};

class WS2812FX : public WS2812Serial {

  public:
    typedef uint16_t (WS2812FX::*mode_ptr)(void);

    // segment parameters
    typedef struct Segment { // 20 bytes
      uint16_t start;
      uint16_t stop;
      uint16_t speed;
      uint8_t  mode;
      uint8_t  options;
      uint32_t colors[MAX_NUM_COLORS];
    } segment;

    // segment runtime parameters
    typedef struct Segment_runtime { // 16 bytes
      unsigned long next_time;
      uint32_t counter_mode_step;
      uint32_t counter_mode_call;
      uint8_t aux_param;   // auxilary param (usually stores a color_wheel index)
      uint8_t aux_param2;  // auxilary param (usually stores bitwise options)
      uint16_t aux_param3; // auxilary param (usually stores a segment index)
    } segment_runtime;

    WS2812FX(uint16_t num_leds, void *fb, void *db, uint8_t pin, uint8_t type,
      uint8_t max_num_segments=MAX_NUM_SEGMENTS,
      uint8_t max_num_active_segments=MAX_NUM_ACTIVE_SEGMENTS)
      : WS2812Serial(num_leds, fb, db, pin, type) {

      drawBuffer = (uint8_t *) db;
      bytesPerPixel = (type < 6) ? 3 : 4; // 3=RGB, 4=RGBW

/*
      brightness = DEFAULT_BRIGHTNESS + 1; // Adafruit_NeoPixel internally offsets brightness by 1
*/
      _running = false;

      _segments_len = max_num_segments;
      _active_segments_len = max_num_active_segments;

      // create all the segment arrays and init to zeros
      _segments = new segment[_segments_len]();
      _active_segments = new uint8_t[_active_segments_len]();
      _segment_runtimes = new segment_runtime[_active_segments_len]();

      // init segment pointers
      _seg     = _segments;
      _seg_rt  = _segment_runtimes;

      resetSegments();
      setSegment(0, 0, num_leds - 1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
    };

    void
//    timer(void),
      init(void),
      start(void),
      stop(void),
      pause(void),
      resume(void),
      strip_off(void),
      fade_out(void),
      fade_out(uint32_t),
      setMode(uint8_t m),
      setMode(uint8_t seg, uint8_t m),
      setOptions(uint8_t seg, uint8_t o),
      setCustomMode(uint16_t (*p)()),
      setCustomShow(void (*p)()),
      setSpeed(uint16_t s),
      setSpeed(uint8_t seg, uint16_t s),
      increaseSpeed(uint8_t s),
      decreaseSpeed(uint8_t s),
      setColor(uint8_t r, uint8_t g, uint8_t b),
      setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w),
      setColor(uint32_t c),
      setColor(uint8_t seg, uint32_t c),
      setColors(uint8_t seg, uint32_t* c),
      setBrightness(uint8_t b),
      increaseBrightness(uint8_t s),
      decreaseBrightness(uint8_t s),
/*
      setLength(uint16_t b),
      increaseLength(uint16_t s),
      decreaseLength(uint16_t s),
*/
      trigger(void),
      setCycle(void),
      setNumSegments(uint8_t n),

      setSegment(),
      setSegment(uint8_t n),
      setSegment(uint8_t n, uint16_t start),
      setSegment(uint8_t n, uint16_t start, uint16_t stop),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options),

      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[]),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),

      setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed),
      setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, uint8_t options),
      setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),
      addActiveSegment(uint8_t seg),
      removeActiveSegment(uint8_t seg),
      swapActiveSegment(uint8_t oldSeg, uint8_t newSeg),

      resetSegments(void),
      resetSegmentRuntimes(void),
      resetSegmentRuntime(uint8_t),
      setPixelColor(uint16_t n, uint32_t c),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w),
      copyPixels(uint16_t d, uint16_t s, uint16_t c),
/*
      setPixels(uint16_t, uint8_t*),
*/
      setRandomSeed(uint16_t),
      fill(uint32_t c=0, uint16_t first=0, uint16_t count=0),
      show(void);

    bool
      service(void),
      isRunning(void),
      isTriggered(void),
      isFrame(void),
      isFrame(uint8_t),
      isCycle(void),
      isCycle(uint8_t),
      isActiveSegment(uint8_t seg);

    uint8_t
      random8(void),
      random8(uint8_t),
      getMode(void),
      getMode(uint8_t),
      getModeCount(void),
      setCustomMode(const __FlashStringHelper* name, uint16_t (*p)()),
      setCustomMode(uint8_t i, const __FlashStringHelper* name, uint16_t (*p)()),
      getNumSegments(void),
      get_random_wheel_index(uint8_t),
      getOptions(uint8_t),
      getNumBytesPerPixel(void);

    uint16_t
      random16(void),
      random16(uint16_t),
      getSpeed(void),
      getSpeed(uint8_t),
      getLength(void),
      getNumBytes(void);

    uint32_t
      color_wheel(uint8_t),
      getColor(void),
      getColor(uint8_t),
      intensitySum(void),
      getPixelColor(uint16_t n) const;

    uint32_t* getColors(uint8_t);
    uint32_t* intensitySums(void);
    uint8_t*  getActiveSegments(void);
    uint8_t*  blend(uint8_t*, uint8_t*, uint8_t*, uint16_t, uint8_t);
    uint8_t*  getPixels(void) const { return drawBuffer; };

    const __FlashStringHelper* getModeName(uint8_t m);

    WS2812FX::Segment* getSegment(void);

    WS2812FX::Segment* getSegment(uint8_t);

    WS2812FX::Segment* getSegments(void);

    WS2812FX::Segment_runtime* getSegmentRuntime(void);

    WS2812FX::Segment_runtime* getSegmentRuntime(uint8_t);

    WS2812FX::Segment_runtime* getSegmentRuntimes(void);

    // mode helper functions
    uint16_t
      blink(uint32_t, uint32_t, bool strobe),
      color_wipe(uint32_t, uint32_t, bool),
      twinkle(uint32_t, uint32_t),
      twinkle_fade(uint32_t),
      sparkle(uint32_t, uint32_t),
      chase(uint32_t, uint32_t, uint32_t),
      chase_flash(uint32_t, uint32_t),
      running(uint32_t, uint32_t),
      fireworks(uint32_t),
      fire_flicker(int),
      tricolor_chase(uint32_t, uint32_t, uint32_t),
      scan(uint32_t, uint32_t, bool);

    uint32_t
      color_blend(uint32_t, uint32_t, uint8_t);

    // builtin modes
    uint16_t
      mode_static(void),
      mode_blink(void),
      mode_blink_rainbow(void),
      mode_strobe(void),
      mode_strobe_rainbow(void),
      mode_color_wipe(void),
      mode_color_wipe_inv(void),
      mode_color_wipe_rev(void),
      mode_color_wipe_rev_inv(void),
      mode_color_wipe_random(void),
      mode_color_sweep_random(void),
      mode_random_color(void),
      mode_single_dynamic(void),
      mode_multi_dynamic(void),
      mode_breath(void),
      mode_fade(void),
      mode_scan(void),
      mode_dual_scan(void),
      mode_theater_chase(void),
      mode_theater_chase_rainbow(void),
      mode_rainbow(void),
      mode_rainbow_cycle(void),
      mode_running_lights(void),
      mode_twinkle(void),
      mode_twinkle_random(void),
      mode_twinkle_fade(void),
      mode_twinkle_fade_random(void),
      mode_sparkle(void),
      mode_flash_sparkle(void),
      mode_hyper_sparkle(void),
      mode_multi_strobe(void),
      mode_chase_white(void),
      mode_chase_color(void),
      mode_chase_random(void),
      mode_chase_rainbow(void),
      mode_chase_flash(void),
      mode_chase_flash_random(void),
      mode_chase_rainbow_white(void),
      mode_chase_blackout(void),
      mode_chase_blackout_rainbow(void),
      mode_running_color(void),
      mode_running_red_blue(void),
      mode_running_random(void),
      mode_larson_scanner(void),
      mode_comet(void),
      mode_fireworks(void),
      mode_fireworks_random(void),
      mode_merry_christmas(void),
      mode_halloween(void),
      mode_fire_flicker(void),
      mode_fire_flicker_soft(void),
      mode_fire_flicker_intense(void),
      mode_circus_combustus(void),
      mode_bicolor_chase(void),
      mode_tricolor_chase(void),
      mode_twinkleFOX(void),
      mode_custom_0(void),
      mode_custom_1(void),
      mode_custom_2(void),
      mode_custom_3(void),
      mode_custom_4(void),
      mode_custom_5(void),
      mode_custom_6(void),
      mode_custom_7(void);

    /*!
      @brief   An 8-bit integer sine wave function, not directly compatible
               with standard trigonometric units like radians or degrees.
      @param   x  Input angle, 0-255; 256 would loop back to zero, completing
                  the circle (equivalent to 360 degrees or 2 pi radians).
                  One can therefore use an unsigned 8-bit variable and simply
                  add or subtract, allowing it to overflow/underflow and it
                  still does the expected contiguous thing.
      @return  Sine result, 0 to 255, or -128 to +127 if type-converted to
               a signed int8_t, but you'll most likely want unsigned as this
               output is often used for pixel brightness in animation effects.
    */
    static uint8_t    sine8(uint8_t x) {
        return pgm_read_byte(&_NeoPixelSineTable[x]); // 0-255 in, 0-255 out
    }
    /*!
      @brief   An 8-bit gamma-correction function for basic pixel brightness
               adjustment. Makes color transitions appear more perceptially
               correct.
      @param   x  Input brightness, 0 (minimum or off/black) to 255 (maximum).
      @return  Gamma-adjusted brightness, can then be passed to one of the
               setPixelColor() functions. This uses a fixed gamma correction
               exponent of 2.6, which seems reasonably okay for average
               NeoPixels in average tasks. If you need finer control you'll
               need to provide your own gamma-correction function instead.
    */
    static uint8_t    gamma8(uint8_t x) {
        return pgm_read_byte(&_NeoPixelGammaTable[x]); // 0-255 in, 0-255 out
    }

  private:
    uint16_t _rand16seed;
    uint16_t (*customModes[MAX_CUSTOM_MODES])(void) {
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; }
    };
    void (*customShow)(void) = NULL;

    bool
      _running,
      _triggered;

    segment* _segments;                 // array of segments (20 bytes per element)
    segment_runtime* _segment_runtimes; // array of segment runtimes (16 bytes per element)
    uint8_t* _active_segments;          // array of active segments (1 bytes per element)

    uint8_t _segments_len = 0;          // size of _segments array
    uint8_t _active_segments_len = 0;   // size of _segments_runtime and _active_segments arrays
    uint8_t _num_segments = 0;          // number of configured segments in the _segments array

    segment* _seg;                      // currently active segment (20 bytes)
    segment_runtime* _seg_rt;           // currently active segment runtime (16 bytes)

    uint16_t _seg_len;                  // num LEDs in the currently active segment

    uint8_t *drawBuffer;
    uint8_t bytesPerPixel;
};

class WS2812FXT {
  public:
    WS2812FXT(uint16_t num_leds, uint8_t pin, neoPixelType type,
      uint8_t max_num_segments=MAX_NUM_SEGMENTS,
      uint8_t max_num_active_segments=MAX_NUM_ACTIVE_SEGMENTS) {
        v1 = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments);
        v2 = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments);
        dest = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments); 
    };

    void init(void) {
      v1->init();
      v2->init();
      v1->setCustomShow([]{ return; });
      v2->setCustomShow([]{ return; });
    }

    void start(void) {
      v1->start();
      v2->start();
    }

    void service(void) {
      bool doShow = v1->service() || v2->service();
      if(doShow) {
        _show();
      }
    }

    void startTransition(uint16_t duration, bool direction = true) {
      transitionStartTime = millis();
      transitionDuration = duration;
      transitionDirection = direction;
    }

  private:
    void _show(void) {
      unsigned long now = millis();

      uint8_t *dest_p = dest->getPixels();
      uint8_t *vstart_p = transitionDirection ? v1->getPixels() : v2->getPixels();
      uint8_t *vstop_p  = transitionDirection ? v2->getPixels() : v1->getPixels();
      uint16_t numBytes = dest->getNumBytes();

      if(now < transitionStartTime) {
        memmove(dest_p, vstart_p, numBytes);
      } else if(now > transitionStartTime + transitionDuration) {
        memmove(dest_p, vstop_p, numBytes);
      } else {
        uint8_t blendAmt = map(now, transitionStartTime, transitionStartTime + transitionDuration, 0, 255);
        dest->blend(dest_p, vstart_p, vstop_p, numBytes, blendAmt);
      }

      dest->Adafruit_NeoPixel::show();
    }

  public:
    WS2812FX* v1 = NULL;
    WS2812FX* v2 = NULL;
    WS2812FX* dest = NULL;
    unsigned long transitionStartTime = MAX_MILLIS;
    uint16_t transitionDuration = 5000;
    bool transitionDirection = true;
};

// define static array of member function pointers.
// function pointers MUST be in the same order as the corresponding name in the _name array.
static WS2812FX::mode_ptr _modes[MODE_COUNT] = {
  &WS2812FX::mode_static,
  &WS2812FX::mode_blink,
  &WS2812FX::mode_breath,
  &WS2812FX::mode_color_wipe,
  &WS2812FX::mode_color_wipe_inv,
  &WS2812FX::mode_color_wipe_rev,
  &WS2812FX::mode_color_wipe_rev_inv,
  &WS2812FX::mode_color_wipe_random,
  &WS2812FX::mode_random_color,
  &WS2812FX::mode_single_dynamic,
  &WS2812FX::mode_multi_dynamic,
  &WS2812FX::mode_rainbow,
  &WS2812FX::mode_rainbow_cycle,
  &WS2812FX::mode_scan,
  &WS2812FX::mode_dual_scan,
  &WS2812FX::mode_fade,
  &WS2812FX::mode_theater_chase,
  &WS2812FX::mode_theater_chase_rainbow,
  &WS2812FX::mode_running_lights,
  &WS2812FX::mode_twinkle,
  &WS2812FX::mode_twinkle_random,
  &WS2812FX::mode_twinkle_fade,
  &WS2812FX::mode_twinkle_fade_random,
  &WS2812FX::mode_sparkle,
  &WS2812FX::mode_flash_sparkle,
  &WS2812FX::mode_hyper_sparkle,
  &WS2812FX::mode_strobe,
  &WS2812FX::mode_strobe_rainbow,
  &WS2812FX::mode_multi_strobe,
  &WS2812FX::mode_blink_rainbow,
  &WS2812FX::mode_chase_white,
  &WS2812FX::mode_chase_color,
  &WS2812FX::mode_chase_random,
  &WS2812FX::mode_chase_rainbow,
  &WS2812FX::mode_chase_flash,
  &WS2812FX::mode_chase_flash_random,
  &WS2812FX::mode_chase_rainbow_white,
  &WS2812FX::mode_chase_blackout,
  &WS2812FX::mode_chase_blackout_rainbow,
  &WS2812FX::mode_color_sweep_random,
  &WS2812FX::mode_running_color,
  &WS2812FX::mode_running_red_blue,
  &WS2812FX::mode_running_random,
  &WS2812FX::mode_larson_scanner,
  &WS2812FX::mode_comet,
  &WS2812FX::mode_fireworks,
  &WS2812FX::mode_fireworks_random,
  &WS2812FX::mode_merry_christmas,
  &WS2812FX::mode_fire_flicker,
  &WS2812FX::mode_fire_flicker_soft,
  &WS2812FX::mode_fire_flicker_intense,
  &WS2812FX::mode_circus_combustus,
  &WS2812FX::mode_halloween,
  &WS2812FX::mode_bicolor_chase,
  &WS2812FX::mode_tricolor_chase,
  &WS2812FX::mode_twinkleFOX,
  &WS2812FX::mode_custom_0,
  &WS2812FX::mode_custom_1,
  &WS2812FX::mode_custom_2,
  &WS2812FX::mode_custom_3,
  &WS2812FX::mode_custom_4,
  &WS2812FX::mode_custom_5,
  &WS2812FX::mode_custom_6,
  &WS2812FX::mode_custom_7
};
#endif
