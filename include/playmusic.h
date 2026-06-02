#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H
#include "set_up.h"
#include "music_node.h"

// Cấu trúc dữ liệu nốt nhạc
typedef struct {
    uint16_t frequency;
    uint16_t duration;
} Note_t;

void Audio_Init(void);
void Delay_ms_Custom(uint16_t ms);
void Play_Tone(uint16_t frequency, uint16_t duration);
void Play_Alarm_Melody(void);

#endif /* AUDIO_PLAYER_H */