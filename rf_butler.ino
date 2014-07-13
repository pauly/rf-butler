/**
 * Uses https://github.com/roberttidey/LightwaveRF
 * and maybe bits of https://github.com/lawrie/LightwaveRF
 * 
 * Arduino home automation experiments. Hardware:
 *  Arduino http://amzn.to/UCKWsq
 *  Ethernet shield http://amzn.to/1akfeTY
 *  LightwaveRF kit: http://amzn.to/RkukDo and http://amzn.to/V7yPPK
 *  RF transmitter and receiver http://bit.ly/HhltyI
 *  Air quality meter from coolcomponents.co.uk
 * 
 * @author PC <paul.clarke+paulclarke@holidayextras.com>
 * @date    Wed 23 Oct 2013 20:57:15 BST
 */

#include <LwRx.h>
#include <LwTx.h>
#include <SPI.h>
#include <Ethernet.h>
// put your twitter.com credentials in Tweet.h - copy Tweet.h.sample - optional and experimental!
#include "Tweet.h"
// put your lightwaverf api host details here
#include "LWRFAPI.h"
#define TIME_BETWEEN_NAGS 60

EthernetClient client; // to make outbound connections
byte mac[] = { 0x64, 0xA7, 0x69, 0x0D, 0x21, 0x21 }; // mac address of this arduino
byte len = 10;
byte lastCommand[10];

#define AIRQUALITYPIN A0

int smell = 0;  // air quality value
long transmitTimeout = 0;
unsigned long lastAlerted = 0;

void setup ( ) {
  Serial.begin( 115200 );
  if ( Ethernet.begin( mac )) {
    Serial.print( F( "DHCP: " ));
    Ethernet.localIP( ).printTo( Serial );
    Serial.println( "" );
  }
  else {
    Serial.println( F( "DHCP configuration failed" ));
  }
  delay( 500 );
  lwrx_setup( 2 );  // set up with rx into pin 2
  lwtx_setup( 3, 10 ); // transmit on pin 3, 10 repeats. transmitter is the small one! http://www.coolcomponents.co.uk/catalogsearch/result/?q=434mhz+transmitter
  Serial.println( F( "Set up completed" ));
}

void loop ( ) {
  if ( lwrx_message( )) {
    byte msg[ len ];
    lwrx_getmessage( msg, &len );
    log( msg, len );
  }
  sniff( );
  delay( 500 );
}

/**
 * Check the air quality
 */
void sniff ( ) {
  smell = analogRead( AIRQUALITYPIN );
  if ( smell > 600 ) {
    Serial.print( F( "air: " ));
    Serial.println( smell );
    if (( lastAlerted == 0 ) || (( millis( ) - lastAlerted ) / 1000 > TIME_BETWEEN_NAGS )) {
      lastAlerted = millis( ); 
      char data[128];
      data[0] = 0;
      strcat( data, "t=something+smells!+air+quality+" );
      char ss[4] = "";
      itoa( smell, ss, 10 );
      strcat( data, ss );
      strcat( data, "&c=" );
      strcat( data, consumer_secret );
      strcat( data, "&a=" );
      strcat( data, access_token_secret );
      tweet( data );

      if ( lwtx_free( )) {
        // message to turn the lights on or something
        byte msg[] = { 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0D, 0x0C, 0x02, 0x08 };
        lwtx_send( msg );
        transmitTimeout = millis( );
      }
      while ( ! lwtx_free( ) && millis( ) < ( transmitTimeout + 1000 )) {
        delay( 10 );
      }
      transmitTimeout = millis( ) - transmitTimeout;
      Serial.print( millis( ));
      Serial.print( F( " msg sent:" ));
      Serial.println( transmitTimeout );

    }    
    delay( 1000 );
  }
}

void tweet ( char *data ) {
  post( tweetHost, tweetPath, data );
}

void post ( char *host, char *path, char *data ) {
  Serial.println( data );  
  if ( client.connect( host, 80 )) {
    delay( 100 );
    client.flush( );
    client.print( F( "POST " ));
    client.print( path );
    client.print( F( "?_=" ));
    client.print( millis( ));
    client.println( F( " HTTP/1.1" ));
    client.print( F( "Host: " ));
    client.println( host );
    client.println( F( "User-Agent: Arduino/1.0" ));
    client.println( F( "Content-Type: application/x-www-form-urlencoded" ));
    client.print( F( "Content-length: " ));
    client.println( strlen( data ));
    client.println( F( "Connection: close" ));
    client.println( );
    client.println( data );
    client.stop( );
  }
  else {
    Serial.print( F( "failed to connect to " ));
    Serial.println( host );
  }
  Serial.print( F( "free memory " ));
  Serial.println( freeRam( ));
}

void log ( byte *msg, byte len ) {
  if ( compare( msg, lastCommand, 0, len )) {
    Serial.print( F( "msg + lastCommand were the same" ));
    return;
  }
  for ( int i = 0; i < len; ++i ) {
    lastCommand[i] = msg[i];
  }
    
  char data[140] = "";
  sprintf(
    data,
    "c=%s&a=%s&t=%x%x%x%x%x%x%x%x%x%x+-+%x%x%x%x%x+set+%x%x+%x+(%x%x)&re=%x%x%x%x%x&ta=%x%x&di=%x&an=%x%x",
    consumer_secret,
    access_token_secret,
    msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9],
    msg[4], msg[5], msg[6], msg[7], msg[8],
    msg[9], msg[2],
    msg[3],
    msg[0], msg[1],
    msg[4], msg[5], msg[6], msg[7], msg[8],
    msg[9], msg[2],
    msg[3],
    msg[0], msg[1]
  );
  Serial.print( F( "Tweeting: " ));
  Serial.println( data );
  Serial.print( freeRam( ));
  Serial.println( F( " bytes free" ));
  delay( 500 );
  tweet( data );
}

/**
 * get a useful string out of an array of bytes
 */
char * tos ( byte * msg, int start, int end ) {
  Serial.print( F( "From " ));
  Serial.print( start );
  Serial.print( F( " to " ));
  Serial.print( end );
  Serial.print( F( "=" ));
  char * name = "";
  name[0] = 0;
  int index = 0;
  for ( int i = start; i < end; i ++ ) {
    name[index++] = alpha( msg[i] );
  }
  name[index++] = '\0';
  Serial.println( name );
  return name;
}

/**
 * get a command string from response
 */
char * command ( byte * msg ) {
  return tos( msg, 0, 4 );
}

/**
  From ScubyD https://github.com/scubyd/ScubyD-LWRF

  Level      2 bit     Device setting
  Device     1 bit     Device ID, relative to room and remote
  Command    1 bit     On/Off/Mood
  Remote ID  5 bit     Remote ID
  Room       1 bit     Room ID, relative to device and remote

  Function          Room  Device  Command Dec Level Hex Level
  Simple On         #     #       1       0         00
  Simple Off        #     #       0       64        40
  Dimmer On         #     #       1       31        1F
  Dimmer 100% (Max) #     #       1       159       9F
  Dimmer 75%        #     #       1       151       97
  Dimmer 50%        #     #       1       143       8F
  Dimmer 25%        #     #       1       135       87
  Dimmer 10%        #     #       1       130       82
  Dimmer 5% (Min)   #     #       1       129       81
  Dimmer Range      #     #       1       129-159   81-9F
  All On (Via Mood) #     F       2       128-132   80-84
  All Off           #     F       0       192       C0
  All Off           #     0       0       192       C0
  Mood              #     F       2       128-132   80-84
*/

/**
 * get device name from response
 */
char * device ( byte * msg ) {
  // byte b[6] = { 0x00, 0x0D, 0x0C, 0x02, 0x08 };
  // if ( compare( msg, b, 5, 10 )) {
  //   return "remote b";
  // }
  return tos( msg, 4, 9 );
}

/**
 * turn a hex value into char for display, like sprintf
 */
char alpha ( byte a ) {
  return ( char )( a < 10 ? a + 48 : a + 55 );
}

boolean compare ( byte a[], byte b[], int start, int end ) {
  for ( int i = start; i < end; ++ i ) {
    Serial.print( F( "a" ));
    Serial.print( i );
    Serial.print( F( "=" ));
    Serial.print( a[i] );
    Serial.print( F( "," ));
    Serial.print( F( "b" ));
    Serial.print( i - start );
    Serial.print( F( "=" ));
    Serial.println( b[i - start] );
    if ( a[i] != b[i - start] ) {
      Serial.print( F( ":-(" ));
      return false;
    }
  }
  return true;
}

/**
 * From http://playground.arduino.cc/Code/AvailableMemory
 */
int freeRam ( ) {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - ( __brkval == 0 ? (int) &__heap_start : (int) __brkval ); 
}
