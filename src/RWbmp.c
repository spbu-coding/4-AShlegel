#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PRINT_ERROR( error ) \
		{ fprintf( stderr, "error: %s\n", BMP_ERRORS_STRING[ error ] ); \
        LAST_ERROR_CODE = error; }

#define BMP_PALETTE_SIZE ( 256 * 4 )
#define HEADER_PACK_SIZE 27

typedef enum {
	NO_ERROR = 0,
    ALLOC_ERROR,
    FILE_NOT_FOUND,
    BMP_READ_ERROR,
    BMP_FORMAT_ERROR,
    INVALID_ARGUMENT,
    BMP_INVALID_ERROR,
    BMP_WRITE_ERROR,
    BMP_BROKEN_ERROR,
    BMP_SIZE_ERROR,
    BMP_PLANES_ERROR,
    BMP_HEADER_SIZE_ERROR,
    BMP_DEPTH_ERROR,
    BMP_COMPRESSION_ERROR
} BMP_ERRORS;

static BMP_ERRORS LAST_ERROR_CODE = NO_ERROR;

static const char* BMP_ERRORS_STRING[] =
{
	"",
    "Could not allocate enough memory",
    "File not found",
    "Read file error",
    "Incorrect file format. File offset 0x00, 2 bytes: expected to get value 0x4D42 (\"BM\")",
    "Invalid arguments",
    "Invalid palette or data",
    "Write file error",
    "Broken file. File offset 0x06, 4 bytes: expected to get value 0",
    "Broken file. height * width != data size",
    "Broken file. File offset 0x1A, 2 bytes: expected to get value 1",
    "Not supported BMP version - incorrect header size. Expected to get value 40",
    "Not supported BMP version - incorrect the number of bits per pixel. Expected to get value 8 or 24",
    "Not supported compression method. File offset 0x1E, 2 bytes: expected to get value 0"
};

#pragma pack(push, 2)
struct _bmp_header {
	uint16_t	format;         //offset 0x00
	uint32_t	file_size;
	uint32_t	reserved;       //offset 0x06
	uint32_t	data_offset;
	uint32_t	header_size;
	int32_t	    width;
	int32_t     height;
	uint16_t	planes;         //offset 0x1A
	uint16_t	bit_per_pixel;
	uint32_t	compression;    //offset 0x1E
	uint32_t	data_size;
	uint32_t	not_matter_1;
	uint32_t	not_matter_2;
	uint32_t	not_matter_3;
	uint32_t	not_matter_4;
};
#pragma pack(pop)

struct _bmp_image {
	struct _bmp_header	header;
	uint8_t*		    palette;
	uint8_t*		    data;
};

typedef struct _bmp_image BMP_IMAGE;

void BMP_Image_Free( BMP_IMAGE* bmp ) {
	if ( bmp == NULL ) {
		return;
	}
	if ( bmp->palette != NULL ) {
		free( bmp->palette );
	}
	if ( bmp->data != NULL ) {
		free( bmp->data );
	}
	free( bmp );
}


BMP_IMAGE* BMP_Read(const char* filename){
    BMP_IMAGE* bmp;
    FILE* file;
    LAST_ERROR_CODE = NO_ERROR;

    if ( filename == NULL ){
	    PRINT_ERROR(INVALID_ARGUMENT)
        return 0;
    }

    bmp = (BMP_IMAGE*)calloc( 1, sizeof( BMP_IMAGE ) );
    if (bmp == NULL){
        PRINT_ERROR(ALLOC_ERROR)
        return NULL;
    }
    file = fopen(filename, "rb");
    if ( file == NULL ) {
		PRINT_ERROR(FILE_NOT_FOUND)
        free(bmp);
        return NULL;
    }
    if (fread(&bmp->header, sizeof(bmp->header), 1, file) != 1){
        PRINT_ERROR(BMP_READ_ERROR)
        fclose(file);
        free(bmp);
        return NULL;
    }

    if ( bmp->header.format != 0x4D42 )
        PRINT_ERROR(BMP_FORMAT_ERROR)
    if ( bmp->header.reserved )
        PRINT_ERROR(BMP_BROKEN_ERROR)
    if ( bmp->header.header_size != 40 )
        PRINT_ERROR(BMP_HEADER_SIZE_ERROR)
    if ( bmp->header.width < 0 )
        bmp->header.width = -bmp->header.width;
    if ( bmp->header.height < 0 )
        bmp->header.height = -bmp->header.height;
    if ( bmp->header.width *  bmp->header.height * bmp->header.bit_per_pixel / 8 != bmp->header.data_size )
        PRINT_ERROR(BMP_SIZE_ERROR)
    if ( bmp->header.planes != 1 )
        PRINT_ERROR(BMP_PLANES_ERROR)
    if ( bmp->header.bit_per_pixel != 8 && bmp->header.bit_per_pixel != 24 )
        PRINT_ERROR(BMP_DEPTH_ERROR)
    if ( bmp->header.compression )
        PRINT_ERROR(BMP_COMPRESSION_ERROR)

    if ( LAST_ERROR_CODE ){
        fclose(file);
        free(bmp);
        return NULL;
    }
    if ( bmp->header.bit_per_pixel == 8 ) {
        bmp->palette = (uint8_t*) malloc( BMP_PALETTE_SIZE * sizeof( uint8_t ) );
        if ( bmp->palette == NULL ) {
			PRINT_ERROR(ALLOC_ERROR)
			fclose( file );
			free( bmp );
			return NULL;
		}
        if ( fread( bmp->palette, sizeof( uint8_t ), BMP_PALETTE_SIZE, file ) != BMP_PALETTE_SIZE ) {
			PRINT_ERROR( BMP_INVALID_ERROR)
			fclose( file );
			free( bmp->palette );
			free( bmp );
			return NULL;
		}
    }else{
        bmp->palette = NULL;
    }

    bmp->data = (uint8_t*) malloc( bmp->header.data_size );
	if ( bmp->data == NULL ) {
		PRINT_ERROR( ALLOC_ERROR)
		fclose( file );
		free( bmp->palette );
		free( bmp );
		return NULL;
	}
    if ( fread( bmp->data, sizeof( uint8_t ), bmp->header.data_size, file ) != bmp->header.data_size ) {
		PRINT_ERROR( BMP_INVALID_ERROR)
		fclose( file );
		free( bmp->data );
		free( bmp->palette );
		free( bmp );
		return NULL;
	}
	fclose( file );
    return bmp;
}

void BMP_Write(BMP_IMAGE* bmp, const char* filename){
    FILE*	file;
    LAST_ERROR_CODE = NO_ERROR;

    if ( filename == NULL || bmp == NULL){
	    PRINT_ERROR(INVALID_ARGUMENT)
	    return;
	}
    file = fopen( filename, "wb" );
	if ( file == NULL ) {
		PRINT_ERROR(BMP_WRITE_ERROR)
		return;
	}

	if ( fwrite( &bmp->header, sizeof( uint16_t ), HEADER_PACK_SIZE, file ) != HEADER_PACK_SIZE ) {
		PRINT_ERROR(BMP_WRITE_ERROR)
		fclose( file );
		return;
	}

    if ( bmp->header.bit_per_pixel == 8 ){
		if ( fwrite( bmp->palette, sizeof( uint8_t ), BMP_PALETTE_SIZE, file ) != BMP_PALETTE_SIZE ) {
			PRINT_ERROR(BMP_WRITE_ERROR)
			fclose( file );
			return;
		}
	}

    	if ( fwrite( bmp->data, sizeof( uint8_t ), bmp->header.data_size, file ) != bmp->header.data_size ) {
		PRINT_ERROR(BMP_WRITE_ERROR)
		fclose( file );
		return;
	}

	fclose( file );
}
