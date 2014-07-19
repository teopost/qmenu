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
	
## Item area
In questa sezione, per ogni item di menu, si definisce:

* l'azione da effettuare
* Un eventuale messaggio di Help
* ???

Il formato da utilizzare per ogni item è composta da 4 righe:

    1. Opzione di menu
    2. Comando da eseguire
    3. Messaggio di help da visualizzare
    4. ???


1. Nella prima riga inserire il primo carattere della stringa che contiene l'ozione di menu)
2. Nella seconda riga, è possibile specificare:

    * Un comando di shell
    * Un sottomenu da aprire ($menuname without extension)
    * Comando riservato (return per tornare al menu precedente, fine per uscire dal menu) 

3. Nella terza riga specificare il messaggio di testo.
Tale messaggio verrà mostrato in corrispondenza della variabile TITLE presente nella "Screen area"

Esempio:

    %1
    ls -lisa; pause
    This item show shell file list
    ?????


## Option area
Questa sezione contiene una sezie di impostazioni generiche:

    TITOLO - Titolo del menu
    RVS_ROW=0,22 - Righe da mostrare in reverse. In questo caso la riga 0 e la riga 22
    NOKEYDISP - Disabilita la visualizzazione automatica delle label dei tasti funzione (ET[1-10])
    ET[1-10] - Testo mostrato per tasto funzione n se NOKEYDISP commentato
    FZ[1-10] - Comando (shell) eseguito per il tastofunzione n
    PW[1-10] - Password da impostare per tasto funzione n
    SHELL=sh - Premendo shift-f1 viene avviata una sessione di shell con questa
    EDITOR=vi - Premente shift-f2 viene avviato il vi con l'editing del menu attualmente in uso
    
Ognuna di queste opzioni può essere commentata con il #




