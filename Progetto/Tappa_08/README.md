# Ottava Tappa del Progetto

Modifica della classe Card con aggiunta del livello, attributo, ...

1. Creazione di più deck preimpostati e scelta
2. Caricamento di texture differenti per diversi tipi di carte --> ora come ora le carte caricano tutte la stessa texture, vorrei differenziare in modo tale da far si che le carte di tipi diversi carichino texture diverse.
3. Creazione di una reale "schermata di intro" con scritta "pressione tasto INVIO"
4. schermata di scelta tra "Gioca" e "Scegli il tuo deck"
5. Sistemare il Drag & drop (Inizio regolamento - più da tappa 09)


Per l'aggiunta di livelli, attributi, ecc ho aggiunto anche le enum classe apposite:
- Attribute.h --> I valori di tutti gli attributi possibili
- Feature.h --> I valori di tutte le feature (tipi aggiuntivi) che possono possedere le carte 

Aggiunta dei campi privati per ogni elemento aggiunto, e modifica del costruttore per poter inizializzare questi valori. Alcuni valori sono stati resi optional perchè non tutte le carte li possiedono --> es. le carte magia non posseggono atk e def.
Ho quindi successivamente cambiato anche la struttura della funzione showCardDetails, per mostrare tutte queste aggiunte. 

---

**Come implementare più deck preimpostati?** Ho iniziato a interrogarmi su come fare ad avere più deck preimpostati, e come gestire le varie texture per ogni carta, dal momento che gestirle come ho fatto finora (quindi caricare all'interno del main e passare tramite parametro) risultava molto scomodo. Quindi facendo qualche ricerca online, guidato anche da alcuni suggerimenti fatti da CGPT, ho pensato di implementare il tutto tramite un file esterno.

--> L'idea è quindi avere un file JSON esterno contente i deck e un altro file esterno contenente i dettagli delle carte. Quindi "l'elenco" di tutte queste risorse viene scritto in questi file, e tramite delle librerie (o un parser JSON) si prendono questi file in input e si eleborano facendo il parsing. Per gestire le texture possiamo invece sfruttare una classe Texture Manager esterna.
Per fare il parsing ho quindi usato la libreria -->  nlohmann/json.hpp 

**Modifica della Cartella Resources e dei relativi file**
- data --> tutte le classi .h di Type, Gamestate, ...
- texture --> tutte le immagini di texture usate finora
- jsonData --> il file hpp per la libreria usata, e i file cards e deck .json