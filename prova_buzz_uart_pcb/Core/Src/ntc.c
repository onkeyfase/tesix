#include "ntc.h"
#include <math.h>

// ─── Variabili private ───────────────────────────────────────────────────────

static ADC_HandleTypeDef *hadc_ptr       = NULL;
static uint32_t           adc_buffer[NTC_COUNT]; // buffer che salva i 6 valori, marco diceva di farlo da 12 (boh)
// queste due variabili vengono scritte nel callback (cos'è il callback?)
static volatile uint8_t   current_channel = 0;
static volatile uint8_t   data_ready      = 0;

// ─── Implementazione ─────────────────────────────────────────────────────────

/*
salva il puntatore all'ADC ed azzera lo stato, e avvia la prima conversione con interrupt
HAL_ADC_Start_IT() fa partire una singola conversione e scatta il callback. ne fa uno alla volta (non 6 insieme)
*/
void NTC_Init(ADC_HandleTypeDef *hadc)
{
    hadc_ptr        = hadc;
    current_channel = 0;
    data_ready      = 0;

    HAL_ADC_Start_IT(hadc);
}

uint8_t NTC_DataReady(void)
{
    return data_ready;
}

void NTC_ClearFlag(void)
{
    data_ready = 0;
}

uint32_t NTC_GetRaw(uint8_t channel)
{
    if (channel >= NTC_COUNT) return 0;
    return adc_buffer[channel];
}
/*

*/
float NTC_GetResistance(uint8_t channel)
{
    if (channel >= NTC_COUNT) return -1.0f;

    uint32_t raw   = adc_buffer[channel]; //valore grezzo digitale 0 - 4095
    float    v_adc = (raw / 4095.0f) * NTC_VCC; //converto il valore grezzo in tensione reale

    // Protezione divisione per zero — se v_adc è 0 l'NTC è aperto
    if (v_adc <= 0.0f) return -1.0f;

    // Partitore: NTC verso 3.3V, R_fixed (100kΩ) verso GND
    // R_NTC = R_fixed * (Vcc - V_adc) / V_adc
    float r_ntc = NTC_R_FIXED * (NTC_VCC - v_adc) / v_adc; //formula inversa per calcolare rNTC

    return r_ntc;
}

void NTC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance != ADC1) return;

    // leggi e salva il valore convertito nel buffer non appena l'adc finisce una conversione
    adc_buffer[current_channel] = HAL_ADC_GetValue(hadc);
    current_channel++; //incrementa

    if (current_channel < NTC_COUNT)
    {
        // Ciclo non ancora completo — avvia la prossima conversione
        HAL_ADC_Start_IT(hadc);
    }
    else
    {
        // appena tutti e 6 i canali letti
        current_channel = 0;
        data_ready      = 1;

        // ricomincia il ciclo azzerando il contatore e mettendo data ready a 1
        HAL_ADC_Start_IT(hadc);
    }
}

float NTC_GetTemperature(uint8_t channel)
{
    float r_ntc = NTC_GetResistance(channel);
    if (r_ntc <= 0.0f) return -273.15f; // Valore di errore

    // costanti tipiche (da verificare sul datasheet
    const float BETA = 3950.0f;    // Beta
    const float R0   = 10000.0f;   // resistenza a 25°C (10k), non ho capito se r0 è un valore fixed o se è r=100k
    const float T0   = 298.15f;    // 25°C espressi in kelvin

    float temperatura_k;
    // Formula inversa: 1/T = 1/T0 + (1/Beta) * ln(R/R0)
    temperatura_k = logf(r_ntc / R0);
    temperatura_k /= BETA;
    temperatura_k += (1.0f / T0);
    temperatura_k = 1.0f / temperatura_k; //inverso e ottengo la temperatura effettiva

    return temperatura_k - 273.15f; // Conversione da Kelvin a Celsius
}
