int afficheur[] = { A2, A3, A4                 }; // selecteur d'afficheur - CD4028: A B C D=GND
int segment1[]  = { 11, 13, A0, A1, 12, 10,  9 }; // segments Off de l'afficheur - ULN2004A 
int segment0[]  = {  8,  7,  6,  5,  2,  3,  4 }; // segments On de l'afficheur - ULN2004A 

int delai = 50; // la moitié du temps (ms) d'alimentation d'une palette avec alim 12V 2A 

const int buttonPin_AV_Minute  = A7;
const int buttonPin_AV_Heure   = A6;
const int buttonPin_Start_Stop = A5;

bool buttonStart = 0;
bool start = 0;
bool pause = 0;
bool buttonStartLast = false;
long unsigned tstart = 0;
char heure[8];
long unsigned t;
bool buttonMinute, buttonHeure;
int s, m, h, sLast;

bool debug = true;

const bool BCD[12][7] = {
  //abcdefg
  {true ,true ,true ,true ,true ,true ,false}, // 0
  {false,true ,true ,false,false,false,false}, // 1
  {true ,true ,false,true ,true ,false,true }, // 2
  {true ,true ,true ,true ,false,false,true }, // 3
  {false,true ,true ,false,false,true ,true }, // 4
  {true ,false,true ,true ,false,true ,true }, // 5
  {true ,false,true ,true ,true ,true ,true }, // 6
  {true ,true ,true ,false,false,false,false}, // 7
  {true ,true ,true ,true ,true ,true ,true }, // 8
  {true ,true ,true ,true ,false,true ,true }, // 9
  {false,false,false,false,false,false,false}, // 10
  {false,false,false,false,false,false,true }, // 11
};

// les bobines à désactiver
bool BCDlast[5][7];

const bool BCDafficheur[5][3] = {
  //A,B,C,D=GND
  {true ,false,false}, // secondes_unite
  {false,true ,false}, // secondes_dizaine
  {true ,true ,false}, // minutes_unite
  {false,false,true }, // minutes_dizaine
  {true ,false,true }, // heures_unite
};

/* 
 * remise à zéro
 */
void raz() {
  for (int i=11; i>=0; i--) {
    ran(i);
  }  
}

/* 
 * remise à n
 */
void ran(int x) {
  for (int i=0; i<5; i++) {
    affiche(x, i);
  }
  if (debug) Serial.println();
}

int segmentLast = 0;

/* 
 * affiche le nombre x sur la palette n
 */
void affiche(int x, int n) {
  if (debug) {
    Serial.print(" af(");
    Serial.print(x);
    Serial.print(",");
    Serial.print(n);
    Serial.print(")");
  }

  segmentLast = 0; // la derniere bobine activée (donc à desactiver)
  if (x > 11) {
    x = 11;
  }
  if (x < 0) x = 0;
  for (int i=0; i<3; i++) {
    digitalWrite(afficheur[i], BCDafficheur[n][i]); // selection de l'afficheur
  }
  for (int i=6; i>=0; i--) { // commence par la barre du milieu
    if (BCD[x][i] && !BCDlast[n][i]) { // si l'état demandé est différent du précédent et qu'il faut allumer
      digitalWrite(segment1[i], HIGH ); // activation de la bobine
      delay(delai);
      if (segmentLast != 0) digitalWrite(segmentLast, LOW ); // si une bobine a été activée avant alors la descativer
      segmentLast = segment1[i]; // la derniere bobine devient celle qui vient d'être activée
      BCDlast[n][i] = true; // on retient que c'est allumé
    } else if (!BCD[x][i] && BCDlast[n][i]) {  // si l'état demandé est différent du précédent et qu'il faut étaindre
      digitalWrite(segment0[i], HIGH );
      delay(delai);
      if (segmentLast != 0) digitalWrite(segmentLast, LOW );
      segmentLast = segment0[i];
      BCDlast[n][i] = false; // on retient que c'est éteind
    }
  }
  if (segmentLast != 0) { // pour arreter d'alimenter le dernier
    if (debug) Serial.print("#");
    delay(delai);
    digitalWrite(segmentLast, LOW );
    segmentLast = 0;
  }
}


/* 
 * retourne la memoire libre
 */
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
  if (debug) {
    Serial.begin(9600);
    Serial.println("setup");
  }
  for (int i=0; i<7; i++) {
    pinMode(segment0[i], OUTPUT);
    pinMode(segment1[i], OUTPUT);
  }
  for (int i=0; i<3; i++) {
    pinMode(afficheur[i], OUTPUT);
  }
  pinMode(buttonPin_AV_Minute , INPUT); //A7 only analog
  pinMode(buttonPin_AV_Heure  , INPUT); //A6 only analog
  pinMode(buttonPin_Start_Stop, INPUT_PULLUP);
  ran(11);
  ran(0);
  delay(20);
}

void loop() {
  buttonStart = !( (bool) digitalRead(buttonPin_Start_Stop) );
  if (buttonStart && !buttonStartLast) {
    start = !start;
    if (start == true) {
      if (tstart == 0) tstart = millis();
      pause = false;
    } else {
      pause = true;
    }
    delay(20);
  } else if (!buttonStart && buttonStartLast) {
    delay(20);
  }
  buttonStartLast = buttonStart;
  if (pause) {
    delay(20);
    
    buttonMinute = (analogRead(buttonPin_AV_Minute) < 16);
    buttonHeure = (analogRead(buttonPin_AV_Heure) < 16);

    if (debug) {
      Serial.println();
      Serial.print("buttonMinute: ");
      Serial.print(buttonMinute);
      Serial.print(" buttonHeure: ");
      Serial.print(buttonHeure);
      Serial.print(" buttonStart: ");
      Serial.print(buttonStart);
      delay(200);
    }
  
    if (buttonMinute && buttonHeure && pause) {
      tstart = 0;
      raz();
    }
  }

  if (start && !pause) {
    t = (millis()-tstart) / 1000;
    s = t % 60;            // secondes
    m = (t-s) /60;         // minutes
    m = m % 60;
    h = (t-m*60-s) /60/60; // heures
    if (s != sLast) {
      if (debug) {
        sprintf(heure, "%02d:%02d:%02d", h, m, s);
        Serial.println();
        Serial.print(heure);
        Serial.print(" fR: ");
        Serial.print(freeRam());
      }
      affiche( ( s    ) % 10 , 0); // secondes_unite
      affiche( ( s/10 ) % 10 , 1); // secondes_dizaine
      affiche( ( m    ) % 10 , 2); // minutes_unite
      affiche( ( m/10 ) % 10 , 3); // minutes_dizaine
      affiche( ( h    ) % 10 , 4); // heures_unite
      sLast = s;
    }
  }
}
