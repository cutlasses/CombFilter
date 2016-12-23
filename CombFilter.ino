#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Use these with the audio adaptor board
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

const int                DRY_SIGNAL_CHANNEL( 0 );
const int                FEED_FORWARD_CHANNEL( 1 );
const int                FEED_BACKWARD_CHANNEL( 2 );

AudioInputI2S            audio_input;
AudioOutputI2S           audio_output;
AudioControlSGTL5000     sgtl5000_1;

AudioMixer4              delay_mixer;
AudioEffectDelay         feed_forward_delay;
AudioEffectDelay         feed_back_delay;

AudioConnection          patch_cord_1( audio_input, 0, delay_mixer, DRY_SIGNAL_CHANNEL );
AudioConnection          patch_cord_2( delay_mixer, 0, audio_output, 0 );
AudioConnection          patch_cord_3( audio_input, 0, feed_forward_delay, 0 );
AudioConnection          patch_cord_4( feed_forward_delay, 0, delay_mixer, FEED_FORWARD_CHANNEL );
AudioConnection          patch_cord_5( delay_mixer, 0, feed_back_delay, 0 );
AudioConnection          patch_cord_6( feed_back_delay, 0, delay_mixer, FEED_BACKWARD_CHANNEL );

float calculate_delay_time_ms( float resonant_frequency )
{
  return 1000.0f / resonant_frequency;
}

float calculate_feedback_multiplier( float delay_time_ms, float resonance_time_ms )
{
  const float p = delay_time_ms / resonance_time_ms;
  return powf( 0.001, p );
}

void setup()
{
  AudioMemory( 128 );

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8f);

  sgtl5000_1.lineInLevel( 10 );  // 0.56volts p-p
  sgtl5000_1.lineOutLevel( 13 );  // 3.16volts p-p

  Serial.begin(9600);
  delay( 1000 );

  Serial.print("Setup started!\n");

  const float resonant_frequency  = 261.3f; // C4
  const float resonance_time      = 1000.0f;   // time taken for resonance to drop 60dB

  const float delay_time_ms       = calculate_delay_time_ms( resonant_frequency );
  const float feedback_mult       = calculate_feedback_multiplier( delay_time_ms, resonance_time );

  feed_forward_delay.delay( 0, delay_time_ms );
  feed_back_delay.delay( 0, delay_time_ms );

  delay_mixer.gain( DRY_SIGNAL_CHANNEL, 1.0f );
  delay_mixer.gain( FEED_BACKWARD_CHANNEL, feedback_mult );
  delay_mixer.gain( FEED_FORWARD_CHANNEL, feedback_mult );

/*
  delay_mixer.gain( DRY_SIGNAL_CHANNEL, 0.33f );
  delay_mixer.gain( FEED_BACKWARD_CHANNEL, feedback_mult * 0.33f );
  delay_mixer.gain( FEED_FORWARD_CHANNEL, feedback_mult * 0.33f );
*/

  Serial.print("Delay time ms:");
  Serial.print(delay_time_ms);
  Serial.print(" feedback mult:");
  Serial.print(feedback_mult);
  Serial.print("\nSetup finished!\n");
}

void loop()
{
}
