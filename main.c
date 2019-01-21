
#include <gtk/gtk.h>
#include "intgrp.h"
#include "bbliv.h"

IV iv = {"joel", "smlavr", "1.1", "12/2016", "parrat"};

int main(int argc, char **argv, char **envv)
{
	gtk_init(&argc, &argv);
	aff_fenetre();
	gtk_main();

	return(0);
}
