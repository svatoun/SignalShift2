typedef LightFunction AspectDefinition[maxOutputsPerMast];
typedef byte AspectDefinitionBytes[maxOutputsPerMast];
typedef AspectDefinitionBytes SignalSet32[32];

#define STRIP_OFF LOFF, LOFF, LOFF, LOFF, LOFF
#define STRIP_40 STRIP_OFF
#define STRIP_60 LOFF, LOFF, LON, LOFF, LOFF
#define STRIP_80 LOFF, LON, LOFF, LOFF, LOFF
#define B54 B(blinking54)
#define B108 B(blinking108)
#define B22 B(blinking22)
#define L___ LOFF

const SignalSet32 csdBasicAspects PROGMEM = {
  { L___, L___, LON, L___, L___, STRIP_OFF },   // Aspect 0: Stuj
  { L___, LON, L___, L___, L___, STRIP_OFF },   // Aspect 1: Volno
  { LON, L___, L___, L___, L___, STRIP_OFF },   // Aspect 2: Výstraha,
  { B54, L___, L___, L___, L___, STRIP_OFF },   // Aspect 3: Očekávej 40
  { B108, L___, L___, L___, L___, STRIP_OFF },  // Aspect 4: Očekávej 60
  { L___, B54, L___, L___, L___, STRIP_OFF },   // Aspect 5: Očekávej 80
  { L___, L___, L___, L___, LON, STRIP_OFF },   // Aspect 6: 40 a volno
  { LON, L___, L___, L___, LON, STRIP_OFF },    // Aspect 7: 40 a výstraha
  { B54, L___, L___, L___, LON, STRIP_OFF },    // Aspect 8: 40 a očekávej 40
  { B108, L___, L___, L___, LON, STRIP_OFF },   // Aspect 9: 40 a očekávej 60
  { L___, B54, L___, L___, LON, STRIP_OFF },    // Aspect 10: 40 a očekávej 80
  { L___, LON, L___, L___, LON, STRIP_60 },     // Aspect 11: 60 a volno
  { LON, L___, L___, L___, LON, STRIP_60 },     // Aspect 12: 60 a výstraha
  { B54, L___, L___, L___, LON, STRIP_60 },     // Aspect 13: 60 a očekávej 40
  { B108, L___, L___, L___, LON, STRIP_60 },    // Aspect 14: 60 a očekávej 60
  { L___, B54, L___, L___, LON, STRIP_60 },     // Aspect 15: 60 a očekávej 80
  { L___, LON, L___, L___, LON, STRIP_80 },     // Aspect 16: 80 a volno
  { LON, L___, L___, L___, LON, STRIP_80 },     // Aspect 17: 80 a výstraha
  { B54, L___, L___, L___, LON, STRIP_80 },     // Aspect 18: 80 a očekávej 40
  { B108, L___, L___, L___, LON, STRIP_80 },    // Aspect 19: 80 a očekávej 60
  { L___, B54, L___, L___, LON, STRIP_80 },     // Aspect 20: 80 a očekávej 80
  { L___, LON, L___, LON, L___, STRIP_OFF },    // Aspect 21: Opakovaná volno
  { LON, L___, L___, LON, L___, STRIP_OFF },    // Aspect 22: Opakovaná výstraha
  { B54, L___, L___, LON, L___, STRIP_OFF },    // Aspect 23: Opakovaná očekávej 40
  { B108, L___, L___, LON, L___, STRIP_OFF },   // Aspect 24: Opakovaná očekávej 60
  { L___, B54, L___, LON, L___, STRIP_OFF },    // Aspect 25: Opakovaná očekávej 80
  { LON, LON, LON, LON, LON, LON, LON, LON, LON, LON },  // Aspect 26: ----------------------------
  { STRIP_OFF, LON, L___, L___, L___, L___ },   // Aspect 27: Posun zakázán
  { L___, L___, L___, LON, L___, STRIP_OFF },   // Aspect 28: Posun dovolen
  { L___, L___, LON, LON, L___, STRIP_OFF },    // Aspect 29: Posun dovolen - nezabezpečený
  { L___, L___, L___, B54, L___, STRIP_OFF },   // Aspect 30: Opatrně na přivolávací návěst bez červené
  { L___, L___, LON, B54, L___, STRIP_OFF },    // Aspect 31: Opatrně na přivolávací návěst
};

const SignalSet32 csdIntermediateAspects PROGMEM = {
  { L___, L___, LON, L___, L___, STRIP_OFF },  // Aspect 0: Stuj
  { L___, LON, L___, L___, L___, STRIP_OFF },  // Aspect 1: Volno
  { LON, L___, L___, L___, L___, STRIP_OFF },  // Aspect 2: Výstraha,
  { B54, L___, L___, L___, L___, STRIP_OFF },  // Aspect 3: Očekávej 40
  { L___, LON, L___, LON, LON, STRIP_OFF },    // Aspect 4: 40 a opakovaná volno             *
  { L___, LON, L___, LON, LON, STRIP_60 },     // Aspect 5: 60 a opakovaná volno             *
  { L___, L___, L___, L___, LON, STRIP_OFF },  // Aspect 6: 40 a volno
  { LON, L___, L___, L___, LON, STRIP_OFF },   // Aspect 7: 40 a výstraha
  { B54, L___, L___, L___, LON, STRIP_OFF },   // Aspect 8: 40 a očekávej 40
  { L___, LON, L___, LON, LON, STRIP_80 },     // Aspect 9: 80 a opakovaná volno             *
  { LON, L___, L___, LON, LON, STRIP_OFF },    // Aspect 10: 40 a opakovaná výstraha         *
  { L___, LON, L___, L___, LON, STRIP_60 },    // Aspect 11: 60 a volno
  { B54, L___, L___, LON, LON, STRIP_OFF },    // Aspect 12: 40 a opakovaná očekávej 40      *
  { B108, L___, L___, LON, LON, STRIP_OFF },   // Aspect 13: 40 a opakovaná očekávej 60      *
  { L___, B54, L___, L___, LON, STRIP_OFF },   // Aspect 14: 40 a opakovaná očekávej 80      *
  { LON, L___, L___, LON, LON, STRIP_60 },     // Aspect 15: 60 a opakovaná výstraha         *
  { L___, LON, L___, L___, LON, STRIP_80 },    // Aspect 16: 80 a volno
  { B54, L___, L___, L___, LON, STRIP_60 },    // Aspect 17: 60 a opakovaná očekávej 40      *
  { B108, L___, L___, L___, LON, STRIP_60 },   // Aspect 18: 60 a opakovaná očekávej 60      *
  { L___, B54, L___, L___, LON, STRIP_60 },    // Aspect 19: 60 a opakovaná očekávej 80      *
  { LON, L___, L___, LON, LON, STRIP_80 },     // Aspect 20: 80 a opakovaná výstraha         *
  { L___, LON, L___, LON, L___, STRIP_OFF },   // Aspect 21: Opakovaná volno
  { LON, L___, L___, LON, L___, STRIP_OFF },   // Aspect 22: Opakovaná výstraha
  { B54, L___, L___, LON, L___, STRIP_OFF },   // Aspect 23: Opakovaná očekávej 40
  { B108, L___, L___, LON, L___, STRIP_OFF },  // Aspect 24: Opakovaná očekávej 60
  { L___, B54, L___, LON, L___, STRIP_OFF },   // Aspect 25: Opakovaná očekávej 80
  { B54, L___, L___, LON, LON, STRIP_80 },     // Aspect 26: 80 a opakovaná očekávej 40      *
  { STRIP_OFF, STRIP_OFF },                    // Aspect 27: ----------------------------
  { L___, L___, L___, LON, L___, STRIP_OFF },  // Aspect 28: Posun dovolen
  { B108, L___, L___, LON, LON, STRIP_80 },    // Aspect 29: 80 a opakovaná očekávej 60
  { L___, L___, L___, B54, L___, STRIP_80 },   // Aspect 30: 80 a opakovaná očekávej 80
  { L___, L___, LON, B54, L___, STRIP_OFF },   // Opatrně na přivolávací návěst
};

const SignalSet32 csdEmbeddedAspects PROGMEM = {
  { L___, L___, LON, L___, L___, STRIP_OFF },                     // Aspect 0: Stuj
  { L___, LON, L___, L___, L___, STRIP_OFF },                     // Aspect 1: Volno
  { LON, L___, L___, L___, L___, STRIP_OFF },                     // Aspect 2: Výstraha,
  { B54, L___, L___, L___, L___, STRIP_OFF },                     // Aspect 3: Očekávej 40
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 4: ----------------------------           *
  { L___, L___, L___, B22, L___, STRIP_OFF },                     // Aspect 5: Odjezdové návěstidlo dovoluje jízdu    *
  { L___, LON, L___, L___, L___, LON, L___, L___, L___, L___ },   // Aspect 6: Stůj s modrou                          *
  { LON, L___, L___, L___, LON, STRIP_OFF },                      // Aspect 7: 40 a výstraha
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 8:  ---------------------------           *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 9:  ---------------------------           *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 10: ---------------------------           *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 11: ---------------------------           *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 12: ---------------------------           *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 13: ---------------------------           *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 14: ---------------------------           *
  { STRIP_OFF, LON, L___, L___, L___, L___ },                     // Aspect 15: Sunout zakázáno opakovaná             *
  { L___, LON, L___, L___, L___, STRIP_OFF },                     // Aspect 16: Sunout zakázáno                       *
  { L___, L___, L___, LON, L___, L___, L___, L___, LON, L___ },   // Aspect 17: Sunout pomalu                         *
  { L___, L___, L___, LON, L___, STRIP_OFF },                     // Aspect 18: Sunout rychleji                       *
  { L___, LON, L___, L___, L___, L___, L___, L___, L___, LON },   // Aspect 19: Zpět                                  *
  { STRIP_OFF, LON, L___, L___, L___, LON },                      // Aspect 20: Zpět opakovaná                        *
  { L___, LON, L___, LON, L___, STRIP_OFF },                      // Aspect 21: Opakovaná volno
  { LON, L___, L___, LON, L___, STRIP_OFF },                      // Aspect 22: Opakovaná výstraha
  { B54, L___, L___, LON, L___, STRIP_OFF },                      // Aspect 23: Opakovaná očekávej 40
  { L___, L___, L___, B54, L___, L___, L___, L___, B54, L___ },   // Aspect 24: Přísun soupravy vozidel ke spádovišti *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 25: Posun zakázán opakovaná               *
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 26: Na spádovišti se neposunuje           *
  { L___, L___, L___, L___, L___, LON, L___, L___, L___, L___ },  // Aspect 27: Posun zakázán
  { L___, L___, L___, LON, L___, STRIP_OFF },                     // Aspect 28: Posun dovolen
  { L___, L___, LON, LON, L___, STRIP_OFF },                      // Aspect 29: Posun dovolen - nezabezpečený
  { L___, L___, L___, B54, L___, STRIP_OFF },                     // Aspect 30: Opatrně na přivolávací návěst bez červené
  { L___, L___, LON, B54, L___, STRIP_OFF },                      // Aspect 31: Opatrně na přivolávací návěst
};

const SignalSet32 szdcBasicAspects PROGMEM = {
  { L___, L___, LON, L___, L___, STRIP_OFF },                     // Aspect 0: Stuj
  { L___, LON, L___, L___, L___, STRIP_OFF },                     // Aspect 1: Volno
  { LON, L___, L___, L___, L___, STRIP_OFF },                     // Aspect 2: Výstraha,
  { B54, L___, L___, L___, L___, STRIP_OFF },                     // Aspect 3: Očekávej 40
  { B108, L___, L___, L___, L___, STRIP_OFF },                    // Aspect 4: Očekávej 60
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 5: -----------------------                *
  { L___, L___, L___, L___, LON, STRIP_OFF },                     // Aspect 6: 40 a volno
  { LON, L___, L___, L___, LON, STRIP_OFF },                      // Aspect 7: 40 a výstraha
  { B54, L___, L___, L___, LON, STRIP_OFF },                      // Aspect 8: 40 a očekávej 40
  { B108, L___, L___, L___, LON, STRIP_OFF },                     // Aspect 9: 40 a očekávej 60
  { L___, B54, L___, L___, LON, STRIP_OFF },                      // Aspect 10: 40 a očekávej 80
  { L___, LON, L___, L___, LON, STRIP_60 },                       // Aspect 11: 60 a volno
  { LON, L___, L___, L___, LON, STRIP_60 },                       // Aspect 12: 60 a výstraha
  { B54, L___, L___, L___, LON, STRIP_60 },                       // Aspect 13: 60 a očekávej 40
  { B108, L___, L___, L___, LON, STRIP_60 },                      // Aspect 14: 60 a očekávej 60
  { L___, B54, L___, L___, LON, STRIP_60 },                       // Aspect 15: 60 a očekávej 80
  { LON, L___, L___, LON, LON, STRIP_OFF },                       // Aspect 16: 40 a opakovaná výstraha               *
  { B54, L___, L___, LON, LON, STRIP_OFF },                       // Aspect 17: 40 a opakovaná očekávej 40            *
  { B108, L___, L___, LON, LON, STRIP_OFF },                      // Aspect 18: 40 a opakovaná očekávej 60            *
  { LON, L___, L___, B54, L___, STRIP_OFF },                      // Aspect 19: Jízda podle rozhledových poměrů       *
  { LON, L___, L___, B54, LON, STRIP_OFF },                       // Aspect 20: 40 a jízda podle rozhledových poměrů  *
  { L___, LON, L___, LON, L___, STRIP_OFF },                      // Aspect 21: Opakovaná volno
  { LON, L___, L___, LON, L___, STRIP_OFF },                      // Aspect 22: Opakovaná výstraha
  { B54, L___, L___, LON, L___, STRIP_OFF },                      // Aspect 23: Opakovaná očekávej 40
  { B108, L___, L___, LON, L___, STRIP_OFF },                     // Aspect 24: Opakovaná očekávej 60
  { STRIP_OFF, STRIP_OFF },                                       // Aspect 25: -----------------------
  { STRIP_OFF, B108, L___, L___, L___, L___ },                    // Aspect 26: Jízda vlaku dovolena                  *
  { L___, L___, L___, L___, L___, LON, L___, L___, L___, L___ },  // Aspect 27: Posun zakázán
  { L___, L___, L___, LON, L___, STRIP_OFF },                     // Aspect 28: Posun dovolen
  { L___, L___, LON, LON, L___, STRIP_OFF },                      // Aspect 29: Posun dovolen - nezabezpečený
  { L___, L___, L___, B54, L___, STRIP_OFF },                     // Aspect 30: Opatrně na přivolávací návěst bez červené
  { L___, L___, LON, B54, L___, STRIP_OFF },                      // Aspect 31: Opatrně na přivolávací návěst
};

const SignalSet32 csdMechanicalAspects PROGMEM = {
  { L___, L___, LON, L___, L___, STRIP_OFF },  // Aspect 0: Stuj
  { STRIP_OFF, STRIP_OFF },                    // Aspect 1:  ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 2:  ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 3:  ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 4: ----------------------------
  { L___, L___, B22, L___, STRIP_OFF },        // Aspect 5: Odjezdové návěstidlo dovoluje jízdu
  { STRIP_OFF, STRIP_OFF },                    // Aspect 6: ----------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 7: ----------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 8: ----------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 9: ----------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 10: ----------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 11: ----------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 12: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 13: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 14: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 15: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 16: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 17: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 18: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 19: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 20: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 21: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 22: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 23: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 24: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 25: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 26: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 27: ---------------------------
  { L___, L___, L___, LON, L___, STRIP_OFF },  // Aspect 28: Posun dovolen
  { STRIP_OFF, STRIP_OFF },                    // Aspect 29: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 30: ---------------------------
  { STRIP_OFF, STRIP_OFF },                    // Aspect 31: ---------------------------
};

const char TYPE_0_NAME[] PROGMEM = "o4";
const char TYPE_1_NAME[] PROGMEM = "v5";
const char TYPE_2_NAME[] PROGMEM = "v4p";
const char TYPE_3_NAME[] PROGMEM = "v4o";
const char TYPE_4_NAME[] PROGMEM = "o3r";
const char TYPE_5_NAME[] PROGMEM = "o3o";
const char TYPE_6_NAME[] PROGMEM = "odd";

const char TYPE_10_NAME[] PROGMEM = "j2";
const char TYPE_11_NAME[] PROGMEM = "j3p";
const char TYPE_12_NAME[] PROGMEM = "j3o";

const char TYPE_15_NAME[] PROGMEM = "p1";
const char TYPE_16_NAME[] PROGMEM = "p2o";
const char TYPE_17_NAME[] PROGMEM = "pv2";
const char TYPE_18_NAME[] PROGMEM = "pv3o";

const char TYPE_22_NAME[] PROGMEM = "kon";
const char TYPE_23_NAME[] PROGMEM = "opo";
const char TYPE_24_NAME[] PROGMEM = "ser";
const char TYPE_25_NAME[] PROGMEM = "spc";
const char TYPE_26_NAME[] PROGMEM = "spm";

const char TYPE_E_NAME[] PROGMEM = "";

const MastTypeNameId mastTypeNames[] PROGMEM = {
  { 1, TYPE_0_NAME }, 
  { 2, TYPE_1_NAME }, 
  { 3, TYPE_2_NAME }, 
  { 4, TYPE_3_NAME }, 
  { 5, TYPE_4_NAME },
  { 6, TYPE_5_NAME },
  { 7, TYPE_6_NAME },

  { 11, TYPE_10_NAME },
  { 12, TYPE_11_NAME },
  { 13, TYPE_12_NAME },
  
  { 16, TYPE_15_NAME },
  { 17, TYPE_16_NAME },
  { 18, TYPE_17_NAME },
  { 19, TYPE_18_NAME },
  
  { 23, TYPE_22_NAME },
  { 24, TYPE_23_NAME },
  { 25, TYPE_24_NAME },
  { 26, TYPE_25_NAME },
  { 27, TYPE_26_NAME },
 
  { 0, NULL }
};

byte mastTypeNameCount = sizeof(mastTypeNames) / sizeof(mastTypeNames[0]);


const MastTypeDefinition mastTypeDefinitions[32] PROGMEM = {
  // ---------------- typ 0/128: Hlavní: Odjezdové - ZČBŽ; 8 znaku
  {
    8, 4, SIGNAL_SET_CSD_BASIC, 0, 
    { 0, 1, 2, 3, 4 },
    {
      1,    // stuj
      2,    // volno
      7,    // 40 a volno (dolni zluta)
      29,   // posun
      // --------------------------------
      32,   // opatrne na privolavaci navest
      30,   // posun dovolen - nezabezpeceny
      0,    // test - zhasnuto
      255,  // test - vse rozsviceno
    }
  },
  // ---------------- typ 1/129: Hlavní: Vjezdové - ŽZČBŽ; 12 znaku
  // Typ #1: 5-svetelne, vjezdove, zluta-zelena-cervena-bila-zluta
  {
    // Pocet kodu (max 32), pocet svetel (max 10), sada aspektu (viz SignalSet), vychozi kod (0.. pocet-kodu - 1).
    12, 5, SIGNAL_SET_CSD_BASIC, 0, 
    // Kontakty pro jednotlive lampy, cislovane od 1 do 10 (vcetne). 0 znamena ze se svetlo nepouzije.
    { 1, 2, 3, 4, 5 },
    // Jednotlive aspekty, cislovane od 1. Cislo kodu se prenasi budto 
    // - extended packetem prislusenstvi, nebo
    // - pomoci 2-koveho rozvoje na DCC adres navestidla, kde "primo" znaci 0 a "do odbocky" znaci 1; navestidlo obsadi vzdy tolik adres, aby 
    // - pomoci ovladani "primo", "do odbocky" vyhybek na DCC adresach navestidla, kde kody odpovidaji postupne:
    //   1. adresa "primo", 1. adresa "odbocka", 2. adresa "primo", 2. adresa "odbocka", 3. adresa "primo", ... az do vycerpani urceneho poctu kodu.
    {
      1,    // stuj
      2,    // volno
      7,    // 40 a volno (dolni zluta)
      29,   // posun
      // --------------------------------
      3,    // vystraha
      4,    // ocekavej 40
      8,    // 40 a vystraha
      9,    // 40 a ocekavej 40
      32,   // opatrne na privolavaci navest
      30,   // posun dovolen - nezabezpeceny
      0,    // test - zhasnuto
      255,  // test - vse rozsviceno
    }
  },
  // ---------------- typ 2/130: Hlavní: Vjezd: Jen přímo - ŽZČB; 8 znaku
  {
    8, 4, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 2, 3, 4 },
    {
      1,    // stuj
      2,    // volno
      2,    // 40 a volno (dolni zluta) => volno
      29,   // posun
      // --------------------------------
      3,    // vystraha
      4,    // ocekavej 40
      32,   // opatrne na privolavaci navest
      30,   // posun dovolen - nezabezpeceny
    }
  },
  // ---------------- typ 3/131: Hlavní: Vjezd: Jen odbočkou - ŽČBŽ; 8 znaku
  {
    8, 4, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 0, 2, 3, 4 },
    {
      1,    // stuj
      7,    // volno -> 40 a volno (nepouzito)
      7,    // 40 a volno 
      29,   // posun
      // --------------------------------
      8,    // vystraha -> 40 a vystraha
      9,    // ocekavej 40 -> 40 a ocekavej 40
      32,   // opatrne na privolavaci navest
      30,   // posun dovolen - nezabezpeceny
    }
  },
  // ---------------- typ 4/132: Hlavní: Odjezd: Jen rovne - ZČB; 8 znaku
  {
    8, 3, SIGNAL_SET_CSD_BASIC, 0, 
    { 0, 1, 2, 3 },
    {
      1,    // stuj
      2,    // volno
      2,    // 40 a volno (dolni zluta) => volno
      29,   // posun
      // --------------------------------
      32,   // opatrne na privolavaci navest
      30,   // posun dovolen - nezabezpeceny
      0,    // test - zhasnuto
      255,  // test - vse rozsviceno
    }
  },
  // ---------------- typ 5/133: Hlavní: Odjezd: Jen odbočkou - ČBŽ; 8 znaku
  {
    8, 3, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 0, 2, 3, 4 },
    {
      1,    // stuj
      7,    // volno -> 40 a volno (nepouzito)
      7,    // 40 a volno 
      29,   // posun
      // --------------------------------
      32,   // opatrne na privolavaci navest
      30,   // posun dovolen - nezabezpeceny
      0,    // test - zhasnuto
      255,  // test - vse rozsviceno
    }
  },

  // ---------------- typ 6/134: Hlavní: Oddílové - ŽZČ; 4 znaky
  {
    4, 3, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 2, 3, 0, 0 },
    {
      1,    // stuj
      2,    // volno
      3,    // vystraha
      4,    // ocekavej 40
    }
  },
// ====================================================================
  // rezerva
  {}, // 135
  {}, // 136
  {}, // 137
  
  // ---------------- typ 10/138: Jednoduché návěstidlo - ZČ; 2 znaky
  {
    2, 2, SIGNAL_SET_CSD_BASIC, 0, 
    { 0, 1, 2, 0, 0 },
    {
      1,    // stuj
      2,    // volno
    }
  },
  // ---------------- typ 11/139: Jednoduché: S posunem - ZČB; 4 znaky
  {
    4, 3, SIGNAL_SET_CSD_BASIC, 0, 
    { 0, 1, 2, 3, 0 },
    {
      1,    // stuj
      2,    // volno
      29,   // posun
      32,   // opatrne na privolavaci navest
    }
  },
  // ---------------- typ 12/140: Jednoduché: S odbočkou - ZČŽ; 4 znakly
  {
    4, 3, SIGNAL_SET_CSD_BASIC, 0, 
    { 0, 1, 2, 0, 3 },
    {
      1,    // stuj
      2,    // volno
      7,    // 40 a volno 
      255,  // test: vsechno ON
    }
  },
// ====================================================================
  // rezerva
  {}, // 141
  {}, // 142

  // ---------------- typ 15/143: Předvěst: výstraha - Ž
  {
    2, 2, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 0, 0, 0, 0 },
    {
      3,    // vystraha
      0,    // volno (vse OFF)
    }
  },
  // ---------------- typ 16/144: Předvěst: opakovací výstraha - ŽB; 2 znaky
  {
    2, 2, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 0, 0, 2, 0 },
    {
      23,   // opakovana vystraha
      0,    // volno (vse OFF)
    }
  },
  // ---------------- typ 17/145: Předvěst: vjezdové - ŽZ; 2 znaky
  {
    4, 2, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 2, 0, 0, 0 },
    {
      3,    // vystraha
      2,    // volno
      4,    // Očekávej 40
      255,  // all ON
    }
  },
  // ---------------- typ 18/146: Předvěst: opakovací vjezdová - ŽZB; 2 znaky
  {
    4, 3, SIGNAL_SET_CSD_BASIC, 0, 
    { 1, 2, 0, 3, 0 },
    {
      23,    // opakovana vystraha
      22,    // opakovana volno
      24,    // Opakovaná očekávej 40
      255,   // all ON
    }
  },
// ====================================================================
  // rezerva
  {}, // 147
  {}, // 148
  {}, // 149
  // ---------------- typ 22/150: Konec trati - Č; 2 znak
  {
    2, 2, SIGNAL_SET_CSD_EMBEDDED, 0, 
    { 0, 0, 1, 0, 0 },
    {
      1,    // stuj
      0     // all OFF
    }
  },
  // ---------------- typ 23/151: Odjezd jen posunem - ČB
  {
    2, 2, SIGNAL_SET_CSD_EMBEDDED, 0, 
    { 0, 0, 1, 2, 0 },
    {
      1,    // stuj
      30    // posun dovolen - nezabezpeceny
    }
  },
  // ---------------- typ 24/152: Seřaďovací - BM
  {
    2, 2, SIGNAL_SET_CSD_EMBEDDED, 0, 
    { 0, 0, 0, 1, 0, 2 },
    {
      28,    // posun zakazan
      29     // posun dovolen
    }
  },
  // ---------------- typ 25/153: Spádovištní - BČBI
  {
    2, 6, SIGNAL_SET_CSD_EMBEDDED, 0,
    { 0, 0, 2, 1, 0, 0, 0, 0, 3, 4 },
    {
      1,  // stůj
      29, // posun
      20, // zpět
      25, // přisun soupravy ke spádovišti
      18, // sunout pomalu
      19  // sunout rychleji
    }
  },
  // ---------------- typ 26/154: Spádovištní - BMBI
  {
    2, 6, SIGNAL_SET_CSD_EMBEDDED, 0,
    { 0, 0, 0, 1, 0, 2, 0, 0, 3, 4 },
    {
      28, // posun zakázán
      29, // posun
      20, // zpět opakovaná
      25, // přisun soupravy ke spádovišti
      18, // sunout pomalu
      19  // sunout rychleji
    }
  }
};
static_assert(((sizeof(mastTypeDefinitions) + sizeof(mastTypeDefinitions[0]) -1) / sizeof(mastTypeDefinitions[0]) <= maxMastTypes), "Too many mast type definitions");

const byte mastTypeDefinitionCount = ((sizeof(mastTypeDefinitions) + sizeof(mastTypeDefinitions[0]) -1) / sizeof(mastTypeDefinitions[0]));


void signalMastChangeAspectCsdBasic(int nrSignalMast, byte newAspect) {
  signalMastChangeAspect((int)&(csdBasicAspects[newAspect]), sizeof(csdBasicAspects) / sizeof(csdBasicAspects[0]), nrSignalMast, newAspect);
}

void signalMastChangeAspectCsdIntermediate(int nrSignalMast, byte newAspect) {
  signalMastChangeAspect((int)&(csdIntermediateAspects[newAspect]), sizeof(csdBasicAspects) / sizeof(csdBasicAspects[0]), nrSignalMast, newAspect);
}

void signalMastChangeAspectCsdEmbedded(int nrSignalMast, byte newAspect) {
  signalMastChangeAspect((int)&(csdEmbeddedAspects[newAspect]), sizeof(csdBasicAspects) / sizeof(csdBasicAspects[0]), nrSignalMast, newAspect);
}

void signalMastChangeAspectSzdcBasic(int nrSignalMast, byte newAspect) {
  signalMastChangeAspect((int)&(szdcBasicAspects[newAspect]), sizeof(csdBasicAspects) / sizeof(csdBasicAspects[0]), nrSignalMast, newAspect);
}

void signalMastChangeAspectCsdMechanical(int nrSignalMast, byte newAspect) {
  signalMastChangeAspect((int)&(csdMechanicalAspects[newAspect]), sizeof(csdBasicAspects) / sizeof(csdBasicAspects[0]), nrSignalMast, newAspect);
}
