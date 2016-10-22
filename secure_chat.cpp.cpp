// A simple chat program implementing a client/server system between 2 connected Arduinos 
// which allow users to type text on their command line which is encrypted using the HD public
// key encryption scheme and sent to the other terminal via the Arduino client/server system.
//
//Done by Avery Tan Section A1 
//        Alden Tan Section A2




#include <Arduino.h>
const int pin13 = 13;
const int analogPin = 1;
uint32_t p = 2147483647;
uint32_t g = 16807;

/** Waits for a certain number of bytes on Serial3 or timeout *@param nbytes: the number of bytes we want  * @param timeout: timeout period (ms); * 
return True if the required number of bytes have arrived.*/
bool wait_on_serial3( uint8_t nbytes, long timeout ) {
   unsigned long deadline = millis() + timeout;//wraparound not a problem
   while (Serial3.available()<nbytes && (timeout<0 || millis()<deadline)) {
      delay(1); // be nice, no busy loop
   }
   return Serial3.available()>=nbytes;
}

//converts string to char
int str2int(char* buf){
   return atoi(buf);
}

//generate a random 32 bit key 
uint32_t get_secretkey() {
   int i = 0;
   uint32_t realsecretkey = 0;
   for (i=1;i<=32;i++){
      int randomanalog = analogRead(analogPin);
         if (randomanalog%2 == 0){
            int secretkey = 1;
            realsecretkey= realsecretkey<<1  ; 
            realsecretkey = realsecretkey + secretkey;
         } else {
            int secretkey = 0;
            realsecretkey = realsecretkey <<1;
         }
   }
   return realsecretkey;
}

//modular multiplication function
uint32_t modmult(uint32_t a, uint32_t b, uint32_t m){
  uint32_t result = 0;
  a=a% m;
  b= b%m;
  while (b){
    if (b%2 !=0){
      result = (result+a)%m;
    }
    a = (a*2)%m;
    b =b/2;
  }
  return result;
}

//generating the public key and the shared secret key
uint32_t public_key_process ( uint32_t base, uint32_t power, uint32_t m ) {
   const int NBITS=33;
   uint32_t result = 1; 
   uint32_t x = base % m;
      for (uint32_t i=0; i<NBITS; ++i) {
         if ((power & (1<<i))!=0) {
            result = modmult(result,x,m);
         }
      x = modmult(x,x,m);
      }
   return result;
}



//the black box which generates the stream cipher.
uint32_t next_key(uint32_t current_key) {
   const uint32_t modulus = 0x7FFFFFFF; // 2^31-1
   const uint32_t consta = 48271; // we use that this is <=15 bits
   uint32_t lo = consta*(current_key & 0xFFFF);  
   uint32_t hi = consta*(current_key >> 16); 
   lo += (hi & 0x7FFF)<<16;
   lo += hi>>15;
   if (lo > modulus) lo -= modulus;
   return lo;
}

//The chat program and encryption
char chatprogram(char klow){
   Serial.println();
   Serial.print("You can now chat  ");
   Serial.println();
   while(true){
   /*   
      uint32_t newnumber= next_key(newnumber);

      uint32_t klow = newnumber; 
      */
      klow = klow % 256;


      char firstByte;
      if (Serial3.available()) {
         if ( firstByte== 10 || firstByte == 13) {
            Serial.println();
         }
         firstByte = Serial3.read();

         char decrypted_message = firstByte^klow;

         Serial.write( decrypted_message );

      }
      if (Serial.available()) {
         firstByte = Serial.read();
         if ( firstByte== 10 || firstByte == 13) {
            Serial.println();
         }

         Serial.write( firstByte );

         char encrypted_message = firstByte^klow;

         Serial3.write( encrypted_message );   
      }
   }
}


   /** Reads an uint32_t from Serial3, starting from the least-significant  * and finishing with the most significant byte.  */
uint32_t uint32_from_serial3() {
   uint32_t num = 0;
   num = num | ((uint32_t) Serial3.read()) << 0;
   num = num | ((uint32_t) Serial3.read()) << 8;
   num = num | ((uint32_t) Serial3.read()) << 16;
   num = num | ((uint32_t) Serial3.read()) << 24;
   return num;
}

   /** Writes an uint32_t to Serial3, starting from the least-significant  * and finishing with the most significant byte.  */
void uint32_to_serial3(uint32_t num) {
   Serial3.write((char) (num >> 0));
   Serial3.write((char) (num >> 8));
   Serial3.write((char) (num >> 16));
   Serial3.write((char) (num >> 24));
}


//our state machine. 
uint32_t ClientServerMachine(uint32_t word) {
   pinMode(pin13, INPUT);
   int pin_state = digitalRead(pin13);



   typedef enum {CStart,  CAckWait, CSendAck,  SListen, SWait4Key_1, SSendAck, SAckWait_1, SWait4Key_2, SAckWait_2, CDataX, SDataX } State;
   State state;

   //High when not connected
   if (pin_state == HIGH){
      Serial.println();
      Serial.print("You are the Server");
      Serial.println();
      state = SListen;   
   }

   //Low when connected to ground
   if (pin_state == LOW){
      Serial.println();
      Serial.print("You are the Client");
      Serial.println();
      state = CStart;
   }

   uint32_t ckey = word;

   uint32_t skey = word; 


   while(true){
/*
      Serial.println();
      Serial.print("state is  ");
      Serial.print(state);
      Serial.println();
*/
      if (state == CStart){
         Serial3.write('C');
         uint32_to_serial3(ckey);
         state = CAckWait;
      }

      else if (state == CAckWait){
         if (wait_on_serial3(1,1000) == true){
            char Ack = Serial3.read();
            if(Ack == 'A'){
               state = CSendAck;
            }
         }
         else {
            state = CStart;
         }
      }

      else if (state == CSendAck){
         if (wait_on_serial3(4,1000) == true){
            skey = uint32_from_serial3();
            Serial3.write('A');
            state = CDataX;
         }
         else{
            state = CStart;
         }
      }

      else if (state == SListen){
         char CR = Serial3.read();
         if(CR == 'C'){
            state = SWait4Key_1;
         }
      }

      else if (state == SWait4Key_1){
         if(wait_on_serial3(4,1000)==true){
            ckey = uint32_from_serial3();
            state = SSendAck;
         }
         else {state = SListen;}
      }
      
      else if (state == SSendAck){
         Serial3.write('A');
         uint32_to_serial3(skey);
         state = SAckWait_1;
      }

      else if (state == SAckWait_1){
         if (wait_on_serial3(1,1000)==true){
            char response = Serial3.read();
            if (response == 'A'){
               state = SDataX;
            }
            if (response == 'C'){
               state = SWait4Key_2;
            }
         }
         else {state = SListen;}
      }

      else if (state == SWait4Key_2){
         if (wait_on_serial3(4,1000)==true){
            ckey = uint32_from_serial3();
         }
         else {state = SListen;}
      }

      else if (state == SAckWait_2){
         if (wait_on_serial3(1,1000)==true){
            char response = Serial3.read();
            if (response == 'A'){
               state = SDataX;
            }
            if (response == 'C'){
               state = SWait4Key_2;
            }
         }
         else{state = SListen;}
      }

      else if (state == SDataX){
         Serial.print(" ckey: ");
         Serial.println(ckey);
         return ckey;
      }

      else if (state == CDataX){
         Serial.print(" skey: ");
         Serial.println(skey);
         return skey;
      }
   }
}

int main() {
// Initialise Arduino functionality
   init();

   // Attach USB for applicable processors
   #ifdef USBCON
      USBDevice.attach();
   #endif


   // Add your custom initialization here
   Serial.begin(9600); // Used for communicating with the PC
   Serial3.begin(9600);

   uint32_t random32number = get_secretkey();

   uint32_t keyA = public_key_process(g,random32number,p);

   uint32_t keyB = ClientServerMachine(keyA);

   Serial.println();
   Serial.print("the public key a from your Arduino is   "); Serial.print(keyA);
   Serial.println();


   Serial.println();
   Serial.print("keyB from other Arduino is    "); Serial.print(keyB);
   Serial.println();

   uint32_t shared_keyk = public_key_process(keyB,random32number,p);

   Serial.println();
   Serial.print("shared key k is   "); Serial.print(shared_keyk);
   Serial.println();




   while(true){
      chatprogram(shared_keyk);
   }

   Serial.end();
   return 0;
   }
