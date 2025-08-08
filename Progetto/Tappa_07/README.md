# Settima Tappa del Progetto

L'obiettivo di questa settima tappa principalmente è quello di rifinire alcune cose della 6.

1. Spostare tutta la logica dell'animazione di pescata all'interno di una classe apposita in modo da ordinare meglio il codice. moveTowards è stata spostata come metodo di un oggetto della classe DrawAnimation
2. Sistemare l'animazione di pescata --> nella tappa precedente ero riuscito a creare con successo un'animazione per la pescata, ma era laggosa e non fatta particolarmente bene; cerco quindi di sistemarla
3. Dopo aver sistemato queste "piccolezze" cerco di andare a costruire una prima forma di drag & drop delle carte in modo da spostarle con successo negli slot corretti.
4. Caricamento di texture differenti per diversi tipi di carte --> ora come ora le carte caricano tutte la stessa texture, vorrei differenziare in modo tale da far si che le carte di tipi diversi carichino texture diverse.

optional --> se il resto della settima tappa viene fatta velocemente controllare anche le altre animazioni, come "ridurre le righe"

Ho quindi leggermente modificato il metodo moveTowards, e cambiata la logica delle animazioni. Al posto che inserirle tutte insieme ne inserisco una per volta e l'altra verrà aggiunta quando la precedente animazione termina. In questo modo al posto di animarle tutte assieme verrà animata solo una carta alla volta. Per far ciò ho creato una coda di carte da pescare
