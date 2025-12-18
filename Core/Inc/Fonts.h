/*
 * Define used font's here
 * Decribe fonts params here
 *
 */
#pragma once

#include "stdint.h"

typedef struct {
	uint8_t wide;    	  /*!< Font width in pixels */
	uint8_t hight;   	  /*!< Font height in pixels */
	uint8_t ASCII_OFFSET; // offset value for ascii interpretation
	uint8_t *data; 		  /*!< Pointer to data font data array */
	uint16_t (*params)[2]; 	  /* Pointer to array with description of every char */
} FontDefine;

extern FontDefine Font_19;
extern FontDefine Font_13;

extern const uint8_t decode_utf[256];