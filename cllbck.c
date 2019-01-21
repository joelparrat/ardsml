
#include "intgrp.h"

// nom du fichier config
#define NOMCNF "/var/local/cnf/ardsml"
// nom du dossier fichier
#define NOMFCH "/var/local/fch/ardsml"

// duree simple click double click
#define DBLCLC 250

#define LNGSND 16
#define LNGRCV 16

LS   *lstprt;						// contenu de controleur.brc mis en forme (port digital analogic timer)
LS   *lstbrc;						// contenu de controleur.brc (brochage)
LS	 *lstanl;						// contenu de controleur.brc (broche speciale uniquement analogique)
int  gi_cnn;						// phase de connection 0...9 (0:rien 1:dev 2:open 3&4:handshaking 9:connecte)
int  gi_clc;						// nombre de click (simple double ...)
int  go_srl[2];						// open port serie (1) ou fifo (2)
char gr_file[2][32];				// open port serie (1) ou fifo (2)
int  gi_rcv;						// indexe reception
char gr_snd[LNGSND+1];				// buffer envoi
char gr_rcv[LNGRCV+3];				// buffer reception
int  gi_cmd;						// indexe couche basse protocole (initialise par la connection)
char gr_cmd[] = ".,:;!?";			// couche basse protocole reception (increment qu'apres reponse)
char gr_rpn[] = "(){}[]";			// couche basse protocole reponse (reponse fonction de la reception)
int  gi_anl;						// analogique affichage dynamique (nombre a afficher)
char gr_anl[90];					// adc actif + correspondance portbit/n°adc
char gr_dgt[90];					// dgt revennu actif suite suppression adc

int  fi_sndmss(void);
int  fi_appcvt(char *, GtkComboBoxText *, char *);
int  fi_gstmcu(const char *);
int  fi_gstlsn(const char *);
int  fi_getcnf(char *, char **);
int	 fi_appcbt(char *pp_fch, GtkComboBoxText *pp_cbt, char *pp_txt);
void fv_majcnf(char *, char *);
void fv_affdnm(void);
void fv_tocs(gpointer);				// simple click
void fv_tocsd(gpointer);			// simple click bouton donnee
void fv_tocss(gpointer);			// simple click bouton tous
void fv_tocd(gpointer);				// double click
void fv_tocdd(gpointer);			// double click bouton donnee
void fv_tocds(gpointer);			// double click bouton tous
gboolean cb_toc(gpointer);			// time out click
gboolean cb_opn(gpointer);
gboolean cb_cnn(gpointer);
gboolean cb_rcv(gpointer);

// Constructeur variable

void fv_cns_vrb(void)
{
	Wnd *wnd = &WND;
	char lr_nomfch[80];
	char *lp_nommcu, *lp_nomlsn;
	gchar *txt;
	
	gi_cnn = 0;
	gi_cmd = -1;
	go_srl[0] = go_srl[1] = -1;
	gr_snd[0] = gr_rcv[0] = 0;
	
	lstprt = ls_cns();
	lstbrc = ls_cns();
	lstanl = ls_cns();
	
	sprintf(lr_nomfch, "%s/controleur.lst", NOMFCH);
	fi_getcnf("mcu", &lp_nommcu);
	fi_appcbt(lr_nomfch, (GtkComboBoxText *) wnd->cbtmcu, lp_nommcu);
	if (lp_nommcu)
		free(lp_nommcu);
	sprintf(lr_nomfch, "%s/liaison.lst", NOMFCH);
	fi_getcnf("liaison", &lp_nomlsn);
	fi_appcbt(lr_nomfch, (GtkComboBoxText *) wnd->cbtlsn, lp_nomlsn);
	if (lp_nomlsn)
		free(lp_nomlsn);
		
	if ((txt = gtk_combo_box_text_get_active_text((GtkComboBoxText *) wnd->cbtlsn)) != NULL)
	{
		if (!strncmp(txt, "RS232 ", 6))
		{
			if (!strncmp(&txt[6], "ttyS", 4))
				sprintf(gr_file[0], "/dev/ttyS%c", txt[10]);
			else
				sprintf(gr_file[0], "/dev/ttyS0");
			gr_file[1][0] = 0;
			gtk_button_set_label((GtkButton *) wnd->bttcnn, gr_file[0]);

			gi_cnn++;
		}
		
		if (!strncmp(txt, "USB ", 4))
		{
			if (!strncmp(&txt[6], "ttyUSB", 6))
				sprintf(gr_file[0], "/dev/ttyUSB%c", txt[10]);
			else
				sprintf(gr_file[0], "/dev/ttyUSB0");
			gr_file[1][0] = 0;
			gtk_button_set_label((GtkButton *) wnd->bttcnn, gr_file[0]);

			gi_cnn++;
		}
		
		if (!strcmp(txt, "AVRX86"))
		{
			gtk_button_set_label((GtkButton *) wnd->bttcnn, "Maitre... ");

			strcpy(gr_file[0], "/tmp/avrsnd");
			strcpy(gr_file[1], "/tmp/avrrcv");
			gi_cnn++;
		}
		g_free(txt);
	}

	g_timeout_add_seconds(1, cb_opn, NULL);
}

// Destructeur variable

void fv_dst_vrb(void)
{
	ls_dst(lstprt);
	ls_dst(lstbrc);
	ls_dst(lstanl);
}

//
// CallBack TimeOut 1S
//
// toutes les secondes test si il y a connection
//
// retour: FALSE = arret declenchement
//

gboolean cb_opn(gpointer dnn)
{
	Wnd *wnd = &WND;
	const gchar *name;
	gchar *txt;
	char  n[2];
	int	  i;
	struct termios tty;
	struct termios old;
	
	// cligno voyant de connection
	
	name = gtk_widget_get_name(wnd->bttcnn);
	if (*name == 'g')
	{
		if (gi_cnn < 2)
		{
			strcpy(n, "r");
			wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
		}
		else if (gi_cnn == 2)
		{
			strcpy(n, "o");
			wnd->pled = gtk_image_new_from_file("/var/local/icn/porange.png");
		}
		else
		{
			strcpy(n, "b");
			wnd->pled = gtk_image_new_from_file("/var/local/icn/pbleu.png");
		}
	}
	else
	{
		strcpy(n, "g");
		wnd->pled = gtk_image_new_from_file("/var/local/icn/pgris.png");
	}
	gtk_button_set_image((GtkButton *) wnd->bttcnn, wnd->pled);
	gtk_widget_set_name(wnd->bttcnn, n);

	// ouverture dev
	
	if (gi_cnn == 1)
	{
		if ((go_srl[0] = open(gr_file[0], O_RDWR | O_NONBLOCK)) != -1)												// O_NONBLOCK or O_NDELAY
		{
			// Config port serie	RAW,9600,8,N,1
			tcgetattr(go_srl[0], &tty);
			tcgetattr(go_srl[0], &old);
			cfsetospeed(&tty, B9600);
			cfsetispeed(&tty, B9600);
			tty.c_cflag &= ~CSIZE;
			tty.c_cflag |= CS8;
			tty.c_cflag &= ~PARENB;
			tty.c_cflag &= ~CSTOPB;
			tty.c_lflag = 0;
			tty.c_cc[VMIN] = 1;
			tty.c_cc[VTIME] = 0;
			tcsetattr(go_srl[0], TCSANOW, &tty);
			tcflush(go_srl[0], TCIOFLUSH);

			gi_cnn++;
		}
/*			
		if (!strcmp(txt, "AVRX86"))
		{
			if (go_srl[0] == -1)
				go_srl[0] = open(gr_file[0], O_RDONLY | O_NONBLOCK);													// O_NONBLOCK or O_NDELAY
			if (go_srl[0] != -1)
			{
				if ((go_srl[1] = open(gr_file[1], O_WRONLY | O_NONBLOCK)) != -1)										// O_NONBLOCK or O_NDELAY
				{
					gtk_button_set_label((GtkButton *) wnd->bttcnn, gr_file[1]);
					gi_cnn++;
				}
			}
				
		}
*/
		return(TRUE);
	}
	
	// handshaking
	
	if (ioctl(go_srl[0], TIOCMGET, &i) == -1)
	{
		gi_cnn = 1;
		close(go_srl[0]);
		go_srl[0] = -1;
		wnd->pled = gtk_image_new_from_file("/var/local/icn/porange.png");
		gtk_button_set_image((GtkButton *) wnd->bttdsr, wnd->pled);
		wnd->pled = gtk_image_new_from_file("/var/local/icn/porange.png");
		gtk_button_set_image((GtkButton *) wnd->bttcts, wnd->pled);
		return(TRUE);
	}

	if (i & TIOCM_DSR)
	{
		if (gi_cnn == 2)
			gi_cnn = 3;
		wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
	}
	else
	{
		if ((gi_cnn == 3) || (gi_cnn == 4))
			gi_cnn = 2;
		wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
	}
	gtk_button_set_image((GtkButton *) wnd->bttdsr, wnd->pled);
	
	if (i & TIOCM_CTS)
	{
		if (gi_cnn == 3)
			gi_cnn = 4;
		wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
	}
	else
	{
		if ((gi_cnn == 3) || (gi_cnn == 4))
			gi_cnn = 2;
		wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
	}
	gtk_button_set_image((GtkButton *) wnd->bttcts, wnd->pled);

	if (gi_cnn != 4)
		return(TRUE);
	
	strcpy(n, "b");
	wnd->pled = gtk_image_new_from_file("/var/local/icn/pbleu.png");
	gtk_button_set_image((GtkButton *) wnd->bttcnn, wnd->pled);
	gtk_widget_set_name(wnd->bttcnn, n);

	while (read(go_srl[0], n, 1) == 1);																// vide buffer
	gi_rcv = 0;
	g_timeout_add(1, cb_cnn, NULL);
	
	return(FALSE);
}

//
// CallBack TimeOut 1mS
//
// toutes les millisecondes test reception signal de connection (>)
//
// retour: FALSE = arret declenchement
//

gboolean cb_cnn(gpointer dnn)
{
	Wnd *wnd = &WND;
	char n[2];
	int  l;
	char *p;
	
	while (1)
	{
		if ((l = read(go_srl[0], &gr_rcv[gi_rcv], 1)) <= 0)
		{
			if (ioctl(go_srl[0], TIOCMGET, &l) == -1)
			{
				gi_cnn = 1;
				close(go_srl[0]);
				go_srl[0] = -1;
				wnd->pled = gtk_image_new_from_file("/var/local/icn/porange.png");
				gtk_button_set_image((GtkButton *) wnd->bttdsr, wnd->pled);
				wnd->pled = gtk_image_new_from_file("/var/local/icn/porange.png");
				gtk_button_set_image((GtkButton *) wnd->bttcts, wnd->pled);
				g_timeout_add_seconds(1, cb_opn, NULL);
				return(FALSE);
			}
			else if (((l & TIOCM_DSR) == 0) || ((l & TIOCM_CTS) == 0))
			{
				gi_cnn = 1;
				close(go_srl[0]);
				go_srl[0] = -1;
				wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
				gtk_button_set_image((GtkButton *) wnd->bttdsr, wnd->pled);
				wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
				gtk_button_set_image((GtkButton *) wnd->bttcts, wnd->pled);
				g_timeout_add_seconds(1, cb_opn, NULL);
				return(FALSE);
			}
			
			return(TRUE);
		}

		gi_rcv++;
		if (gi_rcv < 3)
			continue;
		
		if (gr_rcv[1] == '>')																							// recherche carte
		{
			if ((p = strchr(gr_cmd, gr_rcv[0])) != NULL)
			{
				if (((p - gr_cmd) % 2) == 0)
				{
					gi_cmd = p - gr_cmd;
					if (gr_rcv[2] == gr_cmd[gi_cmd+1])
					{
printf("ardsml <-- %s\n", gr_rcv);
						strcpy(gr_snd, "<");
						fi_sndmss();
						gi_cnn = 9;
/*
						gi_cmd += 2;
						if (gi_cmd > 6)
							gi_cmd = 0;
*/
						//while (read(go_srl[0], &gr_rcv[0], 1) == 1);																// vide buffer
						break;
					}
					else
						gi_rcv = 0;
				}
				else
				{
					gr_rcv[0] = gr_rcv[2];
					gi_rcv -= 2;
				}
			}
			else
			{
				gi_rcv = 0;
			}
		}
		else
		{
			gr_rcv[0] = gr_rcv[1];
			gr_rcv[1] = gr_rcv[2];
			gi_rcv--;
		}
	}

	strcpy(n, "v");
	wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
	gtk_button_set_image((GtkButton *) wnd->bttcnn, wnd->pled);
	gtk_widget_set_name(wnd->bttcnn, n);

	gi_rcv = 0;
	gr_rcv[0] = 0;
	g_timeout_add(1, cb_rcv, NULL);
	return(FALSE);
}

//
// CallBack Reception 1mS
//
// retour FALSE: arret du declenchement
//

gboolean cb_rcv(gpointer dnn)
{
	Wnd *wnd = &WND;
	char n[2];
	int  i, j, l, ret, li_nmb;
	const char *pbc;
	const gchar *acc;																									// mode access du bouton
	const gchar *o_pbc;																									// nom du bouton
	const gchar *lr_anl;																								// entry analogic
	int   li_anl;
	char *p, lr_rcv[LNGRCV+2];
	char port[] = "abcdefghjkl";
	char bit[] = "01234567";
	
	while (1)
	{
		if ((l = read(go_srl[0], &gr_rcv[gi_rcv], 1)) <= 0)
		{
			if (ioctl(go_srl[0], TIOCMGET, &i) == -1)
			{
				gi_cnn = 1;
				close(go_srl[0]);
				go_srl[0] = -1;
				wnd->pled = gtk_image_new_from_file("/var/local/icn/porange.png");
				gtk_button_set_image((GtkButton *) wnd->bttdsr, wnd->pled);
				wnd->pled = gtk_image_new_from_file("/var/local/icn/porange.png");
				gtk_button_set_image((GtkButton *) wnd->bttcts, wnd->pled);
				g_timeout_add_seconds(1, cb_opn, NULL);
				return(FALSE);
			}
			else if (((i & TIOCM_DSR) == 0) || ((i & TIOCM_CTS) == 0))
			{
				gi_cnn = 1;
				close(go_srl[0]);
				go_srl[0] = -1;
				wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
				gtk_button_set_image((GtkButton *) wnd->bttdsr, wnd->pled);
				wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
				gtk_button_set_image((GtkButton *) wnd->bttcts, wnd->pled);
				g_timeout_add_seconds(1, cb_opn, NULL);
				return(FALSE);
			}
			
			return(TRUE);
		}

printf("%c\n", gr_rcv[gi_rcv]);

		if (gr_rcv[gi_rcv] != gr_cmd[gi_cmd+1])																			// pas fin commande
		{
			if (gi_rcv < LNGRCV)
			{
				gi_rcv++;
				continue;
			}
			else
			{
				gi_rcv = 0;
				continue;
			}
		}
		gr_rcv[gi_rcv+1] = 0;
		gi_rcv = 0;

		if (gr_rcv[0] != gr_cmd[gi_cmd])																				// debut commande
		{
			strcpy(gr_snd, "#");
			fi_sndmss();
			return(TRUE);
		}
		
		break;
	}	

	printf("ardsml <-- %s\n", gr_rcv);
	strncpy(lr_rcv, &gr_rcv[1], strlen(gr_rcv)-2);
	lr_rcv[strlen(gr_rcv)-2] = 0;

	ret = 1;
	switch (lr_rcv[0])
	{
		case 'd':																										// commande digital
			// dras
			// 0123
			if (strncmp(lr_rcv, "dr", 2) == 0)
			{
				if (lr_rcv[3] == 's')
				{
					for (li_nmb=i=0; i<8; i++,li_nmb<<=1)
					{
						pbc = gtk_widget_get_name(wnd->bdgt[(lr_rcv[2] - 'a')][7-i]);
						switch (pbc[2])
						{
							case 'v':
								li_nmb |= 1;
								break;
						}
					}
					li_nmb >>= 1;
					sprintf(gr_snd, "dv%02x", li_nmb);
					fi_sndmss();
					ret = 0;
				}
				else
				{
					pbc = gtk_widget_get_name(wnd->bdgt[(lr_rcv[2] - 'a')][(lr_rcv[3] - '0')]);							// port bit couleur
					switch (pbc[2])
					{
						case 'r':
							strcpy(gr_snd, "dv0");
							fi_sndmss();
							ret = 0;
							break;
						case 'v':
							strcpy(gr_snd, "dv1");
							fi_sndmss();
							ret = 0;
							break;
					}
				}
			}
			// dwcsff
			// 012345
			if (strncmp(lr_rcv, "dw", 2) == 0)
			{
				if (lr_rcv[3] == 's')
				{
					sscanf(&lr_rcv[4], "%2x", &l);
					for (i=0; i<8; i++)
					{
						if (l & (1<<i))
						{
							sprintf(n, "%c%dv", lr_rcv[2], i);
							gtk_widget_set_name(wnd->bdgt[lr_rcv[2]-'a'][i], n);										// name (n) port bit couleur
							wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
							gtk_button_set_image((GtkButton *) wnd->bdgt[lr_rcv[2]-'a'][i], wnd->pled);
						}
						else
						{
							sprintf(n, "%c%dr", lr_rcv[2], i);
							gtk_widget_set_name(wnd->bdgt[lr_rcv[2]-'a'][i], n);										// name (n) port bit couleur
							wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
							gtk_button_set_image((GtkButton *) wnd->bdgt[lr_rcv[2]-'a'][i], wnd->pled);
						}
					}
				}
				else
				{
					if (lr_rcv[4] == '0')
					{
						sprintf(n, "%c%cr", lr_rcv[2], lr_rcv[3]);
						gtk_widget_set_name(wnd->bdgt[lr_rcv[2]-'a'][lr_rcv[3]-'0'], n);								// name (n) port bit couleur
						wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
						gtk_button_set_image((GtkButton *) wnd->bdgt[lr_rcv[2]-'a'][lr_rcv[3]-'0'], wnd->pled);
					}
					else
					{
						sprintf(n, "%c%cv", lr_rcv[2], lr_rcv[3]);
						gtk_widget_set_name(wnd->bdgt[lr_rcv[2]-'a'][lr_rcv[3]-'0'], n);								// name (n) port bit couleur
						wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
						gtk_button_set_image((GtkButton *) wnd->bdgt[lr_rcv[2]-'a'][lr_rcv[3]-'0'], wnd->pled);
					}
				}
				
				strcpy(gr_snd, "=");
				fi_sndmss();
				ret = 0;
			}
			if (ret)
			{
				strcpy(gr_snd, "#");
				fi_sndmss();
			}
			break;
		case 'a':																										// commande analogic
			if (strlen(lr_rcv) != 4)
				strcpy(gr_snd, "#");
			else if (lr_rcv[1] != 'r')
				strcpy(gr_snd, "#");
			else if (strchr(port, lr_rcv[2]) == NULL) 
				strcpy(gr_snd, "#");
			else if (strchr(bit, lr_rcv[3]) == NULL) 
				strcpy(gr_snd, "#");
			else
			{
				lr_anl = gtk_entry_get_text((GtkEntry *) wnd->eanl[lr_rcv[2]-'a'][lr_rcv[3]-'0']);
				li_anl = atoi(lr_anl);
				sprintf(gr_snd, "av%02x", li_anl);
			}
			fi_sndmss();
			break;
		case 'c':																										// commande configuration
			// crpas
			// 01234
			if (strncmp(lr_rcv, "crp", 3) == 0)																			// configuration read port
			{
				strcpy(gr_snd, "cv");
				if (lr_rcv[4] == 's')
				{
					for (i=0; i<8; i++)
					{
						acc = gtk_button_get_label((GtkButton *) wnd->bdgt[lr_rcv[3]-'a'][i]);							// mode acces i/o = Z:input HighZ / U:Input PullUp / O:Output
						n[0] = tolower(*acc);
						n[1] = 0;
						strcat(gr_snd, n);
					}
					fi_sndmss();
					ret = 0;
					break;
				}
				else
				{
					acc = gtk_button_get_label((GtkButton *) wnd->bdgt[lr_rcv[3]-'a'][lr_rcv[4]-'0']);					// mode acces i/o = Z:input HighZ / U:Input PullUp / O:Output
					n[0] = tolower(*acc);
					n[1] = 0;
					strcat(gr_snd, n);
					fi_sndmss();
					ret = 0;
					break;
				}
			}
			// cwpaso
			// 012345
			if (strncmp(lr_rcv, "cwp", 3) == 0)																			// configuration write port
			{
				if (lr_rcv[4] == 's')
				{
					if (lr_rcv[6] == 0)																					// Version courte 8 bits identiques
					{
						for (n[1]=0,i=0; i<8; i++)
						{
							n[0] = toupper(lr_rcv[5]);
							gtk_button_set_label((GtkButton *) wnd->bdgt[lr_rcv[3]-'a'][i], n);
						}
						strcpy(gr_snd, "=");
						fi_sndmss();
						ret = 0;
					}
					else																								// Version longue 8 bits independants
					{
						for (n[1]=0,i=0; i<8; i++)
						{
							n[0] = toupper(lr_rcv[5+i]);
							gtk_button_set_label((GtkButton *) wnd->bdgt[lr_rcv[3]-'a'][i], n);
						}
						strcpy(gr_snd, "=");
						fi_sndmss();
						ret = 0;
					}
				}
				else
				{
					n[1] = 0;
					n[0] = toupper(lr_rcv[5]);
					gtk_button_set_label((GtkButton *) wnd->bdgt[lr_rcv[3]-'a'][lr_rcv[4]-'0'], n);
					strcpy(gr_snd, "=");
					fi_sndmss();
					ret = 0;
				}
			}
			if (ret)
			{
				strcpy(gr_snd, "#");
				fi_sndmss();
			}
			break;
		case '<':
		case '*':
		case '@':
			strcpy(gr_snd, "=");
			fi_sndmss();
			break;
		default:
			strcpy(gr_snd, "#");
			fi_sndmss();
			break;
	}

	i = 0;

	return(TRUE);
}

//
// Envoi commande
//
// retour: -1 erreur_send 1 erreur_receive 0 ok_send_receive

int fi_sndmss(void)
{
	//Wnd *wnd = &WND;
	//int ret;
	
	//wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
	//gtk_button_set_image((GtkButton *) wnd->bttsnd, wnd->pled);
	//if (gi_tos == 0)
	//	g_timeout_add(TOV, cb_tos, NULL);																				//millisecondes
	//gi_tos = 1;
	
	if (gi_cmd == -1)
		return(-1);
		
	if (write(go_srl[0], &gr_rpn[gi_cmd], 1) != 1)																		// ajout debut trame
	{
		return(-1);
	}

	if (write(go_srl[0], gr_snd, strlen(gr_snd)) != strlen(gr_snd))														// envoi commande
	{
		return(-1);
	}
	//if (strcmp(gr_snd, "@"))
		printf("ardsml --> %c%s%c\n", gr_rpn[gi_cmd], gr_snd, gr_rpn[gi_cmd+1]);

	if (write(go_srl[0], &gr_rpn[gi_cmd+1], 1) != 1)																	// ajout fin trame
	{
		return(-1);
	}
			
	//ret = fi_rcvmss();
	//fv_affvnt(ret);
	
	gi_cmd += 2;
	if (gi_cmd >= 6)
		gi_cmd = 0;

	return(0);
}

//
// rempli un combobox_text (pp_cbt) avec le contenu d'un fichier (pp_fch)
// selection d'un texte (pp_txt)
//
// retour: 0=ok 1=append_pasdeselection -1=erreur
//

int fi_appcbt(char *pp_fch, GtkComboBoxText *pp_cbt, char *pp_txt)
{
	FILE *lo;
	char *p, lr_lgn[80];
	int  id, ret;

	id = 0;
	ret = 1;
	if ((lo = fopen(pp_fch, "r")) == NULL)																				// fichier inconnu
		return(-1);
		
	while (fgets(lr_lgn, sizeof(lr_lgn), lo) != NULL)
	{
		if ((p = strchr(lr_lgn, '\n')) != NULL)
			*p = 0;
		if ((p = strchr(lr_lgn, '\r')) != NULL)
			*p = 0;
		
		if (lr_lgn[0] != 0)
		{
			gtk_combo_box_text_append_text(pp_cbt, lr_lgn);
			if (pp_txt)
			{
				if (!strcmp(pp_txt, lr_lgn))
				{
					gtk_combo_box_set_active((GtkComboBox *) pp_cbt, id);
					ret = 0;
				}
			}
			id++;
		}
	}
	fclose(lo);
	
	return(ret);
}

int fi_gstmcu(const char *pp_mcu)
{
	CH   *crn;
	Wnd *wnd = &WND;
	FILE *lo;
	char *p, prt[2], mcu[16], txt[80];
	int  pos, a;
	char n[4], d[9];
	
	ls_vid(lstprt);
	ls_vid(lstbrc);
	ls_vid(lstanl);
	gi_anl = 0;
	prt[1] = 0;
	gr_anl[0] = gr_dgt[0] = 0;
	
	sprintf(txt, "%s/%s.brc", NOMFCH, pp_mcu);
	if ((lo = fopen(txt, "r")) == NULL)
		return(-1);
		
	while (fgets(txt, sizeof(txt), lo) != NULL)
	{
		if ((p = strchr(txt, '\n')) != NULL)
			*p = 0;
		
		if (strlen(txt) < 3)
			continue;
		if (strncmp(txt, "ADC", 3) == 0)																				// broche speciale uniquement analogique
		{
			ls_asc(lstanl, "0", txt);
			/*
			wnd->eanl[2][txt[3]-'0'] = gtk_entry_new();																	// analogic
			gtk_entry_set_width_chars((GtkEntry *) wnd->eanl[2][txt[3]-'0'], 5);
			gtk_entry_set_max_length((GtkEntry *) wnd->eanl[2][txt[3]-'0'], 5);
			n[0] = txt[1];
			n[1] = txt[2];
			n[2] = txt[2];//a[txt[2]-'0'];
			n[3] = 0;
			gtk_widget_set_name(wnd->eanl[2][txt[3]-'0'], n);													// name (n) port bit numero
			gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->eanl[2][txt[3]-'0'], 9-(txt[3]-'0'), ya+1, 1, 1);
			gtk_widget_show(wnd->eanl[2][txt[3]-'0']);
			sprintf(d, "ADC%d", (n[2]>='a')?n[2]-'a'+10:n[2]-'0');
			gtk_widget_set_tooltip_text(wnd->eanl[2][txt[3]-'0'], d);
			*/
			continue;
		}
		
		ls_asc(lstbrc, "0", txt);
			
		prt[0] = tolower(txt[1]);
		ls_asc(lstprt, "0", "%s%s%s%s", prt, "........", "........", "........");
		if (ls_rtx(lstprt, prt, 0) == -1)
			continue;
			
		crn = lstprt->current;
		*((crn->mmr+(*(crn->mmr+8)))+(txt[2]-'0')) = txt[2];

		if ((p = strstr(txt, " ADC")) != NULL)
		{
			if (sscanf(p+4, "%d", &a) == 1)
			{
				if (ls_rtx(lstprt, prt, 0) != -1)
				{
					crn = lstprt->current;
					*((crn->mmr+(*(crn->mmr+16)))+(txt[2]-'0')) = (a<10)?'0'+a:'a'+a-10;
				}
			}
		}

		if ((p = strstr(txt, " OC")) != NULL)
		{
			if (sscanf(p+3, "%d", &a) == 1)
			{
				if (ls_rtx(lstprt, prt, 0) != -1)
				{
					crn = lstprt->current;
					*((crn->mmr+(*(crn->mmr+24)))+(txt[2]-'0')) = (a<10)?'0'+a:'a'+a-10;
				}
			}
		}
		/*
		if ((p = strstr(txt, " ICP")) != NULL)
		{
			if (sscanf(p+4, "%d", &a) == 1)
			{
				if (ls_rtx(lstprt, prt, 0) != -1)
				{
					crn = lstprt->current;
					*((crn->mmr+(*(crn->mmr+24)))+(txt[2]-'0')) = (a<10)?'0'+a:'a'+a-10;
				}
			}
		}
		if ((p = strstr(txt, " T")) != NULL)
		{
			if (sscanf(p+2, "%d", &a) == 1)
			{
				if (ls_rtx(lstprt, prt, 0) != -1)
				{
					crn = lstprt->current;
					*((crn->mmr+(*(crn->mmr+24)))+(txt[2]-'0')) = (a<10)?'0'+a:'a'+a-10;
				}
			}
		}
		*/
	}
	fclose(lo);

	//ls_prn(lstprt);
	//ls_prn(lstbrc);
	
	fv_affdnm();																										// affichage dynamique
	return(0);
}

int fi_gstlsn(const char *pp_lsn)
{
	Wnd *wnd = &WND;

	gi_cnn = 0;
	if (go_srl[0]!=-1)
		close(go_srl[0]);
	go_srl[0] = -1;

	if (!strncmp(pp_lsn, "RS232 ", 6))
	{
		if (!strncmp(&pp_lsn[6], "ttyS", 4))
			sprintf(gr_file[0], "/dev/ttyS%c", pp_lsn[10]);
		else
			sprintf(gr_file[0], "/dev/ttyS0");
		gr_file[1][0] = 0;
		gtk_button_set_label((GtkButton *) wnd->bttcnn, gr_file[0]);

		gi_cnn++;
	}
	
	if (!strncmp(pp_lsn, "USB ", 4))
	{
		if (!strncmp(&pp_lsn[6], "ttyUSB", 6))
			sprintf(gr_file[0], "/dev/ttyUSB%c", pp_lsn[10]);
		else
			sprintf(gr_file[0], "/dev/ttyUSB0");
		gr_file[1][0] = 0;
		gtk_button_set_label((GtkButton *) wnd->bttcnn, gr_file[0]);

		gi_cnn++;
	}
	
	if (!strcmp(pp_lsn, "AVRX86"))
	{
		gtk_button_set_label((GtkButton *) wnd->bttcnn, "Maitre... ");

		strcpy(gr_file[0], "/tmp/avrsnd");
		strcpy(gr_file[1], "/tmp/avrrcv");
		gi_cnn++;
	}

	return(0);
}

// lecture element du fichier configuration (.../cnf/avrx86)
//
// pp_elm: l'element dans cnf  (ex:liaison)
// pp_cnt: son contenu         (ex:RS232)
//
// retour: 0=ok 1=pas_trouve -1=erreur
// attention: pp_cnt doit etre libere apres utilisation si retour ok {if (pp_cnt) free(pp_cnt)}

int fi_getcnf(char *pp_elm, char **pp_cnt)
{
	char *d, *f, *lp_bff, *lp_elm;
	int  li_lus;

	if ((lp_bff = io_bfffch(NOMCNF, &li_lus)) == NULL)																	// fichier config n'existe pas
	{
		*pp_cnt = NULL;
		return(1);
	}

	if ((lp_elm = malloc(strlen(pp_elm) + 2)) == NULL)
	{
		*pp_cnt = NULL;
		io_frebff(lp_bff);																								// libere la memoire
		return(-1);
	}
	sprintf(lp_elm, "%s=", pp_elm);	

	if ((d = strstr(lp_bff, lp_elm)) == NULL)																			// element non trouve
	{
		*pp_cnt = NULL;
		free(lp_elm);
		io_frebff(lp_bff);																								// libere la memoire
		return(1);
	}

	d += strlen(lp_elm);
	if ((f = strchr(d, '\n')) != NULL)
	{
		*pp_cnt = malloc((f-d)+1);
		strncpy(*pp_cnt, d, f-d);
		*((*pp_cnt)+(f-d)) = 0;
	}
	else
	{
		*pp_cnt = malloc(strlen(d)+1);
		strcpy(*pp_cnt, d);
	}

	free(lp_elm);
	io_frebff(lp_bff);																									// libere la memoire
	
	return(0);
}

// Mise a jour fichier configuration (.../cnf/ardsml)
//
// pr_txt: le texte dans cnf  (ex:liaison)
// pr_cnt: son contenu        (ex:RS232)

void fv_majcnf(char *pr_txt, char *pr_cnt)
{
	FILE *fo;
	char *d, *f, *bff, lr_cnt[16], lr_txt[16];
	int  lus;

	sprintf(lr_txt, "%s=", pr_txt);	
	if ((bff = io_bfffch(NOMCNF, &lus)) == NULL)																		// fichier config n'existe pas
	{
		if ((fo = fopen(NOMCNF, "w")) != NULL)																			// creation
		{
			fputs(lr_txt, fo);
			fputs(pr_cnt, fo);
			fputs("\n", fo);
			fclose(fo);
		}
	}
	else
	{
		if ((d = strstr(bff, lr_txt)) != NULL)																			// mcu trouve
		{
			d += strlen(lr_txt);
			if ((f = strchr(d, '\n')) != NULL)
			{
				strncpy(lr_cnt, d, f-d);
				lr_cnt[f-d] = 0;
				if (strcmp(lr_cnt, pr_cnt) != 0)
				{
					ht_rmptxt(bff, lr_cnt, pr_cnt, 1);																	// remplace ancien mcu par nouveau
					io_ecrbff(NOMCNF, bff, lus-(f-d)+strlen(pr_cnt));													// ecriture
				}
			}
			else
			{
				strcpy(lr_cnt, d);
				if (strcmp(lr_cnt, pr_cnt) != 0)
				{
					ht_rmptxt(bff, lr_cnt, pr_cnt, 1);																	// remplace ancien mcu par nouveau
					io_ecrbff(NOMCNF, bff, lus-strlen(lr_cnt)+strlen(pr_cnt));											// ecriture
				}
			}
		}
		else
		{
			if ((fo = fopen(NOMCNF, "a")) != NULL)																		// pas encore de mcu -> ajoute
			{
				fputs(lr_txt, fo);
				fputs(pr_cnt, fo);
				fputs("\n", fo);
				fclose(fo);
			}
		}
		io_frebff(bff);																									// libere la memoire
	}
}

void fv_affdnm(void)
{
	CH *crn;
	Wnd *wnd = &WND;
	int  i, j, b, x, y, yd, ya, yt;
	char p, n[4], d[9], a[9], t[9], l[6], lr_tbl[16];																	// port (p) name (n) dgt (d) anl (a) tmr (t) lbl (l)
	char *pnt;
	
	if (wnd->grddgt)
		gtk_widget_destroy(wnd->grddgt);
	if (wnd->grdanl)
		gtk_widget_destroy(wnd->grdanl);
	if (wnd->grdtmr)
		gtk_widget_destroy(wnd->grdtmr);
	if (wnd->grdbrc)
		gtk_widget_destroy(wnd->grdbrc);

	wnd->grddgt = gtk_grid_new();																						// digital
	gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grddgt, TRUE);
	gtk_container_add((GtkContainer *) wnd->algdgt, wnd->grddgt);
	gtk_widget_show(wnd->grddgt);

	wnd->grdanl = gtk_grid_new();																						// analogic
	gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grdanl, TRUE);
	gtk_container_add((GtkContainer *) wnd->alganl, wnd->grdanl);
	gtk_widget_show(wnd->grdanl);

	wnd->grdtmr = gtk_grid_new();																						// timer
	gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grdtmr, TRUE);
	gtk_container_add((GtkContainer *) wnd->algtmr, wnd->grdtmr);
	gtk_widget_show(wnd->grdtmr);

	wnd->grdbrc = gtk_grid_new();																						// broche
	gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grdbrc, TRUE);
	gtk_container_add((GtkContainer *) wnd->algbrc, wnd->grdbrc);
	gtk_widget_show(wnd->grdbrc);

	wnd->ltmrt[0] = gtk_label_new("Name");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[0], 0, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[0]);
	wnd->ltmrt[1] = gtk_label_new("Mode");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[1], 1, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[1]);
	wnd->ltmrt[2] = gtk_label_new("Range");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[2], 2, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[2]);
	wnd->ltmrt[3] = gtk_label_new("Hz");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[3], 3, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[3]);
	wnd->ltmrt[4] = gtk_label_new("%");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[4], 4, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[4]);
	wnd->ltmrt[5] = gtk_label_new("TCN");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[5], 5, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[5]);
	wnd->ltmrt[6] = gtk_label_new("ICR");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[6], 6, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[6]);
	wnd->ltmrt[7] = gtk_label_new("OCR");
	gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ltmrt[7], 7, 0, 1, 1);
	gtk_widget_show(wnd->ltmrt[7]);

	l[2] = 0;
	for (x=0; x<8; x++)
	{
		l[1] = '0' + x;
		l[0] = 'D';
		wnd->ldgtd[x] = gtk_label_new(l);
		gtk_grid_attach((GtkGrid *) wnd->grddgt, wnd->ldgtd[x], 9-x, 0, 1, 1);
		gtk_widget_show(wnd->ldgtd[x]);
		wnd->lbrcd[x] = gtk_label_new(l);
		gtk_grid_attach((GtkGrid *) wnd->grdbrc, wnd->lbrcd[x], 8-x, 0, 1, 1);
		gtk_widget_show(wnd->lbrcd[x]);
		l[0] = 'A';
		wnd->lanla[x] = gtk_label_new(l);
		gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->lanla[x], 9-x, 0, 1, 1);
		gtk_widget_show(wnd->lanla[x]);
	}
	wnd->ldgtd[x] = gtk_label_new("All");
	gtk_grid_attach((GtkGrid *) wnd->grddgt, wnd->ldgtd[x], 9-x, 0, 1, 1);
	gtk_widget_show(wnd->ldgtd[x]);
	wnd->lanla[x] = gtk_label_new("All");
	gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->lanla[x], 9-x, 0, 1, 1);
	gtk_widget_show(wnd->lanla[x]);
	x++;
	wnd->lanla[x] = gtk_label_new("   ");
	gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->lanla[x], 9-x, 0, 1, 1);
	gtk_widget_show(wnd->lanla[x]);
	
	yd = ya = 0;
	yt = -1;
	crn = lstprt->first;
	while (crn)
	{
		p = *(crn->mmr+(*crn->mmr));																					// port (p)
		strcpy(d, crn->mmr+(*(crn->mmr+8)));
		strcpy(a, crn->mmr+(*(crn->mmr+16)));
		strcpy(t, crn->mmr+(*(crn->mmr+24)));

		sprintf(l, "Port%c", toupper(p));
		wnd->ldgtp[p-'a'] = gtk_label_new(l);
		gtk_grid_attach((GtkGrid *) wnd->grddgt, wnd->ldgtp[p-'a'], 0, yd+1, 1, 1);
		gtk_widget_show(wnd->ldgtp[p-'a']);
		wnd->lbrcp[p-'a'] = gtk_label_new(l);
		gtk_grid_attach((GtkGrid *) wnd->grdbrc, wnd->lbrcp[p-'a'], 0, yd+1, 1, 1);
		gtk_widget_show(wnd->lbrcp[p-'a']);

		n[0] = p;
		for (b=0,n[3]=0; d[b]!=0; b++)
		{
			if (d[b] != '.')
			{
				wnd->bdgt[p-'a'][b] = gtk_button_new_with_label("Z ");													// digital
				n[1] = '0' + b;
				n[2] = 'r';
				gtk_widget_set_name(wnd->bdgt[p-'a'][b], n);															// name (n) port bit couleur
				wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
				gtk_button_set_image((GtkButton *) wnd->bdgt[p-'a'][b], wnd->pled);
				gtk_button_set_image_position((GtkButton *) wnd->bdgt[p-'a'][b], GTK_POS_RIGHT);
				gtk_button_set_always_show_image((GtkButton *) wnd->bdgt[p-'a'][b], TRUE);
				gtk_grid_attach((GtkGrid *) wnd->grddgt, wnd->bdgt[p-'a'][b], 9-b, yd+1, 1, 1);
				gtk_widget_show(wnd->bdgt[p-'a'][b]);
				g_signal_connect((GtkWindow *) wnd->bdgt[p-'a'][b], "clicked", (GCallback) on_bdgt_clicked, NULL);

				sprintf(l, "P%c%c", toupper(p), '0'+b);
				wnd->bbrc[p-'a'][b] = gtk_button_new_with_label(l);														// broche
				gtk_widget_set_name(wnd->bbrc[p-'a'][b], &l[1]);
				gtk_grid_attach((GtkGrid *) wnd->grdbrc, wnd->bbrc[p-'a'][b], 8-b, yd+1, 1, 1);
				gtk_widget_show(wnd->bbrc[p-'a'][b]);
				g_signal_connect((GtkWindow *) wnd->bbrc[p-'a'][b], "clicked", (GCallback) on_bbrc_clicked, NULL);
			}
		}

		n[0] = p;
		for (b=0,i=0,n[3]=0; a[b]!=0; b++)
		{
			if (a[b] != '.')
			{
				i = 1;
				/*
				wnd->eanl[p-'a'][b] = gtk_entry_new();																	// analogic
				gtk_entry_set_width_chars((GtkEntry *) wnd->eanl[p-'a'][b], 5);
				gtk_entry_set_max_length((GtkEntry *) wnd->eanl[p-'a'][b], 5);
				n[1] = '0' + b;
				n[2] = a[b];
				gtk_widget_set_name(wnd->eanl[p-'a'][b], n);															// name (n) port bit numero
				gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->eanl[p-'a'][b], 9-b, ya+1, 1, 1);
				gtk_widget_show(wnd->eanl[p-'a'][b]);
				sprintf(d, "ADC%d", (n[2]>='a')?n[2]-'a'+10:n[2]-'0');
				gtk_widget_set_tooltip_text(wnd->eanl[p-'a'][b], d);
				*/
				//g_signal_connect((GtkWindow *) wnd->bdgt[p-'a'][b], "clicked", (GCallback) on_bdgt_clicked, NULL);
			}
		}

		if (i)
		{
			sprintf(l, "ADC%c", toupper(p));
			wnd->lanlc[p-'a'] = gtk_label_new(l);
			gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->lanlc[p-'a'], 0, ya+1, 1, 1);
			gtk_widget_show(wnd->lanlc[p-'a']);
			ya++;
		}

		n[0] = p;
		for (b=0,n[3]=0; t[b]!=0; b++)
		{
			if (t[b] != '.')
			{
				if ((t[b]-'0') > yt)
					yt = t[b] - '0';
			}
		}

		yd++;
		crn = crn->next;
	}

	crn = lstanl->first;
	while (crn)
	{
		pnt = crn->mmr+(*crn->mmr);
		
		wnd->eanl[2][*(pnt+3)-'0'] = gtk_entry_new();																	// analogic
		gtk_entry_set_width_chars((GtkEntry *) wnd->eanl[2][*(pnt+3)-'0'], 5);
		gtk_entry_set_max_length((GtkEntry *) wnd->eanl[2][*(pnt+3)-'0'], 5);
		n[0] = *(pnt+2);
		n[1] = *(pnt+3);
		n[2] = *(pnt+3);//a[txt[2]-'0'];
		n[3] = 0;
		gtk_widget_set_name(wnd->eanl[2][*(pnt+3)-'0'], n);																// name (n) port bit numero
		gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->eanl[2][*(pnt+3)-'0'], 9-(*(pnt+3)-'0'), 1, 1, 1);
		gtk_widget_show(wnd->eanl[2][*(pnt+3)-'0']);
		sprintf(d, "ADC%d", ((n[2]-'0')>='a')?n[2]-'a'+10:n[2]-'0');
		gtk_widget_set_tooltip_text(wnd->eanl[2][*(pnt+3)-'0'], d);

		crn = crn->next;
	}

	for (y=0; y<=yt; y++)
	{
		wnd->ctmrm[y] = gtk_combo_box_text_new();																		// timer
		gtk_combo_box_text_append_text((GtkComboBoxText *) wnd->ctmrm[y], "Off");
		gtk_combo_box_text_append_text((GtkComboBoxText *) wnd->ctmrm[y], "Up");
		gtk_combo_box_text_append_text((GtkComboBoxText *) wnd->ctmrm[y], "Down");
		gtk_combo_box_text_append_text((GtkComboBoxText *) wnd->ctmrm[y], "PWM");
		gtk_combo_box_text_append_text((GtkComboBoxText *) wnd->ctmrm[y], "Gen");
		gtk_combo_box_set_active((GtkComboBox *) wnd->ctmrm[y], 0);
		gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ctmrm[y], 1, y+1, 1, 1);
		gtk_widget_show(wnd->ctmrm[y]);

		wnd->ctmrr[y] = gtk_combo_box_text_new();
		gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->ctmrr[y], 2, y+1, 1, 1);
		gtk_widget_show(wnd->ctmrr[y]);

		wnd->etmrh[y] = gtk_entry_new();
		gtk_entry_set_width_chars((GtkEntry *) wnd->etmrh[y], 7);
		gtk_entry_set_max_length((GtkEntry *) wnd->etmrh[y], 7);
		gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->etmrh[y], 3, y+1, 1, 1);
		gtk_widget_show(wnd->etmrh[y]);

		wnd->etmrp[y] = gtk_entry_new();
		gtk_entry_set_width_chars((GtkEntry *) wnd->etmrp[y], 7);
		gtk_entry_set_max_length((GtkEntry *) wnd->etmrp[y], 7);
		gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->etmrp[y], 4, y+1, 1, 1);
		gtk_widget_show(wnd->etmrp[y]);

		wnd->etmrt[y] = gtk_entry_new();
		gtk_entry_set_width_chars((GtkEntry *) wnd->etmrt[y], 7);
		gtk_entry_set_max_length((GtkEntry *) wnd->etmrt[y], 7);
		gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->etmrt[y], 5, y+1, 1, 1);
		gtk_widget_show(wnd->etmrt[y]);

		wnd->etmri[y] = gtk_entry_new();
		gtk_entry_set_width_chars((GtkEntry *) wnd->etmri[y], 7);
		gtk_entry_set_max_length((GtkEntry *) wnd->etmri[y], 7);
		gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->etmri[y], 6, y+1, 1, 1);
		gtk_widget_show(wnd->etmri[y]);

		wnd->etmro[y] = gtk_entry_new();
		gtk_entry_set_width_chars((GtkEntry *) wnd->etmro[y], 7);
		gtk_entry_set_max_length((GtkEntry *) wnd->etmro[y], 7);
		gtk_grid_attach((GtkGrid *) wnd->grdtmr, wnd->etmro[y], 7, y+1, 1, 1);
		gtk_widget_show(wnd->etmro[y]);
	}
}

void on_cbtmcu_changed(GtkComboBox *obj, gpointer dnn)
{
	Wnd *wnd = &WND;
	gchar *lp_mcu;

	if ((lp_mcu = gtk_combo_box_text_get_active_text((GtkComboBoxText *) wnd->cbtmcu)) == NULL)							// nom mcu non trouve
		return;
		
	fv_majcnf("mcu", lp_mcu);																							// mise a jour fichier config
	fi_gstmcu(lp_mcu);																									// traitement nouveau mcu (affichage)
	g_free(lp_mcu);
}

void on_cbtlsn_changed(GtkComboBox *obj, gpointer dnn)
{
	Wnd *wnd = &WND;
	gchar *lp_lsn;

	if ((lp_lsn = gtk_combo_box_text_get_active_text((GtkComboBoxText *) wnd->cbtlsn)) == NULL)							// liaison non trouve
		return;
		
	fv_majcnf("liaison", lp_lsn);																						// mise a jour fichier config
	fi_gstlsn(lp_lsn);																									// traitement nouvelle liaison (connection)
	g_free(lp_lsn);
}

void on_bdgt_clicked(GtkButton *obj, gpointer dnn)
{
	if (gi_clc == 0)
		g_timeout_add(DBLCLC, cb_toc, obj);																				//millisecondes
	gi_clc++;
	return;
}

// call_back time out click (doubleclick ?)

gboolean cb_toc(gpointer obj)
{
	if (gi_clc == 1)	// simple click
		fv_tocs(obj);
	else				// double click
		fv_tocd(obj);

	gi_clc = 0;
	
	return(FALSE);
}

void fv_tocs(gpointer obj)	// simple click
{
	const gchar *pbc;

	pbc = gtk_widget_get_name((GtkWidget *) obj);	// nom objet = port bit couleur = v:vert(haut) r:rouge(bas) o:orange(indefini) g:gris(inexistant)

	if (*(pbc+1) == 's')		// bouton tous
		fv_tocss(obj);
	else						// bouton donnee
		fv_tocsd(obj);
}

void fv_tocsd(gpointer obj)	// simple click bouton donnee
{
	Wnd *wnd = &WND;
	const gchar *acc;
	const gchar *o_pbc;char s_pbc[]="  r";		// nom du bouton clicke, nom du bouton tous du port du bouton clicke
	char  txt[4];
	int   i, ret;

	acc = gtk_button_get_label((GtkButton *) obj);	// label du bouton = mode acces i/o = Z:input HighZ / U:Input PullUp / O:Output
	o_pbc = gtk_widget_get_name((GtkWidget *) obj);	// nom objet = port bit couleur = v:vert(haut) r:rouge(bas) o:orange(indefini) g:gris(inexistant)
	//s_pbc = gtk_widget_get_name((GtkWidget *) wnd->bdgt[(*o_pbc)-'a'][9]);	// bouton tous en fonction du port du bouton clicke

	strcpy(txt, o_pbc);
	
	// clicke sur bouton donnee en entree si bouton tous vert maj de toutes les entrees sinon maj de l'entree clicke (lecture etat reel)

	switch (*acc)
	{
		case 'Z':																										// le port est en entree ou en sortie
		case 'U':
		case 'O':
		{
			if (txt[2] == 'v')
			{
				txt[2] = 'r';
				wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
			}
			else if (txt[2] == 'r')
			{
				txt[2] = 'v';
				wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
			}
			gtk_button_set_image((GtkButton *) obj, wnd->pled);
			gtk_widget_set_name((GtkWidget *) obj, txt);
			break;
		}

		case 'A':																										// le port est configure en analogique
		{
			//if (*(s_pbc+2) == 'v')																					// bouton tous allume
			//{
			//	sprintf(gr_snd, "dr%cs", *o_pbc);																		// maj voyants en fonction de la reponse (port)
			//	fi_sndmss();
			//}
			//else																										// bouton tous eteint
			//{
			//	txt[1] = *(o_pbc+1);
			//
			//	sprintf(gr_snd, "ar%c%c", *o_pbc, *(o_pbc+1));															// affichage de la valeur analogic fonction de la reponse (int)
			//	fi_sndmss();
			//}
			break;
		}
	}
}

void fv_tocss(gpointer obj)	// simple click bouton tous
{
	Wnd *wnd = &WND;
	const gchar *acc;
	const gchar *pbc;
	char  s_pbc[4];

	acc = gtk_button_get_label((GtkButton *) obj);	// label du bouton = mode acces i/o = Z:input HighZ / U:Input PullUp / O:Output
	pbc = gtk_widget_get_name((GtkWidget *) obj);	// nom objet = port bit couleur = v:vert(haut) r:rouge(bas) o:orange(indefini) g:gris(inexistant)
	strncpy(s_pbc, pbc, 3);
	s_pbc[3] = 0;

	switch (pbc[2])			// couleur active (rouge/vert)
	{
		case 'v':		// vert -> rouge
			s_pbc[2] = 'r';
			gtk_widget_set_name((GtkWidget *) obj, s_pbc);
			wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
			gtk_button_set_image((GtkButton *) obj, wnd->pled);
			break;

		case 'r':		// rouge -> vert + change acces de tous les boutons du meme port + config port
			s_pbc[2] = 'v';
			gtk_widget_set_name((GtkWidget *) obj, s_pbc);
			wnd->pled = gtk_image_new_from_file("/var/local/icn/pvert.png");
			gtk_button_set_image((GtkButton *) obj, wnd->pled);

			sprintf(gr_snd, "cwp%cs%c", *s_pbc, tolower(*acc));
			fi_sndmss();
			break;
	}
}

void fv_tocd(gpointer obj)	// double click
{
	const gchar *pbc;

	pbc = gtk_widget_get_name((GtkWidget *) obj);	// nom objet = port bit couleur = v:vert(haut) r:rouge(bas) o:orange(indefini) g:gris(inexistant)

	if (*(pbc+1) == 's')		// bouton tous
		fv_tocds(obj);
	else						// bouton donnee
		fv_tocdd(obj);
}

void fv_tocdd(gpointer obj)	// double click bouton donnee
{
	Wnd *wnd = &WND;
	const gchar *acc;
	const gchar *o_pbc;char s_pbc[]="  r";		// nom du bouton clicke, nom du bouton tous du port du bouton clicke
	char  zuo[2];

	// si bouton tous en vert fait rien (interdit de changer un bit puisque gestion port entier)

	acc = gtk_button_get_label((GtkButton *) obj);	// label du bouton = mode acces i/o = Z:input HighZ / U:Input PullUp / O:Output
	o_pbc = gtk_widget_get_name((GtkWidget *) obj);	// nom objet = port bit couleur = v:vert(haut) r:rouge(bas) o:orange(indefini) g:gris(inexistant)

	//s_pbc = gtk_widget_get_name((GtkWidget *) wnd->bdgt[(*o_pbc)-'a'][9]);	// bouton tous en fonction du port du bouton clicke

	//if (*(s_pbc+2) == 'v')
	//	return;
	
	strcpy(zuo, acc);
	switch (*acc)
	{
		case 'Z':
			zuo[0] = 'U';
			break;
		case 'U':
			zuo[0] = 'O';
			break;
		case 'O':
			zuo[0] = 'Z';
			break;
		default:
			break;
	}
	gtk_button_set_label((GtkButton *) obj, zuo);
}

void fv_tocds(gpointer obj)	// double click bouton tous
{
	Wnd *wnd = &WND;
	const gchar *acc;		// acces du bouton clicke
	const gchar *pbc;		// nom du bouton clicke
	char  zuo[4];

	acc = gtk_button_get_label((GtkButton *) obj);	// label du bouton = mode acces i/o = Z:input HighZ / U:Input PullUp / O:Output
	pbc = gtk_widget_get_name((GtkWidget *) obj);	// nom objet = port bit couleur = v:vert(haut) r:rouge(bas) o:orange(indefini) g:gris(inexistant)

	zuo[1] = ' ';
	zuo[2] = 0;

	if (*(pbc+2) == 'r')
	{
		switch (*acc)
		{
			case 'Z':
				zuo[0] = 'U';
				gtk_button_set_label((GtkButton *) obj, zuo);
				break;
			case 'U':
				zuo[0] = 'O';
				gtk_button_set_label((GtkButton *) obj, zuo);
				break;
			case 'O':
				zuo[0] = 'Z';
				gtk_button_set_label((GtkButton *) obj, zuo);
				break;
		}
	}
	else
	{
		switch (*acc)
		{
			case 'Z':
				sprintf(gr_snd, "cwp%c%cu", *pbc, *(pbc+1));
				fi_sndmss();
				break;
			case 'U':
				sprintf(gr_snd, "cwp%c%co", *pbc, *(pbc+1));
				fi_sndmss();
				break;
			case 'O':
				sprintf(gr_snd, "cwp%c%cz", *pbc, *(pbc+1));
				fi_sndmss();
				break;
		}
	}
	
}

void on_bbrc_clicked(GtkButton *obj, gpointer dnn)
{
	CH  *crn;
	Wnd *wnd = &WND;
	const char *lbl;
	const char *nam;
	const char *dgt;
	char  *d, *f, *lgn, txt[8], adc[8], spp[8];
	char  a[9], n[4];
			
	lbl = gtk_button_get_label((GtkButton *) obj);
	nam = gtk_widget_get_name((GtkWidget *) obj);
	
	sprintf(txt, "P%s", nam);
	crn = lstbrc->first;
	while (crn)
	{
		lgn = crn->mmr+(*crn->mmr);
		
		if (strncmp(lgn, txt, 3) == 0)
		{
			if ((d = strstr(lgn, lbl)) != NULL)
			{	// PD4 T0
				//     d
				//       f
				f = d + strlen(lbl);
				if (*f == ' ')																							// un autre texte derriere
				{
					f++;
					d = f;
					while ((*f!=0) && (*f!=' '))
						f++;
					strncpy(adc, d, f-d);
					adc[f-d] = 0;
					// deactive bdgt
					if (strncmp(adc, "ADC", 3) == 0)																	// selection d'un ADCn
					{
						gtk_button_set_label((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], "A ");
						dgt = gtk_widget_get_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0']);
						strcpy(spp, dgt);
						spp[2] = 'g';
						gtk_widget_set_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], spp);
						wnd->pled = gtk_image_new_from_file("/var/local/icn/pgris.png");
						gtk_button_set_image((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], wnd->pled);

						wnd->eanl[txt[1]-'A'][txt[2]-'0'] = gtk_entry_new();											// analogic
						gtk_entry_set_width_chars((GtkEntry *) wnd->eanl[txt[1]-'A'][txt[2]-'0'], 5);
						gtk_entry_set_max_length((GtkEntry *) wnd->eanl[txt[1]-'A'][txt[2]-'0'], 5);
						n[0] = txt[1];
						n[1] = txt[2];
						n[2] = txt[2];//a[txt[2]-'0'];
						n[3] = 0;
						gtk_widget_set_name(wnd->eanl[txt[1]-'A'][txt[2]-'0'], n);										// name (n) port bit numero
						gtk_grid_attach((GtkGrid *) wnd->grdanl, wnd->eanl[txt[1]-'A'][txt[2]-'0'], 9-(txt[2]-'0'), /*ya+*/1, 1, 1);
						gtk_widget_show(wnd->eanl[txt[1]-'A'][txt[2]-'0']);
						sprintf(d, "ADC%d", (n[2]>='a')?n[2]-'a'+10:n[2]-'0');
						gtk_widget_set_tooltip_text(wnd->eanl[txt[1]-'A'][txt[2]-'0'], d);
						//gi_anl++;
						//sprintf(spp, "%s=%s,", nam, &adc[3]);
						//strcat(gr_anl, spp);
						//ht_rmptxt(gr_dgt, spp, "", 1);
						////fv_anldnm();
					}
					else if ((adc[0] == 'P') && (adc[1] == txt[1]) && (adc[2] == txt[2]))
					{
						gtk_button_set_label((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], "Z ");
						dgt = gtk_widget_get_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0']);
						strcpy(spp, dgt);
						spp[2] = 'r';
						gtk_widget_set_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], spp);
						wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
						gtk_button_set_image((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], wnd->pled);
					}
					else if ((adc[0]=='O') && (adc[1]=='C'))
					{
						gtk_button_set_label((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], "T ");
						dgt = gtk_widget_get_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0']);
						strcpy(spp, dgt);
						spp[2] = 'g';
						gtk_widget_set_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], spp);
						wnd->pled = gtk_image_new_from_file("/var/local/icn/pgris.png");
						gtk_button_set_image((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], wnd->pled);
					}
					else
					{
						gtk_button_set_label((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], "R ");
						dgt = gtk_widget_get_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0']);
						strcpy(spp, dgt);
						spp[2] = 'g';
						gtk_widget_set_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], spp);
						wnd->pled = gtk_image_new_from_file("/var/local/icn/pgris.png");
						gtk_button_set_image((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], wnd->pled);
					}

					// reactive bdgt
					if (strncmp(lbl, "ADC", 3) == 0)																	// deselection d'un ADCn
					{
						gtk_widget_destroy(wnd->eanl[txt[1]-'A'][txt[2]-'0']);
						/*
						gi_anl--;
						d = strstr(gr_anl, nam);
						f = strchr(d, ',');
						f++;
						strncpy(spp, d, f-d);
						spp[f-d] = 0;
						ht_rmptxt(gr_anl, spp, "", 1);
						strcat(gr_dgt, spp);
						////fv_anldnm();
						*/
					}

					// ATTENTION set_label apres utilisation lbl pour gi_anl--
					gtk_button_set_label((GtkButton *) obj, adc);														// pris le choix alternatif suivant
					break;
				}
				else /* if (*f == 0) */																					// plus de texte derriere
				{
					if (d == lgn)																						// unique (pas de choix alternatif pour la broche)
						break;
					else																								// avait le dernier reprend le premier
					{
						gtk_button_set_label((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], "Z ");
						dgt = gtk_widget_get_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0']);
						strcpy(spp, dgt);
						spp[2] = 'r';
						gtk_widget_set_name((GtkWidget *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], spp);
						wnd->pled = gtk_image_new_from_file("/var/local/icn/prouge.png");
						gtk_button_set_image((GtkButton *) wnd->bdgt[txt[1]-'A'][txt[2]-'0'], wnd->pled);

						// reactive bdgt
						if (strncmp(lbl, "ADC", 3) == 0)																// deselection d'un ADCn
						{
							gtk_widget_destroy(wnd->eanl[txt[1]-'A'][txt[2]-'0']);
							/*
							gi_anl--;
							d = strstr(gr_anl, nam);
							f = strchr(d, ',');
							f++;
							strncpy(spp, d, f-d);
							spp[f-d] = 0;
							ht_rmptxt(gr_anl, spp, "", 1);
							strcat(gr_dgt, spp);
							////fv_anldnm();
							*/
						}

						// ATTENTION ..._set_label... doit etre apres l'utilisation de lbl 
						gtk_button_set_label((GtkButton *) obj, txt);													// le premier est encore dans txt
						break;
					}
				}
			}
		}
		
		crn = crn->next;
	}
}

