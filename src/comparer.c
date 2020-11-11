#include "RWbmp.c"

void BMP_GetRGB(BMP_IMAGE* bmp, uint32_t x, uint32_t y, uint8_t* r, uint8_t* g, uint8_t* b ) {
    uint8_t*	pixel;
	uint32_t	bytes_per_row;
	uint8_t	    bytes_per_pixel;

    if ( bmp == NULL || x < 0 || x >= bmp->header.width || y < 0 || y >= bmp->header.height ) {
		PRINT_ERROR(INVALID_ARGUMENT);
	}else {
		LAST_ERROR_CODE = NO_ERROR;
		bytes_per_pixel = bmp->header.bit_per_pixel >> 3;
		bytes_per_row = bmp->header.data_size / bmp->header.height;
		pixel = bmp->data + ( ( bmp->header.height - y - 1 ) * bytes_per_row + x * bytes_per_pixel );
		if ( bmp->palette != NULL ) {
			pixel = bmp->palette + *pixel * 4;
		}
		if ( r )	*r = *( pixel + 2 );
		if ( g )	*g = *( pixel + 1 );
		if ( b )	*b = *( pixel + 0 );
	}
}

int BMP_Compare(const char* file1, const char* file2) {
    BMP_IMAGE* bmp1 = BMP_Read(file1);
    BMP_IMAGE* bmp2 = BMP_Read(file2);
    if (LAST_ERROR_CODE)
        return -1;

    int count = 0;
    if (bmp1->header.width != bmp2->header.width || bmp1->header.height != bmp2->header.height) {
        BMP_Image_Free(bmp1);
        BMP_Image_Free(bmp2);
        fprintf( stderr, "error: not comparable. Different image sizes\n");
        return -1;
    }else{
        uint8_t r1, g1, b1, r2, g2, b2;
        for (uint32_t x = 0; x < bmp1->header.width; x++) {
            for (uint32_t y = 0; y < bmp1->header.height; y++) {
                BMP_GetRGB(bmp1, x, y, &r1, &g1, &b1);
                BMP_GetRGB(bmp2, x, y, &r2, &g2, &b2);
                if(r1 != r2 || g1 != g2 || b1 != b2) {
                    count++;
                    fprintf(stderr, "%ld %ld\n", x, y);
                }
                if (count == 100){
                    BMP_Image_Free(bmp1);
                    BMP_Image_Free(bmp2);
                    return 1;
                }
            }
        }
    }
    BMP_Image_Free(bmp1);
    BMP_Image_Free(bmp2);
    if (count)
        return 1;
    else
        return 0;
}

int main(int argc, char **argv){
	if ( argc != 3 ){
		fprintf( stderr, "Usage: %s <input file> <output file>\n", argv[0] );
		return -1;
	}
    return BMP_Compare(argv[1], argv[2]);
}
