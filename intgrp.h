
// nom=INTGRP
// version=0.0
// quoi=Extern pour main()...
// qui=GNRGRP

#ifndef INTGRP_H
#define INTGRP_H 1

#include "cllbck.h"

extern void aff_fenetre(void);
extern void create_wnd(void);
extern void show_wnd(void);
extern void hide_wnd(void);

extern void on_cbtmcu_changed(GtkComboBox *, gpointer);
extern void on_cbtlsn_changed(GtkComboBox *, gpointer);
extern void on_bdgt_clicked(GtkButton *, gpointer);
extern void on_bbrc_clicked(GtkButton *, gpointer);

#ifndef Wnd
typedef struct _wnd
{
	GtkWidget *wnd;
	GtkWidget *box1;
	GtkWidget *frmio;
	GtkWidget *algio;
	GtkWidget *box2;
	GtkWidget *frmdgt;
	GtkWidget *algdgt;
	GtkWidget *grddgt;
	GtkWidget *ldgtd[9];
	GtkWidget *ldgtp[12];
	GtkWidget *bdgt[12][9];
	GtkWidget *lbldgt;
	GtkWidget *frmanl;
	GtkWidget *alganl;
	GtkWidget *grdanl;
	GtkWidget *lanla[10];
	GtkWidget *lanlc[12];
	GtkWidget *eanl[12][8];
	GtkWidget *lblanl;
	GtkWidget *frmtmr;
	GtkWidget *algtmr;
	GtkWidget *grdtmr;
	GtkWidget *ltmrt[8];
	GtkWidget *ctmrm[20];
	GtkWidget *ctmrr[20];
	GtkWidget *etmrh[20];
	GtkWidget *etmrp[20];
	GtkWidget *etmrt[20];
	GtkWidget *etmri[20];
	GtkWidget *etmro[20];
	GtkWidget *lbltmr;
	GtkWidget *lblio;
	GtkWidget *frmsst;
	GtkWidget *algsst;
	GtkWidget *frmvnt;
	GtkWidget *algvnt;
	GtkWidget *grdvnt;
	GtkWidget *bttdsr;
	GtkWidget *bttcts;
	GtkWidget *bttcnn;
	GtkWidget *box3;
	GtkWidget *frmcrt;
	GtkWidget *algcrt;
	GtkWidget *box4;
	GtkWidget *cbtmcu;
	GtkWidget *cbtlsn;
	GtkWidget *lblcrt;
	GtkWidget *frmbrc;
	GtkWidget *algbrc;
	GtkWidget *grdbrc;
	GtkWidget *lbrcd[8];
	GtkWidget *lbrcp[12];
	GtkWidget *bbrc[12][8];
	GtkWidget *lblbrc;
	GtkWidget *lblsst;
	GtkWidget *pled;
} Wnd;
#endif

extern Wnd WND;
#endif
