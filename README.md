# ProgettoSO

Consegna 1:
Software che scansiona ricorsivamente le directory e subdirectory di un path dato da utente,e inserisce i nomi dei file trovati in una lista infine stampa la lista.

Consegna 2:
Applicazione che fornisce un servizio di file transfer,utilizzando il codice della consegna 1 per effetturare l'indicizzazione dei file presenti nella directory home del server.

Sommario funzionamento

L'applicazione implementa un sistema client-server ,utilizzando i socket nel dominio AF_INET di ipv4,la comunicazione avviene tramite il protocollo TCP(quindi è di tipo stream),viene instaurata una connessione fra il client e il server,questo comporta un overhead maggiore ma rende l'applicazione affidabile.
Il server inizialmente prima di porsi in attesa di ricevere connessioni con la accept(),effettua la scansione della directory home e salva i file in una lista.
Appena arriva una connessione crea un thread per il client ,e tramite il thread creato il server richiede l accesso tramite un sistema di login minimale.Dopo che il client effettua il login correttamente il server gli invia la lista dei file e il client dopo aver stampato la lista ,stampa un menu che elenca le funzionalità offerte dal server.

-FUNZIONE DI DOWNLOAD

Sintassi: download [pathdelfiledascaricare]

Permette al client di scaricare un file dal server indicando il path

-FUNZIONE DI UPLOAD

Sintassi: upload [filedacaricare]

Permette al client di caricare un file nella directory /home/[nomeuser]/ del server indicando il nome del file da caricare

-FUNZIONE LISTHOME

Sintassi: listhome

Tramite questa funzione il server invia nuovamente la lista contenente i file.

-FUNZIONE ESCI

Sintassi: esci

Permette al client di chiudere la connessione con il server e terminare

----Eventuali sviluppi futuri----
Migliorare il sistema di login rendendolo multiutente,implementando un sistema che permetta a più utenti di collegarsi al server ognuno con i suoi permessi e restrizione all'accesso alle directory e al download dei file.
Rendere più "sicuro" il programma tramite lo scambio di certificati fra il client e il server.
Migliorare il menù rendendolo meno minimale e più intuitivo per l utente.


