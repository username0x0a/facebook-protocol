#include <stdio.h>

int main( void )
{
	int build = 0;
	FILE* f;
	FILE* g;
	f = fopen( "build.h", "r" );
	fscanf( f, "#define __BUILD %d", &build );
	fclose( f );
	g = fopen( "build.h", "w" );
	fprintf( g, "#define __BUILD %d\n", ++build );
	fclose( g );
}
