# Sesta Tappa del Progetto

Dopo aver migliorato la logica del posizionamento delle carte nella mano procedo a migliorare l'estetica generale, aggiungo quindi:
- Un'animazione per la pescata delle carte
- Un'animazione per il caricamento del campo
- Cliccando su una carta adesso oltre a mostrare i dettagli della carta la carta stessa si alzera leggermente

**Animazione per la pescata delle carte**: Quando si pescano le 5 carte iniziali quest'ultime non vengono inserite direttamente nella mano (std::vector<Card> cards), ma per ognuna di esse viene salvata in un vettore temporaneo assime ai dettagli su posizione, tempo trascorso ecc. Successivamente quando vado a disegnare le carte non vado a disegnare direttamente le carte, ma iterando sul vettore temporaneo modifico la posizione di ogni singola carta fino a spostarla nella mano.

**Nota**: L'animazione è strutturata in modo tale da avere più fasi differenti:
- La carta parte da in cima al deck e finisce fuori dallo schermo. 
- Da fuori lo schermo riappare andando fino a davanti al giocatore
- Infine viene posizionata nella mano

Possiamo quindi individuare più fasi. Per aiutarmi nello strutturare l'animazione ho creato una nuova enum class con più valori --> uno per ogni fase differente dell'animazione:
- MovingOut --> prima fase, la carta si sposta fuori dallo schermo
- ShowCard --> seconda fase, la carta si sposta da fuori lo schermo fino a davanti il giocatore
- MovingHand --> terza fase, la carta viene spostata in mano
- Done --> fine animazione

Oltre all'enum class vado a creare una struct che rappresenta l'effettiva animazione. Questo perchè (come specificato precedentemente), quando si pescano le prime 5 carte per ognuna di esse viene creata un'animazione, e successivamente inserita all'interno di un array di animazioni. Successivamente itero su questo array disegnando le singole animazioni. Avere una struttura che mi rappresenta l'animazione stessa è quindi molto comodo. 

Altra aggiunta per facilitare l'implementazione dell'animazione è stata la funzione moveForward(); la quale permette di implementare l'effettivo spostamento da un punto a un altro, e se questo spostamento ha avuto effettivamente successo viene restituito true.
QUindi prendendo un punto corrente, un target e la velocità di movimento vado a calcolare la distanza euclidea tra i due punti, e di volta in volta sposto il current in base alla velocità di movimento e al DeltaTime. Il deltaTime rappresenta il tempo trascorso (in secondi) dall’ultimo aggiornamento del frame. Serve a rendere il movimento indipendente dal frame rate, Se si moltiplica la velocità (speed) per deltaTime, ottieni la distanza che l’oggetto dovrebbe percorrere in quel frame.
Così, anche se il gioco va a 30 FPS o 144 FPS, l’oggetto si muoverà sempre alla stessa velocità reale (ad esempio, 100 pixel al secondo), perché ogni frame si muove solo della distanza appropriata per il tempo trascorso.

La logica effettiva dell'animazione viene gestita dopo la gestine degli eventi; infatti l'animazione non è un evento:
- Non risponde direttamente all'input dell'utente (non è causato da un tasto, click, ecc.).
- È parte della logica di aggiornamento del gioco: fa "progredire" le animazioni nel tempo.
- Dipende dal tempo (deltaTime), non da un input diretto.

**Nota** --> Modifiche alla classe Card: Siccome durante l'animazione è necessario prendere la posizione della carta ho aggiunto all'interno della classe card il metodo getPosition(), che restituisce la posizione della carta. 
Siccome l'implementazione iniziale fatta della funzione moveTowards prende come parametro un riferimento a una posizione, e lo modifica incrementandolo, ho aggiunto anche il metodo getPositionRef() --> questo metodo permette di ottenere un riferimento alla posizione 

---

**Animazione Per il Campo da Gioco**: L'obiettivo di questa animazione è avere una comparsa progressiva del campo da gioco. L'animazione è strutturata con un mix di fade-in e slide-in. è necessario quindi spostare l'offset e mano a mano abbassarlo. In contemporanea a ciò ci basta aggiungere un coeficiente alpha per gestire la trasparenza e ottenere l'effetto fade-in.
Siccome ho introdotto l'animazione posso anche rimuovere il clock per far trascorrere tot secondi (post pressione di INVIO, inizia subito l'animazione), nella parte di gestione degli eventi che si occupava unicamente di passare a Playing, ho quindi modificato la logica per far scorrere sia l'offset per l'efferto di slide-in che l'offset per l'effetto di fade-in. 

**Modifica della classe Field e Slot**: Quando ho progettato l'animazione delle carte ho fatto si che fosse un'altra struttura dati ad occuparsi dell'animazione, di conseguenza è bastato fare una draw di questa struttura dati. In questo caso invece vengono modificati l'offset del campo stesso e l'alpha, è quindi necessario andare a modificare la classe field, in particolare il metodo draw di quest'ultima per far si che riceva anche i 2 valori e cambi la visualizzazione del campo stesso in base ad essi. Ho dovuto modificare leggermente anche il metodo draw della classe Slot per far si che fosse compatibili con i nuovi valori.

Successivamente ho notato che aggiungendo l'animazione il deck veniva visualizzato prima che l'animazione fosse effettivamente terminata, di conseguenza ho aggiunto la stessa animazione per il deck. Per far ciò ho modificato ulteriormente la classe Field, aggiungendo un booleano che segnasse il termine della generazione, oltre a 2 metodi per ricavarne il valore e modificarlo e modificato il metodo draw della classe deck (e della classe card) per poter funzionare con questa logica e modificare la trasparenza. Ho successivamente aggiunto anche all'interno della classe deck i metodi per interagire con il booleano  


---

**Animazione per il sollevamento delle carte**: Questa animazione fa si che quando si clicca su una carta quest'ultima si sollevi. Per realizzarla basta semplicemente aggiornare la poszione della carta, modificando l'offset. A tal proposito andiamo a modificare la classe card aggiungendo proprio un offset, e 2 metodi che permettono di:
- setOffset --> aggiornare l'offset
- getOffset --> ottenere il valore dell'offset
**Modifica della gestione degli eventi**: Per poter gestire meglio il caso in cui si tiene premuto il tasto sinistro del mouse ho modificato il blocco della gestione degli eventi, distinguendo tra pressione del mouse e rilascio del mouse. Adesso si può passare dalla selezione di una carta a un altra tenendo premuto il mouse