#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <iocs.h>

#define STARPALETNUM	64
static	unsigned short	starpalet[ STARPALETNUM ][ 16 ];

#define	__rgb(r, g, b)	((((((g) * 32) + (r)) * 32) + (b)) * 2)
#define	__rgb_r( c )	( ( (c) / 64 ) & 31 )
#define	__rgb_b( c )	( ( (c) / 2 ) & 31 )
#define	__rgb_g( c )	( ( (c) / 2048 ) & 31 )

void bgputc(int page, int x, int y, int colour, char c){
	*(unsigned short *)( ((page) ? 0xebe000 : 0xebc000) + (y) * 128 + (x) * 2 ) = (colour) * 256 + (c);
}

void bgputs( int page, int x, int y, int colour, char *cp ){
	while( *cp != '\0' ){
		bgputc( page, x++, y, colour, -0x20 + *cp++ );
	}
}

void	paletset()
{
	int i, j, cont;

	static unsigned short starpaletorg[ 16 ] = {
		__rgb(  0,  0,  0 ), __rgb(  4,  7, 10 ),
		__rgb( 15, 20, 31 ), __rgb(  8, 10, 15 ),
		__rgb( 20, 24, 31 ), __rgb( 10, 15, 24 ),
		__rgb( 15, 20, 31 ), __rgb(  8, 10, 15 ),
		__rgb(  0,  0,  0 ), __rgb(  4,  7, 10 ),
		__rgb( 20, 24, 31 ), __rgb( 10, 15, 24 ),
		__rgb( 31, 31, 31 ), __rgb( 25, 18, 31 ),
		__rgb( 30, 30, 10 ), __rgb( 31, 20, 10 ),
	};

	for (i = 0; i < 16; i++) {
		int	r,g,b,c,p;
		_iocs_gpalet(i, c = starpaletorg[ i ]) ;
		p = rand();
		for (j = 0; j < STARPALETNUM; j++) {

			p = ( p + 1 ) & ( STARPALETNUM - 1 );
			if (p < 33) cont = p;
			else cont = 64 - p;

			r = __rgb_r( c ) - ( 32 - cont ) ;
			g = __rgb_g( c ) - ( 32 - cont ) ;
			b = __rgb_b( c ) - ( 32 - cont ) ;
			r = r > 0 ? r : 0;
			g = g > 0 ? g : 0;
			b = b > 0 ? b : 0;
			starpalet[ j ][ i ] = __rgb( r, g, b );
		}
	}
}

int main(){

	unsigned int lastmode;

	/* Record current mode */
	lastmode = _iocs_crtmod(-1);

	/* Initialise new screen mode - see
	    CRTMOD mode list*/
	printf("Setting video mode\n");
	_iocs_crtmod(6);
	
	/* Select which graphic page is active 
	    some modes have more than one
	    page (e.g. double buffering) see 
	    CRTMOD mode list */
	printf("Selecting video page\n");
	_iocs_vpage(0);
	
	/* Set display clipping range as some
	    modes have virtual display sizes
	    bigger than visible (x1, y1, x2, y2) */
	printf("Setting visible region\n");
	_iocs_window( 0, 0, 511, 511 );
	
	/* Clear screen contents */
	//_iocs_g_clr_on();
	/* Turn text cursor off */
	//_iocs_b_curoff();

	/* The magic of doing stuff with the screen happens here */
	
	/* ..... */
	
	printf("Setting colour palette\n");
	paletset();
	
	printf("Selecting video page\n");
	_iocs_vpage(15);
	
	printf("Displaying graphics text\n");
	bgputs(0, 3, 4, 2, "Hello world");

	
	
	/* Restore original screen mode */
	//_iocs_crtmod(lastmode);
	/* Enable text cursor */
	//_iocs_b_curon();

	printf("Waiting for quit signal\n");
	_dos_getc();
	
	return 0;
}
