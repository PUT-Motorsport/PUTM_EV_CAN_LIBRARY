import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
from tkinter import filedialog  # Dodane do obsługi plików
import re  # Dodane do wyrażeń regularnych (szukanie w tekście)

# --- Stałe dla Klasycznego CAN (STM32 bxCAN) ---
TARGET_BAUDRATE = 1000000  # 1 Mbps
SAMPLE_POINT_MIN = 70    # 80%
SAMPLE_POINT_MAX = 80    # 90%

BS1_MIN = 1
BS1_MAX = 16
BS2_MIN = 1
BS2_MAX = 8
TOTAL_TQ_MIN = 1 + BS1_MIN + BS2_MIN 
TOTAL_TQ_MAX = 1 + BS1_MAX + BS2_MAX 

def wczytaj_z_ioc():
    """Funkcja parsująca plik .ioc z CubeMX"""
    filepath = filedialog.askopenfilename(
        title="Wybierz plik konfiguracyjny .ioc",
        filetypes=[("Pliki CubeMX", "*.ioc"), ("Wszystkie pliki", "*.*")]
    )
    
    if not filepath:
        return

    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        # Szukamy wartości zegarów APB1 i APB2 w pliku .ioc
        # Format w pliku to zazwyczaj: RCC.APB1Freq_Value=42000000
        apb1_match = re.search(r'RCC\.APB1Freq_Value=(\d+)', content)
        apb2_match = re.search(r'RCC\.APB2Freq_Value=(\d+)', content)

        apb1_freq = int(apb1_match.group(1)) if apb1_match else 0
        apb2_freq = int(apb2_match.group(1)) if apb2_match else 0

        val_to_set = 0
        source_name = ""

        # Logika wyboru (Większość STM32 ma CAN na APB1)
        if apb1_freq > 0:
            val_to_set = apb1_freq
            source_name = "APB1"
            # Jeśli znaleziono oba, a APB1 jest typowe dla CAN, informujemy użytkownika
            if apb2_freq > 0:
                print(f"Znaleziono APB1: {apb1_freq} Hz i APB2: {apb2_freq} Hz. Wybrano APB1 (standard dla CAN).")
        elif apb2_freq > 0:
            val_to_set = apb2_freq
            source_name = "APB2"
        else:
            messagebox.showwarning("Błąd parsowania", "Nie znaleziono ustawień zegarów (APB1Freq/APB2Freq) w tym pliku .ioc.")
            return

        # Konwersja na MHz i wpisanie do pola
        clock_mhz = val_to_set / 1_000_000
        entry_clock.delete(0, tk.END)
        entry_clock.insert(0, f"{clock_mhz:g}") # :g usuwa zbędne zera po przecinku
        
        messagebox.showinfo("Sukces", f"Wczytano zegar z {source_name}: {clock_mhz} MHz")

    except Exception as e:
        messagebox.showerror("Błąd", f"Nie udało się odczytać pliku:\n{e}")

def oblicz_parametry():
    # - Logika obliczeń bez zmian
    for i in tree.get_children():
        tree.delete(i)

    try:
        clock_mhz_str = entry_clock.get().replace(',', '.')
        pclk_hz = float(clock_mhz_str) * 1_000_000
    except ValueError:
        messagebox.showerror("Błąd", "Wprowadź poprawną częstotliwość zegara (liczba).")
        return

    if pclk_hz <= 0:
        messagebox.showerror("Błąd", "Częstotliwość zegara musi być dodatnia.")
        return

    target_divider = pclk_hz / TARGET_BAUDRATE

    if not target_divider.is_integer():
         messagebox.showwarning("Ostrzeżenie", 
                                f"Przy zegarze {clock_mhz_str} MHz nie da się uzyskać idealnie 1 Mbps.\n"
                                f"Wymagany dzielnik całkowity: {target_divider}\n"
                                "Spróbuj zmienić konfigurację zegarów w CubeMX.")
         return

    target_divider = int(target_divider)
    znaleziono_rozwiazanie = False

    for total_tq in range(TOTAL_TQ_MIN, TOTAL_TQ_MAX + 1):
        if target_divider % total_tq == 0:
            prescaler = target_divider // total_tq
            if prescaler < 1: continue

            remaining_tq = total_tq - 1

            for bs2 in range(BS2_MIN, BS2_MAX + 1):
                bs1 = remaining_tq - bs2

                if BS1_MIN <= bs1 <= BS1_MAX:
                    sample_point_position = 1 + bs1
                    sample_point_pct = (sample_point_position / total_tq) * 100

                    if SAMPLE_POINT_MIN <= sample_point_pct <= SAMPLE_POINT_MAX:
                        time_quantum_ns = (prescaler / pclk_hz) * 1e9
                        tree.insert("", tk.END, values=(
                            prescaler,
                            bs1,
                            bs2,
                            f"{sample_point_pct:.2f}%",
                            total_tq,
                            f"{time_quantum_ns:.1f} ns"
                        ))
                        znaleziono_rozwiazanie = True

    if not znaleziono_rozwiazanie:
        messagebox.showinfo("Brak wyników", 
                            "Nie znaleziono kombinacji parametrów spełniających kryteria dla podanego zegara.")

# --- Konfiguracja GUI ---
root = tk.Tk()
root.title("Kalkulator CAN STM32 (1 Mbps) + Parser .ioc")
root.geometry("700x500") # Zwiększyłem nieco wysokość

# Ramka wejściowa
input_frame = tk.Frame(root, padx=10, pady=10)
input_frame.pack(fill=tk.X)

tk.Label(input_frame, text="Zegar CAN (APB1/APB2):").grid(row=0, column=0, sticky="w")

entry_clock = tk.Entry(input_frame, width=15)
entry_clock.grid(row=0, column=1, padx=5)
entry_clock.insert(0, "42") 

tk.Label(input_frame, text="MHz").grid(row=0, column=2, sticky="w")

# --- PRZYCISK WCZYTYWANIA Z PLIKU ---
btn_load = tk.Button(input_frame, text="Wczytaj z pliku .ioc", command=wczytaj_z_ioc, bg="#d9edf7")
btn_load.grid(row=0, column=3, padx=20, sticky="e")
# ------------------------------------

btn_calculate = tk.Button(input_frame, text="Oblicz Parametry", command=oblicz_parametry, bg="#e1e1e1")
btn_calculate.grid(row=1, column=0, columnspan=4, pady=15, sticky="we") # columnspan=4 bo doszedł przycisk

# Ramka wyników
results_frame = tk.Frame(root, padx=10, pady=5)
results_frame.pack(fill=tk.BOTH, expand=True)

tk.Label(results_frame, text="Znalezione konfiguracje:", font=("Arial", 10, "bold")).pack(anchor="w")

columns = ("prescaler", "bs1", "bs2", "sp", "tq_total", "tq_time")
tree = ttk.Treeview(results_frame, columns=columns, show="headings")

tree.heading("prescaler", text="Prescaler")
tree.column("prescaler", width=80, anchor=tk.CENTER)
tree.heading("bs1", text="Time Seg1")
tree.column("bs1", width=100, anchor=tk.CENTER)
tree.heading("bs2", text="Time Seg2")
tree.column("bs2", width=100, anchor=tk.CENTER)
tree.heading("sp", text="Sample Point")
tree.column("sp", width=100, anchor=tk.CENTER)
tree.heading("tq_total", text="Total TQ")
tree.column("tq_total", width=80, anchor=tk.CENTER)
tree.heading("tq_time", text="Time Quantum (ns)")
tree.column("tq_time", width=120, anchor=tk.CENTER)

scrollbar = ttk.Scrollbar(results_frame, orient=tk.VERTICAL, command=tree.yview)
tree.configure(yscroll=scrollbar.set)
scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
tree.pack(fill=tk.BOTH, expand=True)

root.mainloop()