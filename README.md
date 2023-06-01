**Mihalcenco David, 321CB**

### Tema 3

    - Pentru implementare am folosit laboratorul 10, am folosit biblioteca cJSON
        pentru-ca era deja putin cunoscuta pentru mine.
    
    - client.c

        - Mi-am alocat cookies si tokens global, am un response si un message, in care
        mereu salvez mesajul sau raspunsul. 

        - Voi astepta comenzi pana nu voi primi comanda exit, in dependenta de ce 
        comenzi primes fac unele lucruri.

        - In fiecare comanda pot sa fie unele errori care au fost tratate, ca de ex
        daca deja sunt logat atunci nu voi mai putea sa ma logez, daca sunt logat
        nu pot sa inregistrez un cont, daca ma aflu in librarie nu voi putea sa intru 
        iar. Sau in caz ca primesc un raspuns de erroare de la server atunci voi atunta
        clientul ca am primit o erroare.

        - Am folosit cJson pentru a parsa raspunsul serverului, ca de ex pentru get_books 
        unde primesc ca raspuns cartile unui utilizator, fiindca le primesc ca o lista
        de carti am parsat, si am afisat informatiile primite despre carte mai vizibil
        si mai clar pentru utilizator.

        - Extragerea stringurilor necesare din raspunsuri ca de ex pentru get_books 
        acea lista de carti o gasesc cu ajutor functie strstr care imi puncteaza catre
        prima aparitie a elementelor care le caut eu, dupa scap de caracterele care nu
        am nevoie, de ex in login primesc cookies, am gasit unde se afla si am scapat de
        primele 12 caractere, adica de denumirea acelui rand, de care nu aveam nevoie.

        - La fiecare comanda deschid conexiunea cu serverul ii trimit un mesaj si astept
        un raspund, mereu schimband adresa si ceea ce vreau sa trimit.

        