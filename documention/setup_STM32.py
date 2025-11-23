import tkinter as tk
from tkinter import ttk
from tkinter import messagebox

# --- Stałe dla Klasycznego CAN (STM32 bxCAN) ---
TARGET_BAUDRATE = 1000000  # 1 Mbps
SAMPLE_POINT_MIN = 80.0    # 80%
SAMPLE_POINT_MAX = 90.0    # 90%

# Ograniczenia sprzętowe STM32 dla segmentów czasowych (wartości fizyczne, nie rejestrowe)
BS1_MIN = 1
BS1_MAX = 16
BS2_MIN = 1
BS2_MAX = 8
# Całkowita liczba Time Quanta na bit (Sync_Seg jest zawsze 1)
TOTAL_TQ_MIN = 1 + BS1_MIN + BS2_MIN # zazwyczaj 3
TOTAL_TQ_MAX = 1 + BS1_MAX + BS2_MAX # zazwyczaj 25

def oblicz_parametry():
    # Wyczyszczenie tabeli wyników
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

    # Główny cel: PCLK / (Prescaler * TotalTQ) = TargetBaudrate
    # Czyli: Prescaler * TotalTQ = PCLK / TargetBaudrate
    
    target_divider = pclk_hz / TARGET_BAUDRATE

    if not target_divider.is_integer():
         messagebox.showwarning("Ostrzeżenie", 
                                f"Przy zegarze {clock_mhz_str} MHz nie da się uzyskać idealnie 1 Mbps.\n"
                                f"Wymagany dzielnik całkowity: {target_divider}\n"
                                "Spróbuj zmienić konfigurację zegarów w CubeMX.")
         return

    target_divider = int(target_divider)
    znaleziono_rozwiazanie = False

    # Iterujemy po możliwych wartościach Total Time Quanta (Total TQ)
    for total_tq in range(TOTAL_TQ_MIN, TOTAL_TQ_MAX + 1):
        
        # Sprawdzamy, czy dla tego Total TQ istnieje całkowity Prescaler
        if target_divider % total_tq == 0:
            prescaler = target_divider // total_tq
            
            if prescaler < 1: continue

            # Mamy Prescaler i Total TQ. Teraz musimy podzielić TQ na segmenty.
            # Total TQ = 1 (Sync) + BS1 + BS2
            # Dostępne TQ dla BS1 i BS2:
            remaining_tq = total_tq - 1

            # Iterujemy po możliwych wartościach BS2 (Time Seg2), żeby znaleźć BS1
            for bs2 in range(BS2_MIN, BS2_MAX + 1):
                bs1 = remaining_tq - bs2

                # Sprawdzamy czy wyliczony BS1 mieści się w limitach sprzętowych
                if BS1_MIN <= bs1 <= BS1_MAX:
                    # Obliczamy Sample Point
                    # Sample Point znajduje się PO SyncSeg i BS1
                    sample_point_position = 1 + bs1
                    sample_point_pct = (sample_point_position / total_tq) * 100

                    # Sprawdzamy kryterium użytkownika (80-90%)
                    if SAMPLE_POINT_MIN <= sample_point_pct <= SAMPLE_POINT_MAX:
                        
                        # Obliczenia pomocnicze do weryfikacji
                        time_quantum_ns = (prescaler / pclk_hz) * 1e9
                        # actual_baud = pclk_hz / (prescaler * total_tq)

                        # Dodanie wyniku do tabeli
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
                            "Nie znaleziono kombinacji parametrów spełniających kryteria (1Mbps, Sample Point 80-90%) dla podanego zegara.")

# --- Konfiguracja GUI ---
root = tk.Tk()
root.title("Kalkulator CAN STM32 (1 Mbps)")
root.geometry("700x450")

# Ramka wejściowa
input_frame = tk.Frame(root, padx=10, pady=10)
input_frame.pack(fill=tk.X)

tk.Label(input_frame, text="Podaj częstotliwość zegara peryferium CAN (APB1/APB2) z CubeMX:").grid(row=0, column=0, sticky="w")

entry_clock = tk.Entry(input_frame, width=15)
entry_clock.grid(row=0, column=1, padx=5)
entry_clock.insert(0, "42") # Domyślna wartość dla przykładu

tk.Label(input_frame, text="MHz").grid(row=0, column=2, sticky="w")

# --- POPRAWKA W TEJ LINIJCE (zamiana tk.WE na "we") ---
btn_calculate = tk.Button(input_frame, text="Oblicz Parametry", command=oblicz_parametry, bg="#e1e1e1")
btn_calculate.grid(row=1, column=0, columnspan=3, pady=15, sticky="we")
# ------------------------------------------------------

# Ramka wyników
results_frame = tk.Frame(root, padx=10, pady=5)
results_frame.pack(fill=tk.BOTH, expand=True)

tk.Label(results_frame, text="Znalezione konfiguracje (wpisz te wartości do CubeMX):", font=("Arial", 10, "bold")).pack(anchor="w")

# Tabela (Treeview)
columns = ("prescaler", "bs1", "bs2", "sp", "tq_total", "tq_time")
tree = ttk.Treeview(results_frame, columns=columns, show="headings")

tree.heading("prescaler", text="Prescaler")
tree.column("prescaler", width=80, anchor=tk.CENTER)

tree.heading("bs1", text="Time Seg1 (BS1)")
tree.column("bs1", width=100, anchor=tk.CENTER)

tree.heading("bs2", text="Time Seg2 (BS2)")
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