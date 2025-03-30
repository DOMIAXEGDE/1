/**
 * QuantMatrix Encoder/Decoder Suite - C Implementation
 * A GUI application for encoding and decoding source code files
 * using character mapping
 * 
 * Compile with:
 * gcc -o quantmatrix main.c $(pkg-config --cflags --libs gtk+-3.0) -lm
 * 
 * Dependencies: GTK+ 3, GLib
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

#define MAX_CHAR_MAP 256
#define MAX_FILENAME 256
#define MAX_LINE_LENGTH 1024
#define MAX_TEXT_LENGTH 8192

// Define color scheme
typedef struct {
    const char* bg_dark;
    const char* bg_main;
    const char* bg_light;
    const char* accent_blue;
    const char* accent_purple;
    const char* accent_green;
    const char* accent_red;
    const char* text_bright;
    const char* text_normal;
    const char* text_dim;
    const char* border;
    const char* highlight;
    const char* digit_color;
    const char* keyword_color;
    const char* grid_line;
} ColorScheme;

// Structure for the application
typedef struct {
    // Main window and widgets
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *status_bar;
    GtkWidget *progress_bar;
    
    // Character map tab widgets
    GtkWidget *charmap_file_entry;
    GtkWidget *charmap_display;
    
    // Encode tab widgets
    GtkWidget *encode_input_entry;
    GtkWidget *encode_output_entry;
    GtkWidget *encode_input_preview;
    GtkWidget *encode_output_preview;
    
    // Decode tab widgets
    GtkWidget *decode_input_entry;
    GtkWidget *decode_output_entry;
    GtkWidget *decode_input_preview;
    GtkWidget *decode_output_preview;
    
    // Character mapping data
    char *char_map[MAX_CHAR_MAP];
    int char_map_size;
    bool is_map_loaded;
    
    // Color scheme
    ColorScheme colors;
    
    // CSS provider for styling
    GtkCssProvider *provider;
} AppData;

// Function prototypes
static void setup_window(AppData *app);
static void apply_style(AppData *app);
static void create_ui(AppData *app);
static void setup_charmap_tab(AppData *app, GtkWidget *tab);
static void setup_encode_tab(AppData *app, GtkWidget *tab);
static void setup_decode_tab(AppData *app, GtkWidget *tab);
static void browse_charmap_file(GtkWidget *widget, AppData *app);
static void load_char_map(GtkWidget *widget, AppData *app);
static void browse_encode_input(GtkWidget *widget, AppData *app);
static void browse_decode_input(GtkWidget *widget, AppData *app);
static void encode_file(GtkWidget *widget, AppData *app);
static void decode_file(GtkWidget *widget, AppData *app);
static int find_char_index(AppData *app, char c);
static void highlight_c_syntax(AppData *app, GtkTextBuffer *buffer);
static void apply_color_tags_to_charmap(AppData *app);
static void set_status_message(AppData *app, const char *message);
static void set_progress_value(AppData *app, double progress);
static gboolean load_file_content(const char *filename, char *buffer, size_t buffer_size);
static void show_message_dialog(GtkWindow *parent, const char *message, GtkMessageType type);
static GtkWidget* create_matrix_header(AppData *app);
static GtkWidget* create_styled_button(const char *label, GCallback callback, AppData *app);
static GtkWidget* create_section_label(const char *text, const char *color, AppData *app);

// Main function
int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    // Create application data
    AppData app;
    memset(&app, 0, sizeof(AppData));
    
    // Initialize character mapping
    app.char_map_size = 0;
    app.is_map_loaded = false;
    
    // Set up the color scheme
    app.colors.bg_dark = "#121218";
    app.colors.bg_main = "#1a1a24";
    app.colors.bg_light = "#222233";
    app.colors.accent_blue = "#0088cc";
    app.colors.accent_purple = "#6a5acd";
    app.colors.accent_green = "#00cc88";
    app.colors.accent_red = "#ff3366";
    app.colors.text_bright = "#ffffff";
    app.colors.text_normal = "#ccccdd";
    app.colors.text_dim = "#9999aa";
    app.colors.border = "#333344";
    app.colors.highlight = "#3355bb";
    app.colors.digit_color = "#33ccff";
    app.colors.keyword_color = "#ff6688";
    app.colors.grid_line = "#333344";
    
    // Set up the main window and UI
    setup_window(&app);
    apply_style(&app);
    create_ui(&app);
    
    // Run the GTK main loop
    gtk_widget_show_all(app.window);
    gtk_main();
    
    // Cleanup
    for (int i = 0; i < app.char_map_size; i++) {
        free(app.char_map[i]);
    }
    
    return 0;
}

// Set up the main window
static void setup_window(AppData *app) {
    // Create the main window
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "QuantMatrix Encoder/Decoder Suite");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 1100, 750);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}

// Apply custom styling
static void apply_style(AppData *app) {
    // Create CSS provider
    app->provider = gtk_css_provider_new();
    
    // Define CSS styles
    const char *css = 
        "window { background-color: #121218; }\n"
        "notebook { background-color: #1a1a24; }\n"
        "notebook tab { background-color: #222233; color: #ccccdd; padding: 8px 15px; font-weight: bold; }\n"
        "notebook tab:checked { background-color: #0088cc; color: white; }\n"
        ".accent-blue { color: #0088cc; }\n"
        ".accent-purple { color: #6a5acd; }\n"
        ".accent-green { color: #00cc88; }\n"
        ".accent-red { color: #ff3366; }\n"
        ".bg-dark { background-color: #121218; }\n"
        ".bg-main { background-color: #1a1a24; }\n"
        ".bg-light { background-color: #222233; }\n"
        ".txt-bright { color: #ffffff; }\n"
        ".txt-normal { color: #ccccdd; }\n"
        ".txt-dim { color: #9999aa; }\n"
        "entry { background-color: #222233; color: #ffffff; border: 1px solid #333344; }\n"
        "button { background-color: #0088cc; color: white; border: none; padding: 8px 12px; font-weight: bold; }\n"
        "button:hover { background-color: #3355bb; }\n"
        "textview { background-color: #222233; color: #ffffff; font-family: 'Consolas', monospace; }\n"
        "frame { border: 1px solid #333344; padding: 5px; }\n"
        "frame > label { color: #0088cc; font-weight: bold; }\n"
        "progressbar { min-height: 10px; }\n"
        "progressbar trough { background-color: #222233; border: 1px solid #333344; }\n"
        "progressbar progress { background-color: #00cc88; }\n"
        ".status-bar { background-color: #121218; color: #00cc88; padding: 5px; }\n"
        ".header-title { color: #0088cc; font-size: 18px; font-weight: bold; }\n"
        ".math-symbol { color: #6a5acd; font-size: 16px; }\n"
        ".section-header { font-weight: bold; color: #0088cc; }\n"
        ".binary-label { color: #9999aa; font-family: 'Consolas', monospace; font-size: 8px; }\n"
        ".corner-accent { background-color: #0088cc; }\n";
    
    gtk_css_provider_load_from_data(app->provider, css, -1, NULL);
    
    // Apply the CSS provider to the application
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(app->provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

// Create the UI
static void create_ui(AppData *app) {
    // Create main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), main_box);
    
    // Add decorative matrix header
    GtkWidget *header = create_matrix_header(app);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);
    
    // Create notebook (tabs)
    app->notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), app->notebook, TRUE, TRUE, 0);
    
    // Create tabs
    GtkWidget *charmap_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *encode_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *decode_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    // Add padding to tabs
    gtk_widget_set_margin_start(charmap_tab, 10);
    gtk_widget_set_margin_end(charmap_tab, 10);
    gtk_widget_set_margin_top(charmap_tab, 10);
    gtk_widget_set_margin_bottom(charmap_tab, 10);
    
    gtk_widget_set_margin_start(encode_tab, 10);
    gtk_widget_set_margin_end(encode_tab, 10);
    gtk_widget_set_margin_top(encode_tab, 10);
    gtk_widget_set_margin_bottom(encode_tab, 10);
    
    gtk_widget_set_margin_start(decode_tab, 10);
    gtk_widget_set_margin_end(decode_tab, 10);
    gtk_widget_set_margin_top(decode_tab, 10);
    gtk_widget_set_margin_bottom(decode_tab, 10);
    
    // Set up tab content
    setup_charmap_tab(app, charmap_tab);
    setup_encode_tab(app, encode_tab);
    setup_decode_tab(app, decode_tab);
    
    // Add tabs to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), charmap_tab, gtk_label_new("CHARACTER MAP"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), encode_tab, gtk_label_new("ENCODE"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), decode_tab, gtk_label_new("DECODE"));
    
    // Create footer
    GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_style_context_add_class(gtk_widget_get_style_context(footer), "bg-dark");
    gtk_widget_set_size_request(footer, -1, 30);
    
    // Status bar
    app->status_bar = gtk_label_new("SYSTEM READY • AWAITING OPERATION");
    gtk_style_context_add_class(gtk_widget_get_style_context(app->status_bar), "accent-green");
    gtk_label_set_xalign(GTK_LABEL(app->status_bar), 0.0);
    gtk_box_pack_start(GTK_BOX(footer), app->status_bar, TRUE, TRUE, 10);
    
    // Progress bar
    app->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 1.0);
    gtk_widget_set_size_request(app->progress_bar, 200, -1);
    gtk_box_pack_end(GTK_BOX(footer), app->progress_bar, FALSE, FALSE, 10);
    
    gtk_box_pack_end(GTK_BOX(main_box), footer, FALSE, FALSE, 0);
}

// Create matrix-style header
static GtkWidget* create_matrix_header(AppData *app) {
    // Create header container
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(header, -1, 60);
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "bg-dark");
    
    // Create overlay for grid lines and content
    GtkWidget *overlay = gtk_overlay_new();
    gtk_box_pack_start(GTK_BOX(header), overlay, TRUE, TRUE, 0);
    
    // Create fixed container for grid lines
    GtkWidget *grid_container = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(overlay), grid_container);
    
    // Add vertical grid lines
    for (int i = 0; i < 20; i++) {
        GtkWidget *vline = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_size_request(vline, 1, 60);
        gtk_fixed_put(GTK_FIXED(grid_container), vline, i * 55, 0);
    }
    
    // Add horizontal grid lines
    for (int i = 0; i < 3; i++) {
        GtkWidget *hline = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_set_size_request(hline, 1100, 1);
        gtk_fixed_put(GTK_FIXED(grid_container), hline, 0, i * 20);
    }
    
    // Create content box for centered content
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(content_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(content_box, GTK_ALIGN_CENTER);
    
    // Title label
    GtkWidget *title = gtk_label_new("QUANTMATRIX ENCODER/DECODER");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "header-title");
    gtk_box_pack_start(GTK_BOX(content_box), title, TRUE, TRUE, 0);
    
    // Math symbols container
    GtkWidget *symbols_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(symbols_box, GTK_ALIGN_CENTER);
    
    // Add math symbols
    const char *symbols[] = {"∑", "∫", "∂", "√", "π", "Δ", "Ω"};
    for (int i = 0; i < 7; i++) {
        GtkWidget *symbol = gtk_label_new(symbols[i]);
        gtk_style_context_add_class(gtk_widget_get_style_context(symbol), "math-symbol");
        gtk_box_pack_start(GTK_BOX(symbols_box), symbol, FALSE, FALSE, 20);
    }
    
    gtk_box_pack_start(GTK_BOX(content_box), symbols_box, TRUE, TRUE, 0);
    
    // Add the content box to the overlay
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), content_box);
    
    return header;
}

// Set up the Character Map tab
static void setup_charmap_tab(AppData *app, GtkWidget *tab) {
    // Create decorative binary labels
    GtkWidget *binary_label1 = gtk_label_new("01001010110100101010010101");
    gtk_style_context_add_class(gtk_widget_get_style_context(binary_label1), "binary-label");
    gtk_widget_set_halign(binary_label1, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(tab), binary_label1, FALSE, FALSE, 0);
    
    // Input frame for file selection
    GtkWidget *input_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(input_frame), GTK_SHADOW_ETCHED_IN);
    
    // Create input box
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(input_box, 10);
    gtk_widget_set_margin_end(input_box, 10);
    gtk_widget_set_margin_top(input_box, 10);
    gtk_widget_set_margin_bottom(input_box, 10);
    gtk_container_add(GTK_CONTAINER(input_frame), input_box);
    
    // Add corner accent (simulating the decorative corner)
    GtkWidget *corner = gtk_label_new("■");
    gtk_style_context_add_class(gtk_widget_get_style_context(corner), "accent-blue");
    gtk_box_pack_start(GTK_BOX(input_box), corner, FALSE, FALSE, 0);
    gtk_widget_set_margin_end(corner, 20);
    
    // File number input
    GtkWidget *label = gtk_label_new("CHARACTER MAP FILE NUMBER:");
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "txt-normal");
    gtk_box_pack_start(GTK_BOX(input_box), label, FALSE, FALSE, 0);
    
    app->charmap_file_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(app->charmap_file_entry), 20);
    gtk_entry_set_width_chars(GTK_ENTRY(app->charmap_file_entry), 10);
    gtk_box_pack_start(GTK_BOX(input_box), app->charmap_file_entry, FALSE, FALSE, 0);
    
    // Browse button
    GtkWidget *browse_btn = create_styled_button("BROWSE FILES", G_CALLBACK(browse_charmap_file), app);
    gtk_box_pack_start(GTK_BOX(input_box), browse_btn, FALSE, FALSE, 10);
    
    // Load button
    GtkWidget *load_btn = create_styled_button("LOAD MAP", G_CALLBACK(load_char_map), app);
    gtk_box_pack_start(GTK_BOX(input_box), load_btn, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(tab), input_frame, FALSE, FALSE, 10);
    
    // Display frame for character map
    GtkWidget *display_frame = gtk_frame_new("CHARACTER MATRIX VISUALIZATION");
    gtk_frame_set_shadow_type(GTK_FRAME(display_frame), GTK_SHADOW_ETCHED_IN);
    
    // Create overlay for decorative elements and content
    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(display_frame), overlay);
    
    // Create a fixed container for grid lines
    GtkWidget *grid_container = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(overlay), grid_container);
    
    // Add decorative vertical lines
    for (int i = 0; i < 5; i++) {
        GtkWidget *vline = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_size_request(vline, 1, 400);
        gtk_fixed_put(GTK_FIXED(grid_container), vline, (i + 1) * 200, 0);
    }
    
    // Create scrolled window for text display
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), 
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);
    
    // Create text view for character map display
    app->charmap_display = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->charmap_display), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->charmap_display), GTK_WRAP_WORD);
    
    // Add placeholder text
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->charmap_display));
    gtk_text_buffer_set_text(buffer, "< Character mapping will be displayed here >\nLoad a character map file to begin...", -1);
    
    // Add text tags for syntax highlighting
    gtk_text_buffer_create_tag(buffer, "blue", "foreground", app->colors.accent_blue, NULL);
    gtk_text_buffer_create_tag(buffer, "green", "foreground", app->colors.accent_green, NULL);
    gtk_text_buffer_create_tag(buffer, "red", "foreground", app->colors.accent_red, NULL);
    gtk_text_buffer_create_tag(buffer, "purple", "foreground", app->colors.accent_purple, NULL);
    
    gtk_container_add(GTK_CONTAINER(scroll), app->charmap_display);
    
    // Add the scroll window to the overlay
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), scroll);
    
    gtk_box_pack_start(GTK_BOX(tab), display_frame, TRUE, TRUE, 10);
}

// Set up the Encode tab
static void setup_encode_tab(AppData *app, GtkWidget *tab) {
    // Create decorative elements
    GtkWidget *label1 = create_section_label("▶▶▶ ENCODING MATRIX ▶▶▶", app->colors.accent_green, app);
    gtk_widget_set_halign(label1, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(tab), label1, FALSE, FALSE, 0);
    
    GtkWidget *hex_label = gtk_label_new("0x01 0x02 0x03 0x04");
    gtk_style_context_add_class(gtk_widget_get_style_context(hex_label), "binary-label");
    gtk_widget_set_halign(hex_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(tab), hex_label, FALSE, FALSE, 0);
    
    // Input frame
    GtkWidget *input_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(input_frame), GTK_SHADOW_ETCHED_IN);
    
    // Create input box
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(input_box, 10);
    gtk_widget_set_margin_end(input_box, 10);
    gtk_widget_set_margin_top(input_box, 10);
    gtk_widget_set_margin_bottom(input_box, 10);
    gtk_container_add(GTK_CONTAINER(input_frame), input_box);
    
    // Source file input
    GtkWidget *source_label = gtk_label_new("SOURCE CODE FILE:");
    gtk_style_context_add_class(gtk_widget_get_style_context(source_label), "txt-normal");
    gtk_box_pack_start(GTK_BOX(input_box), source_label, FALSE, FALSE, 0);
    
    app->encode_input_entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(app->encode_input_entry), 30);
    gtk_box_pack_start(GTK_BOX(input_box), app->encode_input_entry, TRUE, TRUE, 0);
    
    GtkWidget *browse_btn = create_styled_button("BROWSE", G_CALLBACK(browse_encode_input), app);
    gtk_box_pack_start(GTK_BOX(input_box), browse_btn, FALSE, FALSE, 0);
    
    // Separator
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(input_box), separator, FALSE, FALSE, 10);
    
    // Output file number
    GtkWidget *output_label = gtk_label_new("OUTPUT FILE ID:");
    gtk_style_context_add_class(gtk_widget_get_style_context(output_label), "txt-normal");
    gtk_box_pack_start(GTK_BOX(input_box), output_label, FALSE, FALSE, 0);
    
    app->encode_output_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(app->encode_output_entry), 10);
    gtk_entry_set_width_chars(GTK_ENTRY(app->encode_output_entry), 10);
    gtk_box_pack_start(GTK_BOX(input_box), app->encode_output_entry, FALSE, FALSE, 0);
    
    // Encode button
    GtkWidget *encode_btn = create_styled_button("▶ ENCODE", G_CALLBACK(encode_file), app);
    gtk_box_pack_start(GTK_BOX(input_box), encode_btn, FALSE, FALSE, 10);
    
    gtk_box_pack_start(GTK_BOX(tab), input_frame, FALSE, FALSE, 10);
    
    // Preview frame
    GtkWidget *preview_frame = gtk_frame_new("CODE TRANSFORMATION MATRIX");
    gtk_frame_set_shadow_type(GTK_FRAME(preview_frame), GTK_SHADOW_ETCHED_IN);
    
    // Create a paned window for split view
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(preview_frame), paned);
    
    // Input preview
    GtkWidget *input_preview_frame = gtk_frame_new("SOURCE CODE");
    GtkWidget *input_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(input_scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    
    app->encode_input_preview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->encode_input_preview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->encode_input_preview), GTK_WRAP_WORD);
    
    // Create buffer and add tags for syntax highlighting
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->encode_input_preview));
    gtk_text_buffer_create_tag(input_buffer, "keyword", "foreground", app->colors.keyword_color, NULL);
    gtk_text_buffer_create_tag(input_buffer, "preprocessor", "foreground", app->colors.accent_purple, NULL);
    gtk_text_buffer_create_tag(input_buffer, "comment", "foreground", app->colors.text_dim, NULL);
    gtk_text_buffer_create_tag(input_buffer, "string", "foreground", app->colors.accent_green, NULL);
    gtk_text_buffer_create_tag(input_buffer, "number", "foreground", app->colors.digit_color, NULL);
    
    gtk_container_add(GTK_CONTAINER(input_scroll), app->encode_input_preview);
    gtk_container_add(GTK_CONTAINER(input_preview_frame), input_scroll);
    
    // Output preview
    GtkWidget *output_preview_frame = gtk_frame_new("ENCODED OUTPUT");
    GtkWidget *output_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    
    app->encode_output_preview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->encode_output_preview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->encode_output_preview), GTK_WRAP_WORD);
    
    // Set custom color for digit output
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->encode_output_preview));
    GtkTextTag *digit_tag = gtk_text_buffer_create_tag(output_buffer, "digit", 
                            "foreground", app->colors.digit_color, NULL);
    
    gtk_container_add(GTK_CONTAINER(output_scroll), app->encode_output_preview);
    gtk_container_add(GTK_CONTAINER(output_preview_frame), output_scroll);
    
    // Add the frames to the paned view
    gtk_paned_add1(GTK_PANED(paned), input_preview_frame);
    gtk_paned_add2(GTK_PANED(paned), output_preview_frame);
    gtk_paned_set_position(GTK_PANED(paned), 450);
    
    gtk_box_pack_start(GTK_BOX(tab), preview_frame, TRUE, TRUE, 10);
}

// Set up the Decode tab
static void setup_decode_tab(AppData *app, GtkWidget *tab) {
    // Create decorative elements
    GtkWidget *label1 = create_section_label("◀◀◀ DECODING MATRIX ◀◀◀", app->colors.accent_purple, app);
    gtk_widget_set_halign(label1, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(tab), label1, FALSE, FALSE, 0);
    
    GtkWidget *matrix_label = gtk_label_new("MATRIX TRANSLATION ACTIVE");
    gtk_style_context_add_class(gtk_widget_get_style_context(matrix_label), "binary-label");
    gtk_widget_set_halign(matrix_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(tab), matrix_label, FALSE, FALSE, 0);
    
    // Input frame
    GtkWidget *input_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(input_frame), GTK_SHADOW_ETCHED_IN);
    
    // Create input box
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(input_box, 10);
    gtk_widget_set_margin_end(input_box, 10);
    gtk_widget_set_margin_top(input_box, 10);
    gtk_widget_set_margin_bottom(input_box, 10);
    gtk_container_add(GTK_CONTAINER(input_frame), input_box);
    
    // Encoded file input
    GtkWidget *source_label = gtk_label_new("ENCODED FILE ID:");
    gtk_style_context_add_class(gtk_widget_get_style_context(source_label), "txt-normal");
    gtk_box_pack_start(GTK_BOX(input_box), source_label, FALSE, FALSE, 0);
    
    app->decode_input_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(app->decode_input_entry), 20);
    gtk_entry_set_width_chars(GTK_ENTRY(app->decode_input_entry), 10);
    gtk_box_pack_start(GTK_BOX(input_box), app->decode_input_entry, FALSE, FALSE, 0);
    
    GtkWidget *browse_btn = create_styled_button("BROWSE", G_CALLBACK(browse_decode_input), app);
    gtk_box_pack_start(GTK_BOX(input_box), browse_btn, FALSE, FALSE, 0);
    
    // Separator
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(input_box), separator, FALSE, FALSE, 10);
    
    // Output file number
    GtkWidget *output_label = gtk_label_new("OUTPUT FILE ID:");
    gtk_style_context_add_class(gtk_widget_get_style_context(output_label), "txt-normal");
    gtk_box_pack_start(GTK_BOX(input_box), output_label, FALSE, FALSE, 0);
    
    app->decode_output_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(app->decode_output_entry), 10);
    gtk_entry_set_width_chars(GTK_ENTRY(app->decode_output_entry), 10);
    gtk_box_pack_start(GTK_BOX(input_box), app->decode_output_entry, FALSE, FALSE, 0);
    
    // Decode button
    GtkWidget *decode_btn = create_styled_button("◀ DECODE", G_CALLBACK(decode_file), app);
    gtk_box_pack_start(GTK_BOX(input_box), decode_btn, FALSE, FALSE, 10);
    
    gtk_box_pack_start(GTK_BOX(tab), input_frame, FALSE, FALSE, 10);
    
    // Preview frame
    GtkWidget *preview_frame = gtk_frame_new("REVERSE ENGINEERING MATRIX");
    gtk_frame_set_shadow_type(GTK_FRAME(preview_frame), GTK_SHADOW_ETCHED_IN);
    
    // Create a paned window for split view
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(preview_frame), paned);
    
    // Input preview
    GtkWidget *input_preview_frame = gtk_frame_new("ENCODED INPUT");
    GtkWidget *input_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(input_scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    
    app->decode_input_preview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->decode_input_preview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->decode_input_preview), GTK_WRAP_WORD);
    
    // Set custom color for digit input
    GtkTextBuffer *input_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->decode_input_preview));
    GtkTextTag *digit_tag = gtk_text_buffer_create_tag(input_buffer, "digit", 
                            "foreground", app->colors.digit_color, NULL);
    
    gtk_container_add(GTK_CONTAINER(input_scroll), app->decode_input_preview);
    gtk_container_add(GTK_CONTAINER(input_preview_frame), input_scroll);
    
    // Output preview
    GtkWidget *output_preview_frame = gtk_frame_new("DECODED SOURCE CODE");
    GtkWidget *output_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scroll),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    
    app->decode_output_preview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->decode_output_preview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->decode_output_preview), GTK_WRAP_WORD);
    
    // Create buffer and add tags for syntax highlighting (for when we decode C code)
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->decode_output_preview));
    gtk_text_buffer_create_tag(output_buffer, "keyword", "foreground", app->colors.keyword_color, NULL);
    gtk_text_buffer_create_tag(output_buffer, "preprocessor", "foreground", app->colors.accent_purple, NULL);
    gtk_text_buffer_create_tag(output_buffer, "comment", "foreground", app->colors.text_dim, NULL);
    gtk_text_buffer_create_tag(output_buffer, "string", "foreground", app->colors.accent_green, NULL);
    gtk_text_buffer_create_tag(output_buffer, "number", "foreground", app->colors.digit_color, NULL);
    
    gtk_container_add(GTK_CONTAINER(output_scroll), app->decode_output_preview);
    gtk_container_add(GTK_CONTAINER(output_preview_frame), output_scroll);
    
    // Add the frames to the paned view
    gtk_paned_add1(GTK_PANED(paned), input_preview_frame);
    gtk_paned_add2(GTK_PANED(paned), output_preview_frame);
    gtk_paned_set_position(GTK_PANED(paned), 450);
    
    gtk_box_pack_start(GTK_BOX(tab), preview_frame, TRUE, TRUE, 10);
}

// Create a styled section label
static GtkWidget* create_section_label(const char *text, const char *color, AppData *app) {
    GtkWidget *label = gtk_label_new(text);
    
    // Create custom CSS for this label
    GtkCssProvider *provider = gtk_css_provider_new();
    char css[256];
    snprintf(css, sizeof(css), "label { color: %s; font-size: 10pt; }", color);
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    
    // Apply the CSS
    GtkStyleContext *context = gtk_widget_get_style_context(label);
    gtk_style_context_add_provider(context,
                                GTK_STYLE_PROVIDER(provider),
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    g_object_unref(provider);
    return label;
}

// Create a styled button
static GtkWidget* create_styled_button(const char *label, GCallback callback, AppData *app) {
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", callback, app);
    return button;
}

// Open a file browser dialog for character map
static void browse_charmap_file(GtkWidget *widget, AppData *app) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("SELECT CHARACTER MAP FILE",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Add file filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Text files");
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    GtkFileFilter *all_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filter, "All files");
    gtk_file_filter_add_pattern(all_filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);
    
    // Show the dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Extract file number from filename (if possible)
        char *basename = g_path_get_basename(filename);
        char *file_number = g_strdup(basename);
        
        // Remove extension
        char *dot = strrchr(file_number, '.');
        if (dot) {
            *dot = '\0';
        }
        
        // Check if it's a number
        bool is_number = true;
        for (int i = 0; file_number[i] != '\0'; i++) {
            if (!isdigit(file_number[i])) {
                is_number = false;
                break;
            }
        }
        
        if (is_number) {
            gtk_entry_set_text(GTK_ENTRY(app->charmap_file_entry), file_number);
        } else {
            gtk_entry_set_text(GTK_ENTRY(app->charmap_file_entry), filename);
        }
        
        g_free(basename);
        g_free(file_number);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Open a file browser dialog for source file to encode
static void browse_encode_input(GtkWidget *widget, AppData *app) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("SELECT SOURCE CODE FILE",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Add file filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "C files");
    gtk_file_filter_add_pattern(filter, "*.c");
    gtk_file_filter_add_pattern(filter, "*.h");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    GtkFileFilter *all_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filter, "All files");
    gtk_file_filter_add_pattern(all_filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);
    
    // Show the dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(app->encode_input_entry), filename);
        
        // Load file content for preview
        char content[MAX_TEXT_LENGTH];
        if (load_file_content(filename, content, MAX_TEXT_LENGTH)) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->encode_input_preview));
            gtk_text_buffer_set_text(buffer, content, -1);
            
            // Highlight C syntax if it's a C file
            if (strstr(filename, ".c") || strstr(filename, ".h")) {
                highlight_c_syntax(app, buffer);
            }
            
            // Update progress bar and status
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.5);
            set_status_message(app, "SOURCE CODE LOADED • READY FOR ENCODING");
        } else {
            show_message_dialog(GTK_WINDOW(app->window), "Failed to load file content", GTK_MESSAGE_ERROR);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Open a file browser dialog for encoded file to decode
static void browse_decode_input(GtkWidget *widget, AppData *app) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("SELECT ENCODED FILE",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Add file filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Text files");
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    GtkFileFilter *all_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filter, "All files");
    gtk_file_filter_add_pattern(all_filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);
    
    // Show the dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Extract file number from filename (if possible)
        char *basename = g_path_get_basename(filename);
        char *file_number = g_strdup(basename);
        
        // Remove extension
        char *dot = strrchr(file_number, '.');
        if (dot) {
            *dot = '\0';
        }
        
        // Check if it's a number
        bool is_number = true;
        for (int i = 0; file_number[i] != '\0'; i++) {
            if (!isdigit(file_number[i])) {
                is_number = false;
                break;
            }
        }
        
        if (is_number) {
            gtk_entry_set_text(GTK_ENTRY(app->decode_input_entry), file_number);
        } else {
            gtk_entry_set_text(GTK_ENTRY(app->decode_input_entry), filename);
        }
        
        // Load file content for preview
        char content[MAX_TEXT_LENGTH];
        if (load_file_content(filename, content, MAX_TEXT_LENGTH)) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->decode_input_preview));
            gtk_text_buffer_set_text(buffer, content, -1);
            
            // Apply digit coloring to all text
            GtkTextIter start, end;
            gtk_text_buffer_get_bounds(buffer, &start, &end);
            gtk_text_buffer_apply_tag_by_name(buffer, "digit", &start, &end);
            
            // Update progress bar and status
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.5);
            set_status_message(app, "ENCODED FILE LOADED • READY FOR DECODING");
        } else {
            show_message_dialog(GTK_WINDOW(app->window), "Failed to load file content", GTK_MESSAGE_ERROR);
        }
        
        g_free(basename);
        g_free(file_number);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Load a character map from file
static void load_char_map(GtkWidget *widget, AppData *app) {
    const char *file_input = gtk_entry_get_text(GTK_ENTRY(app->charmap_file_entry));
    
    // Animation effect
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
    
    // Process file input - could be a number or a path
    char filename[MAX_FILENAME];
    if (file_input[0] >= '0' && file_input[0] <= '9') {
        bool is_number = true;
        for (int i = 0; file_input[i] != '\0'; i++) {
            if (!isdigit(file_input[i])) {
                is_number = false;
                break;
            }
        }
        
        if (is_number) {
            snprintf(filename, sizeof(filename), "%s.txt", file_input);
        } else {
            strncpy(filename, file_input, sizeof(filename)-1);
            filename[sizeof(filename)-1] = '\0';
        }
    } else {
        strncpy(filename, file_input, sizeof(filename)-1);
        filename[sizeof(filename)-1] = '\0';
    }
    
    set_status_message(app, "LOADING CHARACTER MAP...");
    
    // Open the file
    FILE *file = fopen(filename, "r");
    if (!file) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Failed to open character map file", 
                           GTK_MESSAGE_ERROR);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
        set_status_message(app, "ERROR: Failed to open file");
        return;
    }
    
    // Reset the character map
    for (int i = 0; i < app->char_map_size; i++) {
        if (app->char_map[i] != NULL) {
            free(app->char_map[i]);
            app->char_map[i] = NULL;
        }
    }
    app->char_map_size = 0;
    
    // Buffer for the character map text display
    GString *char_map_text = g_string_new("");
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.2);
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Read and process each line
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        // Parse the line: index<tab>character
        char *tab_pos = strchr(line, '\t');
        if (!tab_pos) {
            g_string_append_printf(char_map_text, 
                                  "WARNING: Line %d is not in the expected format (index<tab>character), skipping\n", 
                                  line_num);
            continue;
        }
        
        // Temporarily terminate string at tab position
        *tab_pos = '\0';
        
        // Convert the index part to an integer
        int index;
        if (sscanf(line, "%d", &index) != 1) {
            g_string_append_printf(char_map_text, 
                                  "WARNING: Line %d has an invalid index, skipping\n", 
                                  line_num);
            continue;
        }
        
        // Process the character part
        char *char_part = tab_pos + 1;
        
        // Handle special cases
        if (strcmp(char_part, "Space") == 0) {
            if (index > app->char_map_size) {
                app->char_map_size = index;
            }
            app->char_map[index - 1] = strdup(" ");
        } else if (strcmp(char_part, "Tab") == 0) {
            if (index > app->char_map_size) {
                app->char_map_size = index;
            }
            app->char_map[index - 1] = strdup("\t");
        } else if (strcmp(char_part, "") == 0) {
            // Empty character treated as space
            if (index > app->char_map_size) {
                app->char_map_size = index;
            }
            app->char_map[index - 1] = strdup(" ");
            g_string_append_printf(char_map_text, 
                                  "Note: Empty character at line %d interpreted as space\n", 
                                  line_num);
        } else if (strlen(char_part) == 1) {
            // Regular single character
            if (index > app->char_map_size) {
                app->char_map_size = index;
            }
            app->char_map[index - 1] = strdup(char_part);
        } else if (strlen(char_part) == 2 && char_part[0] == '\\') {
            // Escape sequences
            if (index > app->char_map_size) {
                app->char_map_size = index;
            }
            
            char escape_char[2] = {0, 0};
            
            switch (char_part[1]) {
                case 'n': escape_char[0] = '\n'; break;
                case 't': escape_char[0] = '\t'; break;
                case 'r': escape_char[0] = '\r'; break;
                case '0': escape_char[0] = '\0'; break;
                case '\\': escape_char[0] = '\\'; break;
                case '\'': escape_char[0] = '\''; break;
                case '\"': escape_char[0] = '\"'; break;
                default:
                    g_string_append_printf(char_map_text, 
                                         "WARNING: Unknown escape sequence %s, ignoring\n", 
                                         char_part);
                    continue;
            }
            
            app->char_map[index - 1] = strdup(escape_char);
        } else if (strlen(char_part) > 0) {
            // More complex representation - just take the first character
            g_string_append_printf(char_map_text, 
                                 "WARNING: Character part '%s' contains multiple characters, using first one: '%c'\n", 
                                 char_part, char_part[0]);
            
            if (index > app->char_map_size) {
                app->char_map_size = index;
            }
            
            char first_char[2] = {char_part[0], '\0'};
            app->char_map[index - 1] = strdup(first_char);
        }
    }
    
    fclose(file);
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.6);
    while (gtk_events_pending()) gtk_main_iteration();
    
    app->is_map_loaded = true;
    g_string_append_printf(char_map_text, 
                         "\n✓ CHARACTER MAPPING LOADED SUCCESSFULLY WITH %d CHARACTERS.\n\n", 
                         app->char_map_size);
    g_string_append(char_map_text, "== LOADED CHARACTER MAP ==\n");
    
    // Display the character map
    for (int i = 0; i < app->char_map_size; i++) {
        if (app->char_map[i] == NULL) {
            continue;
        }
        
        // Check if the character is printable
        char c = app->char_map[i][0];
        if (isprint(c) && c != '\t' && c != '\n') {
            g_string_append_printf(char_map_text, "MAP[%d] = '%s'\n", i+1, app->char_map[i]);
        } else {
            g_string_append_printf(char_map_text, "MAP[%d] = '\\x%02x'\n", i+1, (unsigned char)c);
        }
    }
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.9);
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Update the display
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->charmap_display));
    gtk_text_buffer_set_text(buffer, char_map_text->str, -1);
    
    // Apply color tags
    apply_color_tags_to_charmap(app);
    
    // Update status and complete progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 1.0);
    
    char status_msg[256];
    snprintf(status_msg, sizeof(status_msg), 
            "CHARACTER MAP LOADED FROM %s • %d CHARACTERS", 
            filename, app->char_map_size);
    set_status_message(app, status_msg);
    
    g_string_free(char_map_text, TRUE);
}

// Find the index of a character in the char_map array
static int find_char_index(AppData *app, char c) {
    char search_char[2] = {c, '\0'};
    
    for (int i = 0; i < app->char_map_size; i++) {
        if (app->char_map[i] != NULL && strcmp(app->char_map[i], search_char) == 0) {
            return i + 1; // +1 because our list is 1-indexed
        }
    }
    return -1; // Character not found
}

// Encode a source file
static void encode_file(GtkWidget *widget, AppData *app) {
    if (!app->is_map_loaded) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Please load a character map first", 
                           GTK_MESSAGE_WARNING);
        return;
    }
    
    const char *input_filename = gtk_entry_get_text(GTK_ENTRY(app->encode_input_entry));
    const char *output_file_number = gtk_entry_get_text(GTK_ENTRY(app->encode_output_entry));
    
    if (strlen(input_filename) == 0) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Please enter an input filename", 
                           GTK_MESSAGE_WARNING);
        return;
    }
    
    if (strlen(output_file_number) == 0) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Please enter an output file number", 
                           GTK_MESSAGE_WARNING);
        return;
    }
    
    // Check if output file number is valid
    for (int i = 0; output_file_number[i] != '\0'; i++) {
        if (!isdigit(output_file_number[i])) {
            show_message_dialog(GTK_WINDOW(app->window), 
                               "Output file number must be a number", 
                               GTK_MESSAGE_WARNING);
            return;
        }
    }
    
    // Animation effect
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
    set_status_message(app, "INITIALIZING ENCODING PROCESS...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    char output_filename[MAX_FILENAME];
    snprintf(output_filename, sizeof(output_filename), "%s.txt", output_file_number);
    
    // Open input file
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Could not open input file", 
                           GTK_MESSAGE_ERROR);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
        set_status_message(app, "ERROR: Failed to open input file");
        return;
    }
    
    // Open output file
    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        fclose(input_file);
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Could not open output file", 
                           GTK_MESSAGE_ERROR);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
        set_status_message(app, "ERROR: Failed to open output file");
        return;
    }
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.3);
    set_status_message(app, "ANALYZING SOURCE CODE...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Buffer for encoded output
    GString *encoded_content = g_string_new("");
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.5);
    set_status_message(app, "APPLYING CHARACTER MAPPING...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Read each character from the input file
    int c;
    while ((c = fgetc(input_file)) != EOF) {
        int index = find_char_index(app, (char)c);
        if (index != -1) {
            g_string_append_printf(encoded_content, "%d ", index);
            fprintf(output_file, "%d ", index);
        } else {
            // For characters not in our mapping, use 0
            g_string_append(encoded_content, "0 ");
            fprintf(output_file, "0 ");
        }
    }
    
    fclose(input_file);
    fclose(output_file);
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.8);
    set_status_message(app, "FINALIZING ENCODED OUTPUT...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Update output preview
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->encode_output_preview));
    gtk_text_buffer_set_text(buffer, encoded_content->str, -1);
    
    // Apply digit coloring to all text
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_apply_tag_by_name(buffer, "digit", &start, &end);
    
    // Complete progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 1.0);
    
    char status_msg[256];
    snprintf(status_msg, sizeof(status_msg), 
            "ENCODING COMPLETE: %s → %s", 
            input_filename, output_filename);
    set_status_message(app, status_msg);
    
    // Show completion message
    char message[256];
    snprintf(message, sizeof(message), 
            "File encoded successfully:\n%s → %s", 
            input_filename, output_filename);
    show_message_dialog(GTK_WINDOW(app->window), message, GTK_MESSAGE_INFO);
    
    g_string_free(encoded_content, TRUE);
}

// Decode an encoded file
static void decode_file(GtkWidget *widget, AppData *app) {
    if (!app->is_map_loaded) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Please load a character map first", 
                           GTK_MESSAGE_WARNING);
        return;
    }
    
    const char *input_file_number = gtk_entry_get_text(GTK_ENTRY(app->decode_input_entry));
    const char *output_file_number = gtk_entry_get_text(GTK_ENTRY(app->decode_output_entry));
    
    if (strlen(input_file_number) == 0) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Please enter an input file number", 
                           GTK_MESSAGE_WARNING);
        return;
    }
    
    if (strlen(output_file_number) == 0) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Please enter an output file number", 
                           GTK_MESSAGE_WARNING);
        return;
    }
    
    // Animation effect
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
    set_status_message(app, "INITIALIZING DECODING PROCESS...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Determine input filename
    char input_filename[MAX_FILENAME];
    bool is_number = true;
    for (int i = 0; input_file_number[i] != '\0'; i++) {
        if (!isdigit(input_file_number[i])) {
            is_number = false;
            break;
        }
    }
    
    if (is_number) {
        snprintf(input_filename, sizeof(input_filename), "%s.txt", input_file_number);
    } else {
        strncpy(input_filename, input_file_number, sizeof(input_filename)-1);
        input_filename[sizeof(input_filename)-1] = '\0';
    }
    
    char output_filename[MAX_FILENAME];
    snprintf(output_filename, sizeof(output_filename), "%s.txt", output_file_number);
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.2);
    set_status_message(app, "READING ENCODED DATA...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Open input file
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Could not open input file", 
                           GTK_MESSAGE_ERROR);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
        set_status_message(app, "ERROR: Failed to open input file");
        return;
    }
    
    // Open output file
    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        fclose(input_file);
        show_message_dialog(GTK_WINDOW(app->window), 
                           "Could not open output file", 
                           GTK_MESSAGE_ERROR);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
        set_status_message(app, "ERROR: Failed to open output file");
        return;
    }
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.4);
    set_status_message(app, "REVERSING CHARACTER MAPPING...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Buffer for decoded output
    GString *decoded_content = g_string_new("");
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.6);
    set_status_message(app, "GENERATING SOURCE CODE...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Read each number from the input file
    int index;
    while (fscanf(input_file, "%d", &index) == 1) {
        if (index > 0 && index <= app->char_map_size && app->char_map[index - 1] != NULL) {
            g_string_append(decoded_content, app->char_map[index - 1]);
            fprintf(output_file, "%s", app->char_map[index - 1]);
        } else if (index == 0) {
            // Handle unmapped characters (we used 0 as a special code)
            g_string_append_c(decoded_content, '?');
            fprintf(output_file, "?");
        }
    }
    
    fclose(input_file);
    fclose(output_file);
    
    // Update progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.8);
    set_status_message(app, "FINALIZING DECODED OUTPUT...");
    while (gtk_events_pending()) gtk_main_iteration();
    
    // Update output preview
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->decode_output_preview));
    gtk_text_buffer_set_text(buffer, decoded_content->str, -1);
    
    // Apply syntax highlighting if it's C code
    if (strstr(output_filename, ".c") || 
        strstr(decoded_content->str, "#include") || 
        strstr(decoded_content->str, "int ") || 
        strstr(decoded_content->str, "void ")) {
        highlight_c_syntax(app, buffer);
    }
    
    // Complete progress
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 1.0);
    
    char status_msg[256];
    snprintf(status_msg, sizeof(status_msg), 
            "DECODING COMPLETE: %s → %s", 
            input_filename, output_filename);
    set_status_message(app, status_msg);
    
    // Show completion message
    char message[256];
    snprintf(message, sizeof(message), 
            "File decoded successfully:\n%s → %s", 
            input_filename, output_filename);
    show_message_dialog(GTK_WINDOW(app->window), message, GTK_MESSAGE_INFO);
    
    g_string_free(decoded_content, TRUE);
}

// Apply color tags to character map display
static void apply_color_tags_to_charmap(AppData *app) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->charmap_display));
    GtkTextIter start, end;
    
    // Find and tag all MAP entries
    gtk_text_buffer_get_start_iter(buffer, &start);
    while (gtk_text_iter_forward_search(&start, "MAP[", GTK_TEXT_SEARCH_TEXT_ONLY, &start, &end, NULL)) {
        GtkTextIter line_end = end;
        gtk_text_iter_forward_to_line_end(&line_end);
        
        GtkTextIter equals_pos = start;
        if (gtk_text_iter_forward_search(&equals_pos, "=", GTK_TEXT_SEARCH_TEXT_ONLY, &equals_pos, NULL, &line_end)) {
            // Color the MAP part blue
            gtk_text_buffer_apply_tag_by_name(buffer, "blue", &start, &equals_pos);
            
            // Color the value part green
            gtk_text_buffer_apply_tag_by_name(buffer, "green", &equals_pos, &line_end);
        }
        
        start = line_end;
    }
    
    // Find and tag all WARNING entries
    gtk_text_buffer_get_start_iter(buffer, &start);
    while (gtk_text_iter_forward_search(&start, "WARNING:", GTK_TEXT_SEARCH_TEXT_ONLY, &start, &end, NULL)) {
        GtkTextIter line_end = end;
        gtk_text_iter_forward_to_line_end(&line_end);
        
        // Color the warning red
        gtk_text_buffer_apply_tag_by_name(buffer, "red", &start, &line_end);
        
        start = line_end;
    }
    
    // Find and highlight success message
    gtk_text_buffer_get_start_iter(buffer, &start);
    if (gtk_text_iter_forward_search(&start, "✓", GTK_TEXT_SEARCH_TEXT_ONLY, &start, &end, NULL)) {
        GtkTextIter line_end = end;
        gtk_text_iter_forward_to_line_end(&line_end);
        
        // Color the success message green
        gtk_text_buffer_apply_tag_by_name(buffer, "green", &start, &line_end);
    }
    
    // Find and highlight header
    gtk_text_buffer_get_start_iter(buffer, &start);
    if (gtk_text_iter_forward_search(&start, "== LOADED CHARACTER MAP ==", GTK_TEXT_SEARCH_TEXT_ONLY, &start, &end, NULL)) {
        GtkTextIter line_end = end;
        gtk_text_iter_forward_to_line_end(&line_end);
        
        // Color the header purple
        gtk_text_buffer_apply_tag_by_name(buffer, "purple", &start, &line_end);
    }
}

// Highlight C syntax in a text buffer
static void highlight_c_syntax(AppData *app, GtkTextBuffer *buffer) {
    // Define C keywords for syntax highlighting
    const char *keywords[] = {
        "int", "char", "void", "if", "else", "for", "while", "return",
        "break", "continue", "switch", "case", "default", "struct",
        "typedef", "union", "extern", "static", "const", "enum", "sizeof",
        NULL
    };
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    
    // Get all text
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    // Highlight keywords
    for (int i = 0; keywords[i] != NULL; i++) {
        gtk_text_buffer_get_start_iter(buffer, &start);
        
        const char *keyword = keywords[i];
        int keyword_len = strlen(keyword);
        
        while (gtk_text_iter_forward_search(&start, keyword, GTK_TEXT_SEARCH_VISIBLE_ONLY, &start, &end, NULL)) {
            // Check if the keyword is a whole word
            if ((gtk_text_iter_starts_word(&start) || !gtk_text_iter_inside_word(&start)) &&
                (gtk_text_iter_ends_word(&end) || !gtk_text_iter_inside_word(&end))) {
                
                gtk_text_buffer_apply_tag_by_name(buffer, "keyword", &start, &end);
            }
            
            start = end;
        }
    }
    
    // Highlight preprocessor directives
    gtk_text_buffer_get_start_iter(buffer, &start);
    while (gtk_text_iter_forward_line(&start)) {
        GtkTextIter line_start = start;
        GtkTextIter line_end = start;
        gtk_text_iter_forward_to_line_end(&line_end);
        
        // Get the line text
        char *line_text = gtk_text_buffer_get_text(buffer, &line_start, &line_end, FALSE);
        
        // Check if it's a preprocessor directive
        if (line_text[0] == '#') {
            gtk_text_buffer_apply_tag_by_name(buffer, "preprocessor", &line_start, &line_end);
        }
        
        g_free(line_text);
        
        // No need to update start, as it will be moved in the next iteration
    }
    
    g_free(text);
}

// Set status message
static void set_status_message(AppData *app, const char *message) {
    gtk_label_set_text(GTK_LABEL(app->status_bar), message);
}

// Set progress bar value
static void set_progress_value(AppData *app, double progress) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), progress);
}

// Load file content into a buffer
static gboolean load_file_content(const char *filename, char *buffer, size_t buffer_size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return FALSE;
    }
    
    size_t bytes_read = fread(buffer, 1, buffer_size - 1, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return TRUE;
}

// Show a message dialog
static void show_message_dialog(GtkWindow *parent, const char *message, GtkMessageType type) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        type,
        GTK_BUTTONS_OK,
        "%s",
        message
    );
    
    // Use the right title based on message type
    const char *title;
    switch (type) {
        case GTK_MESSAGE_INFO:
            title = "OPERATION SUCCESSFUL";
            break;
        case GTK_MESSAGE_WARNING:
            title = "WARNING";
            break;
        case GTK_MESSAGE_ERROR:
            title = "ERROR";
            break;
        default:
            title = "MESSAGE";
            break;
    }
    
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}