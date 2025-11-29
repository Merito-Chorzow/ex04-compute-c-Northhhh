# Ćwiczenie 2 - Zaawansowana aplikacja obliczeniowa: regulator PID w q15

## Po co PID i co to jest?

Regulator PID steruje układem na podstawie błędu e = setpoint − measurement:
- P (proportional) — reaguje proporcjonalnie do bieżącego błędu (szybka reakcja).
- I (integral) — sumuje błąd w czasie, usuwa błąd ustalony (ale grozi „pompującą” całką).
- D (derivative) — przewiduje trend (tłumi przeregulowania, ale lubi „szum”).

Forma ciągła:
`u(t) = Kp * e(t) + Ki * ∫ e(t) dt + Kd * de(t)/dt`

Forma dyskretna (w odstępach k = 0,1,2,…):
- P_k = Kp * e_k
- I_k = I_{k-1} + Ki * e_k * Ts
- D_k = Kd * (e_k − e_{k-1}) / Ts
- u_k = P_k + I_k + D_k (z ograniczeniami wyjścia)

W praktyce D filtrujemy (LPF pierwszego rzędu), a I ograniczamy (anti-windup), aby układ był stabilny i powtarzalny.

---

## Dlaczego q15 (fixed-point)?

Na mikrokontrolerach często brakuje FPU lub zależy nam na deterministycznym czasie wykonania. q15 to format stałoprzecinkowy z zakresem ~[-1.0, +1.0), reprezentowany jako int16_t.
Reguły:
- mnożenie dwóch q15 → wynik Q30 → przesuń w prawo o 15 bitów,
- zawsze saturuj (obcinaj do [-32768, 32767]),
- pilnuj skalowania: wszystkie sygnały (błąd, u, y, set) i wzmocnienia (Kp, Ki, Kd) muszą być spójne.

---

## Model rośliny do testów

Używamy prostej rośliny 1-rzędu:
y[k+1] = y[k] + α * (−y[k] + u[k])

Interpretacja:
- α ~ Ts/τ (0 < α < 1) — im większe α, tym „szybsza” roślina.
- Na wejściu u (sterowanie), na wyjściu y (pomiar).
- To tylko symulator do testów PID — dzięki temu zobaczysz wpływ parametrów bez sprzętu.

## Struktura
```
ex04_compute_c/
├─ .vscode/
│  ├─ tasks.json       # Build/Clean w VS Code
│  └─ launch.json      # Debug (Windows: GDB, macOS: LLDB)
├─ src/
│  ├─ q15.h            # konwersje i mnożenie q15
│  ├─ pid.h, pid.c     # PID z anty‑windup i filtrem D
│  ├─ plant.h, plant.c # prosta roślina 1‑rzędu
│  └─ main.c           # pętla symulacji + log co 50 kroków
└─ Makefile
```

## Jak się za to zabrać — plan działania (krok po kroku)

Krok 0 — Sanity check
- Zbuduj projekt w trybie debug (-O0 -g) i uruchom pętlę testową (1000 kroków).
- Wypisuj log co 50 kroków: k, set, y, u.
Cel: sprawdzić, że środowisko działa i masz konsolę z danymi.

Krok 1 — Ustal interfejs i skalowanie
- Wszystkie wartości (set, y, u, e, Kp/Ki/Kd) w q15.
- Załóż Ts = 1 (jednostkowy krok), czyli w dyskretnych wzorach nie musisz mnożyć/ dzielić przez Ts.
- Dobierz zakres u i y do [-1, 1). Jeśli potrzebujesz większego zasięgu, użyj skalowania po stronie „świata rzeczywistego”, ale w PID trzymaj q15.

Krok 2 — Zaimplementuj bezpieczne operacje q15
- mnożenie q15×q15 → (Q30 >> 15)
- dodawanie z saturacją
- konwersja float ↔ q15 tylko w warstwie logów/parametryzacji (nie w samym algorytmie)

Uwaga: nie podawaj gotowego kodu w raporcie — pokaż własną implementację i testy graniczne.

Krok 3 — Część P
- Zaimplementuj P_k = Kp * e_k.
- Sprawdź, czy dla Kp małego, y podąża w stronę set, ale powoli; dla zbyt dużego — układ oscyluje.

Krok 4 — Część I + anti-windup (AW)
- Zapamiętuj I_k = I_{k-1} + Ki * e_k.
- Zabezpiecz i_acc: |I_k| ≤ i_limit (clamping).
- Test windupu: ustaw u_min/u_max wąsko, aby wejść w saturację i zobacz, że bez AW całka „pompuje się” i układ wolno wraca.

Rozszerzenie (bez kodu tutaj): back-calculation (korekta całki o różnicę między u_raw i u_sat).

Krok 5 — Część D z filtrem
- Policz różnicę błędu de = e_k − e_{k-1}.
- Zastosuj LPF pierwszego rzędu: d_filt[k] = α * d_filt[k-1] + (1 − α) * de.
- α blisko 1 = mocniejsze tłumienie szumu, ale większe opóźnienie.

Krok 6 — Suma i saturacja wyjścia
- u_raw = P + I + D_filt
- u = saturate(u_raw, u_min, u_max)
- Dopiero po saturacji zastosuj wybraną strategię AW (jeśli to back-calculation — odnieś całkę do różnicy u_raw − u).

Krok 7 — Zdefiniuj metryki
- Czas narastania: minimalny k, od którego y ≥ 0.9 * set.
- Przeregulowanie [%]: max(y) powyżej set.
- Błąd ustalony: średni błąd w ostatnich, np. 100 krokach.
- Czas ustalenia: pierwszy k, od którego |y − set| ≤ 0.02*|set| „na stałe”.

> Nie musisz rysować wykresów — wystarczy tabela/wartości i krótkie uzasadnienie.

Krok 8 — Eksperymenty (obowiązkowe)
- Zrób przynajmniej 3 zestawy {Kp, Ki, Kd} oraz 2 wartości α rośliny.
- Dla każdego zestawu podaj metryki i krótki komentarz: czy układ jest stabilny, czy oscyluje, co zmienia filtr D i ograniczenie całki.

Krok 9 — Deterministyczność
- Uruchom tę samą konfigurację 5× i porównaj logi (powinny być identyczne).
- Jeśli różnią się — sprawdź, czy gdzieś nie użyłeś losowości lub zmiennoprzecinkowych obliczeń w algorytmie.

Krok 10 — (Opcjonalnie) rozszerzenia
- Back-calculation AW (porównanie z clampingiem).
- Ważenie setpointu w P / D (mniejsza reakcja na skok zadania).
- Bumpless transfer (płynna zmiana trybów/parametrów).
- „Leak” całki (powolny spadek, gdy błąd=0, by zapobiec zastoju).

---

## Pseudokod (dla orientacji — nie wklejaj go 1:1)

To nie jest gotowe rozwiązanie, tylko plan obliczeń.

```
init:
  zainicjalizuj pid: kp, ki, kd, i_acc=0, d_prev=0, d_filt=0
  ustaw u_min, u_max, i_limit, alpha_d (filtr D)

loop (k = 0..N-1):
  e = set - y

  // P
  p = Kp * e

  // I (zabezpieczenie)
  i_acc = clamp(i_acc + Ki * e, -i_limit, +i_limit)

  // D z filtrem
  de = e - e_prev
  d_filt = alpha_d * d_filt + (1 - alpha_d) * de
  e_prev = e

  u_raw = p + i_acc + d_filt
  u = clamp(u_raw, u_min, u_max)

  // (opcjonalnie back-calculation AW)
  // i_acc += Kaw * (u - u_raw)

  y = plant_step(y, u, alpha_plant)

  jeżeli k % 50 == 0 → wypisz k, set, y, u
```

---

## Co oddać
1.	Kod: moduły PID i plant (q15), pętla testowa z logiem.
2.	REPORT.md:
  - krótki opis algorytmu (P/I/D, filtr D, AW — wybrana metoda),
  - tabela metryk dla min. 3 × {Kp, Ki, Kd} i 2 × α,
  - 2–3 akapity wniosków: wpływ parametrów na stabilność/przeregulowanie, rola filtra D, sens i-limit.
3.	Deterministyczność: informacja, czy logi powtórzeń były identyczne; jeśli nie — dlaczego.

Do autogradingu możesz dodać w swoim programie marker jednej linii po zakończeniu testów, np. OK [B] (sam marker nie jest rozwiązaniem).

---

## Samokontrola i typowe pułapki
- Każde mnożenie q15 kończysz przesunięciem o 15 bitów i saturacją.
- i_acc ograniczony (bez tego windup popsuje regulację).
- Filtr D ma 0 < alpha_d < 1 i działa na pochodnej błędu (albo rozważ pochodną pomiaru).
- Najpierw liczysz u_raw, robisz saturację, potem AW (jeśli back-calculation).
- Metryki liczysz w sposób powtarzalny (te same warunki startowe).
- Nie porównujesz konfiguracji z różnymi set/y0 — inaczej testy nie są miarodajne.

Najczęstsze błędy:
- Podwójne/pominięte przesunięcie >>15.
- Przypadkowe użycie float w środku algorytmu (niedeterministyczne na MCU).
- Zbyt duże Kp/Ki → oscylacje, „pompowanie” całki.
- Brak filtracji D → duże przeregulowanie przy szumie/kwantyzacji.