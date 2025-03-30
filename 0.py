import tkinter as tk
from tkinter import ttk, scrolledtext, filedialog, messagebox
import os
from tkinter.font import Font

class SourceCodeEncoderDecoder:
    def __init__(self, root):
        self.root = root
        self.root.title("QuantMatrix Encoder/Decoder Suite")
        self.root.geometry("1100x750")
        
        # Define color scheme - mathematical game aesthetic
        self.colors = {
            "bg_dark": "#121218",
            "bg_main": "#1a1a24",
            "bg_light": "#222233",
            "accent_blue": "#0088cc",
            "accent_purple": "#6a5acd",
            "accent_green": "#00cc88",
            "accent_red": "#ff3366",
            "text_bright": "#ffffff",
            "text_normal": "#ccccdd",
            "text_dim": "#9999aa",
            "border": "#333344",
            "highlight": "#3355bb",
            "digit_color": "#33ccff",
            "keyword_color": "#ff6688",
            "grid_line": "#333344"
        }
        
        # Custom fonts
        self.fonts = {
            "title": Font(family="Arial", size=16, weight="bold"),
            "header": Font(family="Arial", size=12, weight="bold"),
            "code": Font(family="Consolas", size=11),
            "status": Font(family="Arial", size=10),
            "button": Font(family="Arial", size=10, weight="bold")
        }
        
        # Configure root window
        self.root.configure(bg=self.colors["bg_dark"])
        
        # Variables
        self.char_map = [None] * 256
        self.char_map_size = 0
        self.is_map_loaded = False
        
        # Create the UI
        self.apply_style()
        self.create_ui()
    
    def apply_style(self):
        # Create a custom theme
        style = ttk.Style()
        style.theme_create("MathGameTheme", parent="alt", settings={
            "TFrame": {
                "configure": {
                    "background": self.colors["bg_main"],
                    "borderwidth": 0,
                }
            },
            "TLabelframe": {
                "configure": {
                    "background": self.colors["bg_main"],
                    "foreground": self.colors["text_bright"],
                    "borderwidth": 2,
                    "relief": "groove"
                }
            },
            "TLabelframe.Label": {
                "configure": {
                    "background": self.colors["bg_main"],
                    "foreground": self.colors["accent_blue"],
                    "font": self.fonts["header"]
                }
            },
            "TLabel": {
                "configure": {
                    "background": self.colors["bg_main"],
                    "foreground": self.colors["text_normal"],
                    "font": ("Arial", 11)
                }
            },
            "TButton": {
                "configure": {
                    "background": self.colors["accent_blue"],
                    "foreground": self.colors["text_bright"],
                    "borderwidth": 0,
                    "font": self.fonts["button"],
                    "focuscolor": self.colors["highlight"],
                    "padding": (10, 5)
                },
                "map": {
                    "background": [("active", self.colors["highlight"])],
                    "relief": [("pressed", "sunken"), ("!pressed", "raised")]
                }
            },
            "TEntry": {
                "configure": {
                    "fieldbackground": self.colors["bg_light"],
                    "foreground": self.colors["text_bright"],
                    "bordercolor": self.colors["border"],
                    "insertcolor": self.colors["text_bright"],
                    "padding": 5
                }
            },
            "TNotebook": {
                "configure": {
                    "background": self.colors["bg_main"],
                    "tabmargins": [2, 5, 2, 0],
                    "tabposition": "n",
                    "padding": [5, 5]
                }
            },
            "TNotebook.Tab": {
                "configure": {
                    "background": self.colors["bg_light"],
                    "foreground": self.colors["text_normal"],
                    "padding": [15, 5],
                    "font": ("Arial", 11, "bold")
                },
                "map": {
                    "background": [("selected", self.colors["accent_blue"])],
                    "foreground": [("selected", self.colors["text_bright"])],
                    "expand": [("selected", [1, 1, 1, 0])]
                }
            },
            "TPanedwindow": {
                "configure": {
                    "background": self.colors["bg_main"],
                    "sashrelief": "raised"
                }
            },
            "Horizontal.TProgressbar": {
                "configure": {
                    "background": self.colors["accent_green"],
                    "troughcolor": self.colors["bg_light"],
                    "bordercolor": self.colors["border"]
                }
            }
        })
        
        style.theme_use("MathGameTheme")
        
        # Configure Text widgets globally
        self.root.option_add("*Text.background", self.colors["bg_light"])
        self.root.option_add("*Text.foreground", self.colors["text_bright"])
        self.root.option_add("*Text.font", self.fonts["code"])
        self.root.option_add("*Text.borderwidth", 0)
        self.root.option_add("*Text.highlightthickness", 1)
        self.root.option_add("*Text.highlightbackground", self.colors["border"])
        self.root.option_add("*Text.highlightcolor", self.colors["accent_blue"])
        self.root.option_add("*Text.insertbackground", self.colors["accent_blue"])
        
        # Configure Entry widgets globally
        self.root.option_add("*Entry.background", self.colors["bg_light"])
        self.root.option_add("*Entry.foreground", self.colors["text_bright"])
        self.root.option_add("*Entry.font", self.fonts["code"])
        self.root.option_add("*Entry.borderwidth", 1)
        self.root.option_add("*Entry.highlightthickness", 1)
        self.root.option_add("*Entry.highlightbackground", self.colors["border"])
        self.root.option_add("*Entry.highlightcolor", self.colors["accent_blue"])
        self.root.option_add("*Entry.insertbackground", self.colors["accent_blue"])
    
    def create_ui(self):
        # Create decorative header with matrix-style elements
        header_frame = tk.Frame(self.root, bg=self.colors["bg_dark"], height=60)
        header_frame.pack(fill=tk.X)
        
        # Create grid pattern in header
        for i in range(20):
            vertical_line = tk.Frame(header_frame, width=1, bg=self.colors["grid_line"])
            vertical_line.place(x=i*55, y=0, height=60)
        
        for i in range(3):
            horizontal_line = tk.Frame(header_frame, height=1, bg=self.colors["grid_line"])
            horizontal_line.place(x=0, y=i*20, width=1100)
        
        # Add title with decorative elements
        title_label = tk.Label(header_frame, text="QUANTMATRIX ENCODER/DECODER", 
                              font=("Arial", 18, "bold"), 
                              bg=self.colors["bg_dark"], 
                              fg=self.colors["accent_blue"])
        title_label.place(relx=0.5, rely=0.5, anchor="center")
        
        # Add decorative math symbols
        symbols = ["∑", "∫", "∂", "√", "π", "Δ", "Ω"]
        for i, symbol in enumerate(symbols):
            label = tk.Label(header_frame, text=symbol, font=("Arial", 16), 
                           bg=self.colors["bg_dark"], fg=self.colors["accent_purple"])
            label.place(x=50 + i*150, y=30)
        
        # Main frame
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Create a notebook (tabs) with custom styling
        self.notebook = ttk.Notebook(main_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Tab 1: Character Map
        charmap_tab = ttk.Frame(self.notebook)
        self.notebook.add(charmap_tab, text="CHARACTER MAP")
        self.setup_charmap_tab(charmap_tab)
        
        # Tab 2: Encode
        encode_tab = ttk.Frame(self.notebook)
        self.notebook.add(encode_tab, text="ENCODE")
        self.setup_encode_tab(encode_tab)
        
        # Tab 3: Decode
        decode_tab = ttk.Frame(self.notebook)
        self.notebook.add(decode_tab, text="DECODE")
        self.setup_decode_tab(decode_tab)
        
        # Add decorative footer with a progress bar
        footer_frame = tk.Frame(self.root, bg=self.colors["bg_dark"], height=30)
        footer_frame.pack(fill=tk.X, side=tk.BOTTOM)
        
        # Status bar with animated elements
        self.status_var = tk.StringVar()
        self.status_var.set("SYSTEM READY • AWAITING OPERATION")
        status_bar = tk.Label(footer_frame, textvariable=self.status_var, 
                             bg=self.colors["bg_dark"], 
                             fg=self.colors["accent_green"],
                             font=self.fonts["status"],
                             anchor=tk.W)
        status_bar.pack(side=tk.LEFT, padx=10)
        
        # Add decorative progress bar
        self.progress = ttk.Progressbar(footer_frame, orient=tk.HORIZONTAL, length=200, mode='determinate')
        self.progress.pack(side=tk.RIGHT, padx=10, pady=5)
        self.progress['value'] = 100
    
    def setup_charmap_tab(self, parent):
        # Create decorative binary background pattern
        binary_label = tk.Label(parent, text="01001010110100101010010101", 
                              font=("Consolas", 8), fg=self.colors["text_dim"],
                              bg=self.colors["bg_main"])
        binary_label.place(x=10, y=10)
        
        binary_label2 = tk.Label(parent, text="10101001010100101001010010", 
                               font=("Consolas", 8), fg=self.colors["text_dim"],
                               bg=self.colors["bg_main"])
        binary_label2.place(x=750, y=50)
        
        # Create frames with digital border style
        input_frame = ttk.Frame(parent)
        input_frame.pack(fill=tk.X, padx=10, pady=20)
        
        # Add decorative corner accents
        for corner in [(0, 0), (0, 48), (270, 0), (270, 48)]:
            corner_accent = tk.Frame(input_frame, width=10, height=10, bg=self.colors["accent_blue"])
            corner_accent.place(x=corner[0], y=corner[1])
        
        # File number input with custom styling
        input_label = ttk.Label(input_frame, text="CHARACTER MAP FILE NUMBER:", 
                              font=("Arial", 10, "bold"))
        input_label.pack(side=tk.LEFT, padx=15)
        
        self.charmap_file_entry = ttk.Entry(input_frame, width=10, font=self.fonts["code"])
        self.charmap_file_entry.pack(side=tk.LEFT, padx=5)
        
        # Stylized buttons
        browse_btn = ttk.Button(input_frame, text="BROWSE FILES", command=self.browse_charmap_file)
        browse_btn.pack(side=tk.LEFT, padx=10)
        
        load_btn = ttk.Button(input_frame, text="LOAD MAP", command=self.load_char_map)
        load_btn.pack(side=tk.LEFT, padx=15)
        
        # Display frame for character map with math/game-like header
        display_frame = ttk.LabelFrame(parent, text="CHARACTER MATRIX VISUALIZATION")
        display_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Add decorative elements to the frame
        for i in range(5):
            vertical_line = tk.Frame(display_frame, width=1, bg=self.colors["grid_line"])
            vertical_line.place(relx=0.2*i, y=0, relheight=1)
        
        # Scrolled text widget with custom styling for the loaded character map
        self.charmap_display = scrolledtext.ScrolledText(
            display_frame, 
            width=40, 
            height=20,
            bg=self.colors["bg_light"],
            fg=self.colors["text_bright"],
            insertbackground=self.colors["accent_blue"],
            font=self.fonts["code"]
        )
        self.charmap_display.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        self.charmap_display.config(state=tk.DISABLED)
        
        # Add placeholder text
        self.charmap_display.config(state=tk.NORMAL)
        self.charmap_display.insert(tk.END, "< Character mapping will be displayed here >\n")
        self.charmap_display.insert(tk.END, "Load a character map file to begin...\n")
        self.charmap_display.tag_configure("blue", foreground=self.colors["accent_blue"])
        self.charmap_display.tag_configure("green", foreground=self.colors["accent_green"])
        self.charmap_display.tag_configure("red", foreground=self.colors["accent_red"])
        self.charmap_display.tag_configure("purple", foreground=self.colors["accent_purple"])
        self.charmap_display.config(state=tk.DISABLED)
    
    def setup_encode_tab(self, parent):
        # Create decorative elements
        binary_label = tk.Label(parent, text="▶▶▶ ENCODING MATRIX ▶▶▶", 
                              font=("Arial", 10), fg=self.colors["accent_green"],
                              bg=self.colors["bg_main"])
        binary_label.place(x=10, y=10)
        
        hex_label = tk.Label(parent, text="0x01 0x02 0x03 0x04", 
                           font=("Consolas", 9), fg=self.colors["text_dim"],
                           bg=self.colors["bg_main"])
        hex_label.place(x=850, y=10)
        
        # Create frames
        input_frame = ttk.Frame(parent)
        input_frame.pack(fill=tk.X, padx=10, pady=25)
        
        # Source file input with neon accents
        source_label = ttk.Label(input_frame, text="SOURCE CODE FILE:", font=("Arial", 10, "bold"))
        source_label.pack(side=tk.LEFT, padx=5)
        
        self.encode_input_entry = ttk.Entry(input_frame, width=30)
        self.encode_input_entry.pack(side=tk.LEFT, padx=5)
        
        browse_btn = ttk.Button(input_frame, text="BROWSE", command=self.browse_encode_input)
        browse_btn.pack(side=tk.LEFT, padx=5)
        
        # Add visual separator
        separator = ttk.Separator(input_frame, orient='vertical')
        separator.pack(side=tk.LEFT, padx=15, fill='y')
        
        # Output file number
        output_label = ttk.Label(input_frame, text="OUTPUT FILE ID:", font=("Arial", 10, "bold"))
        output_label.pack(side=tk.LEFT, padx=5)
        
        self.encode_output_entry = ttk.Entry(input_frame, width=10)
        self.encode_output_entry.pack(side=tk.LEFT, padx=5)
        
        # Encode button with accent
        encode_btn = ttk.Button(input_frame, text="▶ ENCODE", command=self.encode_file)
        encode_btn.pack(side=tk.LEFT, padx=15)
        
        # Preview frame with futuristic styling
        preview_frame = ttk.LabelFrame(parent, text="CODE TRANSFORMATION MATRIX")
        preview_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Split view for input and output preview
        preview_paned = ttk.PanedWindow(preview_frame, orient=tk.HORIZONTAL)
        preview_paned.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Input preview with math/code styling
        input_preview_frame = ttk.LabelFrame(preview_paned, text="SOURCE CODE")
        preview_paned.add(input_preview_frame, weight=1)
        
        self.encode_input_preview = scrolledtext.ScrolledText(
            input_preview_frame,
            bg=self.colors["bg_light"],
            fg=self.colors["text_bright"],
            font=self.fonts["code"]
        )
        self.encode_input_preview.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Output preview with numeric styling
        output_preview_frame = ttk.LabelFrame(preview_paned, text="ENCODED OUTPUT")
        preview_paned.add(output_preview_frame, weight=1)
        
        self.encode_output_preview = scrolledtext.ScrolledText(
            output_preview_frame,
            bg=self.colors["bg_light"],
            fg=self.colors["digit_color"],
            font=self.fonts["code"]
        )
        self.encode_output_preview.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
    
    def setup_decode_tab(self, parent):
        # Create decorative elements
        binary_label = tk.Label(parent, text="◀◀◀ DECODING MATRIX ◀◀◀", 
                              font=("Arial", 10), fg=self.colors["accent_purple"],
                              bg=self.colors["bg_main"])
        binary_label.place(x=10, y=10)
        
        hex_label = tk.Label(parent, text="MATRIX TRANSLATION ACTIVE", 
                           font=("Consolas", 9), fg=self.colors["text_dim"],
                           bg=self.colors["bg_main"])
        hex_label.place(x=800, y=10)
        
        # Create frames
        input_frame = ttk.Frame(parent)
        input_frame.pack(fill=tk.X, padx=10, pady=25)
        
        # Encoded file input
        source_label = ttk.Label(input_frame, text="ENCODED FILE ID:", font=("Arial", 10, "bold"))
        source_label.pack(side=tk.LEFT, padx=5)
        
        self.decode_input_entry = ttk.Entry(input_frame, width=10)
        self.decode_input_entry.pack(side=tk.LEFT, padx=5)
        
        browse_btn = ttk.Button(input_frame, text="BROWSE", command=self.browse_decode_input)
        browse_btn.pack(side=tk.LEFT, padx=5)
        
        # Add visual separator
        separator = ttk.Separator(input_frame, orient='vertical')
        separator.pack(side=tk.LEFT, padx=15, fill='y')
        
        # Output file number
        output_label = ttk.Label(input_frame, text="OUTPUT FILE ID:", font=("Arial", 10, "bold"))
        output_label.pack(side=tk.LEFT, padx=5)
        
        self.decode_output_entry = ttk.Entry(input_frame, width=10)
        self.decode_output_entry.pack(side=tk.LEFT, padx=5)
        
        # Decode button with accent
        decode_btn = ttk.Button(input_frame, text="◀ DECODE", command=self.decode_file)
        decode_btn.pack(side=tk.LEFT, padx=15)
        
        # Preview frame with futuristic styling
        preview_frame = ttk.LabelFrame(parent, text="REVERSE ENGINEERING MATRIX")
        preview_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Split view for input and output preview
        preview_paned = ttk.PanedWindow(preview_frame, orient=tk.HORIZONTAL)
        preview_paned.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Input preview with numeric styling
        input_preview_frame = ttk.LabelFrame(preview_paned, text="ENCODED INPUT")
        preview_paned.add(input_preview_frame, weight=1)
        
        self.decode_input_preview = scrolledtext.ScrolledText(
            input_preview_frame,
            bg=self.colors["bg_light"],
            fg=self.colors["digit_color"],
            font=self.fonts["code"]
        )
        self.decode_input_preview.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Output preview with code styling
        output_preview_frame = ttk.LabelFrame(preview_paned, text="DECODED SOURCE CODE")
        preview_paned.add(output_preview_frame, weight=1)
        
        self.decode_output_preview = scrolledtext.ScrolledText(
            output_preview_frame,
            bg=self.colors["bg_light"],
            fg=self.colors["text_bright"],
            font=self.fonts["code"]
        )
        self.decode_output_preview.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
    
    def browse_charmap_file(self):
        file_path = filedialog.askopenfilename(
            title="SELECT CHARACTER MAP FILE",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        if file_path:
            # Extract file number from filename (if possible)
            try:
                file_name = os.path.basename(file_path)
                file_number = file_name.split('.')[0]
                if file_number.isdigit():
                    self.charmap_file_entry.delete(0, tk.END)
                    self.charmap_file_entry.insert(0, file_number)
                else:
                    self.charmap_file_entry.delete(0, tk.END)
                    self.charmap_file_entry.insert(0, file_path)
            except:
                self.charmap_file_entry.delete(0, tk.END)
                self.charmap_file_entry.insert(0, file_path)
    
    def browse_encode_input(self):
        file_path = filedialog.askopenfilename(
            title="SELECT SOURCE CODE FILE",
            filetypes=[("C files", "*.c"), ("Header files", "*.h"), ("All files", "*.*")]
        )
        if file_path:
            self.encode_input_entry.delete(0, tk.END)
            self.encode_input_entry.insert(0, file_path)
            
            # Load preview
            try:
                with open(file_path, 'r') as f:
                    content = f.read()
                self.encode_input_preview.delete(1.0, tk.END)
                self.encode_input_preview.insert(tk.END, content)
                
                # Syntax highlighting for C code
                self.highlight_c_syntax(self.encode_input_preview)
                
                # Set progress indicator
                self.progress['value'] = 50
                self.status_var.set("SOURCE CODE LOADED • READY FOR ENCODING")
            except Exception as e:
                messagebox.showerror("ERROR", f"Failed to load file preview: {str(e)}")
    
    def browse_decode_input(self):
        file_path = filedialog.askopenfilename(
            title="SELECT ENCODED FILE",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        if file_path:
            # Extract file number from filename (if possible)
            try:
                file_name = os.path.basename(file_path)
                file_number = file_name.split('.')[0]
                if file_number.isdigit():
                    self.decode_input_entry.delete(0, tk.END)
                    self.decode_input_entry.insert(0, file_number)
                else:
                    self.decode_input_entry.delete(0, tk.END)
                    self.decode_input_entry.insert(0, file_path)
            except:
                self.decode_input_entry.delete(0, tk.END)
                self.decode_input_entry.insert(0, file_path)
            
            # Load preview
            try:
                with open(file_path, 'r') as f:
                    content = f.read()
                self.decode_input_preview.delete(1.0, tk.END)
                self.decode_input_preview.insert(tk.END, content)
                
                # Set progress indicator
                self.progress['value'] = 50
                self.status_var.set("ENCODED FILE LOADED • READY FOR DECODING")
            except Exception as e:
                messagebox.showerror("ERROR", f"Failed to load file preview: {str(e)}")
    
    def highlight_c_syntax(self, text_widget):
        # Define C keywords for syntax highlighting
        keywords = ["int", "char", "void", "if", "else", "for", "while", "return", 
                   "break", "continue", "switch", "case", "default", "struct", 
                   "typedef", "union", "extern", "static", "const", "enum", "sizeof"]
        
        # Configure tags
        text_widget.tag_configure("keyword", foreground=self.colors["keyword_color"])
        text_widget.tag_configure("preprocessor", foreground=self.colors["accent_purple"])
        text_widget.tag_configure("comment", foreground=self.colors["text_dim"])
        text_widget.tag_configure("string", foreground=self.colors["accent_green"])
        text_widget.tag_configure("number", foreground=self.colors["digit_color"])
        
        # Simple highlighting for keywords
        content = text_widget.get(1.0, tk.END)
        
        # Highlight keywords
        for keyword in keywords:
            start_index = 1.0
            while True:
                start_index = text_widget.search(r'\y' + keyword + r'\y', start_index, tk.END, regexp=True)
                if not start_index:
                    break
                
                end_index = f"{start_index}+{len(keyword)}c"
                text_widget.tag_add("keyword", start_index, end_index)
                start_index = end_index
        
        # Highlight preprocessor directives
        start_index = 1.0
        while True:
            start_index = text_widget.search(r'^#[a-zA-Z]+', start_index, tk.END, regexp=True)
            if not start_index:
                break
            
            line_end = text_widget.index(f"{start_index} lineend")
            text_widget.tag_add("preprocessor", start_index, line_end)
            start_index = f"{start_index}+1l"
    
    def load_char_map(self):
        file_input = self.charmap_file_entry.get().strip()
        
        # Animation effect
        self.progress['value'] = 0
        self.root.update()
        
        # Process file input - could be a number or a path
        if file_input.isdigit():
            filename = f"{file_input}.txt"
        else:
            filename = file_input
            
        self.status_var.set("LOADING CHARACTER MAP...")
        
        try:
            with open(filename, 'r') as file:
                # Reset the character map
                self.char_map = [None] * 256
                self.char_map_size = 0
                
                # Update progress
                self.progress['value'] = 20
                self.root.update()
                
                # Read and process each line
                line_num = 0
                char_map_text = ""
                
                for line in file:
                    line_num += 1
                    line = line.strip()
                    
                    # Parse the line: index<tab>character
                    if '\t' not in line:
                        char_map_text += f"WARNING: Line {line_num} is not in the expected format (index<tab>character), skipping\n"
                        continue
                    
                    parts = line.split('\t', 1)
                    if len(parts) != 2:
                        char_map_text += f"WARNING: Line {line_num} is not in the expected format (index<tab>character), skipping\n"
                        continue
                    
                    index_str, char_part = parts
                    
                    try:
                        index = int(index_str)
                    except ValueError:
                        char_map_text += f"WARNING: Line {line_num} has an invalid index, skipping\n"
                        continue
                    
                    # Process the character part
                    if char_part == "Space":
                        self.char_map[index - 1] = ' '
                        if index > self.char_map_size:
                            self.char_map_size = index
                    elif char_part == "Tab":
                        self.char_map[index - 1] = '\t'
                        if index > self.char_map_size:
                            self.char_map_size = index
                    # Add this special case to handle empty character part as space
                    elif char_part == "":
                        self.char_map[index - 1] = ' '
                        if index > self.char_map_size:
                            self.char_map_size = index
                        char_map_text += f"Note: Empty character at line {line_num} interpreted as space\n"
                    elif len(char_part) == 1:
                        # Regular single character
                        self.char_map[index - 1] = char_part
                        if index > self.char_map_size:
                            self.char_map_size = index
                    elif len(char_part) == 2 and char_part[0] == '\\':
                        # Escape sequences
                        if index > self.char_map_size:
                            self.char_map_size = index
                        
                        if char_part[1] == 'n':
                            self.char_map[index - 1] = '\n'
                        elif char_part[1] == 't':
                            self.char_map[index - 1] = '\t'
                        elif char_part[1] == 'r':
                            self.char_map[index - 1] = '\r'
                        elif char_part[1] == '0':
                            self.char_map[index - 1] = '\0'
                        elif char_part[1] == '\\':
                            self.char_map[index - 1] = '\\'
                        elif char_part[1] == '\'':
                            self.char_map[index - 1] = '\''
                        elif char_part[1] == '\"':
                            self.char_map[index - 1] = '\"'
                        else:
                            char_map_text += f"WARNING: Unknown escape sequence {char_part}, ignoring\n"
                            continue
                    elif len(char_part) > 0:
                        # More complex representation - just take the first character
                        char_map_text += f"WARNING: Character part '{char_part}' contains multiple characters, using first one: '{char_part[0]}'\n"
                        if index > self.char_map_size:
                            self.char_map_size = index
                        self.char_map[index - 1] = char_part[0]
                
                # Update progress
                self.progress['value'] = 60
                self.root.update()
                
                self.is_map_loaded = True
                char_map_text += f"\n✓ CHARACTER MAPPING LOADED SUCCESSFULLY WITH {self.char_map_size} CHARACTERS.\n\n"
                char_map_text += "== LOADED CHARACTER MAP ==\n"
                
                # Display the character map with futuristic styling 
                for i in range(self.char_map_size):
                    char = self.char_map[i]
                    if char is None:
                        continue
                    
                    if char.isprintable() and char != '\t' and char != '\n':
                        char_map_text += f"MAP[{i+1}] = '{char}'\n"
                    else:
                        ord_val = ord(char)
                        char_map_text += f"MAP[{i+1}] = '\\x{ord_val:02x}'\n"
                
                # Update progress
                self.progress['value'] = 90
                self.root.update()
                
                # Update the display with color coding
                self.charmap_display.config(state=tk.NORMAL)
                self.charmap_display.delete(1.0, tk.END)
                self.charmap_display.insert(tk.END, char_map_text)
                
                # Apply color tags
                self.apply_color_tags_to_charmap()
                
                self.charmap_display.config(state=tk.DISABLED)
                
                # Update status and complete progress
                self.progress['value'] = 100
                self.status_var.set(f"CHARACTER MAP LOADED FROM {filename} • {self.char_map_size} CHARACTERS")
                
        except Exception as e:
            self.progress['value'] = 0
            messagebox.showerror("ERROR", f"Failed to load character map: {str(e)}")
            self.status_var.set(f"ERROR: {str(e)}")
    
    def apply_color_tags_to_charmap(self):
        # Apply color to various parts of the charmap display
        
        # Find and tag all MAP entries
        start_idx = "1.0"
        while True:
            map_idx = self.charmap_display.search("MAP[", start_idx, tk.END)
            if not map_idx:
                break
                
            line_end = self.charmap_display.index(f"{map_idx} lineend")
            equals_pos = self.charmap_display.search("=", map_idx, line_end)
            
            # Color the MAP part blue
            self.charmap_display.tag_add("blue", map_idx, equals_pos)
            
            # Color the value part green or purple
            self.charmap_display.tag_add("green", equals_pos, line_end)
            
            # Move to next line
            start_idx = f"{map_idx}+1l"
        
        # Find and tag all WARNING entries
        start_idx = "1.0"
        while True:
            warning_idx = self.charmap_display.search("WARNING:", start_idx, tk.END)
            if not warning_idx:
                break
                
            line_end = self.charmap_display.index(f"{warning_idx} lineend")
            self.charmap_display.tag_add("red", warning_idx, line_end)
            
            # Move to next line
            start_idx = f"{warning_idx}+1l"
        
        # Find and highlight success message
        start_idx = "1.0"
        success_idx = self.charmap_display.search("✓", start_idx, tk.END)
        if success_idx:
            line_end = self.charmap_display.index(f"{success_idx} lineend")
            self.charmap_display.tag_add("green", success_idx, line_end)
        
        # Find and highlight header
        start_idx = "1.0"
        header_idx = self.charmap_display.search("== LOADED CHARACTER MAP ==", start_idx, tk.END)
        if header_idx:
            line_end = self.charmap_display.index(f"{header_idx} lineend")
            self.charmap_display.tag_add("purple", header_idx, line_end)
    
    def find_char_index(self, c):
        for i in range(self.char_map_size):
            if self.char_map[i] == c:
                return i + 1  # +1 because our list is 1-indexed
        return -1  # Character not found
    
    def encode_file(self):
        if not self.is_map_loaded:
            messagebox.showwarning("WARNING", "Please load a character map first")
            return
        
        input_filename = self.encode_input_entry.get().strip()
        output_file_number = self.encode_output_entry.get().strip()
        
        if not input_filename:
            messagebox.showwarning("WARNING", "Please enter an input filename")
            return
        
        if not output_file_number:
            messagebox.showwarning("WARNING", "Please enter an output file number")
            return
        
        if not output_file_number.isdigit():
            messagebox.showwarning("WARNING", "Output file number must be a number")
            return
        
        # Animation effect
        self.progress['value'] = 0
        self.status_var.set("INITIALIZING ENCODING PROCESS...")
        self.root.update()
        
        output_filename = f"{output_file_number}.txt"
        
        try:
            # Open files
            with open(input_filename, 'r') as input_file, open(output_filename, 'w') as output_file:
                content = input_file.read()
                
                # Update progress
                self.progress['value'] = 30
                self.status_var.set("ANALYZING SOURCE CODE...")
                self.root.update()
                
                encoded_content = ""
                
                # Update progress
                self.progress['value'] = 50
                self.status_var.set("APPLYING CHARACTER MAPPING...")
                self.root.update()
                
                # Generate output for preview
                for c in content:
                    index = self.find_char_index(c)
                    if index != -1:
                        encoded_content += f"{index} "
                        output_file.write(f"{index} ")
                    else:
                        # For characters not in our mapping, use 0
                        encoded_content += "0 "
                        output_file.write("0 ")
                
                # Update progress
                self.progress['value'] = 80
                self.status_var.set("FINALIZING ENCODED OUTPUT...")
                self.root.update()
                
                # Update output preview
                self.encode_output_preview.delete(1.0, tk.END)
                self.encode_output_preview.insert(tk.END, encoded_content)
                
                # Complete progress
                self.progress['value'] = 100
                self.status_var.set(f"ENCODING COMPLETE: {input_filename} → {output_filename}")
                
                # Show completion message
                messagebox.showinfo("OPERATION SUCCESSFUL", f"File encoded successfully:\n{input_filename} → {output_filename}")
        
        except Exception as e:
            self.progress['value'] = 0
            messagebox.showerror("ERROR", f"Failed to encode file: {str(e)}")
            self.status_var.set(f"ERROR: {str(e)}")
    
    def decode_file(self):
        if not self.is_map_loaded:
            messagebox.showwarning("WARNING", "Please load a character map first")
            return
        
        input_file_number = self.decode_input_entry.get().strip()
        output_file_number = self.decode_output_entry.get().strip()
        
        if not input_file_number:
            messagebox.showwarning("WARNING", "Please enter an input file number")
            return
        
        if not output_file_number:
            messagebox.showwarning("WARNING", "Please enter an output file number")
            return
        
        # Animation effect
        self.progress['value'] = 0
        self.status_var.set("INITIALIZING DECODING PROCESS...")
        self.root.update()
        
        # Determine if input is a number or a file path
        if input_file_number.isdigit():
            input_filename = f"{input_file_number}.txt"
        else:
            input_filename = input_file_number
            
        output_filename = f"{output_file_number}.txt"
        
        try:
            # Update progress
            self.progress['value'] = 20
            self.status_var.set("READING ENCODED DATA...")
            self.root.update()
            
            with open(input_filename, 'r') as input_file, open(output_filename, 'w') as output_file:
                content = input_file.read()
                decoded_content = ""
                
                # Update progress
                self.progress['value'] = 40
                self.status_var.set("REVERSING CHARACTER MAPPING...")
                self.root.update()
                
                # Split content by whitespace to get the index numbers
                indices = content.split()
                
                # Update progress
                self.progress['value'] = 60
                self.status_var.set("GENERATING SOURCE CODE...")
                self.root.update()
                
                for index_str in indices:
                    try:
                        index = int(index_str)
                        
                        if index > 0 and index <= self.char_map_size:
                            char = self.char_map[index - 1]  # -1 because our list is 1-indexed
                            if char is not None:
                                decoded_content += char
                                output_file.write(char)
                        elif index == 0:
                            # Handle unmapped characters (we used 0 as a special code)
                            decoded_content += '?'
                            output_file.write('?')
                    except ValueError:
                        # Skip non-numeric values
                        continue
                
                # Update progress
                self.progress['value'] = 80
                self.status_var.set("FINALIZING DECODED OUTPUT...")
                self.root.update()
                
                # Update output preview
                self.decode_output_preview.delete(1.0, tk.END)
                self.decode_output_preview.insert(tk.END, decoded_content)
                
                # Apply syntax highlighting to the decoded output if it's C code
                if output_filename.endswith('.c') or any(keyword in decoded_content for keyword in ['int', 'void', 'char', '#include']):
                    self.highlight_c_syntax(self.decode_output_preview)
                
                # Complete progress
                self.progress['value'] = 100
                self.status_var.set(f"DECODING COMPLETE: {input_filename} → {output_filename}")
                
                # Show completion message
                messagebox.showinfo("OPERATION SUCCESSFUL", f"File decoded successfully:\n{input_filename} → {output_filename}")
        
        except Exception as e:
            self.progress['value'] = 0
            messagebox.showerror("ERROR", f"Failed to decode file: {str(e)}")
            self.status_var.set(f"ERROR: {str(e)}")

if __name__ == "__main__":
    root = tk.Tk()
    app = SourceCodeEncoderDecoder(root)
    root.mainloop()

'''
This styling scheme transforms the application into an immersive mathematical video game experience with these key design elements:

### Visual Theme Elements
1. **Dark Sci-Fi Color Scheme** - Deep blues and blacks with bright accent colors for a futuristic mathematical/code environment
2. **Matrix-inspired Grid Elements** - Decorative grid patterns and binary numbers in the background
3. **Mathematical Symbols** - Integration of symbols like ∑, ∫, ∂, √, π, etc. to reinforce the mathematical theme
4. **Vibrant Data Visualization** - Color coding for different data types (digits in cyan, keywords in magenta)
5. **Stylized Progress Indicators** - Dynamic progress bar and status messages using game-like terminology

### UI Enhancements
1. **Custom Font Hierarchy** - Bold, technical fonts for headers and monospaced coding fonts for data display
2. **Animated Loading Effects** - Progress indicators that simulate data processing
3. **Syntax Highlighting** - Code syntax highlighting for C code in previews
4. **Futuristic Labels** - Renamed elements with terms like "MATRIX", "QUANTMATRIX", and directional indicators (▶▶▶, ◀◀◀)
5. **Restructured Layout** - More spacious layout with decorative elements that resembles a high-tech control panel

### Functional Improvements
1. **Enhanced Visual Feedback** - Color-coded messages and results
2. **Improved Code Highlighting** - Basic syntax highlighting for C code
3. **Status Messages** - More detailed, styled status messages in "computer system" terminology
4. **Streamlined Workflow** - Clear visual hierarchy and process indicators

The result is a visually immersive IDE that not only maintains all the functionality of the original C program but presents it in the style of a sophisticated mathematical video game interface that a global enterprise might develop.
'''