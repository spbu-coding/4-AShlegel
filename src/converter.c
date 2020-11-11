#include "RWbmp.c"
#include "qdbmp.h"

void BMP_Convert(const char* filein, const char* fileout) {
    BMP_IMAGE* bmp = BMP_Read(filein);
    if (LAST_ERROR_CODE)
        return;

    if ( bmp->header.bit_per_pixel == 24 ) {
        for (unsigned i = 0; i < bmp->header.data_size; i++) {
            bmp->data[i] = ~bmp->data[i];
        }
    }else if ( bmp->header.bit_per_pixel == 8 ) {
        for (unsigned i = 0; i < 256 ; i++) {
            bmp->palette[i<<2]     = ~bmp->palette[i<<2];
            bmp->palette[(i<<2)+1] = ~bmp->palette[(i<<2)+1];
            bmp->palette[(i<<2)+2] = ~bmp->palette[(i<<2)+2];
        }
    }
    BMP_Write( bmp, fileout );
    if (LAST_ERROR_CODE)
        return;
    BMP_Image_Free(bmp);
}

int qdbmp_Convert(const char* filein, const char* fileout){
    UCHAR	r, g, b;
	UINT	width, height;
	UINT	x, y;
	BMP*	bmp = BMP_ReadFile( filein );
	BMP_CHECK_ERROR( stdout, -3 );

	width = BMP_GetWidth( bmp );
	height = BMP_GetHeight( bmp );

    if ( bmp->Palette == NULL && BMP_GetDepth(bmp) == 24){
	    for ( x = 0 ; x < width ; ++x ){
		    for ( y = 0 ; y < height ; ++y ){
		    	BMP_GetPixelRGB( bmp, x, y, &r, &g, &b );
		    	BMP_SetPixelRGB( bmp, x, y, 255 - r, 255 - g, 255 - b );
		    }
	    }
    }else if (BMP_GetDepth(bmp) == 8){
        for (unsigned i = 0; i < 256 ; i++){
            bmp->Palette[i<<2]     = bmp->Palette[i<<2]     ^ 0xFF;
            bmp->Palette[(i<<2)+1] = bmp->Palette[(i<<2)+1] ^ 0xFF;
            bmp->Palette[(i<<2)+2] = bmp->Palette[(i<<2)+2] ^ 0xFF;
        }
    }else{
        BMP_LAST_ERROR_CODE = BMP_FILE_NOT_SUPPORTED;
        fprintf(stderr, "BMP error: %s\n", BMP_GetErrorDescription() );
        BMP_Free( bmp );
		return( -3 );
    }

	BMP_WriteFile( bmp, fileout );
    if (BMP_LAST_ERROR_CODE){
        fprintf(stderr, "BMP error: %s\n", BMP_GetErrorDescription() );
        BMP_Free( bmp );
		return( -3 );
    }
	BMP_Free( bmp );
	return 0;
}

int main(int argc, char **argv){
	if ( argc != 4 ){
		fprintf( stderr, "Usage: %s --key <input file> <output file>\n", argv[0] );
		return -1;
	}
    if (!strcmp(argv[1], "--mine")){
        BMP_Convert(argv[2], argv[3]);
        if (LAST_ERROR_CODE)
            return -(LAST_ERROR_CODE / 8 + 1);
        return 0;
    }else if (!strcmp(argv[1], "--theirs")){
        return qdbmp_Convert(argv[2], argv[3]);
    }else{
        fprintf( stderr, "Usage: --mine or --theirs instead %s\n", argv[1] );
		return -1;
    }
}
