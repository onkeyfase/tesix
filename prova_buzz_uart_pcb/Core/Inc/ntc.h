#ifndef NTC_H
#define NTC_H

#include "main.h"
#include <stdint.h>

// ─── Configurazione partitore ───────────────────────────────────────────────
#define NTC_COUNT       6
#define NTC_R_FIXED     100000.0f   // R verso GND (100kΩ)
#define NTC_VCC         3.3f        // tensione alimentazione

// ─── Parametri NTC — da completare con datasheet ────────────────────────────
// #define NTC_R_NOMINAL   100000.0f  // resistenza NTC a 25°C (ohm)
// #define NTC_T_NOMINAL   25.0f      // temperatura nominale (°C)
// #define NTC_B_COEFF     3950.0f    // coefficiente B

// ─── API pubblica ────────────────────────────────────────────────────────────

// Inizializza e avvia la prima conversione ADC con interrupt
void NTC_Init(ADC_HandleTypeDef *hadc);

// Ritorna 1 se un ciclo completo di 6 canali è pronto, 0 altrimenti
uint8_t NTC_DataReady(void);

// Resetta il flag data_ready dopo averlo letto
void NTC_ClearFlag(void);

// Ritorna il valore ADC grezzo (0-4095) del canale specificato (0-5)
uint32_t NTC_GetRaw(uint8_t channel);

// Ritorna la resistenza NTC in ohm del canale specificato (0-5)
float NTC_GetResistance(uint8_t channel);

// Da chiamare dentro HAL_ADC_ConvCpltCallback nel main.c
void NTC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif // NTC_H
