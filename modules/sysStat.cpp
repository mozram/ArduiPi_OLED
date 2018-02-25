#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
#include "config.h"

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PRG_NAME        "textFlow"
#define PRG_VERSION     "1.0"

ArduiPi_OLED display;  ///< Instantiate the display

struct s_opts
{
    int oled;
	int verbose;
};

int sleep_divisor = 1 ;
	
// default options values
s_opts opts = {
	OLED_ADAFRUIT_I2C_128x64,	// Default oled
    false										// Not verbose
};

/* ======================================================================
Function: usage
Purpose : display usage
Input 	: program name
Output	: -
Comments: 
====================================================================== */
void usage( char * name)
{
	printf("%s\n", name );
	printf("Usage is: %s --oled type [options]\n", name);
	printf("  --<o>led type\nOLED type are:\n");
	for (int i=0; i<OLED_LAST_OLED;i++)
		printf("  %1d %s\n", i, oled_type_str[i]);
	
	printf("Options are:\n");
	printf("  --<v>erbose  : speak more to user\n");
	printf("  --<h>elp\n");
	printf("<?> indicates the equivalent short option.\n");
	printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
	printf("Example :\n");
	printf( "%s -o 1 use a %s OLED\n\n", name, oled_type_str[1]);
	printf( "%s -o 4 -v use a %s OLED being verbose\n", name, oled_type_str[4]);
}

/* ======================================================================
Function: parse_args
Purpose : parse argument passed to the program
Input 	: -
Output	: -
Comments: 
====================================================================== */
void parse_args(int argc, char *argv[])
{
	static struct option longOptions[] =
	{
		{"oled"	  , required_argument,0, 'o'},
		{"verbose", no_argument,	  	0, 'v'},
		{"help"		, no_argument, 			0, 'h'},
		{0, 0, 0, 0}
	};

	int optionIndex = 0;
	int c;

	while (1) 
	{
		/* no default error messages printed. */
		opterr = 0;

    c = getopt_long(argc, argv, "vho:", longOptions, &optionIndex);

		if (c < 0)
			break;

		switch (c) 
		{
			case 'v': opts.verbose = true	;	break;

			case 'o':
				opts.oled = (int) atoi(optarg);
				
				if (opts.oled < 0 || opts.oled >= OLED_LAST_OLED )
				{
						fprintf(stderr, "--oled %d ignored must be 0 to %d.\n", opts.oled, OLED_LAST_OLED-1);
						fprintf(stderr, "--oled set to 0 now\n");
						opts.oled = 0;
				}
			break;

			case 'h':
				usage(argv[0]);
				exit(EXIT_SUCCESS);
			break;
			
			case '?':
			default:
				fprintf(stderr, "Unrecognized option.\n");
				fprintf(stderr, "Run with '--help'.\n");
				exit(EXIT_FAILURE);
		}
	} /* while */

	if (opts.verbose)
	{
		printf("%s v%s\n", PRG_NAME, PRG_VERSION);
		printf("-- OLED params -- \n");
		printf("Oled is    : %s\n", oled_type_str[opts.oled]);
		printf("-- Other Stuff -- \n");
		printf("verbose is : %s\n", opts.verbose? "yes" : "no");
		printf("\n");
	}	
}

void drawText(const char * text, uint8_t textSize, int16_t x, int16_t y)
{
    display.setTextSize(textSize);
    display.setTextColor(WHITE);
    display.setCursor(x,y);
    display.printf(text);
    // display.setTextColor(BLACK, WHITE); // 'inverted' text
    // display.printf("%f\n", 3.141592);
    // display.setTextSize(2);
    // display.setTextColor(WHITE);
    // display.printf("0x%8X\n", 0xDEADBEEF);
    // display.display();
}

/* ======================================================================
Function: drawTextSmall
Purpose : draw small sized text on screen
Input 	: text[text to be drawn]; line[y axis line to be drawn. 1-8 only]
Output	: -
Comments: 
====================================================================== */
void drawTextSmall(const char * text, uint8_t line)
{
    line--;
    uint8_t length = strlen(text);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, line*8);
    if(length > CONFIG_MAX_CHAR_PER_LINE)
    {
        display.printf("ERROR:MAX CHAR IS %d!", CONFIG_MAX_CHAR_PER_LINE);
    }
    else
    {
        display.printf(text);
    }
    // display.printf(text);
    // display.setTextColor(BLACK, WHITE); // 'inverted' text
    // display.printf("%f\n", 3.141592);
    // display.setTextSize(2);
    // display.setTextColor(WHITE);
    // display.printf("0x%8X\n", 0xDEADBEEF);
    // display.display();
}

void drawHorizontalBar(float value, float maxValue)
{
	uint16_t percent = (uint16_t)((value/maxValue) * 100.0);
	display.drawHorizontalBargraph(0,16, (int16_t) display.width(),8,1, percent);
}

void printOled()
{
    display.display();
}

void getTemp(uint8_t line)
{
	FILE *fp;
  	char path[21];

	/* Open the command for reading. */
	fp = popen("/opt/vc/bin/vcgencmd measure_temp", "r");
	if (fp == NULL) {
		drawTextSmall("Failed to run command\n", line);
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		drawTextSmall(path, line);
	}

	/* close */
	pclose(fp);
}

void getMemInfo(uint8_t line)
{
	FILE *fp;
  	char path[1024];
	char mem[21];
	char memTotal[21];
	char buff[21];

	/* Open the command for reading. */
	fp = popen("cat /proc/meminfo | grep MemAvailable:", "r");
	if (fp == NULL) {
		drawTextSmall("Failed to run command\n", line);
	}
	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		memcpy(mem, &path[18], 6);
	}
	pclose(fp);

	/* Open the command for reading. */
	fp = popen("cat /proc/meminfo | grep MemTotal:", "r");
	if (fp == NULL) {
		drawTextSmall("Failed to run command\n", line);
	}
	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		memcpy(memTotal, &path[18], 6);
	}
	pclose(fp);

	memcpy(buff, mem, 6);
	memcpy(&buff[6], "/", 1);
	memcpy(&buff[7], memTotal, 6);
	memcpy(&buff[13], " kB free", 8);

	drawTextSmall(buff, line);
	drawHorizontalBar((float)atoi(memTotal) - (float)atoi(mem), (float)atoi(memTotal));
}

/* ======================================================================
Function: main
Purpose : Main entry Point
Input 	: -
Output	: -
Comments: 
====================================================================== */
int main(int argc, char **argv)
{
    // Oled supported display in ArduiPi_SSD1306.h
	// Get OLED type
	parse_args(argc, argv);

	// SPI
	if (display.oled_is_spi_proto(opts.oled))
	{
		// SPI change parameters to fit to your LCD
		if ( !display.init(OLED_SPI_DC,OLED_SPI_RESET,OLED_SPI_CS, opts.oled) )
			exit(EXIT_FAILURE);
	}
	else
	{
		// I2C change parameters to fit to your LCD
		if ( !display.init(OLED_I2C_RESET,opts.oled) )
			exit(EXIT_FAILURE);
	}

	display.begin();
	
    // init done
    display.clearDisplay();   // clears the screen  buffer
    display.display();   		// display it (clear display)

	if (opts.oled == 5)
	{
		// showing on this display is very slow (the driver need to be optimized)
		sleep_divisor = 4;

		for(char i=0; i < 12 ; i++)
		{
			display.setSeedTextXY(i,0);  //set Cursor to ith line, 0th column
			display.setGrayLevel(i); //Set Grayscale level. Any number between 0 - 15.
			display.putSeedString("Hello World"); //Print Hello World
		}
		
		sleep(2);

	}

	while(1)
	{
		display.clearDisplay();
		getTemp(1);
		getMemInfo(2);
		drawText("cr1tikal", 2, 0, 32);
		// drawHorizontalBar(10,100);
		
		printOled();
		sleep(1);
	}
}