/*
 * crc8.c
 * Karl Zeilhofer
 * 15.2.2010
 *
 * Source-File zu crc8.h
 *
 * Berechnet einen 8-Bit CRC-Code aus einem Daten-Array mit beliebiger Länge
 * Algorithmus von Wikipedia
 */

#include "crc8.h"
#include <stdio.h>

#ifdef CRC8_DEBUG
#undef CRC8_DEBUG
#endif

/*
 * Mit den Nutzdaten wird ein CRC8-Code erzeugt.
 * Wird dieser dann an die Nutzdaten ansgehängt und wieder darüber wieder ein CRC8-Code
 * gebildet, so muss dieser 0 ergeben! Sonst ist ein Fehler aufgetreten.
 */
uint8_t CRC8_run(const uint8_t* data, uint16_t length) // Nutzdaten
// (CRC8=0x00 beim generieren)
{
	int i, b;
	uint8_t crc8 = 0xFF; // Schieberegister mit 0xFF initialisiert
	uint8_t crcData;

#ifdef CRC8_DEBUG
	printf("Data: ");
	for (i = 0; i < length; ++i) { // byte counter
		for (b = 7; b >= 0; b--) { // bit counter
			printf("%d", (int) ((data[i] & (1 << b)) ? 1 : 0));
		}
		printf(" ");
	}
	printf("\n");
#endif

	// crc-algorithmus nach TI-Design-Note DN502 (von Werner Scheiblhofer)
	for (i = 0; i < length; ++i) { // byte counter
		crcData = data[i];
		for (b = 0; b < 8; b++) { // bit counter
			if ((crc8 & 0x80) ^ (crcData & 0x80)) {
				crc8 = (crc8 << 1) ^ CRC8POLY;
			} else {
				crc8 = crc8 << 1;
			}
			crcData <<= 1;
		}
	}

#ifdef CRC8_DEBUG
	printf("CRC-8: ");
	for (b = 7; b >= 0; b--) { // bit counter
		printf("%d", (int) ((crc8 & (1 << b)) ? 1 : 0));
	}
	printf("\n");
#endif

	return crc8;
}

