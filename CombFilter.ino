#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define TEST_TONE

// Use these with the audio adaptor board
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

const int                DRY_SIGNAL_CHANNEL( 0 );
const int                FEED_FORWARD_CHANNEL( 1 );
const int                FEED_BACKWARD_CHANNEL( 2 );

const int                RESONANT_FREQ_PIN( 20 );
const int                RESONANCE_TIME_PIN( 17 );

const float              FREQUENCIES[] =   {  65.41,            // C2
                                              69.30,            // C#2
                                              73.42,            // D2
                                              77.78,            // D#2
                                              82.41,            // E2
                                              87.31,            // F2
                                              92.50,            // F#2
                                              98.00,            // G2
                                              103.83,           // G#2
                                              110.00,           // A2
                                              116.54,           // A#2/Bb2
                                              123.47,           // B2
                                            };


#ifdef TEST_TONE

AudioSynthToneSweep      test_tone;
AudioOutputI2S           audio_output;
AudioControlSGTL5000     sgtl5000_1;

AudioMixer4              delay_mixer;
AudioEffectDelay         feed_forward_delay;
AudioEffectDelay         feed_back_delay;

AudioConnection          patch_cord_1( test_tone, 0, delay_mixer, DRY_SIGNAL_CHANNEL );
AudioConnection          patch_cord_2( delay_mixer, 0, audio_output, 0 );
AudioConnection          patch_cord_3( test_tone, 0, feed_forward_delay, 0 );
AudioConnection          patch_cord_4( feed_forward_delay, 0, delay_mixer, FEED_FORWARD_CHANNEL );
AudioConnection          patch_cord_5( delay_mixer, 0, feed_back_delay, 0 );
AudioConnection          patch_cord_6( feed_back_delay, 0, delay_mixer, FEED_BACKWARD_CHANNEL );

#else // !TEST_TONE

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

#endif // !TEST_TONE

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

  const float delay_time_ms       = calculate_delay_time_ms( FREQUENCIES[0] );
  const float feedback_mult       = calculate_feedback_multiplier( delay_time_ms, 1000.0f );

  feed_forward_delay.delay( 0, delay_time_ms );
  feed_back_delay.delay( 0, delay_time_ms );

  delay_mixer.gain( DRY_SIGNAL_CHANNEL, 0.5f );
  delay_mixer.gain( FEED_BACKWARD_CHANNEL, feedback_mult * 0.0f );
  delay_mixer.gain( FEED_FORWARD_CHANNEL, feedback_mult * 0.0f );

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
#ifdef TEST_TONE
  if( !test_tone.isPlaying() )
  {
    test_tone.play( 0.5f, 100.0f, 1000.0f, 10 );
  }
#endif
  
  static int current_freq_index = 0;

  const int next_freq_index = analogRead( RESONANT_FREQ_PIN ) / ( 1024.0f / 12.0f );

  if( current_freq_index != next_freq_index )
  {
    Serial.print("Freq index:");
    Serial.println( next_freq_index );

    current_freq_index = next_freq_index;

    const float delay_time_ms       = calculate_delay_time_ms( FREQUENCIES[current_freq_index] );
  
    feed_forward_delay.delay( 0, delay_time_ms );
    feed_back_delay.delay( 0, delay_time_ms );

    Serial.print("Delay time ms:");
    Serial.println(delay_time_ms);
  }

  delay( 5 );
}
