
// nom=INTGRP
// version=0.0
// quoi=Interface graphique
// qui=GNRGRP

// Include

#include "intgrp.h"

// Define

// Declaration fonction

void aff_fenetre(void);
void create_wnd(void);
void show_wnd(void);
void hide_wnd(void);

void fv_cns_vrb(void);
void fv_dst_vrb(void);

// Declaration callback

void on_cbtmcu_changed(GtkComboBox *, gpointer);
void on_cbtlsn_changed(GtkComboBox *, gpointer);
void on_bdgt_clicked(GtkButton *, gpointer);
void on_bbrc_clicked(GtkButton *, gpointer);
void on_wnd_destroy(GtkWindow *obj, gpointer dnn);

// Variable globale

Wnd WND;

// Creation fenetre

void create_wnd(void)
{
	Wnd *wnd = &WND;
//	PangoFontDescription *pfd;
	char t[32];
	int x, y;
	
	wnd->wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title((GtkWindow *) wnd->wnd, "Simulateur AVR / Vue generale");
	gtk_window_set_icon_from_file((GtkWindow *) wnd->wnd, "/var/local/icn/ardsml.png", NULL);
	gtk_window_set_default_size((GtkWindow *) wnd->wnd, 910, 440);
	//gtk_window_set_position((GtkWindow *) wnd->wnd, GTK_WIN_POS_CENTER_ALWAYS);

	wnd->box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_container_add((GtkContainer *) wnd->wnd, wnd->box1);

	wnd->frmio = gtk_frame_new(NULL);
	//gtk_widget_set_margin_start(wnd->frmio, 5);
	//gtk_widget_set_margin_end(wnd->frmio, 5);
	gtk_widget_set_margin_bottom(wnd->frmio, 5);
	gtk_box_pack_start((GtkBox *) wnd->box1, wnd->frmio, FALSE, TRUE, 5);

	wnd->algio = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmio, wnd->algio);

	wnd->box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add((GtkContainer *) wnd->algio, wnd->box2);

	wnd->frmdgt = gtk_frame_new(NULL);																					// digital
	gtk_widget_set_margin_start(wnd->frmdgt, 5);
	gtk_widget_set_margin_end(wnd->frmdgt, 5);
	//gtk_widget_set_margin_bottom(wnd->frmdgt, 5);
	gtk_box_pack_start((GtkBox *) wnd->box2, wnd->frmdgt, FALSE, TRUE, 5);

	wnd->algdgt = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmdgt, wnd->algdgt);

	wnd->grddgt = NULL;
	//wnd->grddgt = gtk_grid_new();																						// digital
	//gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grddgt, TRUE);
	//gtk_container_add((GtkContainer *) wnd->algdgt, wnd->grddgt);
	//gtk_widget_show(wnd->grddgt);
	
	wnd->lbldgt = gtk_label_new(" Numerique ");
	gtk_label_set_markup(wnd->lbldgt, "<span foreground=\"#0066AA\"> Numerique </span>");
	gtk_frame_set_label_widget((GtkFrame *) wnd->frmdgt, wnd->lbldgt);

	wnd->frmanl = gtk_frame_new(NULL);																					// analogic
	gtk_widget_set_margin_start(wnd->frmanl, 5);
	gtk_widget_set_margin_end(wnd->frmanl, 5);
	//gtk_widget_set_margin_bottom(wnd->frmanl, 5);
	gtk_box_pack_start((GtkBox *) wnd->box2, wnd->frmanl, FALSE, TRUE, 5);

	wnd->alganl = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmanl, wnd->alganl);

	wnd->grdanl = NULL;
	//wnd->grdanl = gtk_grid_new();																						// analogic
	//gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grdanl, TRUE);
	//gtk_container_add((GtkContainer *) wnd->alganl, wnd->grdanl);
	//gtk_widget_show(wnd->grdanl);
	
	wnd->lblanl = gtk_label_new(" Analogique ");
	gtk_label_set_markup(wnd->lblanl, "<span foreground=\"#0066AA\"> Analogique </span>");
	gtk_frame_set_label_widget((GtkFrame *) wnd->frmanl, wnd->lblanl);

	wnd->frmtmr = gtk_frame_new(NULL);																					// timer
	gtk_widget_set_margin_start(wnd->frmtmr, 5);
	gtk_widget_set_margin_end(wnd->frmtmr, 5);
	//gtk_widget_set_margin_bottom(wnd->frmtmr, 5);
	gtk_box_pack_start((GtkBox *) wnd->box2, wnd->frmtmr, FALSE, TRUE, 5);

	wnd->algtmr = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmtmr, wnd->algtmr);

	wnd->grdtmr = NULL;
	//wnd->grdtmr = gtk_grid_new();																						// timer
	//gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grdtmr, TRUE);
	//gtk_container_add((GtkContainer *) wnd->algtmr, wnd->grdtmr);
	//gtk_widget_show(wnd->grdtmr);

	wnd->lbltmr = gtk_label_new(" Compteur ");
	gtk_label_set_markup(wnd->lbltmr, "<span foreground=\"#0066AA\"> Compteur </span>");
	gtk_frame_set_label_widget((GtkFrame *) wnd->frmtmr, wnd->lbltmr);

	wnd->lblio = gtk_label_new(" E/S ");
	gtk_label_set_markup(wnd->lblio, "<span foreground=\"#0066AA\"><b> E/S </b></span>");
	//pfd = pango_font_description_new();
	//pango_font_description_set_weight(pfd, PANGO_WEIGHT_BOLD);
	//gtk_widget_override_font(wnd->lblio, pfd);
	//pango_font_description_free(pfd);
	gtk_frame_set_label_widget((GtkFrame *) wnd->frmio, wnd->lblio);

	wnd->frmsst = gtk_frame_new(NULL);
	//gtk_widget_set_margin_start(wnd->frmsst, 5);
	//gtk_widget_set_margin_end(wnd->frmsst, 5);
	gtk_widget_set_margin_bottom(wnd->frmsst, 5);
	gtk_box_pack_start((GtkBox *) wnd->box1, wnd->frmsst, FALSE, TRUE, 5);

	wnd->algsst = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmsst, wnd->algsst);

	wnd->box3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add((GtkContainer *) wnd->algsst, wnd->box3);

	wnd->frmvnt = gtk_frame_new(NULL);
	gtk_widget_set_margin_start(wnd->frmvnt, 5);
	gtk_widget_set_margin_end(wnd->frmvnt, 5);
	//gtk_widget_set_margin_bottom(wnd->frmvnt, 5);
	gtk_box_pack_start((GtkBox *) wnd->box3, wnd->frmvnt, FALSE, TRUE, 5);

	wnd->algvnt = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmvnt, wnd->algvnt);

	wnd->grdvnt = gtk_grid_new();
	gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grdvnt, TRUE);
	gtk_container_add((GtkContainer *) wnd->algvnt, wnd->grdvnt);

	wnd->bttdsr = gtk_button_new_with_label("dsr ");
	wnd->pled = gtk_image_new_from_file("/var/local/icn/pgris.png");
	gtk_button_set_image((GtkButton *) wnd->bttdsr, wnd->pled);
	gtk_button_set_image_position((GtkButton *) wnd->bttdsr, GTK_POS_RIGHT);
	gtk_button_set_always_show_image((GtkButton *) wnd->bttdsr, TRUE);
	gtk_grid_attach((GtkGrid *) wnd->grdvnt, wnd->bttdsr, 0, 0, 1, 1);

	wnd->bttcts = gtk_button_new_with_label("cts ");
	wnd->pled = gtk_image_new_from_file("/var/local/icn/pgris.png");
	gtk_button_set_image((GtkButton *) wnd->bttcts, wnd->pled);
	gtk_button_set_image_position((GtkButton *) wnd->bttcts, GTK_POS_RIGHT);
	gtk_button_set_always_show_image((GtkButton *) wnd->bttcts, TRUE);
	gtk_grid_attach((GtkGrid *) wnd->grdvnt, wnd->bttcts, 1, 0, 1, 1);

	wnd->bttcnn = gtk_button_new_with_label("Connection... ");
	wnd->pled = gtk_image_new_from_file("/var/local/icn/pgris.png");
	gtk_button_set_image((GtkButton *) wnd->bttcnn, wnd->pled);
	gtk_button_set_image_position((GtkButton *) wnd->bttcnn, GTK_POS_RIGHT);
	gtk_button_set_always_show_image((GtkButton *) wnd->bttcnn, TRUE);
	gtk_widget_set_name(wnd->bttcnn, "g");
	gtk_grid_attach((GtkGrid *) wnd->grdvnt, wnd->bttcnn, 2, 0, 2, 1);

	wnd->frmcrt = gtk_frame_new(NULL);																					// carte
	gtk_widget_set_margin_start(wnd->frmcrt, 5);
	gtk_widget_set_margin_end(wnd->frmcrt, 5);
	//gtk_widget_set_margin_bottom(wnd->frmcrt, 5);
	gtk_box_pack_start((GtkBox *) wnd->box3, wnd->frmcrt, FALSE, TRUE, 5);

	wnd->algcrt = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmcrt, wnd->algcrt);

	wnd->box4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_container_add((GtkContainer *) wnd->algcrt, wnd->box4);

	wnd->cbtmcu = gtk_combo_box_text_new();
	gtk_box_pack_start((GtkBox *) wnd->box4, wnd->cbtmcu, TRUE, TRUE, 5);

	wnd->cbtlsn = gtk_combo_box_text_new();
	gtk_box_pack_start((GtkBox *) wnd->box4, wnd->cbtlsn, TRUE, TRUE, 5);

	wnd->lblcrt = gtk_label_new(" Carte ");
	gtk_label_set_markup(wnd->lblcrt, "<span foreground=\"#0066AA\"> Carte </span>");
	gtk_frame_set_label_widget((GtkFrame *) wnd->frmcrt, wnd->lblcrt);

	wnd->frmbrc = gtk_frame_new(NULL);																					// broche
	gtk_widget_set_margin_start(wnd->frmbrc, 5);
	gtk_widget_set_margin_end(wnd->frmbrc, 5);
	//gtk_widget_set_margin_bottom(wnd->frmbrc, 5);
	gtk_box_pack_start((GtkBox *) wnd->box3, wnd->frmbrc, FALSE, TRUE, 5);

	wnd->algbrc = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_container_add((GtkContainer *) wnd->frmbrc, wnd->algbrc);

	wnd->grdbrc = NULL;
	//wnd->grdbrc = gtk_grid_new();																						// broche
	//gtk_grid_set_column_homogeneous((GtkGrid *) wnd->grdbrc, TRUE);
	//gtk_container_add((GtkContainer *) wnd->algbrc, wnd->grdbrc);
	//gtk_widget_show(wnd->grdbrc);
	
	wnd->lblbrc = gtk_label_new(" Broche ");
	gtk_label_set_markup(wnd->lblbrc, "<span foreground=\"#0066AA\"> Broche </span>");
	gtk_frame_set_label_widget((GtkFrame *) wnd->frmbrc, wnd->lblbrc);

	wnd->lblsst = gtk_label_new(" Systeme ");
	gtk_label_set_markup(wnd->lblsst, "<span foreground=\"#0066AA\"><b> Systeme </b></span>");
	//pfd = pango_font_description_new();
	//pango_font_description_set_weight(pfd, PANGO_WEIGHT_BOLD);
	//gtk_widget_override_font(wnd->lblsst, pfd);
	//pango_font_description_free(pfd);
	gtk_frame_set_label_widget((GtkFrame *) wnd->frmsst, wnd->lblsst);
	
	g_signal_connect((GtkWindow *) wnd->wnd, "destroy", (GCallback) on_wnd_destroy, NULL);
	g_signal_connect((GtkComboBox *) wnd->cbtmcu, "changed", (GCallback) on_cbtmcu_changed, "MCU");
	g_signal_connect((GtkComboBox *) wnd->cbtlsn, "changed", (GCallback) on_cbtlsn_changed, "Liaison");
}

// Affichage Fenetre

void show_wnd(void)
{
	Wnd *wnd = &WND;

	gtk_widget_show_all(wnd->wnd);
}

// Cache fenetre

void hide_wnd(void)
{
	Wnd *wnd = &WND;

	gtk_widget_hide(wnd->wnd);
}

// Quitte le programme

void on_wnd_destroy(GtkWindow *obj, gpointer dnn)
{
	fv_dst_vrb();
	gtk_main_quit();
}

//
// Affichage fenetre unique
//
// Appele par main.c
//

void aff_fenetre(void)
{
	create_wnd();

	fv_cns_vrb();

	show_wnd();
}
