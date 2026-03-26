#include "ntc.h"
#include <math.h>

// ─── Variabili private ───────────────────────────────────────────────────────

static ADC_HandleTypeDef *hadc_ptr       = NULL;
static uint32_t           adc_buffer[NTC_COUNT];
static volatile uint8_t   current_channel = 0;
static volatile uint8_t   data_ready      = 0;

// ─── Implementazione ─────────────────────────────────────────────────────────

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

float NTC_GetResistance(uint8_t channel)
{
    if (channel >= NTC_COUNT) return -1.0f;

    uint32_t raw   = adc_buffer[channel];
    float    v_adc = (raw / 4095.0f) * NTC_VCC;

    // Protezione divisione per zero — se v_adc è 0 l'NTC è aperto
    if (v_adc <= 0.0f) return -1.0f;

    // Partitore: NTC verso 3.3V, R_fixed (100kΩ) verso GND
    // R_NTC = R_fixed * (Vcc - V_adc) / V_adc
    float r_ntc = NTC_R_FIXED * (NTC_VCC - v_adc) / v_adc;

    return r_ntc;
}

void NTC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance != ADC1) return;

    // Salva il valore convertito nel buffer
    adc_buffer[current_channel] = HAL_ADC_GetValue(hadc);
    current_channel++;

    if (current_channel < NTC_COUNT)
    {
        // Ciclo non ancora completo — avvia la prossima conversione
        HAL_ADC_Start_IT(hadc);
    }
    else
    {
        // Tutti e 6 i canali letti
        current_channel = 0;
        data_ready      = 1;

        // Ricomincia il ciclo immediatamente
        HAL_ADC_Start_IT(hadc);
    }
}
