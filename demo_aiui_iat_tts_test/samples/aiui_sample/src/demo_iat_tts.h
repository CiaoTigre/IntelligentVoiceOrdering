#ifndef DEMO_IAT_TTS_H
#define DEMO_IAT_TTS_H


/* Reconition result is saved in  g_result */
int SpeechRecognition(void);
int SpeechSynthesis(const char* string);

extern char *g_result;

#endif