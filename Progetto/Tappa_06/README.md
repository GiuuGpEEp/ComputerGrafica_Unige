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



