/*
================================================================================
qmenu.c - Gestore di menu
--------------------------------------------------------------------------------
Compilazione su sistema 3000 -> cc -o qmenu qmenu.c -lcurses 
Compilazione su SCO          -> cc -xenix -lcurses -ltermcap -o qmenu qmenu.c
================================================================================
*/
/* Per Unix System V NCR -> #include <cursesr2>  */
#include <curses.h>             
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dirent.h"
#include <term.h>
					/* COSTANTI GENERICHE                 */
					/* ==================                 */
#define VERSION "Rel. 5.0, 16/09/2003"	/* Versione del programma 	      */
#define SUPER   1			/* Pop-up  Stampanti Superutente      */
#define USER    2 			/* Pop-up  Stampanti Utente           */
#define ON      1			/* Costante stato ON                  */
#define OFF     0			/* Costante stato OFF                 */
#define MENU   -1			/* Check Password da voce Menu        */
#define EDIT   -2 			/* Check Password da <Shift-@>        */
#define SHELL  -3			/* Check Password da <Shift-!>        */
#define SUPER_MENU  -4			/* Check Password da <Shift-#>        */
#define PASSWD_MODE -5			/* Check Password in password mode    */
#define TL      1	 		/* Flag simbolo grafico Top-Left      */
#define BL      2		        /* Flag simbolo grafico Bottom-Left   */
#define TR      3			/* Flag simbolo grafico Top-Right     */
#define BR      4			/* Flag simbolo grafico Bottom-Right  */
#define ROW     5		 	/* Flag simbolo grafico Row           */
#define COL     6			/* Flag simbolo grafico Col           */
#define SHADOW  7			/* Flag simbolo grafico Shadow        */
#define LEFT_SYMBOL  '>'                /* Simbolo Sx della barra di selezione*/
#define RIGHT_SYMBOL ' '		/* Simbolo Dx della barra di selezione*/
#define LOGFILE_DIM  50000		/* Dimensione max del file di LOG     */
#define CONFIG_FILE qmenu.cfg 		/* Nome File di configurazione        */
#define MAXR 24				/* Numero massimo righe Display       */
#define MAXC 90				/* Numero massimo colonne Display     */
#define MAXITEMS 40 		 	/* Massimo Numero Voci di menu        */
#define MAXFRMITEMS 20 		 	/* Massimo Numero Voci di menu        */
#define PREV_FIELD "-2"		 	/* Massimo Numero Voci di menu        */
#define ABORT "-1"     		 	/* Massimo Numero Voci di menu        */
#define C_U 256   			/* Cursore Su 			      */
#define C_D 257   			/* Cursore Giu' 		      */
#define C_L 258   			/* Cursore Sinistra 		      */
#define C_R 259   			/* Cursore Destra 		      */
#define FZ1 261  			/* Tasto Funzione F1                  */
#define FZ2 262  			/* Tasto Funzione F2                  */
#define FZ3 263  			/* Tasto Funzione F3                  */
#define FZ4 264  			/* Tasto Funzione F4                  */
#define FZ5 265  			/* Tasto Funzione F5                  */
#define FZ6 266  			/* Tasto Funzione F6                  */
#define FZ7 267  			/* Tasto Funzione F7                  */
#define FZ8 268  			/* Tasto Funzione F8                  */
#define FZ9 269  			/* Tasto Funzione F9                  */
#define FZ0 270  			/* Tasto Funzione F10                 */
#define FZA 271  			/* Tasto Funzione F11                 */
#define FZB 272  			/* Tasto Funzione F12                 */
#define DEL 18  			/* Tasto DELETE             	      */
#define BKSP 8				/* Tasto BACK-SPACE         	      */
#define INS 5				/* Tasto INSERT             	      */
#define RET 13 				/* Tasto RETURN         	      */
#define NL 10				/* Tasto NEW-LINE  	              */
#define PGUP 300   			/* Tasto Pagina Su' 		      */
#define PGDN 301 			/* Tasto Pagina Giu'     	      */
#define HOME 302 			/* Tasto Pagina Giu'     	      */
#define END  303 			/* Tasto Pagina Giu'     	      */
#define ESC 27 				/* Tasto ESCAPE			      */
#define X_COORD 3			/* Coord. X Display Help              */
#define Y_COORD 5 			/* Coord. Y Display Help              */
#define MAX_ROW 16			/* Righe Max Display Help             */
#define MAX_COL 70			/* Colonne Max Display Help           */


					/* FILE POINTER                       */
					/* ============                       */
FILE *infile,*fopen();          	/* File Generici di Input/Output      */
FILE *ofile,*helpfile;			/* File Generici di Input/Output      */
				
					/* CHAR VARIABILI GLOBALI             */
					/* ======================             */
signed char attrb[MAXR+1][MAXC];       	/* Campi di selezione Dati            */
char vscreen[MAXR+1][MAXC];     	/* Schermo di Lavoro                  */
char linea[200];                	/* Buffer Globale Generico            */
char finestra[23][200];
int max_elem_fin=0;
int max_lun_fin=0;
int j=0;
char help_line[800][255];
char nometemp[64];                      /* File temporaneo                    */
char lpdestenv[32];             	/* Spazio environment x LPDEST        */
char lpdestval[32];             	/* Spazio environment x LPDEST        */
char sistema[12];               	/* Nome sistema                       */
char imenu[80][40];             	/* Catasta di Menu                    */
char NULSTRING[] = { 0 , 0 }; 		/* Stringa nulla statica per funzioni */
char titolo[40];   			/* Titolo del menu                    */
char editor[40];			/* Editor utilizzato (vi)             */
char rvs_row[40];			/* Stringa di righe da evidenziare    */
char rvs_tmp[40][10];			/* Stringa temp. di righe da evidenz. */
char shell[40];				/* Shell lanciata		      */
char porta[20];				/* Nome tty di input                  */
char utente[20];			/* Nome login di input                */
char spoolernm[16];      		/* Nome dello spooler assegnato       */
char ability_user[80];
char prsuper[80];		
char pruser[80];		
char super_passwd[80];		
char menu_dir[64];			/* Nome directory dei menu            */
char sh_passwd[64];			/* Nome parola chiave per shell       */
char sh_passwd_ok=0;			/* Flag parola chiave per shell       */
char edit_passwd[64];			/* Nome parola chiave per edit menu   */
char edit_passwd_ok=0;			/* Flag parola chiave per edit menu   */
char intflg=0;				/* Flag Interrupt da subroutine       */
char tfu[10][300];			/* Tasti Funzione Utente              */
char lfu[10][20];			/* Label Tasti Funzione Utente        */
char pfu[10][26];			/* Password Tasti Funzione Utente     */

					/* INT VARIABILI GLOBALI              */
					/* =====================              */
int selmenu[40];                 	/* Catasta di Menu                    */
int help_item=0;
int rvs_r;				/* Riga da evidenziare                */
int ipmenu=0;             		/* Puntatore a Catasta di Menu        */
int nitems;              		/* Numero di Voci                     */
int curitem;                    	/* Voce corrente                      */
int iy=21,ix=1;         		/* Posizione di default per input     */
int keydisp=1;				/* Flag Diplay Tasti Funzione         */
int prdisp=1;				/* Flag display Stampante             */
int itime_out=900;			/* Time Out di Procedura              */
int mnu_tim=1;				/* Inattivita tastiera attivata       */
int bks_exit=0;				/* Se 1 esce dal menu principale      */
int neterr=0;				/* Flag Errore di non trovato login   */
int en, o=0, junk, d;	 		/* Variabili generiche, contatori ecc.*/
int user=OFF;
int super=OFF;
int superpasswd=OFF;
int passwd_mode=OFF;
int inc=0,memoinc=0;  			/* Variabile pointer file memorizzato */
int graf=1;				/* Flag Grafica Abilitata (se=1)      */
double procid;   			/* Numero processo in corso           */
long logfile_dim=LOGFILE_DIM;		/* Dimensione max file di log         */
char env_var[80]; 			/* Stringa variabile d' ambiente      */
char simb_var[80]; 			/* Stringa variabile simbolo 	      */
char env_value[80];			/* Valore variabile d' environment    */
char memofile[15000];		        /* File memorizzato in un array       */
char hlp_filename[60]; 			/* Nome file di Help 		      */
char to_found[60]; 			/* Stringa da ricercare nel file Help */
int mnuname_y;				/* Riga simbolo MNUNAME (Nome menu)   */
int mnuname_x;				/* Colonna simbolo MNUNAME            */
int mnuname_g;				/* Giustificazione simbolo MNUNAME    */
int mnuname_flag=OFF;			/* Flag ON/OFF simbolo MNUNAME        */
int mnuname_rvs=OFF;			/* Flag reverse on simbolo MNUNAME    */
int msg_y;				/* Riga simbolo MSG (Messaggio Help)  */
int msg_x;				/* Colonna simbolo MSG                */
int msg_g; 				/* Giustificazione simbolo MSG        */
int msg_flag=OFF; 			/* Flag ON/OFF simbolo MSG   	      */
int msg_rvs=OFF; 			/* Flag reverse on simbolo MSG        */
int time_y;				/* Riga simbolo TIME (Orario) 	      */
int time_x; 			        /* Colonna simbolo TIME  	      */
int time_g;		  	        /* Giustificazione simbolo TIME       */
int time_flag=0; 			/* Flag ON/OFF simbolo TIME 	      */
int time_rvs=OFF; 			/* Flag reverse on simbolo TIME	      */
int title_y; 				/* Riga simbolo TITLE (Titolo)	      */
int title_x;				/* Colonna simbolo TITLE              */
int title_g;				/* Giustificazione simbolo TITLE      */
int title_flag=0; 			/* Flag ON/OFF simbolo TITLE          */
int title_rvs=OFF;  			/* Flag reverse on simbolo TITLE      */
int infoterm_y;				/* Riga simbolo INFOTERM (Info term.) */
int infoterm_x;				/* Colonna simbolo INFOTERM	      */
int infoterm_g;				/* Giustificazione simbolo INFOTER    */
int infoterm_flag=0;			/* Flag ON/OFF simbolo INFOTERM       */
int infoterm_rvs=OFF;  			/* Flag reverse on simbolo INFOTERM   */
int mnutrace_y;				/* Riga simbolo MNUTRACE (Elenc.menu) */
int mnutrace_x;				/* Colonna simbolo MNUTRACE    	      */
int mnutrace_g;				/* Giustificazione simbolo MNUTRACE   */
int mnutrace_flag=0;			/* Flag ON/OFF simbolo MNUTRACE       */
int mnutrace_rvs=OFF;   		/* Flag reverse on simbolo MNUTRACE   */
int a=0;

struct menuitem         		/* STRUTTURA DATI DEL MENU'           */
{ 					/* =======================            */
	char desc[80];			/* Descrizione a Video                */
	char execute[300];      	/* Comando Corrispondente Eseguito    */
	char msg[80];           	/* Messaggio descrittore del comando  */
	char passw[26];         	/* Eventuale Password                 */
	int  x;                 	/* Posizione x 	                      */
	int  y;                 	/* Posizione y                        */
	int len;                	/* Lunghezza Descizione               */
	int inplen;			/* Lunghezza indice Selezione         */
} scelta[MAXITEMS];			/* Massimo numero scelte (01-99,A-Z)  */

struct form
{
	char desc[80];
	int x;
	int y;
	int lun;
} frm[MAXFRMITEMS];

					/* FUNZIONI GLOBALI                   */
					/* ================       	      */
char *getlogin();
char *ttyname();
char *cercadop();
char *getenv();
int senum();
void fine1();
void fine2();
void fine3();
void fine4();
void fine5();
void templim();
void helper();
void cre_cfg();
void cre_model();
int  form(char *forma);
int  disegna_form();

time_t t1;
struct tm *tptr;



main(int argc, char **argv)		/* MODULO PRINCIPALE                  */
{					/* =================                  */
int errorlvl=0; 	 		/* Variabile controllo ritorno funz.  */
int ii=0, c, c1, c2, t, idx; 		/* Variabili di utilizzo generale     */
int k=0;
int primavolta;				/* Primo utilizzo frecce e tastiera   */
int tmpcnt,flg1,flg2;
char command[60];  			/* Command Variab.usato dalla system()*/
char prgname[80];
char menu_name[128];
char compare[80];
char buf0[4];
char buf1[4];
char formid[30];
char shellid[30];
char frm_argument[255];
int dentro_parentesi=FALSE;
int p, n, r;


if(argc==1)				/* Se non esistono argomenti, errore  */
{
	printf("\nqmenu   - %s ", VERSION);
	printf("Gestione menu. (C)Copyright Stefano Teodorani\n\n");
	printf("Sintassi : qmenu [-init] [-model] [menu_file]\n\n");
	printf("           -init : Crea il file configurazione \"qmenu.cfg\"\n");
	printf("           -model: Crea un modello di file menu col nome \"model.mnu\"\n\n");
	exit(0);
}
if(strcmp(argv[1], "-init")==0)
{
	cre_cfg();
	exit(0);
}
if(strcmp(argv[1], "-model")==0)
{
	cre_model();
	exit(0);
}
readcfg();				/* Lettura Variabili d' ambiente      */
strcat(imenu[ipmenu],argv[1]);  	/* Rilevo il nome del Menu passato    */
strcat(imenu[ipmenu],".mnu");		/* Aggiungo estensione al nome menu   */
strcpy(menu_name,menu_dir);
strcat(menu_name,imenu[ipmenu]);

if ((infile=fopen(menu_name,"r")) == NULL )
{
	printf("\n\nqmenu - %s ", VERSION);
	printf("Gestione menu. (C)Copyright Stefano Teodorani\n\n");
	printf("Sintassi : qmenu [menu_file]\n\n");
	printf("File %s inesistente\n\n", menu_name);
	exit(0);
}
else
	fclose(infile);

signal(SIGINT,  fine1); 		/* Interruzione utente (^C, Del, Trap)*/
signal(SIGFPE,  fine2);			/* Errore Aritmetico 		      */
signal(SIGBUS,  fine3);			/* Bus Error     		      */
signal(SIGSEGV, fine4);			/* Segmentation fault		      */
signal(SIGTERM, fine5);			/* Segnale interruzione esterno       */
signal(SIGALRM, templim);		/* Tempo di Inattiviva utente         */

         		   		/* Lettura Variabili d' ambiente      */
strcpy(porta,ttyname(fileno(stdin))); 	/* Identificazione porta 	      */
initscr();				/* Inizializzazione schermo curses    */
noecho();
cbreak();
clear();
refresh();
noecho();
crmode();
strcpy(compare,argv[0]);                /* Ottiene nome programma di lancio   */
while(compare[0] != 0 )
{
	strcpy(prgname,compare);
	strcpy(compare,cercadop(prgname,"/"));
}
if (argc < 3)				/* Se non esistono 3 argomenti, uso   */
	strcpy(editor,"vi");		/* il vi come editor di modifica menu */
else					/* altrimenti 	   		      */
	strcpy(editor,argv[2]);  	/* uso il terzo argomento come Editor */

strcpy(shell,"sh");			/* Uso sh come shell di sistema       */

if (getlogin())				/* Letto il nome login                */
	strcpy(utente,getlogin());      /* e ne copio il contenuto in utente  */

strcpy(nometemp,"\0");			/* Copio su sistema il sistema in uso */
strcpy(nometemp,tmpnam(nometemp));
strcpy(linea,"uuname -l > ");
strcat(linea,nometemp);
system(linea);
infile=fopen(nometemp,"r");
getline(sistema);
fclose(infile);
unlink(nometemp);

errorlvl=loadmenu();
if (errorlvl)
{
	if (errorlvl!=10)
	{
		alert(" ATTENZIONE ", " ERRORE IN FILE MENU ");
		errorlvl=3;
	}
	beep();
	goto the_end ;
}
clear();
dispmenu();
history_string("[INIZIO LAVORO]");
curitem=1;
tmpcnt=0;
c=0;
primavolta=0;
tmpcnt=0;
if (passwd_mode)
{
	if(chkinit())
		dispmenu();	
	else
		goto the_end;
}
	
while(1) 			/* inizio ciclo controllo menu'     */
{
	c1=c;
	if ((tmpcnt==1) && (c1 > 1000))
	{
		smove(iy,ix-1);
		sprintf(buf0,"%.2d",c1-1000);
		addch('?');
	}
	smove(iy,ix);
	if (time_flag)
		orolog();
	if (mnu_tim)
		alarm(itime_out); 		/* Delay di inattivita'       */
	if ((msg_flag) && (primavolta))
		hlpmsg();
	if (infoterm_flag)
		infoterm();
	inpsel(primavolta);
	refresh();
	c=getkey();
	if (mnu_tim)
		alarm(0);
	if ((senum(c)) && (tmpcnt==0)) /* Input numerico allora verifico */
	{	  			  /* se puo essere una decina       */
		tmpcnt=1;		  /* Flag controllo primo input     */
		buf0[0]=c; 		  /* numerico                       */
		buf0[1]=0;
		c=atoi(buf0)+1000;	  /* Sommo 1000 all'input c per     */
	 	flg1 = -1;		  /* distinguerlo come primo input  */
		flg2 = -1;
		for (t=1;t<=nitems;t++)
		{
			buf1[0]=scelta[t].desc[0];
			buf1[1]=scelta[t].desc[1];
			buf1[2]=0;
			if ((senum(buf1[0])==0 ) && (senum(buf1[1])== 0))
				continue;
			if (c==(atoi(buf1)+1000))
				flg1=0;
		}
		for (t=1;t<=nitems;t++)
			if (scelta[t].desc[0]==buf0[0])
				if (senum(scelta[t].desc[1])==1)
					flg2=0;
		if (flg1 == -1)
		{
			if ((flg2 == -1) && (buf0[0] != '0'))
			{
				beep();
				tmpcnt=0;
			}	
			continue;
		}
		if ((flg1 == 0) && (flg2 == -1))
		{
			tmpcnt=0;
			inpsel(primavolta);
		}
	}
	if ((tmpcnt==1) && (senum(c))) /* Se sono nel secondo ciclo      */
	{				  /* dell'input numerico  	    */
		tmpcnt=0;
		buf0[0]=(c1-1000)+'0';
		buf0[1]=c;
		buf0[2]=0;
		c=atoi(buf0)+1000;
		flg1 = -1;
		for (t=1;t<=nitems;t++)
		{
			buf1[0]=scelta[t].desc[0];
			buf1[1]=scelta[t].desc[1];
			buf1[2]=0;
			if ((senum(buf1[0])==0 ) && (senum(buf1[1])== 0))
				continue;
			if (c==(atoi(buf1)+1000))
				flg1=0;
		}
		if (flg1 != 0) 
		{
			smove(iy,ix-1);
			printw("%.2d",c-1000);
			scarica();
			alert(" ATTENZIONE ", " SELEZIONE NON VALIDA ");
			dispmenu();
			selez(1);
			c=0;
			continue;	
		}
	}
	if ((senum(c) != 1) && (c < 1000))    /* Non si ammettono cifre     */
		tmpcnt=0;		      /* mischiate a lettere        */
    	if (c == 0)       		      /* Tasto nullo 		    */	
		continue;
	if (c == BKSP )   		      /* Ritorno a menu' Precedente */
	{
		history_string("[Menu precedente]");
		ipmenu--;
		if (ipmenu < 0)
		{
			if(bks_exit)
				goto the_end;
			else
			{
				ipmenu=0;
				alert(" ATTENZIONE ", " QUESTO E' IL MENU PRINCIPALE ");
				dispmenu();
				refresh();
				primavolta=0;
				selez(0);
				curitem=1;
				c=0;
				continue;
			}
		}
		if(errorlvl=loadmenu(imenu[ipmenu]))
		{
			alert(" ATTENZIONE ", " MENU CHIAMANTE MODIFICATO ");
			goto the_end;
		}
		clear();
		dispmenu();
		primavolta=1;
		curitem=selmenu[ipmenu]; 
		selez(1);
		continue;
	}
    	if (c == '#')   /*--- Richiesta Shell Assegnata -----*/
	{
		if(superpasswd)
		{
			if (chkpass(SUPER_MENU)) 
			{
				dispmenu();
				selez(1);
				continue;
			}
		switch(win_option("ACCESSO A SHELL,MODIFICA MENU,MODIFICA STAMPANTE DI DEFAULT,VISUAL. FILE DI CONFIGURAZIONE,VISUAL. FILE DI LOG,MODIFICA FILE DI CONFIGURAZIONE,MODIFICA FILE DI FORM,MODIFICA FILE DI HELP,INFORMAZIONI SU APPSEL"))
		{
			case 0:
 	    			strcpy(command,shell);
				esegui(command);
				errorlvl=loadmenu(imenu[ipmenu]);
				if (errorlvl)
				{
					if (errorlvl!=10)
					{
						alert(" ATTENZIONE ",\
						" ERRORE IN FILE MENU ");
						errorlvl=3;
					}
					goto the_end;
				}	
				break;
			case 1:
 	    			strcpy(command,editor);
				strcat(command," ");
				strcat(command,menu_dir);
				strcat(command,imenu[ipmenu]);
				esegui(command);
				errorlvl=loadmenu(imenu[ipmenu]);
				if (errorlvl)
				{
					if (errorlvl!=10)
					{
						alert(" ATTENZIONE ",\
						" ERRORE IN FILE MENU ");
						errorlvl=3;
					}
					goto the_end ;
				}	
				break;
			case 2:
				selspool(SUPER);
				break;
			case 3:
				display_config();
				break;
			case 4:
				if((infile=fopen("qmenu.log","r"))==NULL)
				{
				      printw("Non riesco ad aprire il file.\n");
				      break;
				}
				help_item=0;
				while((a=getline(help_line[help_item]))!=EOF)
					help_item++;
				help_item--;
				fclose(infile);
				helper();
				for(a=0;a!=help_item;a++)
					help_line[a][0]='\0';
				break;
			case 5:
				esegui("vi qmenu.cfg");
				errorlvl=loadmenu(imenu[ipmenu]);
				break;
			case 6:
				esegui("vi qmenu.frm");
				errorlvl=loadmenu(imenu[ipmenu]);
				break;
			case 7:
 	    			strcpy(command,editor);
				strcat(command," ");
				strcat(command,hlp_filename);
				esegui(command);
				errorlvl=loadmenu(imenu[ipmenu]);
				break;
 			case 8:
win(" INFORMAZIONI SU APPSEL ", 2, 2, 18, 78);
move(4,3);
printw(" %s ", VERSION);
move(6,3);
printw(" Gestore di Menu in ambiente Unix");
move(8,3);
printw(" Creato da Stefano Teodorani");
move(10,3);
printw(" Elenco files  : qmenu        - eseguibile");
move(11,3);
printw("                 qmenu.cfg     - file di configurazione");
move(12,3);
printw("                 qmenu.hlp     - file di help (facoltativo)");
move(13,3);
printw("                 qmenu.log     - file di log  (creato da qmenu)");
move(14,3);
printw("                 qmenu.log.bak - file di log  (copia del vecchio)");
move(15,3);
printw("                 qmenu.frm     - file di form");
move(18,4);
printw(" F4 - Esce ");
while((a=getkey())!=FZ4);
				break;
			default :
				break;
		} 
		}
		dispmenu();
		selez(1);
		continue;
	}
	if (c == '\t')     /*--- Richiesta Help su argomento ----*/
	{
		strcpy(to_found,imenu[ipmenu]);
		strcat(to_found,":");
		if(primavolta!=0)
			strncat(to_found,&scelta[curitem].desc,2);
		strcat(to_found,"\0");
		if(junk=ext(to_found)==1)
			alert(" ATTENZIONE ", " MENU FILE NON PRESENTE \0");
		dispmenu();
		if(primavolta!=0)
			selez(1);
		inpsel(primavolta);
		continue;
	}
	if ((c >= FZ1) && (c <= FZ0))   /*--- Tasti funzione ----*/
	{
		idx=c-FZ1;
		if (keydisp==1) /* Flash del Tasto funzione     */
		{
			if (tfu[idx][0] > 0)
			{
				for (t=0 ; t < 9  ; t++)
				{
					for (ii=0;ii<50000;ii++);
					{
						if (t % 2)
							standout();
						else
							standend();
					}
					move(23,idx*8);
					if (idx<9)
						printw("F%.1d",idx+1);
					else
						printw("F0");
					refresh();
				}
				standend();
				refresh();
			}
		}
		if (tfu[idx][0]==0)
			continue;
		if (strncmp(tfu[idx],"exit",4)==0)
			goto the_exit;
		if (strncmp(tfu[idx],"fine",4)==0)
		{
			if (confirm())
			{
				clear();
				refresh();
				history_string("[FINE LAVORO]");
				goto the_end ;
			}
			dispmenu();
			selez(1);
			continue;
		}
		if (tfu[idx][0]=='$')
		{
  			if ((pfu[idx][0]) > 0)
				if (chkpass(idx))
				{
					dispmenu();
					selez(1);
					continue;
				}
			selmenu[ipmenu]=curitem;
			ipmenu++;
			strcpy(imenu[ipmenu],&tfu[idx][1]);
			strcat(imenu[ipmenu],".mnu");
			errorlvl=loadmenu(imenu[ipmenu]);
            		if (errorlvl)
			{
			 	ipmenu--;	 
				if (ipmenu==-1)
					ipmenu=0;
			    	errorlvl=loadmenu(imenu[ipmenu]);
				if (errorlvl)
				{
					alert(" ATTENZIONE ", \
				        " MENU CHIAMANTE MODIFICATO ");
					goto the_end;
				}
			}
		  	clear();
			dispmenu();
			curitem=1;
			selez(0);
			primavolta=0;
			continue;
		}
		if (strncmp(tfu[idx],"return",6)==0)
		{
		  	if (strncmp(tfu[idx],"returnmain",10)==0)
				ipmenu=0;
			else
				ipmenu--;
			if (ipmenu < 0)
			{
				clear();
				refresh();
				goto the_end;
			}
            		if(errorlvl=loadmenu(imenu[ipmenu]))
			{
				alert(" ATTENZIONE ", \
				" MENU CHIAMANTE MODIFICATO ");
				goto the_end;
			}
			clear();
			curitem=1;
			continue;
		}
  		if ((pfu[idx][0]) > 0)
			if (chkpass(idx))
			{
				dispmenu();
				selez(1);
				continue;
			}
		strcpy(command,tfu[idx]);
		esegui(command);
		crmode();
		noecho();
		clear();
		dispmenu();
		selez(1);
		inpsel(primavolta);
		continue;
	}
	if (c == NL)        /*--- Esecuzione ----*/
	{
		if (primavolta==0)
		{
			beep();
			continue;
		}
		if (curitem == 0)
			continue;
		if (scelta[curitem].execute[0]==0)
			continue;
		if (strncmp(scelta[curitem].execute,"exit",4)==0)
			goto the_exit;
		if (strncmp(scelta[curitem].execute,"fine",4)==0)
		{
			if (confirm())
			{
				clear();
				refresh();
				history_string("[FINE LAVORO]");
				goto the_end ;
			}
			dispmenu();
			selez(1);
			continue;
		}
		if (scelta[curitem].execute[0]=='$')
		{
  			if (strlen(scelta[curitem].passw) > 0)
				if (chkpass(MENU))
				{
					dispmenu();
					selez(1);
					continue;
				}

			history_string(scelta[curitem].desc);
			selmenu[ipmenu]=curitem;
			ipmenu++;
			strcpy(imenu[ipmenu],&scelta[curitem].execute[1]);
			strcat(imenu[ipmenu],".mnu");
			errorlvl=loadmenu(imenu[ipmenu]);
            		if (errorlvl)
			{
			 	ipmenu--;	 
				if (ipmenu==-1)
					ipmenu=0;
			    	errorlvl=loadmenu(imenu[ipmenu]);
				if (errorlvl)
				{
					alert(" ATTENZIONE ", \
					" MENU CHIAMANTE MODIFICATO ");
			
	goto the_end;
				}
			}
		  	clear();
			dispmenu();
			curitem=1;
			selez(0);
		 	primavolta=0;
			c=0;
			refresh();
			continue;
		}
		/* FORM */
		if (scelta[curitem].execute[0]=='*')
		{
			p=1;
			n=0;
			r=0;
			while(scelta[curitem].execute[p]!='\0')
			{
				if(scelta[curitem].execute[p]=='(')
				{
					p++;
					dentro_parentesi=TRUE;
				}

				if(scelta[curitem].execute[p]==')')
				{
					p++;
					dentro_parentesi=FALSE;
				}

				if(dentro_parentesi)
					shellid[n++]=scelta[curitem].execute[p];
				else
					formid[r++]=scelta[curitem].execute[p];

				p++;
			}
			shellid[n]='\0';
			formid[r]='\0';

			if(!form(formid))
			{
				sprintf(frm_argument, "%s ", shellid);
				for(k=0;k<j;k++)
				{
					strcat(frm_argument, " \"");
					strcat(frm_argument, frm[k].desc);
					strcat(frm_argument, "\"");
				}
				strcat(frm_argument, "\0");
				resetty();
				esegui(frm_argument);
				crmode();
				noecho();
			}
			dispmenu();
			selez(1);
			continue;
		}
		if (strncmp(scelta[curitem].execute,"return",6)==0)
		{
		  	if (strncmp(scelta[curitem].execute,"returnmain",10)==0)
				ipmenu=0;
			else
				ipmenu--;
			if (ipmenu < 0)
			{
				clear();
				refresh();
				goto the_end;
			}
            		if(errorlvl=loadmenu(imenu[ipmenu]))
			{
				alert(" ATTENZIONE ", \
				" MENU CHIAMANTE MODIFICATO ");
				goto the_end;
			}
			dispmenu();
		  	curitem=1;
			primavolta=0;
			continue;
		}
  		if (strlen(scelta[curitem].passw) > 0)
			if (chkpass(MENU))
			{
				dispmenu();
				selez(1);
				continue;
			}
		resetty();
		strcpy(command,scelta[curitem].execute);
		history_string(scelta[curitem].execute);
		esegui(command);
		crmode();
		noecho();
		clear();
		dispmenu();
		selez(1);
		inpsel(primavolta);
		primavolta=1;
		continue;
	}
	if ((c > 255) && (c < 1000))   /* Frecce                */
	{
		if (primavolta==0)
		{
			selez(0);
			curitem=1;
			selez(1);
		  	buf0[0]=scelta[curitem].desc[0]; 
		  	buf0[1]=scelta[curitem].desc[1]; 
			buf0[2]=0;
			if ((senum(buf0[0])) || (senum(buf0[1])))
				c=1000+atoi(buf0);
			primavolta=1;
		}
		else
		{
			selez(0);
			curitem=selectnext(c);
			selez(1);
		  	buf0[0]=scelta[curitem].desc[0]; 
		  	buf0[1]=scelta[curitem].desc[1]; 
			buf0[2]=0;
			if ((senum(buf0[0])) || (senum(buf0[1])))
				c=1000+atoi(buf0);
		}
	}
	else				/* Lettere o decine numeriche  */
	{
		if ( c < 1000)   /*--- Lettere ---*/
		{
			c=toupper(c);
			primavolta=1;
			selez(0);
			for (t=1;t<=nitems;t++)
			{
				if (c==scelta[t].desc[0])
					curitem=t;
			}
			selez(1);
			refresh();
		}
		else
		{
			primavolta=1;
			selez(0);
			for (t=1;t<=nitems;t++)
			{
				buf1[0]=scelta[t].desc[0];
				buf1[1]=scelta[t].desc[1];
				buf1[2]=0;
				if((senum(buf1[0])==0)&&(senum(buf1[1])==0))
					continue;
				if (c==(atoi(buf1)+1000))
					curitem=t;
			}
			selez(1);
		}
	}
	if (tmpcnt==0)
		inpsel(primavolta);
}
the_end: ;    				/* Label di fine procedura 	      */
resetty();
if (errorlvl)
        putchar('\n');
standend();
/*
sleep(1);
*/
endwin();
/*
sleep(1);
*/
exit(errorlvl);

the_exit: ;    				/* Label di fine procedura 	      */
resetty();
if (errorlvl)
        putchar('\n');
standend();
/*
sleep(1);
*/
endwin();
/*
sleep(1);
*/
exit(curitem);
}

int loadmenu()				/* CARICAMENTO DI UN MENU             */
{				        /* ======================             */
char menu_file[128];
char buf0[3];
char buf1[3];
int sn;
int ii,t,c0,c1,c,y,x,invoce,r,ik;
int kk,fine,getmemo();
int startpos;

mnuname_flag=OFF;
msg_flag=OFF;
time_flag=OFF;  
title_flag=OFF;  
infoterm_flag=OFF;  
mnutrace_flag=OFF;  

strcpy(menu_file,menu_dir);
strcat(menu_file,imenu[ipmenu]);
				
for (y=0;y < MAXR ; y++)		/* Azzeramento Descrizioni	      */
{
	for(x=0;x < MAXC;x++)
	{
		attrb[y][x]=0; 
		vscreen[y][x]=0;
	}
}
attrb[0][0]=0;
for (t=0;t < MAXITEMS ;t++)
{
	scelta[t].desc[0]=0;
	scelta[t].desc[1]=0;
	scelta[t].passw[0]=0;
	scelta[t].execute[0]=0;
	scelta[t].msg[0]=0;
	scelta[t].len=0;
	scelta[t].y=0;
	scelta[t].x=0;
}

if ((infile=fopen(menu_file,"r")) == NULL )	/* Memorizzazione File        */
       	return 10;
memoinc=0;					/* Azzeramento Pointer array  */
while ((c=getc(infile)) != EOF)
{
	if ((c=='{') && (c0=='$'))
	{   
		o=0;
		while ((en=getc(infile)) != '}')
			env_var[o++]=en;

		env_var[o]='\0';
		if (getenv(env_var))
			strcpy(env_value,getenv(env_var));
		else
			strcpy(env_value,"");

		d=0;
		memoinc=memoinc-1;
		while(d<strlen(env_value))
			memofile[memoinc++]=env_value[d++];
		continue;
	}
	memofile[memoinc++]=c;
c0=c;
}
memofile[memoinc]='\0';
fclose(infile);

ii=0;
c0=0;
y=0;
x=0;
r= -1;
invoce=0;
nitems=1;

inc=0;
fine=0;
while ((c=memofile[inc++]) != EOF)
{
	if (y>MAXR)
		return (3);	
	if ((c=='[') && ((c0=='L') || (c0=='l') || (c0=='C') \
	              || (c0=='c') || (c0=='R') || (c0=='r') \
	              || (c0=='E') || (c0=='e') || (c0=='B') || (c0=='b')))
	{   
		o=0;
		startpos=x-1;
		while ((en=memofile[inc++]) != ']')
			simb_var[o++]=en;
		simb_var[o]='\0';
		move(0,0);
		x--;
		for(ii=0;ii<=strlen(simb_var)+2;ii++)
			vscreen[y][x++]=32;
		simb_pos(simb_var,y,startpos,c0);
		continue;
	}
	if ((c0=='$') && (c=='$'))        /* Termine Parte Descrittiva */
	{
		fine=1;
		vscreen[y][x-1]=32;
		break;
	}
	if ((c=='^') || (invoce==1))
	{	
		if ((c=='^') && (invoce==0))
		{
			invoce=1;
			ii=0;
			attrb[y][x+1]=nitems;
			vscreen[y][x++]=32;
			scelta[nitems].y=y;
			scelta[nitems].x=x;
			c=c0;
			continue;
		} 
		if ((c=='^') && (invoce==1))
		{
			scelta[nitems].desc[ii]=0;
			invoce=0;
			vscreen[y][x++]=32;
			c=c0;
			nitems++;
			continue;
		}
		if ((c<32) || (c>126))
			return (3);
		if (ii>0)
			vscreen[y][x++]=c;
		else
			vscreen[y][x++]=toupper(c);
		attrb[y][x]=nitems;
		if (ii>0)
			scelta[nitems].desc[ii++]=c;
 		else
			scelta[nitems].desc[ii++]=toupper(c);
		scelta[nitems].len++;
		c0=c;
		continue;
	}
	if (c=='-')
	{
		attrb[y][x]= -3;
		vscreen[y][x++]=32;
		c0=c;
		continue;
	}
	if (c=='|')
	{
		attrb[y][x]= -4;
		vscreen[y][x++]=32;
		c0=c;
		continue;
	}
	if (c=='\\')
	{
		attrb[y][x]= -2;
		vscreen[y][x++]=32;
		c0=c;
		continue;
	}
	if (c=='@')
	{
		attrb[y][x]= -1;
		vscreen[y][x++]=32;
		if (r == -1)
			r= 1;
		else
			r= -1;
		c0=c;
		continue;
	}	
	if (c=='\n')
	{
		for(ii=x;ii<=79;ii++)
			vscreen[y][ii]=32;
		vscreen[y][ii]=0;
		y++;
		x=0;
		continue;
	}
 	if ((c<32)||(c>127))
		continue;
	vscreen[y][x++]=c;
	if ((c=='&') && (c0 != '$'))
	{   
		ix=x-1;
		iy=y;
		vscreen[y][x-1]=' ';
	}
	if ((c=='&') && (c0=='$'))
	{   
		vscreen[y][x-1]=' ';
		vscreen[y][x-2]=' ';
	}
	if ((c=='%') && (c0=='$'))
	{   
		vscreen[y][x-1]=' ';
		vscreen[y][x-2]=' ';
	}
	c0=c;
}

if (fine==0)				/* Caricamento dei parametri Menu     */
{
	move(0,0);
	printw("File non terminato con $$");
	refresh();
	sleep(2);
	return (3);
}
while(1)
{
	c=memofile[inc++];
	if (c=='\n')
		break;
	if (inc==memoinc)
	{
		alert(" ATTENZIONE ", " MANCA FINE FILE 1 ");
		return (3);
	}
}
while(1)
{
	if ((kk=getmemo(linea)) == -1)
	{
		alert(" ATTENZIONE ",\
		" MANCA FILE FILE 2, VERIFICARE VOCI MENU ");
		return (3);
	}
	if ((linea[0]=='%')&&(linea[1]=='%'))
		break;
	if (linea[0] != '%')
	{
		alert(" ATTENZIONE ",\
		" MANCA %% PRIMA DELL' INDICE VOCE ");
	}
	buf0[0]=0;	/* Azzero Indice Voci numeriche */
	buf0[1]=0;
	buf0[2]=0;
	sn= -1;
	c1=0;
	c0=toupper(linea[1]);
	if (strlen(linea) > 2)
		c1=linea[2];
	if (senum(c0) ||  senum(c1))
	{
		buf0[0]=c0;
		if (c1)
			buf0[1]=c1;
		sn=atoi(buf0)+1000;
	}
	ii=0;
	buf1[0]=0;
	buf1[2]=0;
	for (t=1;t<=nitems;t++)
	{
		if (sn == -1)
		{
			if (c0==scelta[t].desc[0])
			{
				ii=t;
				break;
			}
		}
		else
		{
			buf1[0]=scelta[t].desc[0];
			buf1[1]=scelta[t].desc[1];
			if ( sn == (atoi(buf1)+1000))
			{
				ii=t;
				break;
			}
		}
	}

/* 	if (ii == 0)			Segnala errore se le Voci Menu non
	{				corrispondono ai punti %% inseriti
		 move(22,0);
		 standout();
		 printw("Cross-Reference  Error ...");
		 standend();
		 refresh();
	}
*/
	kk=getmemo(linea);
	if (kk== -1)
		 return (3);
	strcpy(scelta[ii].execute,linea);
	kk=getmemo(linea);
	if (kk== -1)
		 return (3);
	strcpy(scelta[ii].msg,linea);

	kk=getmemo(linea);
	if (kk== -1)
		 return (3);
	strncpy(scelta[ii].passw,linea,11);
}

ik=ipmenu;					/* Valori di Default	   */
keydisp=1;
for (t=0;t<=9;t++)
{
	tfu[t][0]=0;
	pfu[t][0]=0;
	strcpy(lfu[t],"          ");
}

strcpy(titolo,"");				/* Interprete comandi Menu   */	
strcpy(rvs_row,"");
while((kk=getmemo(linea))!= -1)
{
	if (strncmp("TITOLO=",linea,7)==0)
		strncpy(titolo,&linea[7],38);
	if (strncmp("RVS_ROW=",linea,8)==0)
		strcpy(rvs_row,&linea[8]);
	if (strncmp("EDITOR=",linea,7)==0)
		strcpy(editor,&linea[7]);
	if (strncmp("SHELL=",linea,6)==0)
		strcpy(shell,&linea[6]);
	if (strncmp("FZ1=",linea,4)==0)
		strcpy(tfu[0],&linea[4]);
	if (strncmp("FZ2=",linea,4)==0)
		strcpy(tfu[1],&linea[4]);
	if (strncmp("FZ3=",linea,4)==0)
		strcpy(tfu[2],&linea[4]);
	if (strncmp("FZ4=",linea,4)==0)
		strcpy(tfu[3],&linea[4]);
	if (strncmp("FZ5=",linea,4)==0)
		strcpy(tfu[4],&linea[4]);
	if (strncmp("FZ6=",linea,4)==0)
		strcpy(tfu[5],&linea[4]);
	if (strncmp("FZ7=",linea,4)==0)
		strcpy(tfu[6],&linea[4]);
	if (strncmp("FZ8=",linea,4)==0)
		strcpy(tfu[7],&linea[4]);
	if (strncmp("FZ9=",linea,4)==0)
		strcpy(tfu[8],&linea[4]);
	if (strncmp("FZ10=",linea,5)==0)
		strcpy(tfu[9],&linea[5]);
	if (strncmp("ET1=",linea,4)==0)
		strncpy(lfu[0],&linea[4],18);
	if (strncmp("ET2=",linea,4)==0)
		strncpy(lfu[1],&linea[4],18);
	if (strncmp("ET3=",linea,4)==0)
		strncpy(lfu[2],&linea[4],18);
	if (strncmp("ET4=",linea,4)==0)
		strncpy(lfu[3],&linea[4],18);
	if (strncmp("ET5=",linea,4)==0)
		strncpy(lfu[4],&linea[4],18);
	if (strncmp("ET6=",linea,4)==0)
		strncpy(lfu[5],&linea[4],18);
	if (strncmp("ET7=",linea,4)==0)
		strncpy(lfu[6],&linea[4],18);
	if (strncmp("ET8=",linea,4)==0)
		strncpy(lfu[7],&linea[4],18);
	if (strncmp("ET9=",linea,4)==0)
		strncpy(lfu[8],&linea[4],12);
	if (strncmp("ET10=",linea,5)==0)
		strncpy(lfu[9],&linea[5],6);
	if (strncmp("PW1=",linea,4)==0)
		strcpy(pfu[0],&linea[4]);
	if (strncmp("PW2=",linea,4)==0)
		strcpy(pfu[1],&linea[4]);
	if (strncmp("PW3=",linea,4)==0)
		strcpy(pfu[2],&linea[4]);
	if (strncmp("PW4=",linea,4)==0)
		strcpy(pfu[3],&linea[4]);
	if (strncmp("PW5=",linea,4)==0)
		strcpy(pfu[4],&linea[4]);
	if (strncmp("PW6=",linea,4)==0)
		strcpy(pfu[5],&linea[4]);
	if (strncmp("PW7=",linea,4)==0)
		strcpy(pfu[6],&linea[4]);
	if (strncmp("PW8=",linea,4)==0)
		strcpy(pfu[7],&linea[4]);
	if (strncmp("PW9=",linea,4)==0)
		strcpy(pfu[8],&linea[4]);
	if (strncmp("PW10=",linea,5)==0)
		strcpy(pfu[9],&linea[5]);
	if (strncmp("KEYDISP",linea,7)==0)
		keydisp=1;
	if (strncmp("NOKEYDISP",linea,9)==0)
		keydisp=0;
}
if (time_flag)
	orolog();
return 0;
}

dispmenu()  					/* MOSTRA LA VIDEATA MENU     */
{						/* ======================     */
int i,t,r,ia;
int scan_pointer=0;
int item=0,a=0;
r= -1;
if(strlen(rvs_row)>1)
{
	while(scan_pointer<strlen(rvs_row))
	{
		if(rvs_row[scan_pointer]==',')
		{
			rvs_tmp[item][a]='\0';
			scan_pointer++;
			item++;
			a=0;
		}
		else
		{
			rvs_tmp[item][a]=rvs_row[scan_pointer++];
			a++;
		}
	}
	rvs_tmp[item][a]='\0';
	for(ia=0;ia<=item;ia++)
	{
		rvs_r=atoi(rvs_tmp[ia]);
		attrb[rvs_r][0]= -1;
		attrb[rvs_r][79]= -1;
	}
}

for (t=0;t<MAXR;t++)				/* Display della Pagina       */
{
	move(t,0);
    	for (i=0;vscreen[t][i];i++)
	{
		if (attrb[t][i] == -1)
		{
			if ( r == -1)
			{
				standout();
				r = 1;
			}
			else
			{
				addch(vscreen[t][i]);
				standend();
				r = -1;
			}
			addch(vscreen[t][i]);
		}
		else
		{
			if (attrb[t][i]== -3)
			{
				grafic(ROW);
				continue;
			}
			if (attrb[t][i]== -4)
			{
				grafic(COL);
				continue;
			}
			if (attrb[t][i]== -2)
			{
				if (r == -1)
				{
					standout();
					addch(vscreen[t][i]);
					standend();
				}
				else
				{
					standend();
					addch(vscreen[t][i]);
					standout();
				}
			}		
			else
				addch(vscreen[t][i]);
		}
	}
}

if (keydisp==1)					/* Display Label Tasto Funz.  */
{
	for (t=0;t<=9;t++)
	{
		if (tfu[t][0])
		{
			standout();
			move(23,(t*8));
			if (t<9)
				printw("F%.1d",t+1);
			else
				printw("F0");
			standend();
			if (lfu[t][0])
				addstr(lfu[t]);
			refresh();
		}
	}
}
refresh();
smove(iy,ix);
if(time_flag)
	orolog();
if(mnuname_flag)
	menu_name_disp();
if(title_flag)
{
	if(title_g=='L')
		move(title_y,title_x);
	if(title_g=='R')
		move(title_y,title_x-strlen(titolo));
	if(title_g=='C')
		move(title_y,38-(strlen(titolo)/2)+1);
	if(title_g=='E')
		move(title_y,79-(strlen(titolo)+2));
	if(title_g=='B')
		move(title_y,0);
	if(title_rvs==ON)
		standout();
	printw(" %s ",titolo);
	standend();
	refresh();
}
if(mnutrace_flag)
	mnutrace();
refresh();
}

int selectnext(int c)   	/* Cerca Argomento successivo da Frecce */
{
int y,x,t;
y=scelta[curitem].y;
x=scelta[curitem].x;
if (c==C_U)
{
	y--;
	do 
	{
		if(attrb[y][x]>0)
		{
			curitem=attrb[y][x];	
			break;
		}
	} while (--y);
	return(curitem);
}
if (c==C_D)
{	
	y++;
	do
	{
		if(attrb[y][x]>0)
		{
			curitem=attrb[y][x];	
			break;
		}
	} while ((++y) < MAXR);
	return(curitem);
}
if (c==C_R)
{
	while (attrb[y][x++]>0);
	do
	{
		if(attrb[y][x]>0)
		{
			curitem=attrb[y][x];
			break;
		} 
	} while((++x)<MAXC);
	return(curitem);
}
if (c==C_L)
{
	x--;
	do	
	{
		if(attrb[y][x]>0)
		{
			curitem=attrb[y][x];
			break;
		}
	} while((--x)>=0);
        return(curitem);
}
return(curitem);
}

int getline(char *tamp)			/* Memorizza una riga fino al new-line*/
{
int ii,c;
ii=0;
while((c=getc(infile)) != EOF)
{
	if (c=='\n')
	{
		tamp[ii]=0;
		return(ii);
	}
	tamp[ii++]=c;
	tamp[ii]=0;
}
return(EOF);
}

int getmemo(char *tamp)			/* Memorizza una riga dall'array file */
{
int ii,c;
ii=0;
while(inc!=memoinc)
{
	c=memofile[inc++];
	if (c=='\n')
	{
		tamp[ii]=0;
		return(ii);
	}
	tamp[ii++]=c;
	tamp[ii]=0;
}
return(-1);
}
/*
chkentry()
{
char cmpred[40];
char tmp[40];

win(" INSERIMENTO ", 8,15,12,65);
standout();
move(10,17);
printw(" Valore variabile  : ");
standend();
printw("                        ");
move(10,38);
refresh();
getentry(cmpred);
standend();
return;
}
*/
int chkpass(int tipo) 			/* Controllo parola Chiave  */
{
char cmpred[40]; 			/* Variabile utilizzata per paragone */
char tmp_passwd[26];
int ii=0;

switch(tipo)
{
	case MENU:
		strcpy(tmp_passwd,scelta[curitem].passw);
		break;
	case SHELL:
		strcpy(tmp_passwd, sh_passwd);
		break;
	case EDIT:
		strcpy(tmp_passwd, edit_passwd);
		break;
	case SUPER_MENU:
		strcpy(tmp_passwd, super_passwd);
		break;
	case PASSWD_MODE:
		strcpy(tmp_passwd, super_passwd);
		break;
	default :
		strcpy(tmp_passwd,pfu[tipo]);
		break;
}

while(ii<=strlen(tmp_passwd))
{
	tmp_passwd[ii]=toupper(tmp_passwd[ii]);
	ii++;
}

tmp_passwd[ii]='\0';

win(" ACCESSO RISERVATO ", 8,15,12,65);
move(12,17);
printw(" F4 - Esce ");
standend();
move(10,17);
printw(" Inserire Password : ");
standout();
printw("                        ");
move(10,38);
refresh();
getpasswd(cmpred);
if (strcmp(cmpred,"ABORT") == 0) 
	return 1;
standout();
if (strcmp(cmpred,tmp_passwd) != 0) 
{
	alert(" ATTENZIONE ", " PAROLA CHIAVE ERRATA ");
	clear();
	dispmenu();
	selez(1);
	return 1 ;
}
else
	return 0;
}

int chkinit() 				/* Controllo parola Chiave  */
{
char cmpred[40]; 			/* Variabile utilizzata per paragone */
char tmp_passwd[26];
int ii=0;

	win(" ACCESSO RISERVATO ", 8,15,12,65);
	move(12,17);
	printw(" F4 - Esce ");
	standend();
	move(10,17);
	printw(" Inserire Password : ");
	standout();
	printw("                        ");
	move(10,38);
	refresh();
	getpasswd(cmpred);
	if (ricerca_passwd(cmpred))
	{
		alert(ability_user, " ACCESSO ALLA PROCEDURA ABILITATO ");
		return 1;
	}
	else
	{
	alert(" ACCESSO NEGATO ", \
	" L' UTENTE ATTUALE NON HA I PERMESSI NECESSARI PER CONTINUARE ");
	return 0;
	}
}

int ricerca_passwd(char *etichetta)
{
int opi;
int c, c0, a=0;
char tmp_linea[256];
char linea[256];

	if ((infile=fopen("qmenu.cfg","r")) == NULL )
	{
		history_string("ERRORE, non riesco ad aprire qmenu.cfg");
       		return 10;
	}
	else
	{
		while((opi=getline(linea)) != EOF) 
		{
			a=0;
			while(linea[a]!=0)
			{
				if (linea[a]!='=')
					tmp_linea[a]=linea[a++];
				else
					break;
			}
			tmp_linea[a]='\0';

			if (strcmp(etichetta,tmp_linea)==0)
			{
				strcpy(ability_user, &linea[a+1]);
				fclose(infile);
				return 1;
			}

		}
	fclose(infile);
	return 0;
	}
}
inpsel(int op)				/* Mostra selezioni tastiera a video  */
{
char buf1[3];
buf1[2]=0;
if (op != 1)
{
	smove(iy,ix-1);
	printw("??");
	refresh();
	return;
}

if (senum(scelta[curitem].desc[0]) || senum(scelta[curitem].desc[1]))
{
	buf1[0]=scelta[curitem].desc[0];
	buf1[1]=scelta[curitem].desc[1];
	smove(iy,ix-1);
	printw("%.2d",atoi(buf1));
}
else
{
	smove(iy,ix-1);
	addch(' ');
	addch(scelta[curitem].desc[0]);
}
} 
	
selez(int op)   		/* Attiva e Disattiva Evidenziatore scelte   */
{
if(curitem==0)
	return;
move(scelta[curitem].y,scelta[curitem].x);
if (op)
	standout();
else
	standend();
printw("%s",scelta[curitem].desc);
standend();
move(scelta[curitem].y,scelta[curitem].x-1);
if (op)
	addch(LEFT_SYMBOL);
else
	addch(' ');
move(scelta[curitem].y,scelta[curitem].x+strlen(scelta[curitem].desc));
if (op)
	addch(RIGHT_SYMBOL);
else
	addch(' ');
standend();
refresh();
}

history_string(char *message)		/* Memorizza message in un LOGFILE */
{

struct stat stbuf;

char datario[10];
/*
int c;

c=setuid(0);
*/

dataoggi(datario);
if (stat("qmenu.log", &stbuf)== -1) 	/* Se file non c'e' lo crea a 0 bytes */
{
	system("echo \"\" > qmenu.log");
	return;
}
if (stbuf.st_size > logfile_dim)	/* Se dim. max superata copio in bak */
	system("mv  qmenu.log qmenu.log.bak");

if((ofile=fopen("qmenu.log","a+")) != NULL)
{  
	fprintf(ofile,"");
	fprintf(ofile,"%-10s, ",utente);
	if (passwd_mode)
		fprintf(ofile,"%s, ",ability_user);
	fprintf(ofile,"%-10s, ",porta);
	fprintf(ofile,"%s, ",datario);
	
	fprintf(ofile,"%d.%d.%d\n", tptr->tm_hour,             tptr->tm_min,tptr->tm_sec);
	fprintf(ofile,"%-13s, ", imenu[ipmenu]);
	fprintf(ofile,"%s", message);
	fprintf(ofile,"\n");
}
else
{
	alert(" ATTENZIONE ", "ERRORE IN SCRITTURA LOG");
	clear();
	dispmenu();
	selez(1);
	refresh();
}
fclose(ofile);
}

orolog()				        /* Stampa Orario a Video */
{

int ly,lx;

t1=time((time_t *)0);  

tptr=localtime(&t1);

getyx(stdscr,ly,lx);
if(time_g=='L')
	move(time_y,time_x);
if(time_g=='R')
	move(time_y,time_x-5);
if(time_g=='C')
	move(time_y,38-5);
if(time_g=='E')
	move(time_y,79-5);
if(time_g=='B')
	move(time_y,0);
if(time_rvs==ON)
	standout();
else 
	standend();

printw("%.2d:%.2d\n", tptr->tm_hour,   tptr->tm_min);

standend();
move(ly,lx);
refresh();
intflg=1;
}

menu_name_disp()			/* Stampa il nome del menu a Video */
{
if(mnuname_g=='L')
	move(mnuname_y,mnuname_x);
if(mnuname_g=='R')
	move(mnuname_y,mnuname_x-strlen(imenu[ipmenu]));
if(mnuname_g=='C')
	move(mnuname_y,38-(strlen(imenu[ipmenu])/2)+1);
if(mnuname_g=='E')
	move(mnuname_y,79-(strlen(imenu[ipmenu])+2));
if(mnuname_g=='B')
	move(mnuname_y,0);
if(mnuname_rvs)
	standout();
printw(" %s ",imenu[ipmenu]);

standend();
refresh();
}

scarica()    /* Scarica residuo tastiera dopo esecuzione comando solo SCO  */
{
#ifdef  SCO
while(rdchk(fileno(stdin)))
	getc(stdin);
#endif
return;
}

void templim() 		/* Uscita da programma se l'utente non preme tasti */
{
alert(" ATTENZIONE ", \
" TEMPO MASSIMO DI INATTIVITA' RAGGIUNTO, ESECUZIONE TERMINATA ");
sleep(9);
endwin();
sleep(1);
exit(0);
}

/* Trap Interruzione Utente (^C o DEL , BREAK , etc ) */
/*
void fine() 
{
switch(ON)
{
    case 1:
	alert(" ATTENZIONE "," ESECUZIONE TERMINATA, SEGNALE UTENTE RICEVUTO ");
	resetty();
	sleep(1);
	endwin();
	sleep(1);
	exit(9);
	break;
    case 2:
	resetty();
	sleep(1);
	endwin();
	perror("FPE ERROR !!!!");
	sleep(1);
	exit(SIGFPE);
	break;
    case 3:
	resetty();
	sleep(1);
	endwin();
	sleep(1);
	exit(SIGBUS);
	break;
    case 4:
	endwin();
	alert(" ATTENZIONE ", " ERRORE DI MEMORIA, ESECUZIONE SOSPESA ");
	abort(SIGSEGV);
	break;
    case 5:
	alert(" ATTENZIONE ", " SEGNALE INTERRUZIONE ESTERNO RICEVUTO ");
	resetty();
	sleep(1);
	endwin();
	sleep(1);
	exit(SIGTERM);
	break;
}
}
*/

void fine1()  /* Trap Interruzione Utente (^C o DEL , BREAK , etc ) */
{
alert(" ATTENZIONE ", " SEGNALE UTENTE RICEVUTO, ESECUZIONE TERMINATA ");
resetty();
sleep(1);
endwin();
sleep(1);
exit(9);
}

void fine2()
{
resetty();
sleep(1);
endwin();
perror("FPE ERROR !!!!");
sleep(1);
exit(SIGFPE);
}

void fine3()
{
resetty();
sleep(1);
endwin();
sleep(1);
exit(SIGBUS);
}

void fine4()
{
alert(" ATTENZIONE ", " ERRORE DI MEMORIA, ESECUZIONE SOSPESA ");
endwin();
abort(SIGSEGV);
}

void fine5()
{
alert(" ATTENZIONE ", " SEGNALE ESTERNO RICEVUTO, ESECUZIONE TERMINATA ");
refresh();
resetty();
sleep(1);
endwin();
sleep(1);
exit(SIGTERM);
}

smove(int y, int x) /* Slow move , move ritardata per problemi di refresh */
{
if (y<0) y=0;
if (y>79) y=23;
if (x<0) x=0;
if (x>79) x=0;
move(0,0);
refresh();
move(y,x);
}

char *cercadop(char *string0, char *string1)
{
int t;
for (t=0; t <= (strlen(string0)-strlen(string1)) ; t++)
	if (strncmp(&string0[t],string1,strlen(string1)) == 0)
		return(&string0[t+strlen(string1)]);
return (NULSTRING);
}

int senum(numero)		/* Verifica se l'argomento e' un numero */
{
if ((numero >= '0') && (numero <= '9'))
	return(1);
else 
	return(0);
}

esegui(char *comando)  /* Esecutore comandi in ambiente shell prescelto */
{
if (strncmp(comando,"prsuper",7) == 0)
{
	selspool(SUPER);
	return(0);
}
if (strncmp(comando,"config",7) == 0)
{
	display_config();
	return(0);
}
if (strncmp(comando,"pruser",6) == 0)
{
	selspool(USER);
	return(0);
}
/*
if (strncmp(comando,"getentry",8) == 0)
{
	chkentry();
	return(0);
}
*/
signal(SIGINT,SIG_DFL);
endwin();
if (comando[0] == '~')
{
	printw("%s",comando);
	refresh();
	junk=getch();
	exit(system(&comando[1]));
}
system(comando);
initscr();
noecho();
cbreak();
scarica();
crmode();
noecho();
refresh();
standend();
signal(SIGINT,fine1);
return;
}

selspool(int type_sel) 			/* Selezione le stampanti */
{
int nprts;
int row_pos, col_pos;
char prts[16][32];
char p[30];
char prlist[256];
char linea[32];
int ia=0,iu,max_len=0,c,t,scan_pointer=0,a=0,item=0;
nprts=0;

if(type_sel==SUPER)
{
	if (super)
		strcpy(prlist,prsuper);
	else
	{
		alert(" ATTENZIONE ", \
		" STAMPANTI SUPERUTENTE NON DEFINITE NELLA VARIABILE PRSUPER ");
		return(0);
	}
}
if(type_sel==USER)
{
	if (user)
		strcpy(prlist,pruser);
	else
	{
		alert(" ATTENZIONE ", \
		" STAMPANTI UTENTE NON DEFINITE NELLA VARIABILE PRUSER ");
		return(0);
	}
}

while(scan_pointer<strlen(prlist))
{
	if(prlist[scan_pointer]==',')
	{
		prts[item][a]='\0';
		scan_pointer++;
		item++;
		a=0;
	}
	else
	{
		prts[item][a]=prlist[scan_pointer++];
		a++;
	}
}

for(ia=0,max_len=0;ia<item;ia++)
{
	if(strlen(prts[ia])>max_len)
		max_len=strlen(prts[ia]);
}

nprts=item+1;
prts[item][a]='\0';
if (max_len<10)
	max_len=11;
row_pos=12-(nprts/2)-2;
col_pos=38-(max_len/2);
win(" Printer ", row_pos-1,col_pos-2,row_pos+nprts,col_pos+max_len);
standend();
for(iu=0;iu<nprts;iu++)
{
	move(row_pos+iu,col_pos-1);
	printw(" %s",prts[iu]);
}
standout();

for(t=0;t<nprts;t++)
	if (strcmp(spoolernm,prts[t])  == 0)
		break;
if (t >= nprts)
	t=0;
standend();
move(row_pos+t,col_pos-1);
printw("%c",LEFT_SYMBOL);
standout();
move(row_pos+t,col_pos);
printw(prts[t]);
while(1)
{
	refresh();
	c=getkey();
	if ((c == C_U) || (c == '8'))
	{
		standend();
		move(row_pos+t,col_pos-1);
		addstr("    ");
		move(row_pos+t,col_pos);
		printw(prts[t]);
		t--;
		if (t < 0)
			t=nprts-1;
		move(row_pos+t,col_pos-1);
		printw("%c",LEFT_SYMBOL);
		standout();
		move(row_pos+t,col_pos);
		printw(prts[t]);
		standend();
	}
	if ((c == C_D) || (c == '2'))
	{
		standend();
		move(row_pos+t,col_pos-1);
		addstr("    ");
		move(row_pos+t,col_pos);
		printw(prts[t]);
		t++;
		if (t >= nprts)
			t=0;
		move(row_pos+t,col_pos-1);
		printw("%c",LEFT_SYMBOL);
		standout();
		move(row_pos+t,col_pos);
		printw(prts[t]);
		standend();
	}
	if (c == NL)
	{
		standend();
		break;
	}
}
strcpy(lpdestval,"LPDEST=");
strcat(lpdestval,prts[t]);
strcpy(lpdestenv, prts[t]);
putenv(lpdestval);
ia=loadmenu(imenu[ipmenu]);
return(0);
}

/* ========================================================================== */
int ext(char *label)		/* Estrazione per Help della label  */
{
	char head[500];
	signed char c;
	int h;
	int a=0,kk=0;
	int k=1;
	int found=0;

	if ((helpfile=fopen(hlp_filename,"r"))==NULL)
	{
		return(1);
	}
	/* - Scansione file ------------------------------------------- */
	while((c=getc(helpfile)) != EOF)
	{
		if (c == '[')
		{
			while((c=getc(helpfile)) != ']')
 				head[a++]=c;
			head[a]='\0';
			a=0;
			if (strcmp(head,label) == 0)  
			{
				while(h!='[')
				{
					found=1;
					a=getline(help_line[kk]);
					h=help_line[kk][0];
					kk++;
				}
				help_item=--kk;
				help_line[kk][0]='\0';
			}
		}
		
	}
	fclose(helpfile);
	if(found)
	{
		helper();
		for(a=0;a!=help_item;a++)
			help_line[a][0]='\0';
	}
}

void helper()
{
int a, pippo, j=0;
int x=0,g=0,ch;
int c=0, r=0;

	for(a=0; a!=help_item; a++)
	{
		if(strlen(help_line[x])>j)
			j=strlen(help_line[x]);
		x++;
	}
	x--;

	win(" HELP ", \
	X_COORD-1, Y_COORD-2, X_COORD+MAX_ROW, Y_COORD+MAX_COL, 1);
	move(X_COORD+MAX_ROW, Y_COORD+3);
	printw(" F4 - Esce ");
	move(X_COORD+MAX_ROW, Y_COORD+41);
	printw(" <=Sx, >=Dx, ^=Su, v=Giu' ");

	disphelp(1,0,0,0);
	
	while(1)
	{
		move(X_COORD-1, Y_COORD+55);
		printw("[%3i,%3i]", r, c);
 		ch=getkey();
	
		if(ch==HOME)
		{
			c=0;
			r=0;
			disphelp(1,0,0,0);	
		}

		if(ch==END)
		{
			c=0;
			if(help_item < MAX_ROW)
				beep();
			else
			{
				beep();
				r=x-MAX_ROW+1;
				disphelp(1,0,r,c);	
			}
		}

		if(ch==C_R)
		{
			if(c+MAX_COL<j)
				disphelp(1,0,r, ++c);	
			else
				beep();
		}

		if(ch==C_L)
		{
			if(c != 0)
				disphelp(1,0,r, --c);	
			else
				beep();
		}

		if(ch==C_D)
		{
			if(r+(MAX_ROW-1) < x)
				disphelp(1,0,++r, c);	
			else
				beep();
		}
		if(ch==C_U)
		{
			if(r>0)
				disphelp(1,0,--r,c);	
			else
				beep();
		}
		if(ch==PGDN)
		{
			if(help_item < MAX_ROW)
				beep();
			else
			{
				if(x-(r+MAX_ROW) < MAX_ROW)
				{
					r=x-MAX_ROW+1;
					beep();
				}
				else
					r=r+MAX_ROW;
				disphelp(1,0,r,c);	
			}
		}
		if(ch==PGUP)
		{
			if(r<MAX_ROW)
			{
				beep();
				r=0;
			}
			else
				r=r-MAX_ROW;
			disphelp(1,0,r,c);	
		}
		if(ch==FZ4)
			return;
		
	}
}

disphelp(int row, int col, int first_row, int first_col)
{
int  g, k;
int  tot_righe;
char buffer[255];
int  lun;
	
	for(g=first_row, tot_righe=0; tot_righe != MAX_ROW; g++, tot_righe++)
	{
		if (strlen(&help_line[g][0]) < MAX_COL)
			lun=strlen(&help_line[g][0]);
		else
			lun=MAX_COL;

		strncpy(buffer, &help_line[g][first_col], lun);
		buffer[lun]='\0';
		if(strlen(buffer)<MAX_COL)
		{
			for(k=strlen(buffer);k!=MAX_COL;k++)
				buffer[k]=32;
			buffer[k]='\0';
			
		}
		move(X_COORD+tot_righe, Y_COORD);
		addstr(buffer);
	}
	refresh();
}

/* ========================================================================== */

alert(char *titolo, char *testo)      /* Giustifica al centro una finestra di */
{				      /* allarme 			      */
int len_text;
int start_col, ii;

len_text=strlen(testo);
start_col=40-(len_text/2);
/* standout(); */
move(8,start_col);
grafic(TL);
for(ii=1;ii<=len_text;ii++)
	grafic(ROW);
grafic(TR);
move(9,start_col-1);
grafic(SHADOW);
move(9,start_col);
grafic(COL);
for(ii=1;ii<=len_text;ii++)
	printw(" ");
grafic(COL);

move(10,start_col-1);
grafic(SHADOW);
grafic(COL);
printw("%s",testo);
grafic(COL);

move(11,start_col-1);
grafic(SHADOW);
grafic(COL);
for(ii=1;ii<=len_text;ii++)
	printw(" ");
grafic(COL);

move(12,start_col-1);
grafic(SHADOW);
grafic(BL);
for(ii=1;ii<=len_text;ii++)
	grafic(ROW);
grafic(BR);
move(13,start_col-1);
grafic(SHADOW);
for(ii=1;ii<=len_text+1;ii++)
	grafic(SHADOW);
/* standend(); */
standout();
move(8,40-strlen(titolo)/2);
printw("%s", titolo);
standend();
refresh();
beep();
sleep(2);
}

win(char *titolo, int x, int y, int z, int k)    /* Finestra con titolo */
{
int largh=0, altez=0;
int ii=0, ia=0;

largh=k-y;
altez=z-x;

move(x,y);
grafic(TL);
for(ii=1;ii<=largh;ii++)
	grafic(ROW);
grafic(TR);

for(ii=1;ii<altez;ii++)
{
	move(x+ii,y-1);
	grafic(SHADOW);
	grafic(COL);
	for(ia=1;ia<=largh;ia++)
		printw(" ");
	grafic(COL);
}

move(x+ii,y-1);
grafic(SHADOW);
grafic(BL);
for(ia=1;ia<=largh;ia++)
	grafic(ROW);
grafic(BR);

move(x+ii+1,y-1);
for(ii=0;ii<=largh+1;ii++)
	grafic(SHADOW);
if(titolo)
{
	move(x,1+(y+(k-y)/2)-strlen(titolo)/2);
	standout();
	printw("%s", titolo);
	standend();
}
}

hlpmsg()  			/* Stampa messaggio di Help */
{
int op=0;
int x_coord;

if(msg_g=='L')
	x_coord=msg_x;
if(msg_g=='R')
	x_coord=msg_x-strlen(scelta[curitem].msg);
if(msg_g=='C')
	x_coord=38-strlen(scelta[curitem].msg)/2;
if(msg_g=='E')
	x_coord=79-strlen(scelta[curitem].msg);
if(msg_g=='B')
	x_coord=0;

move(msg_y,0);
for(op=0;op<80;op++)
	printw(" ");

if(msg_rvs==ON)
	standout();

move(msg_y,x_coord);
printw("%s",scelta[curitem].msg);
standend();
refresh();
}

mnutrace()			/* Stampa traccia menu */
{
char riga[120];
int io;

strcpy(riga, "");
for(io=0;io<=ipmenu;io++)
{
	strcat(riga, imenu[io]);
	strcat(riga, " ");
}
strcat(riga, "\0");

if(mnutrace_g=='L')
	move(mnutrace_y,mnutrace_x);
if(mnutrace_g=='R')
	move(mnutrace_y,mnutrace_x-strlen(riga));
if(mnutrace_g=='C')
	move(mnutrace_y,38-(strlen(riga)/2));
if(mnutrace_g=='E')
	move(mnutrace_y,79-(strlen(riga)));
if(mnutrace_g=='B')
	move(mnutrace_y,0);
if(mnutrace_rvs==ON)
	standout();

printw("%s", riga);
standend();
refresh();
}
					/* Sintattica simboli 	*/
simb_pos(char *simb_name, int y_pos, int x_pos, int gst)
{
int reverse;
	if (gst=='R')
		x_pos=x_pos+strlen(simb_name)+2;
	if ((gst=='L') || (gst=='C') || (gst=='R') || (gst=='E') || (gst=='B'))
		reverse=ON;
        if ((gst=='l') || (gst=='c') || (gst=='r') || (gst=='e') || (gst=='b'))
		reverse=OFF;
	gst=toupper(gst);
	if(strcmp(simb_name, "MENUNAME") == 0)
	{
		mnuname_rvs=reverse;
		mnuname_flag=ON;
		mnuname_g=gst;
		mnuname_y=y_pos;
		mnuname_x=x_pos;
	}
	if(strcmp(simb_name, "TIME") == 0)
	{
		time_rvs=reverse;
		time_flag=ON;
		time_g=gst;
		time_y=y_pos;
		time_x=x_pos;
	}
	if(strcmp(simb_name, "MSG") == 0)
	{
		msg_rvs=reverse;
		msg_flag=ON;
		msg_g=gst;
		msg_y=y_pos;
		msg_x=x_pos;
	}
	if(strcmp(simb_name, "TITLE") == 0)
	{
		title_rvs=reverse;
		title_flag=ON;
		title_g=gst;
		title_y=y_pos;
		title_x=x_pos;
	}
	if(strcmp(simb_name, "INFOTERM") == 0)
	{
		infoterm_rvs=reverse;
		infoterm_flag=ON;
		infoterm_g=gst;
		infoterm_y=y_pos;
		infoterm_x=x_pos;
	}
	if(strcmp(simb_name, "MNUTRACE") == 0)
	{
		mnutrace_rvs=reverse;
		mnutrace_flag=ON;
		mnutrace_g=gst;
		mnutrace_y=y_pos;
		mnutrace_x=x_pos;
	}
}

infoterm()		/* Stampa Info Terminale a Video */
{
int x_coord;
char riga[80];

sprintf(riga, "%s %s:%s,[%s] %s",sistema, utente, porta, termname(), lpdestenv);
if(infoterm_g=='L')
	move(infoterm_y,infoterm_x);
if(infoterm_g=='R')
	move(infoterm_y,infoterm_x-strlen(riga));
if(infoterm_g=='C')
	move(infoterm_y,38-(strlen(riga)/2));
if(infoterm_g=='E')
	move(infoterm_y,79-(strlen(riga)));
if(infoterm_g=='B')
	move(infoterm_y,0);
if(infoterm_rvs==ON)
	standout();

printw("%s", riga);
standend();
refresh();
}

confirm()
{
int nprts;
int row_pos, col_pos;
char tag_item[16][32];
char p[30];
char prlist[256];
char linea[32];
int ia=0,iu,max_len=12,c,t,scan_pointer=0,a=0,item=0;

nprts=2;
strcpy(tag_item[0], "    No    ");
strcpy(tag_item[1], "    Si    ");

row_pos=12-(nprts/2)-2;
col_pos=38-(max_len/2);
win(" Confermi ? ",row_pos-1,col_pos-2,row_pos+nprts,col_pos+max_len);
move(row_pos-1,col_pos);
for(iu=0;iu<nprts;iu++)
{
	move(row_pos+iu,col_pos-1);
	printw(" %s",tag_item[iu]);
}
t=0;
	
standend();
move(row_pos+t,col_pos-1);
printw("%c",LEFT_SYMBOL);
standout();
move(row_pos+t,col_pos);
printw(tag_item[t]);
while(1)
{
	refresh();
	c=getkey();
	if ((c == C_U) || (c == '8'))
	{
		standend();
		move(row_pos+t,col_pos-1);
		addstr("    ");
		move(row_pos+t,col_pos);
		printw(tag_item[t]);
		t--;
		if (t < 0)
			t=nprts-1;
		move(row_pos+t,col_pos-1);
		printw("%c",LEFT_SYMBOL);
		standout();
		move(row_pos+t,col_pos);
		printw(tag_item[t]);
		standend();
	}
	if ((c == C_D) || (c == '2'))
	{
		standend();
		move(row_pos+t,col_pos-1);
		addstr("    ");
		move(row_pos+t,col_pos);
		printw(tag_item[t]);
		t++;
		if (t >= nprts)
			t=0;
		move(row_pos+t,col_pos-1);
		printw("%c",LEFT_SYMBOL);
		standout();
		move(row_pos+t,col_pos);
		printw(tag_item[t]);
		standend();
	}
	if (c == NL)
	{
		standend();
		break;
	}
}
return(t);
}

grafic(int etichetta)
{
switch(etichetta)
{
	/* Top Left */ 
	case TL :
		if(graf==0)
			printw("+");
		if(graf==1)
		{
			attrset(A_ALTCHARSET);
			printw("%s",lab_f0);
			attroff(A_ALTCHARSET);
		}
		if(graf==2)
			addch(ACS_ULCORNER);
	 	break;
	
	/* Botton Left */
	case BL :
		if(graf==0)
			printw("+");
		if(graf==1)
		{
			attrset(A_ALTCHARSET);
			printw("%s",lab_f3);
			attroff(A_ALTCHARSET);
		}
		if(graf==2)
			addch(ACS_LLCORNER);
		break;
	/* Top Right */ 
	case TR :
		if(graf==0)
			printw("+");
		if(graf==1)
		{
			attrset(A_ALTCHARSET);
			printw("%s",lab_f1);
			attroff(A_ALTCHARSET);
		}
		if(graf==2)
			addch(ACS_URCORNER);
		break;
	/* Botton Right */
	case BR :
		if(graf==0)
			printw("+");
		if(graf==1)
		{
			attrset(A_ALTCHARSET);
			printw("%s",lab_f2);
			attroff(A_ALTCHARSET);
		}
		if(graf==2)
			addch(ACS_LRCORNER);
		break;
	/* Row */
	case ROW :
		if(graf==0)
			printw("-");
		if(graf==1)
		{
			attrset(A_ALTCHARSET);
			printw("%s",lab_f4);
			attroff(A_ALTCHARSET);
		}
		if(graf==2)
			addch(ACS_HLINE);
		break;
	/* Col */
	case COL :
		if(graf==0)
			printw("|");
		if(graf==1)
		{
			attrset(A_ALTCHARSET);
			printw("%s",lab_f5);
			attroff(A_ALTCHARSET);
		}
		if(graf==2)
			addch(ACS_VLINE);
		break;
	/* Shadow */
	case SHADOW :
		if(graf==0)
			printw("#");
		if(graf==1)
		{
			attrset(A_ALTCHARSET);
			printw("%s",lab_f7);
			attroff(A_ALTCHARSET);
		}
		if(graf==2)
			addch(ACS_CKBOARD);
		break;
}
}

int getkey()		/* Legge un tasto premuto 		*/
{
int c;

nonl();
keypad(stdscr,TRUE);
c=getch();
if (c==' ')
	return(' ');
if (c==RET )
	return('\n');
if (c==NL)
	return(C_D);
if (c==KEY_BACKSPACE)
	return(BKSP);
if (c==DEL)
	return(DEL);
if (c==INS)
	return(INS);
if (c==KEY_UP)
	return (C_U);
if (c==KEY_DOWN)
	return (C_D);
if (c==KEY_LEFT)
	return (C_L);
if (c==KEY_RIGHT)
	return (C_R);
if (c==KEY_HOME)
	return (HOME);
if (c==KEY_END)
	return (END);
if (c==KEY_F0)
	return (FZ0);
if (c==KEY_F0+ 1)
	return (FZ1);
if (c==KEY_F0+ 2)
	return (FZ2);
if (c==KEY_F0+ 3)
	return (FZ3);
if (c==KEY_F0+ 4)
	return (FZ4);
if (c==KEY_F0+ 5)
	return (FZ5);
if (c==KEY_F0+ 6)
	return (FZ6);
if (c==KEY_F0+ 7)
	return (FZ7);
if (c==KEY_F0+ 8)
	return (FZ8);
if (c==KEY_F0+ 9)
	return (FZ9);
if (c==KEY_F0+ 10)
	return (FZ0);
if (c==KEY_F0+ 11)
	return (FZA);
if (c==KEY_F0+ 12)
	return (FZB);
if (c==KEY_PPAGE)
	return(PGUP);
if (c==KEY_NPAGE)
	return(PGDN);
if (c==ESC)
	return(ESC);
return (c);
}

getpasswd(char *entry)
{
int i,c;
int xpos,ypos;
i=0;
c=0;
standout();
while ((c != '\n') && (c != '\r')) 
{
	c=getkey();
	if (c == FZ4)
	{
		strcpy(entry,"ABORT");
		break;
	}
	if (((c >= 32) && (c < 127)) && (i<24))
	{
		printw("*");
		refresh();
		entry[i++]=toupper(c);
		entry[i]=0;
	}
	if ((c=='\b') && (i!=0))
	{
		getyx(stdscr,ypos,xpos);
		move(ypos,xpos-1);
		printw(" ");
		move(ypos,xpos-1);
		refresh();
		entry[--i]=0;
	}
}
standend();

if(i==0)
	strcpy(entry,"ABORT");
}

getentry(int xpos, int ypos, int tot_entry, char *entry)
{
int i=0,c=0,k=0;
standout();
if(strlen(entry)!=0)
{
	move(xpos, ypos);
	i=strlen(entry);
	printw("%s", entry);
}
else
{
move(xpos, ypos);
for(k=0;k<tot_entry;k++)
	printw(" ");
move(xpos, ypos);
}
while ((c != '\n') && (c != '\r')) 
{
	c=getkey();
	if (c == C_U)
	{
		strcpy(entry, PREV_FIELD);
		break;
	}
	if (c == C_D)
	{
		c='\n';
	}
	if (c == FZ4)
	{
		strcpy(entry, ABORT);
		break;
	}
	if (((c >= 32) && (c < 127)) && (i<tot_entry))
	{
		printw("%c",c);
		refresh();
		entry[i++]=c;
		entry[i]=0;
	}
	if ((c=='\b') && (i==0))
	{
		strcpy(entry, PREV_FIELD);
		break;
	}
	if ((c=='\b') && (i!=0))
	{
		getyx(stdscr,ypos,xpos);
		move(ypos,xpos-1);
		printw(" ");
		move(ypos,xpos-1);
		refresh();
		entry[--i]=0;
	}
}
standend();
}

dataoggi(char *datastr)		/* Estrae la data di oggi */
{
long tempo;

/* tempo=time(0L); */

t1=time((time_t *)0); 
tptr=localtime(&t1);

sprintf(datastr,"%d/%d/%d",\
	tptr->tm_mday,\
	1+tptr->tm_mon,\
	1900+tptr->tm_year);
}

readcfg()
{
int opi;
int c, c0;

if(getenv("LPDEST"))
	strcpy(lpdestenv,getenv("LPDEST"));

if ((infile=fopen("qmenu.cfg","r")) == NULL )	/* Apertura file di config.   */
{
	history_string("ERRORE, non riesco ad aprire il file configurazione");
       	return 10;
}
else
{
 			             /* Sostituisco le variabili d' ambiente */
	memoinc=0;
	while ((c=getc(infile)) != EOF)
	{
		if ((c=='{') && (c0=='$'))
		{   
			o=0;
			while ((en=getc(infile)) != '}')
				env_var[o++]=en;

			env_var[o]='\0';
			if (getenv(env_var))
				strcpy(env_value,getenv(env_var));
			else
				strcpy(env_value,"");

			d=0;
			memoinc=memoinc-1;
			while(d<strlen(env_value))
				memofile[memoinc++]=env_value[d++];
			continue;
		}
		memofile[memoinc++]=c;
	c0=c;
	}
	memofile[memoinc]='\0';
	fclose(infile);
	inc=0;
	while((opi=getmemo(linea)) != -1)      /* Leggo i parametri config. */
	{
		if (strncmp("MNU_TIM=",linea,8)==0)
		{
			itime_out=atoi(&linea[8]);
			if (itime_out == 0)
				mnu_tim=0;
		}

		if (strncmp("LOGFILE_DIM=",linea,12)==0)
			logfile_dim=atol(&linea[12]);

		if (strncmp("BKS_EXIT=",linea,9)==0)
			bks_exit=atoi(&linea[9]);

		if (strncmp("PASSWD_MODE=ON",linea,14)==0)
			passwd_mode=ON;

		if (strncmp("MNU_PATH=",linea,9)==0)
			strcpy(menu_dir,&linea[9]);

		if (strncmp("GRAPH=",linea,6)==0)
			graf=atoi(&linea[6]);

		if (strncmp("SH_PASSWD=",linea,10)==0)
		{
			strcpy(sh_passwd,&linea[10]);
			sh_passwd_ok=1;
		}

		if (strncmp("EDIT_PASSWD=",linea,12)==0)
		{
			strcpy(edit_passwd,&linea[12]);
			edit_passwd_ok=1;
		}

		if (strncmp("HLP_FILENAME=",linea,13)==0)
			strcpy(hlp_filename, &linea[13]);

		if (strncmp("PRSUPER=",linea,8)==0)
		{
			strcpy(prsuper,&linea[8]);
			super=ON;		
		}

		if (strncmp("PRUSER=",linea,7)==0)
		{
			strcpy(pruser,&linea[7]);
			user=ON;		
		}

		if (strncmp("MENU_SUPER_PASSWD=",linea,18)==0)
		{
			strcpy(super_passwd,&linea[18]);
			superpasswd=ON;		
		}

	}
}
}

display_config()
{
int u;

win(" FILE DI CONFIGURAZIONE (qmenu.cfg) ", 2, 2, 18, 78);
move(18,4);
printw(" F4 - Esce ");
move(4,4);
printw(" MNU_TIM     = %i ", itime_out);
move(5,4);
printw(" LOGFILE_DIM = %i ", logfile_dim);
move(6,4);
printw(" GRAPH       = %i ", graf);
move(7,4);
printw(" MNU_PATH    = %s ", menu_dir);
move(8,4);
printw(" PASSWD_MODE = %i ", passwd_mode);
move(9,4);
printw(" MENU_SUPER_PASSWD= %s ", super_passwd);
move(10,4);
printw(" HLP_FILENAME= %s ", hlp_filename);
move(11,4);
printw(" LPDEST      = %s ", lpdestenv);
move(12,4);
printw(" PRSUPER     = %s ", prsuper);
move(13,4);
printw(" PRUSER      = %s ", pruser);
refresh();
while((u=getkey())!=FZ4);
}

int win_option(char *winlist)		/* Selezione le stampanti */
{
int nprts;
int row_pos, col_pos;
char prts[16][32];
char p[30];
char linea[32];
int ia=0,iu,max_len=0,c,t,scan_pointer=0,a=0,item=0;
nprts=0;

while(scan_pointer<strlen(winlist))
{
	if(winlist[scan_pointer]==',')
	{
		prts[item][a]='\0';
		scan_pointer++;
		item++;
		a=0;
	}
	else
	{
		prts[item][a]=winlist[scan_pointer++];
		a++;
	}
}

for(ia=0,max_len=0;ia<=item;ia++)
{
	if(strlen(prts[ia])>max_len)
		max_len=strlen(prts[ia]);
}

nprts=item+1;
prts[item][a]='\0';
if (max_len<10)
	max_len=11;
row_pos=12-(nprts/2)-2;
col_pos=38-(max_len/2);
standend();
win(" OPZIONI SUPERUTENTE ", row_pos-1,col_pos-2,row_pos+nprts,col_pos+max_len);
move(row_pos+nprts,col_pos);
printw(" F4 - Esce ");

standend();
for(iu=0;iu<nprts;iu++)
{
	move(row_pos+iu,col_pos-1);
	printw(" %s",prts[iu]);
}
standout();

for(t=0;t<nprts;t++)
	if (strcmp(spoolernm,prts[t])  == 0)
		break;
if (t >= nprts)
	t=0;
standend();
move(row_pos+t,col_pos-1);
printw("%c",LEFT_SYMBOL);
standout();
move(row_pos+t,col_pos);
printw(prts[t]);
while(1)
{
	refresh();
	c=getkey();
	if ((c == C_U) || (c == '8'))
	{
		standend();
		move(row_pos+t,col_pos-1);
		addstr("    ");
		move(row_pos+t,col_pos);
		printw(prts[t]);
		t--;
		if (t < 0)
			t=nprts-1;
		move(row_pos+t,col_pos-1);
		printw("%c",LEFT_SYMBOL);
		standout();
		move(row_pos+t,col_pos);
		printw(prts[t]);
		standend();
	}
	if ((c == C_D) || (c == '2'))
	{
		standend();
		move(row_pos+t,col_pos-1);
		addstr("    ");
		move(row_pos+t,col_pos);
		printw(prts[t]);
		t++;
		if (t >= nprts)
			t=0;
		move(row_pos+t,col_pos-1);
		printw("%c",LEFT_SYMBOL);
		standout();
		move(row_pos+t,col_pos);
		printw(prts[t]);
		standend();
	}
	if ((c == ESC)||(c == FZ4))
	{
		t=-1;
		standend();
		break;
	}
	if (c == NL)
	{
		standend();
		break;
	}
}
return(t);
}
void cre_cfg()
{
if((ofile=fopen("qmenu.cfg","w")) == NULL)
{
	printf("Non riesco a scrivere il file qmenu.cfg\n");
	exit(0);
}
printf("Ricostruzione \"qmenu.cfg\"\n");
fprintf(ofile,"\
[Option]\n\
MNU_TIM=1000\n\
LOGFILE_DIM=50000\n\
GRAPH=0\n\
PASSWD_MODE=OFF\n\
MENU_SUPER_PASSWD=MENU\n\
BKS_EXIT=0\n\
\n\
[FileName]\n\
HLP_FILENAME=qmenu.hlp\n\
\n\
[Printers]\n\
PRSUPER=printer0,printer1\n\
PRUSER=printer0,printer1\n\
\n\
[Path]\n\
#MNU_PATH=${HOME}/\n\
\n\
[PassWord]\n\
#PAROLA_CHIAVE=NOMINATIVO\n\
\n");
fclose(ofile);

}

void cre_model()
{
if((ofile=fopen("model.mnu","w")) == NULL)
{
	printf("Non riesco a scrivere il file model.mnu\n");
	exit(0);
}
printf("Generazione \"model.mnu\"\n");

fprintf(ofile,"\
 Intestazione Menu Modello                                          E[TIME] \n\
--------------------------------------------------------------------------------\n\
\n\
                                   C[TITLE]\n\
\n\
            ^1.   ................................................ ^\n\
            ^2.   ................................................ ^\n\
            ^3.   ................................................ ^\n\
            ^4.   ................................................ ^\n\
            ^5.   ................................................ ^\n\
            ^6.   ................................................ ^\n\
            ^7.   ................................................ ^\n\
            ^8.   ................................................ ^\n\
            ^9.   ................................................ ^\n\
            ^0.   ................................................ ^\n\
\n\
            ^F.   Fine Lavoro                                      ^\n\
\n\
             Selezione..: [ &]\n\
\n\
                               c[MSG]\n\
--------------------------------------------------------------------------------\n\
 L[INFOTERM]                                                      E[MENUNAME]\n\
F1=Stampanti <X]=Menu' Precedente\n\
$$\n\
%%1\n\
\n\
\n\
\n\
%%2\n\
\n\
\n\
\n\
%%3\n\
\n\
\n\
\n\
%%4\n\
\n\
\n\
\n\
%%5\n\
\n\
\n\
\n\
%%6\n\
\n\
\n\
\n\
%%7\n\
\n\
\n\
\n\
%%8\n\
\n\
\n\
\n\
%%9\n\
\n\
\n\
\n\
%%0\n\
\n\
\n\
\n\
%%F\n\
fine\n\
\n\
\n\
%%%%\n\
#[Opzioni Display]\n\
#=================\n\
TITOLO=MODELLO DI MENU\n\
RVS_ROW=0,22\n\
\n\
#[Configurazione Tasti Funzione]\n\
#===============================\n\
NOKEYDISP\n\
#KEYDISP\n\
\n\
#ET1=\n\
#FZ1=\n\
#PW1=\n\
\n\
#ET2=\n\
#FZ2=\n\
#PW2=\n\
\n\
#ET3=\n\
#FZ3=\n\
#PW3=\n\
\n\
#ET4=\n\
#FZ4=\n\
#PW4=\n\
\n\
#ET5=\n\
#FZ5=\n\
#PW5=\n\
\n\
#ET6=\n\
#FZ6=\n\
#PW6=\n\
\n\
#ET7=\n\
#FZ7=\n\
#PW7=\n\
\n\
#ET8=\n\
#FZ8=\n\
#PW8=\n\
\n\
#ET9=\n\
#FZ9=\n\
#PW9=\n\
\n\
#ET10=\n\
#FZ10=\n\
#PW10=\n\
\n\
[Varie]\n\
SHELL=sh\n\
EDITOR=vi\n\
");
fclose(ofile);

}

int form(char *forma)
{
int opi, a=0;
int loop_OK=TRUE;
int trovato=FALSE;
int abnormal=FALSE;
char da_trovare[10];
char result[40];
int u;
int s=0;

j=0;

max_lun_fin=19;

if ((infile=fopen("qmenu.frm","r")) == NULL)	/* Apertura file di form    */
{
	history_string("ERRORE, non riesco ad aprire il file qmenu.frm");
       	return 10;
}
sprintf(da_trovare, "[%s]\0", forma);
	while((opi=getline(linea)) != EOF) 
	{
		if((trovato==TRUE)&&(linea[0]=='['))
			trovato=FALSE;

		if(strcmp(linea, da_trovare)==0)
			trovato=TRUE;

		if((trovato==TRUE)&&(linea[0] != '['))
		{
			if(strlen(linea) > max_lun_fin)
				max_lun_fin=strlen(linea);
			strcpy(finestra[a++], linea);
		}
	}
fclose(infile);
max_elem_fin=a;
disegna_form();
if (j!=0)
{
while(loop_OK)
{
	getentry(frm[s].x,frm[s].y,frm[s].lun, frm[s].desc);
	switch(atoi(frm[s].desc))
	{
	case -1 :
		strcpy(frm[s].desc,"");
		loop_OK=FALSE;
		abnormal=TRUE;
		break;
	case -2 :
		strcpy(frm[s].desc,"");
		if(s==0)
			loop_OK=FALSE;
		s--;
		abnormal=TRUE;
		break;
	default :
		s++;
		abnormal=FALSE;
		if(s==j)
			loop_OK=FALSE;
		break;
	}
}
}
else
	while((u=getkey())!=FZ4);

return abnormal;
}


disegna_form() 
{
int top=0;
int left=0;
int a=0, b=0, aa=0;
int inizio=0, fine=0;
int y=0;

	top=11-(max_elem_fin/2);
	left=39-(max_lun_fin/2);
	win(" INSERIMENTO DATI ",top-1 , left-1, top+max_elem_fin, left+max_lun_fin);
	for(a=0;a<max_elem_fin;a++)
	{

/* ============================================== */
inizio=0;
fine=0;
y=0;

move(top+a,left);
printw("%s", finestra[a]);
for(b=0;finestra[a][b]!='\0';b++)
{	
	if(finestra[a][b]=='<')
	{
		finestra[a][b]=' ';
		inizio=b+1;
	}
	
	if(finestra[a][b]=='>')
	{
		finestra[a][b]=' ';
		fine=b;
		move(top+a,left+inizio-1);
		printw(" ");
		standout();
		/* =========== */
		frm[j].x=top+a;
		frm[j].y=left+inizio;
		frm[j].lun=fine-inizio;
		strcpy(frm[j].desc, "");
		j++;
		/* =========== */
		move(top+a,left+inizio);
		for(y=inizio;y!=fine;y++)
			printw(" ");
		standend();
		move(top+a,left+fine);
		printw(" ");
	}
}
refresh();
/* ============================================== */

	}
	move(top+max_elem_fin,left+1);
	printw(" F4 - Esce ");
refresh();
}

pausa()
{
int u;
	move(22,50);
	printw(" Premi [F4] per uscire ");
	refresh();
	while((u=getkey())!=FZ4);
}
