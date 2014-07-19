# How it work
qmenu permette di creare in maniera semplice e intuitiva menu per interfacce a caratteri.
Ogni menu è definito in un file (con estensione .mnu) che contiene sia l'aspetto visuale che la definizione delle varie voci.

Il file è suddiviso in 3 sezioni:

    1. Screen Area
    2. Item Area
    3. Option Area

Ogni area è delimitata dai caratteri $$ e %%

    Screen area
    $$
    Item area
    %%
    Option area

## Screen area
Questa area va da inizio file fino al simbolo $$, e contiene il "disegno" vero e proprio del menu in formato testuale.
Ogni opzione di menu, per essere riconosciuta come selezionabile deve essere racchiuso fra il simbolo ^

Esempio:

    ^1. Option 1^
    ^2. Option 2^

Le opzioni si possono selezionare con la freccia della tastiera ma anche premendo la prima lettera della stringa

All'interno di questa area si possono anche aggiungere alcune variabili con la sintassi:

    x[NOME_VARIABILE]

In pratica il nome della variabile è composto da un prefisso (x) e, fra parentesi quadre il nome della variabile. Questi sono i valori disponibili:

    * c - Centrato
    * l - Giustificato a sinistra
    * r - Giustificato a destra

Nota: Se il simbolo viene messo maiuscolo il valore viene mostrato in reverse

Le variabili possono assumere il seguente valore:

    * TIME - Orario attuale
    * MENUNAME - Nome del menu
    * INFOTERM - Nome rel terminale
    * MNUTRACE - Percorso dei menu

Variabili speciali:

    & - Posizione del cursore
    ${PATH} - Visualizza il contenuto della variabile d'ambiente PATH

Esempio:

	C[TITLE] - Titolo del menu centrato ed in reverse
	
## Item area - TODO
## Option area - TODO
===




