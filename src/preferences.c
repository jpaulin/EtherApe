/* EtherApe
 * Copyright (C) 2001 Juan Toledo
 * $Id$
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "preferences.h"
#include "diagram.h"
#include "capture.h"
#include "datastructs.h"

static void color_list_to_pref (void);
static void load_color_list (void);
static gboolean get_version_levels (const gchar * version_string,
				    guint * major, guint * minor,
				    guint * patch);
static gint version_compare (const gchar * a, const gchar * b);


static gboolean colors_changed = FALSE;
static GtkWidget *diag_pref = NULL;		/* Pointer to the diagram configuration window */


/* loads configuration from .gnome/Etherape */
void
load_config (const char *prefix)
{
  gboolean u;
  gchar *config_file_version;

  gnome_config_push_prefix (prefix);

  config_file_version =
    gnome_config_get_string_with_default ("General/version=0.5.4", &u);
  pref.diagram_only =
    gnome_config_get_bool_with_default ("Diagram/diagram_only=FALSE", &u);
  pref.group_unk =
    gnome_config_get_bool_with_default ("Diagram/group_unk=TRUE", &u);
  pref.stationary
    = gnome_config_get_bool_with_default ("Diagram/stationary=FALSE", &u);
  pref.nofade = gnome_config_get_bool_with_default ("Diagram/nofade=FALSE", &u);
  pref.cycle = gnome_config_get_bool_with_default ("Diagram/cycle=TRUE", &u);
  pref.new_infodlg = gnome_config_get_bool_with_default ("Diagram/new_infodlg=TRUE", &u);
  pref.antialias =
    gnome_config_get_bool_with_default ("Diagram/antialias=TRUE", &u);
  pref.name_res =
    gnome_config_get_bool_with_default ("Diagram/name_res=TRUE", &u);
  pref.node_timeout_time =
  gnome_config_get_float_with_default("Diagram/node_timeout_time=3600000.0", &u); 
  pref.gui_node_timeout_time =
    gnome_config_get_float_with_default("Diagram/gui_node_timeout_time=60000.0", &u);
  pref.proto_node_timeout_time =
    gnome_config_get_float_with_default("Diagram/proto_node_timeout_time=60000.0", &u);

  pref.link_timeout_time =
      gnome_config_get_float_with_default("Diagram/link_timeout_time=60000.0", &u);
  pref.gui_link_timeout_time =
      gnome_config_get_float_with_default("Diagram/gui_link_timeout_time=20000.0", &u);
  pref.proto_link_timeout_time =
      gnome_config_get_float_with_default("Diagram/proto_link_timeout_time=20000.0", &u);

  pref.proto_timeout_time =
      gnome_config_get_float_with_default("Diagram/proto_timeout_time=86400000.0", &u);

  pref.averaging_time =
    gnome_config_get_float_with_default ("Diagram/averaging_time=3000.0", &u);
  pref.node_radius_multiplier =
    gnome_config_get_float_with_default
    ("Diagram/node_radius_multiplier=0.0005", &u);
  if (u)
    pref.node_radius_multiplier = 0.0005;	/* This is a bug with gnome_config */
  pref.link_width_multiplier =
    gnome_config_get_float_with_default
    ("Diagram/link_width_multiplier=0.0005", &u);
  if (u)
    pref.link_width_multiplier = 0.0005;
  pref.mode = gnome_config_get_int_with_default ("General/mode=-1", &u);	/* DEFAULT */
  if (pref.mode == IP || pref.mode == TCP)
    pref.refresh_period =
      gnome_config_get_int_with_default ("Diagram/refresh_period=3000", &u);
  else
    pref.refresh_period =
      gnome_config_get_int_with_default ("Diagram/refresh_period=800", &u);

  pref.size_mode = gnome_config_get_int_with_default ("Diagram/size_mode=0", &u);	/* LINEAR */
  pref.node_size_variable = gnome_config_get_int_with_default ("Diagram/node_size_variable=2", &u);	/* INST_OUTBOUND */
  pref.stack_level =
    gnome_config_get_int_with_default ("Diagram/stack_level=0", &u);
  if ((pref.stack_level != 0)
      && (version_compare (config_file_version, "0.5.4") < 0))
    g_warning (_("Stack Level is not set to Topmost Recognized Protocol. "
		 "Please check in the preferences dialog that this is what "
		 "you really want"));
  pref.fontname =
    gnome_config_get_string_with_default
    ("Diagram/fontname=-*-*-*-*-*-*-*-140-*-*-*-*-iso8859-1", &u);
  gnome_config_get_vector_with_default
    ("Diagram/colors=#ff0000;WWW #0000ff;DOMAIN #00ff00 #ffff00 #ff00ff #00ffff #ffffff #ff7700 #ff0077 #ffaa77 #7777ff #aaaa33",
     &(pref.n_colors), &(pref.colors), &u);

  if (!pref.n_colors || !pref.colors || !strlen(pref.colors[0]))
  {
     /* color array defined in prefs, but empty */
     pref.n_colors = 1;
     g_free(pref.colors[0]);
     pref.colors[0] = g_strdup("#7f7f7f");
  }

  g_free (config_file_version);
  gnome_config_pop_prefix ();

  protohash_read_prefvect(pref.colors, pref.n_colors);
}				/* load_config */

/* saves configuration to .gnome/Etherape */
/* It's not static since it will be called from the GUI */
void
save_config (const char *prefix)
{
  gnome_config_push_prefix (prefix);
  gnome_config_set_bool ("Diagram/diagram_only", pref.diagram_only);
  gnome_config_set_bool ("Diagram/group_unk", pref.group_unk);
  gnome_config_set_bool ("Diagram/nofade", pref.nofade);
  gnome_config_set_bool ("Diagram/cycle", pref.cycle);
  gnome_config_set_bool ("Diagram/antialias", pref.antialias);
  gnome_config_set_bool ("Diagram/name_res", pref.name_res);
  gnome_config_set_bool ("Diagram/new_infodlg", pref.new_infodlg);
  gnome_config_set_float ("Diagram/node_timeout_time",
			  pref.node_timeout_time);
  gnome_config_set_float ("Diagram/gui_node_timeout_time",
			  pref.gui_node_timeout_time);
  gnome_config_set_float ("Diagram/proto_node_timeout_time",
			  pref.proto_node_timeout_time);
  gnome_config_set_float ("Diagram/link_timeout_time",
			  pref.link_timeout_time);
  gnome_config_set_float ("Diagram/gui_link_timeout_time",
			  pref.gui_link_timeout_time);
  gnome_config_set_float ("Diagram/proto_link_timeout_time",
			  pref.proto_link_timeout_time);
  gnome_config_set_float ("Diagram/proto_timeout_time",
			  pref.proto_timeout_time);
  gnome_config_set_float ("Diagram/averaging_time", pref.averaging_time);
  gnome_config_set_float ("Diagram/node_radius_multiplier",
			  pref.node_radius_multiplier);
  gnome_config_set_float ("Diagram/link_width_multiplier",
			  pref.link_width_multiplier);
#if 0				/* TODO should we save this? */
  gnome_config_set_int ("General/mode", pref.mode);
#endif
  gnome_config_set_int ("Diagram/refresh_period", pref.refresh_period);
  gnome_config_set_int ("Diagram/size_mode", pref.size_mode);
  gnome_config_set_int ("Diagram/node_size_variable",
			pref.node_size_variable);
  gnome_config_set_int ("Diagram/stack_level", pref.stack_level);
  gnome_config_set_string ("Diagram/fontname", pref.fontname);

  gnome_config_set_vector ("Diagram/colors", pref.n_colors,
			   (const gchar * const *) pref.colors);

  gnome_config_set_string ("General/version", VERSION);

  gnome_config_sync ();
  gnome_config_pop_prefix ();

  g_my_info (_("Preferences saved"));

}				/* save_config */

void
initialize_pref_controls(void)
{
  GtkWidget *widget;
  GtkSpinButton *spin;

  diag_pref = glade_xml_get_widget (xml, "diag_pref");

  /* Updates controls from values of variables */
  widget = glade_xml_get_widget (xml, "node_radius_slider");
  gtk_adjustment_set_value (GTK_RANGE (widget)->adjustment,
			    log (pref.node_radius_multiplier) / log (10));
  g_signal_emit_by_name (G_OBJECT (GTK_RANGE (widget)->adjustment),
			 "changed");
  widget = glade_xml_get_widget (xml, "link_width_slider");
  gtk_adjustment_set_value (GTK_RANGE (widget)->adjustment,
			    log (pref.link_width_multiplier) / log (10));
  g_signal_emit_by_name (GTK_OBJECT (GTK_RANGE (widget)->adjustment),
			 "changed");
  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "averaging_spin"));
  gtk_spin_button_set_value (spin, pref.averaging_time);
  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "refresh_spin"));
  gtk_spin_button_set_value (spin, pref.refresh_period);
  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "node_to_spin"));
  gtk_spin_button_set_value (spin, pref.node_timeout_time);
  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "gui_node_to_spin"));
  gtk_spin_button_set_value (spin, pref.gui_node_timeout_time);
  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "proto_node_to_spin"));
  gtk_spin_button_set_value (spin, pref.proto_node_timeout_time);

  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "link_to_spin"));
  gtk_spin_button_set_value (spin, pref.link_timeout_time);
  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "gui_link_to_spin"));
  gtk_spin_button_set_value (spin, pref.gui_link_timeout_time);
  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "proto_link_to_spin"));
  gtk_spin_button_set_value (spin, pref.proto_link_timeout_time);

  spin = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "proto_to_spin"));
  gtk_spin_button_set_value (spin, pref.proto_timeout_time);

  widget = glade_xml_get_widget (xml, "diagram_only_toggle");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
				pref.diagram_only);
  widget = glade_xml_get_widget (xml, "group_unk_check");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), pref.group_unk);
  widget = glade_xml_get_widget (xml, "fade_toggle");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), !pref.nofade);
  widget = glade_xml_get_widget (xml, "cycle_toggle");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), pref.cycle);
  widget = glade_xml_get_widget (xml, "aa_check");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), pref.antialias);
  widget = glade_xml_get_widget (xml, "name_res_check");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), pref.name_res);
  widget = glade_xml_get_widget (xml, "new_infodlg_check");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), pref.new_infodlg);

  widget = glade_xml_get_widget (xml, "size_mode_menu");
  gtk_option_menu_set_history (GTK_OPTION_MENU (widget), pref.size_mode);
  widget = glade_xml_get_widget (xml, "node_size_optionmenu");
  gtk_option_menu_set_history (GTK_OPTION_MENU (widget),
			       pref.node_size_variable);
  widget = glade_xml_get_widget (xml, "stack_level_menu");
  gtk_option_menu_set_history (GTK_OPTION_MENU (widget), pref.stack_level);
  widget = glade_xml_get_widget (xml, "filter_gnome_entry");
  widget = glade_xml_get_widget (xml, "file_filter_entry");
  widget = glade_xml_get_widget (xml, "fileentry");
  widget = gnome_file_entry_gnome_entry (GNOME_FILE_ENTRY (widget));

  load_color_list ();		/* Updates the color preferences table with pref.colors */

  /* Connects signals */
  widget = glade_xml_get_widget (xml, "node_radius_slider");
  g_signal_connect (G_OBJECT (GTK_RANGE (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_node_radius_slider_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "link_width_slider");
  g_signal_connect (G_OBJECT (GTK_RANGE (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_link_width_slider_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "averaging_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_averaging_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "refresh_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_refresh_spin_adjustment_changed),
		    glade_xml_get_widget (xml, "canvas1"));
  widget = glade_xml_get_widget (xml, "node_to_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_node_to_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "gui_node_to_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_gui_node_to_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "proto_node_to_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_proto_node_to_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "link_to_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_link_to_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "gui_link_to_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_gui_link_to_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "proto_link_to_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_proto_link_to_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "proto_to_spin");
  g_signal_connect (G_OBJECT (GTK_SPIN_BUTTON (widget)->adjustment),
		    "value_changed",
		    GTK_SIGNAL_FUNC
		    (on_proto_to_spin_adjustment_changed), NULL);
  widget = glade_xml_get_widget (xml, "size_mode_menu");
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (widget)->menu),
		    "deactivate",
		    GTK_SIGNAL_FUNC (on_size_mode_menu_selected), NULL);
  widget = glade_xml_get_widget (xml, "node_size_optionmenu");
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (widget)->menu),
		    "deactivate",
		    GTK_SIGNAL_FUNC (on_node_size_optionmenu_selected), NULL);
  widget = glade_xml_get_widget (xml, "stack_level_menu");
  g_signal_connect (G_OBJECT (GTK_OPTION_MENU (widget)->menu),
		    "deactivate",
		    GTK_SIGNAL_FUNC (on_stack_level_menu_selected), NULL);
}

void
on_preferences1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  GtkEditable *entry;
  gint position = 0;

  entry = GTK_EDITABLE (glade_xml_get_widget (xml, "filter_entry"));
  gtk_editable_delete_text (entry, 0, -1);
  if (pref.filter)
    gtk_editable_insert_text (entry, pref.filter, strlen (pref.filter),
			      &position);
  else
    gtk_editable_insert_text (entry, "", 0, &position);
  gtk_widget_show (diag_pref);
  gdk_window_raise (diag_pref->window);
}

void
on_node_radius_slider_adjustment_changed (GtkAdjustment * adj)
{

  pref.node_radius_multiplier = exp ((double) adj->value * log (10));
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
	 _("Adjustment value: %g. Radius multiplier %g"),
	 adj->value, pref.node_radius_multiplier);

}

void
on_link_width_slider_adjustment_changed (GtkAdjustment * adj)
{

  pref.link_width_multiplier = exp ((double) adj->value * log (10));
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
	 _("Adjustment value: %g. Radius multiplier %g"),
	 adj->value, pref.link_width_multiplier);

}

void
on_averaging_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.averaging_time = adj->value;	/* Control and value in ms */
}

void
on_refresh_spin_adjustment_changed (GtkAdjustment * adj, GtkWidget * canvas)
{
  pref.refresh_period = adj->value;
  /* When removing the source (which could either be an idle or a timeout
   * function, I'm also forcing the callback for the corresponding 
   * destroying function, which in turn will install a timeout or idle
   * function using the new refresh_period. It might take a while for it
   * to settle down, but I think it works now */
  g_source_remove (diagram_timeout);
}

void
on_node_to_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.node_timeout_time = adj->value;	/* Control and value in ms */
}				/* on_node_to_spin_adjustment_changed */

void
on_gui_node_to_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.gui_node_timeout_time = adj->value;	/* Control and value in ms */
}

void
on_proto_node_to_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.proto_node_timeout_time = adj->value;	/* Control and value in ms */
}

void
on_link_to_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.link_timeout_time = adj->value;	/* Control and value in ms */
}

void
on_gui_link_to_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.gui_link_timeout_time = adj->value;	/* Control and value in ms */
}

void
on_proto_link_to_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.proto_link_timeout_time = adj->value;	/* Control and value in ms */
}

void
on_proto_to_spin_adjustment_changed (GtkAdjustment * adj)
{
  pref.proto_timeout_time = adj->value;	/* Control and value in ms */
}

void
on_font_button_clicked (GtkButton * button, gpointer user_data)
{
  static GtkWidget *fontsel = NULL;
  if (!fontsel)
    fontsel = glade_xml_get_widget (xml, "fontselectiondialog1");
  gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG
					   (fontsel), pref.fontname);
  gtk_widget_show (fontsel);
}


void
on_ok_button1_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *fontsel;
  gchar *str;
  fontsel = glade_xml_get_widget (xml, "fontselectiondialog1");
  str =
    gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG
					     (fontsel));
  if (str)
    {
      if (pref.fontname)
	g_free (pref.fontname);
      pref.fontname = g_strdup (str);
      g_free (str);
      need_reposition = TRUE;
    }

  gtk_widget_hide (fontsel);
}


void
on_cancel_button1_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *fontsel;
  fontsel = glade_xml_get_widget (xml, "fontselectiondialog1");
  gtk_widget_hide (fontsel);
}				/* on_cancel_button1_clicked */


void
on_apply_button1_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *fontsel;
  gchar *str;

  fontsel = glade_xml_get_widget (xml, "fontselectiondialog1");
  str =
    gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG
					     (fontsel));
  if (str)
    {
      if (pref.fontname)
	g_free (pref.fontname);
      pref.fontname = g_strdup (str);
      g_free (str);
      need_reposition = TRUE;
    }
}				/* on_apply_button1_clicked */

void
on_size_mode_menu_selected (GtkMenuShell * menu_shell, gpointer data)
{
  GtkWidget *active_item;

  active_item = gtk_menu_get_active (GTK_MENU (menu_shell));
  /* Beware! Size mode is an enumeration. The menu options
   * must much the enumaration values */
  pref.size_mode = g_list_index (menu_shell->children, active_item);

}				/* on_size_mode_menu_selected */

void
on_node_size_optionmenu_selected (GtkMenuShell * menu_shell, gpointer data)
{
  GtkWidget *active_item;

  active_item = gtk_menu_get_active (GTK_MENU (menu_shell));
  /* Beware! Size mode is an enumeration. The menu options
   * must much the enumaration values */
  pref.node_size_variable = g_list_index (menu_shell->children, active_item);

}				/* on_node_size_optionmenu_selected */

void
on_stack_level_menu_selected (GtkMenuShell * menu_shell, gpointer data)
{
  GtkWidget *active_item;

  active_item = gtk_menu_get_active (GTK_MENU (menu_shell));
  pref.stack_level = g_list_index (menu_shell->children, active_item);

  delete_gui_protocols ();

}				/* on_stack_level_menu_selected */

void
on_diagram_only_toggle_toggled (GtkToggleButton * togglebutton,
				gpointer user_data)
{

  pref.diagram_only = gtk_toggle_button_get_active (togglebutton);
  need_reposition = TRUE;

}				/* on_diagram_only_toggle_toggled */

void
on_group_unk_check_toggled (GtkToggleButton * togglebutton,
			    gpointer user_data)
{
  enum status_t old_status = status;

  if ((status == PLAY) || (status == PAUSE))
    gui_stop_capture ();

  pref.group_unk = gtk_toggle_button_get_active (togglebutton);

  if (old_status == PLAY)
    gui_start_capture ();

}				/* on_group_unk_check_toggled */

void
on_aa_check_toggled (GtkToggleButton * togglebutton, gpointer user_data)
{
  enum status_t old_status = status;

  if ((status == PLAY) || (status == PAUSE))
    gui_stop_capture ();

  pref.antialias = gtk_toggle_button_get_active (togglebutton);

  if (old_status == PLAY)
    gui_start_capture ();
}				/* on_group_unk_check_toggled */

/*
 * TODO
 * I have to change the whole preferences workings, so that OK, apply and 
 * cancel have all the proper semantics
 */
void
on_ok_pref_button_clicked (GtkButton * button, gpointer user_data)
{
  on_apply_pref_button_clicked (button, NULL);
  gtk_widget_hide (diag_pref);

}				/* on_ok_pref_button_clicked */

void
on_apply_pref_button_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *widget = NULL;

  widget = glade_xml_get_widget (xml, "filter_entry");
  on_filter_entry_changed (GTK_EDITABLE (widget), NULL);
  widget = glade_xml_get_widget (xml, "");

  /* add proto name to history */
  gnome_entry_append_history (GNOME_ENTRY
			      (glade_xml_get_widget
			       (xml, "filter_gnome_entry")), FALSE,
			      pref.filter);

  if (colors_changed)
    {
      color_list_to_pref ();
      delete_gui_protocols ();
      colors_changed = FALSE;
    }
}				/* on_apply_pref_button_clicked */

void
on_cancel_pref_button_clicked (GtkButton * button, gpointer user_data)
{
  gtk_widget_hide (diag_pref);

}				/* on_cancel_pref_button_clicked */

void
on_save_pref_button_clicked (GtkButton * button, gpointer user_data)
{
  on_apply_pref_button_clicked (button, user_data);	/* to save we simulate apply */
  save_config ("/Etherape/");
}				/* on_save_pref_button_clicked */


/* Makes a new filter */
void
on_filter_entry_changed (GtkEditable * editable, gpointer user_data)
{
  gchar *str;
  /* TODO should make sure that for each mode the filter is set up
   * correctly */
  str = gtk_editable_get_chars (editable, 0, -1);
  if (pref.filter)
    g_free (pref.filter);
  pref.filter = g_strdup (str);
  g_free (str);
  /* TODO We should look at the error code from set_filter and pop
   * up a window accordingly */
  set_filter (pref.filter, NULL);
}				/* on_filter_entry_changed */

void
on_fade_toggle_toggled (GtkToggleButton * togglebutton, gpointer user_data)
{
  pref.nofade = !gtk_toggle_button_get_active (togglebutton);
}				/* on_fade_toggle_toggled */

void
on_cycle_toggle_toggled (GtkToggleButton * togglebutton, gpointer user_data)
{
  pref.cycle = gtk_toggle_button_get_active (togglebutton);
  colors_changed = TRUE;
}				/* on_cycle_toggle_toggled */


void
on_numeric_toggle_toggled (GtkToggleButton * togglebutton, gpointer user_data)
{
  pref.name_res = gtk_toggle_button_get_active (togglebutton);
}				/* on_numeric_toggle_toggled */

void
on_new_infodlg_check_toggled (GtkToggleButton * togglebutton, gpointer user_data)
{
  pref.new_infodlg = gtk_toggle_button_get_active (togglebutton);
}

/* ----------------------------------------------------------

   Color-proto preferences handling

   ---------------------------------------------------------- */

/* helper struct used to move around trees data ... */
typedef struct _EATreePos
{
  GtkTreeView *gv;
  GtkListStore *gs;
} EATreePos;


/* fill ep with the listore for the color treeview, creating it if necessary
   Returns FALSE if something goes wrong
*/
static gboolean
get_color_store (EATreePos * ep)
{
  /* initializes the view ptr */
  ep->gs = NULL;
  ep->gv = GTK_TREE_VIEW (glade_xml_get_widget (xml, "color_list"));
  if (!ep->gv)
    return FALSE;		/* error */

  /* retrieve the model (store) */
  ep->gs = GTK_LIST_STORE (gtk_tree_view_get_model (ep->gv));
  if (ep->gs)
    return TRUE;		/* model already initialized, finished */

  /* store not found, must be created  - it uses 3 values: 
     First the color string, then the gdk color, lastly the protocol */
  ep->gs =
    gtk_list_store_new (3, G_TYPE_STRING, GDK_TYPE_COLOR, G_TYPE_STRING);
  gtk_tree_view_set_model (ep->gv, GTK_TREE_MODEL (ep->gs));

  /* the view columns and cell renderers must be also created ... 
     Note: the bkg color is linked to the second column of store
   */
  gtk_tree_view_append_column (ep->gv,
			       gtk_tree_view_column_new_with_attributes
			       ("Color", gtk_cell_renderer_text_new (),
				"text", 0, "background-gdk", 1, NULL));
  gtk_tree_view_append_column (ep->gv,
			       gtk_tree_view_column_new_with_attributes
			       ("Protocol", gtk_cell_renderer_text_new (),
				"text", 2, NULL));

  return TRUE;
}


void
on_color_add_button_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *colorseldiag =
    glade_xml_get_widget (xml, "colorselectiondialog");
  gtk_widget_show (colorseldiag);
}				/* on_color_add_button_clicked */

void
on_color_remove_button_clicked (GtkButton * button, gpointer user_data)
{
  GtkTreePath *gpath = NULL;
  GtkTreeViewColumn *gcol = NULL;
  GtkTreeIter it;
  EATreePos ep;
  if (!get_color_store (&ep))
    return;

  /* gets the row (path) at cursor */
  gtk_tree_view_get_cursor (ep.gv, &gpath, &gcol);
  if (!gpath)
    return;			/* no row selected */

  /* get iterator from path  and removes from store */
  if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (ep.gs), &it, gpath))
    return;			/* path not found */

#if GTK_CHECK_VERSION(2,2,0)
  if (gtk_list_store_remove (ep.gs, &it))
    {
      /* iterator still valid, selects current pos */
      gpath = gtk_tree_model_get_path (GTK_TREE_MODEL (ep.gs), &it);
      gtk_tree_view_set_cursor (ep.gv, gpath, NULL, 0);
      gtk_tree_path_free (gpath);
    }
#else
  /* gtk < 2.2 had gtk_list_store_remove void */
  gtk_list_store_remove (ep.gs, &it);
#endif

  colors_changed = TRUE;
}				/* on_color_remove_button_clicked */

void
on_colordiag_ok_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *colorsel, *colorseldiag;
  GdkColor gdk_color;
  gchar tmp[64];
  GtkTreePath *gpath = NULL;
  GtkTreeViewColumn *gcol = NULL;
  GtkTreeIter it;
  EATreePos ep;
  if (!get_color_store (&ep))
    return;

  /* gets the row (path) at cursor */
  gtk_tree_view_get_cursor (ep.gv, &gpath, &gcol);
  if (gpath)
    {
      /* row sel, add before */
      GtkTreeIter itsibling;
      if (!gtk_tree_model_get_iter
	  (GTK_TREE_MODEL (ep.gs), &itsibling, gpath))
	return;			/* path not found */
      gtk_list_store_insert_before (ep.gs, &it, &itsibling);
    }
  else
    gtk_list_store_append (ep.gs, &it);	/* no row selected, append */

  /* get the selected color */
  colorseldiag = glade_xml_get_widget (xml, "colorselectiondialog");
  colorsel = GTK_COLOR_SELECTION_DIALOG (colorseldiag)->colorsel;
  gtk_color_selection_get_current_color (GTK_COLOR_SELECTION (colorsel),
					 &gdk_color);

  /* Since we are only going to save 24bit precision, we might as well
   * make sure we don't display any more than that */
  gdk_color.red = (gdk_color.red >> 8) << 8;
  gdk_color.green = (gdk_color.green >> 8) << 8;
  gdk_color.blue = (gdk_color.blue >> 8) << 8;

  /* converting color */
  g_snprintf (tmp, sizeof (tmp), "#%02x%02x%02x", gdk_color.red >> 8,
	      gdk_color.green >> 8, gdk_color.blue >> 8);

  /* fill data */
  gtk_list_store_set (ep.gs, &it, 0, tmp, 1, &gdk_color, 2, "", -1);

  gtk_widget_hide (colorseldiag);

  colors_changed = TRUE;
}				/* on_colordiag_ok_clicked */



void
on_protocol_edit_button_clicked (GtkButton * button, gpointer user_data)
{
  GtkWidget *protocol_edit_dialog = NULL;
  protocol_edit_dialog = glade_xml_get_widget (xml, "protocol_edit_dialog");
  gtk_widget_show (protocol_edit_dialog);
}				/* on_protocol_edit_button_clicked */

void
on_protocol_edit_dialog_show (GtkWidget * wdg, gpointer user_data)
{
  gchar *protocol_string;
  GtkTreePath *gpath = NULL;
  GtkTreeViewColumn *gcol = NULL;
  GtkTreeIter it;
  GtkWidget *protocol_entry = NULL;
  int pos = 0;
  EATreePos ep;
  if (!get_color_store (&ep))
    return;

  /* gets the row (path) at cursor */
  gtk_tree_view_get_cursor (ep.gv, &gpath, &gcol);
  if (!gpath)
    return;			/* no row selected */

  if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (ep.gs), &it, gpath))
    return;			/* path not found */

  gtk_tree_model_get (GTK_TREE_MODEL (ep.gs), &it, 2, &protocol_string, -1);

  protocol_entry = glade_xml_get_widget (xml, "protocol_entry");

  gtk_editable_delete_text (GTK_EDITABLE (protocol_entry), 0, -1);
  gtk_editable_insert_text (GTK_EDITABLE (protocol_entry), protocol_string,
			    strlen (protocol_string), &pos);

  g_free (protocol_string);
}


void
on_protocol_edit_ok_clicked (GtkButton * button, gpointer user_data)
{
  gchar *proto_string;
  GtkTreePath *gpath = NULL;
  GtkTreeViewColumn *gcol = NULL;
  GtkTreeIter it;
  GtkWidget *protocol_entry = NULL;
  EATreePos ep;
  if (!get_color_store (&ep))
    return;

  /* gets the row (path) at cursor */
  gtk_tree_view_get_cursor (ep.gv, &gpath, &gcol);
  if (!gpath)
    return;			/* no row selected */

  if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (ep.gs), &it, gpath))
    return;			/* path not found */


  protocol_entry = glade_xml_get_widget (xml, "protocol_entry");
  proto_string =
    gtk_editable_get_chars (GTK_EDITABLE (protocol_entry), 0, -1);
  proto_string = g_utf8_strup (proto_string, -1);

  gtk_list_store_set (ep.gs, &it, 2, proto_string, -1);

  /* add proto name to history */
  gnome_entry_append_history (GNOME_ENTRY
			      (glade_xml_get_widget
			       (xml, "protocol_gnome_entry")), FALSE,
			      proto_string);

  g_free (proto_string);

  colors_changed = TRUE;
  gtk_widget_hide (glade_xml_get_widget (xml, "protocol_edit_dialog"));
}				/* on_protocol_edit_ok_clicked */



static void
load_color_list (void)
{
  gint i;
  EATreePos ep;
  if (!get_color_store (&ep))
    return;

  /* clear list */
  gtk_list_store_clear (ep.gs);

  for (i = 0; i < pref.n_colors; i++)
    {
      GdkColor gdk_color;
      gchar **colors_protocols = NULL;
      gchar *protocol = NULL;
      gchar tmp[64];
      GtkTreeIter it;

      colors_protocols = g_strsplit (pref.colors[i], ";", 0);

      /* converting color */
      gdk_color_parse (colors_protocols[0], &gdk_color);
      g_snprintf (tmp, sizeof (tmp), "#%02x%02x%02x", gdk_color.red >> 8,
		  gdk_color.green >> 8, gdk_color.blue >> 8);

      /* converting proto name */
      if (!colors_protocols[1])
	protocol = "";
      else
	protocol = colors_protocols[1];

      /* adds a new row */
      gtk_list_store_append (ep.gs, &it);
      gtk_list_store_set (ep.gs, &it, 0, tmp, 1, &gdk_color, 2, protocol, -1);
    }
}

/* Called whenever preferences are applied or OKed. Copied whatever there is
 * in the color table to the color preferences in memory */
static void
color_list_to_pref (void)
{
  gint i;
  GtkTreeIter it;

  EATreePos ep;
  if (!get_color_store (&ep))
    return;

  while (pref.colors && pref.n_colors)
    {
      g_free (pref.colors[pref.n_colors - 1]);
      pref.n_colors--;
    }
  g_free (pref.colors);
  pref.colors = NULL;

  pref.n_colors =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (ep.gs), NULL);
  pref.colors = g_malloc (sizeof (gchar *) * pref.n_colors);

  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (ep.gs), &it);
  for (i = 0; i < pref.n_colors; i++)
    {
      gchar *color, *protocol;

      /* reads the list */
      gtk_tree_model_get (GTK_TREE_MODEL (ep.gs), &it,
                          0, &color, 2, &protocol, -1);

      if (strcmp ("", protocol))
	pref.colors[i] = g_strdup_printf ("%s;%s", color, protocol);
      else
	pref.colors[i] = g_strdup (color);

      g_free (color);
      g_free (protocol);

      gtk_tree_model_iter_next (GTK_TREE_MODEL (ep.gs), &it);
    }

  protohash_read_prefvect(pref.colors, pref.n_colors);
}

static gint
version_compare (const gchar * a, const gchar * b)
{
  guint a_mj, a_mi, a_pl, b_mj, b_mi, b_pl;

  g_assert (a != NULL);
  g_assert (b != NULL);

  /* TODO What should we return if there was a problem? */
  g_return_val_if_fail ((get_version_levels (a, &a_mj, &a_mi, &a_pl)
			 && get_version_levels (b, &b_mj, &b_mi, &b_pl)), 0);
  if (a_mj < b_mj)
    return -1;
  else if (a_mj > b_mj)
    return 1;
  else if (a_mi < b_mi)
    return -1;
  else if (a_mi > b_mi)
    return 1;
  else if (a_pl < b_pl)
    return -1;
  else if (a_pl > b_pl)
    return 1;
  else
    return 0;
}

static gboolean
get_version_levels (const gchar * version_string,
		    guint * major, guint * minor, guint * patch)
{
  gchar **tokens;

  g_assert (version_string != NULL);

  tokens = g_strsplit (version_string, ".", 0);
  g_return_val_if_fail ((tokens
			 && tokens[0] && tokens[1] && tokens[2]
			 && sscanf (tokens[0], "%d", major)
			 && sscanf (tokens[1], "%d", minor)
			 && sscanf (tokens[2], "%d", patch)), FALSE);
  g_strfreev (tokens);
  return TRUE;
}
