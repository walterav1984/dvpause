#include <gtk/gtk.h>
#include <libsoup/soup.h>
#include <assert.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "dv.h"
#include "global.h"
#include "config.h"

// Globals
struct _DVControl DVControl;

// Widgets
GtkWidget *framelabel;
GtkWidget *playbutton;
GtkWidget *frameentry;
GtkWidget *playbuttonimage;
GObject   *frameadjustment;

/* Signal handlers */
void destroy (GtkWidget *widget, gpointer data)
{
	playerstate = PLAYER_STATE_TERMINATE;
	while(playerstate != PLAYER_STATE_TERMINATED);
	gtk_main_quit();
}

int update_framedisp()
{
	if (!GTK_IS_LABEL(framelabel)) return FALSE; // program closed

	// Label
	char labeltext[100];
	strcpy(labeltext, long2string(DVControl.currentframe + 1));
	strcat(labeltext, " / ");
	strcat(labeltext, long2string(DVControl.totalframes));
	gtk_label_set_text(GTK_LABEL(framelabel), labeltext);

	// Slider
	if (DVControl.totalframes != 0 && playerstate != PLAYER_STATE_NEWPOS_PERCENT)
		DVControl.pos_percent = DVControl.currentframe * 100. / (DVControl.totalframes - 1);

	gtk_adjustment_set_value(GTK_ADJUSTMENT(frameadjustment), DVControl.pos_percent);

	return TRUE;
}

gboolean button_play_toggle(GtkWidget *widget)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))) // State before switching
	{
		gtk_image_set_from_icon_name (GTK_IMAGE(playbuttonimage),
			"media-playback-start", GTK_ICON_SIZE_DIALOG);
		playerstate = PLAYER_STATE_PAUSE;
	}
	else
	{
		playerstate = PLAYER_STATE_CONTINUE;
		gtk_image_set_from_icon_name (GTK_IMAGE(playbuttonimage),
			"media-playback-pause", GTK_ICON_SIZE_DIALOG);
	}

	return FALSE;
}

void add_movie_to_list(GtkWidget *movlist, const gchar *name)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(movlist)));

	gtk_list_store_append(store, &iter); // get iter at end
	gtk_list_store_set(store, &iter, 0, name, -1);
}

// Change filename if different List item (=movie) is selected
void on_movlist_selection_changed(GtkWidget *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *value;

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter))
	{
		gtk_tree_model_get(model, &iter, 0, &value, -1);
		static char *path;
		path = movie_get_path_by_name(value);

		DVControl.filename = path;
		playerstate = PLAYER_STATE_PAUSE_NEWSOURCE;
		// Start into the movie with a pause, wait for button press

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(playbutton), FALSE);
		gtk_image_set_from_icon_name (GTK_IMAGE(playbuttonimage),
			"media-playback-start", GTK_ICON_SIZE_DIALOG);
	}
}

void setframe_activated(GtkWidget *widget)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(playbutton), FALSE);
	gtk_image_set_from_icon_name (GTK_IMAGE(playbuttonimage),
		"media-playback-start", GTK_ICON_SIZE_DIALOG);

	char *frametext = (char *)gtk_entry_get_text(GTK_ENTRY(frameentry));
	long frame = string2long(frametext);
	if (frame < 0) frame = 0;

	DVControl.newpos = (frame - 1)*BYTES_PER_FRAME;
	playerstate = PLAYER_STATE_NEWPOS;
}

void on_frameadjustment_changed(GObject *frameadj)
{
	double sliderpos = gtk_adjustment_get_value(GTK_ADJUSTMENT(frameadj));

	// if slider was manually moved (not by playing the video)
	if (sliderpos != DVControl.pos_percent)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(playbutton), FALSE);
		gtk_image_set_from_icon_name (GTK_IMAGE(playbuttonimage),
			"media-playback-start", GTK_ICON_SIZE_DIALOG);

		DVControl.pos_percent = sliderpos;
		playerstate = PLAYER_STATE_NEWPOS_PERCENT;
	}
}

/* Initializer */
static GtkWidget *create_window (void)
{
	GtkWidget *window;
	GtkWidget *movlist;
	GError* error = NULL;

	/* Load UI from file */
	GtkBuilder *builder;
	{
		builder = gtk_builder_new ();
		if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
		{
			g_critical ("Couldn't load builder file: %s", error->message);
			g_error_free (error);
		}

		/* Auto-connect signal handlers */
		gtk_builder_connect_signals (builder, NULL);

		/* Get the window object from the ui file */
		window		= GTK_WIDGET (gtk_builder_get_object (builder, "window"		));
		movlist		= GTK_WIDGET (gtk_builder_get_object (builder, "movlist"	));
		framelabel	= GTK_WIDGET (gtk_builder_get_object (builder, "framelabel"	));
		playbutton	= GTK_WIDGET (gtk_builder_get_object (builder, "playbutton"	));
		frameentry	= GTK_WIDGET (gtk_builder_get_object (builder, "frameentry"	));
		playbuttonimage	= GTK_WIDGET (gtk_builder_get_object (builder, "playbuttonimage"));

		frameadjustment	= G_OBJECT(gtk_builder_get_object (builder, "frameadjustment"));

		gtk_widget_set_visible(movlist, TRUE);

		/* Load Movies */
		{
			int mnum = 0;
			char **movienames = get_movie_names();
			while(movienames[mnum]) add_movie_to_list(movlist, movienames[mnum++]);
		}
	}

	g_object_unref (builder);


	return window;
}

static void get_countdown(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *client, gpointer user_data)
{
	if (msg->method != SOUP_METHOD_GET)
	{
		soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}


	soup_message_set_status(msg, SOUP_STATUS_OK);
	
	char seconds_remaining[50];
	sprintf(seconds_remaining, "%.1f", (float)(DVControl.totalframes - DVControl.currentframe) / FPS);
	soup_message_set_response(msg, "text/html", SOUP_MEMORY_COPY, seconds_remaining, strlen(seconds_remaining));
}

static void get_start_playback(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *client, gpointer user_data)
{
	if (msg->method != SOUP_METHOD_GET)
	{
		soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}


	soup_message_set_status(msg, SOUP_STATUS_OK);
	playerstate = PLAYER_STATE_CONTINUE;
	gtk_image_set_from_icon_name (GTK_IMAGE(playbuttonimage), "media-playback-pause", GTK_ICON_SIZE_DIALOG);
	soup_message_set_response(msg, "text/html", SOUP_MEMORY_COPY, "ok", 3);
}

void init_httpsrv()
{
	SoupServer *ssrv;

	ssrv = soup_server_new(SOUP_SERVER_SERVER_HEADER, "dvpause", NULL);
	GError *error = NULL;
	soup_server_listen_all(ssrv, 8080, 0, &error);

	soup_server_add_handler(ssrv, "/countdown", get_countdown, NULL, NULL);
	soup_server_add_handler(ssrv, "/play", get_start_playback, NULL, NULL);
}

int main (int argc, char *argv[])
{
	// Read config
	if (!readmovies())
	{
		fprintf(stderr, "ERROR: Config File not found!\n");
		exit(EXIT_FAILURE);
	}

	// Start application
 	GtkWidget *window;

	gtk_init (&argc, &argv);

	window = create_window ();
	gtk_widget_show (window);
	init_dvsystem();
	g_timeout_add(1000/25, update_framedisp, NULL);

	// Start HTTP server
	init_httpsrv();

	gtk_main();

	exit(EXIT_SUCCESS);
}
