# Server HTTP Multi-Threaded

Un server HTTP simplu și eficient implementat în C, cu suport pentru procesare multi-thread, CGI, PHP și diverse metode HTTP.

## Cuprins
- [Prezentare generală](#prezentare-generală)
- [Caracteristici](#caracteristici)
- [Structura proiectului](#structura-proiectului)
- [Configurare](#configurare)
- [Compilare](#compilare)
- [Utilizare](#utilizare)
- [API și funcționalități](#api-și-funcționalități)
- [Exemple](#exemple)
- [Contribuții](#contribuții)

## Prezentare generală

Acest proiect implementează un server HTTP lightweight capabil să proceseze mai multe cereri simultan folosind un pool de thread-uri. Serverul suportă metodele HTTP standard (GET, POST, PUT), execuție de scripturi CGI și PHP, precum și procesarea diferitelor tipuri de conținut.

## Caracteristici

- **Multi-threading**: Utilizează un pool de thread-uri pentru procesarea cererilor în paralel
- **Configurabil**: Parametrii serverului pot fi configurați prin fișierul JSON
- **Suport pentru metode HTTP**: GET, POST, PUT
- **Suport pentru tipuri de conținut**:
  - application/json
  - application/x-www-form-urlencoded
  - text/plain
  - multipart/form-data
- **Execuție scripturi**:
  - Suport CGI
  - Suport PHP
- **Compresie**: Suport pentru compresie GZIP
- **Autentificare**: Implementare simplă de autentificare pe bază de token

## Structura proiectului

```
App/
├── cJSON/                 # Biblioteca cJSON (dependență)
├── Resources/             # Fișiere de resurse (HTML, PHP, etc.)
├── Server-files/          # Fișierele core ale serverului
│   ├── Server.c          # Implementarea serverului HTTP
│   ├── Server.h          # Definițiile serverului
│   ├── ThreadPool.c      # Implementarea pool-ului de thread-uri
│   ├── ThreadPool.h      # Definițiile pool-ului
│   ├── ServerConfig.c    # Gestionarea configurării
│   └── ServerConfig.h    # Definițiile configurării
├── Server-files.h         # Header centralizat
├── libmini_server.h       # Header pentru biblioteca 
├── config.json            # Configurarea serverului
└── main.c                 # Punctul de intrare al aplicației
```

## Configurare

Serverul poate fi configurat prin modificarea fișierului `config.json`:

```json
{
  "server": {
    "domain": "AF_INET",
    "service": "SOCK_STREAM",
    "protocol": 0,
    "interface": "INADDR_ANY",
    "port": 8080,
    "backlog": 255,
    "thread_pool_size": 20
  }
}
```

### Parametrii de configurare:

- **domain**: Domeniul socket-ului (AF_INET pentru IPv4)
- **service**: Tipul socket-ului (SOCK_STREAM pentru TCP)
- **protocol**: Protocolul de transport (0 pentru autodetectare)
- **interface**: Interfața de rețea (INADDR_ANY pentru toate interfețele)
- **port**: Portul pe care ascultă serverul (implicit 8080)
- **backlog**: Dimensiunea cozii de conexiuni în așteptare
- **thread_pool_size**: Numărul de thread-uri din pool

## Compilare

Pentru a compila serverul, folosiți comanda `make` din directorul rădăcină al proiectului:

```bash
make
```

Acest lucru va compila toate componentele necesare și va genera executabilul `main`.

## Utilizare

După compilare, puteți rula serverul cu:

```bash
./main
```

Serverul va porni și va asculta pe portul configurat (implicit 8080). Puteți accesa serverul dintr-un browser web sau utilizând instrumente precum `curl` la adresa:

```
http://localhost:8080/
```

## API și funcționalități

### Structuri principale

- **Server**: Gestionează conexiunile socket
- **ThreadPool**: Gestionează pool-ul de thread-uri pentru procesarea cererilor
- **ServerConfig**: Încarcă și stochează configurația serverului

### Procesarea cererilor

Serverul suportă diverse tipuri de cereri HTTP:

- **GET**: Pentru obținerea resurselor
- **POST**: Pentru trimiterea datelor către server
- **PUT**: Pentru actualizarea resurselor

### Tipuri de conținut suportate

- **application/json**: Pentru date în format JSON
- **application/x-www-form-urlencoded**: Pentru date de formular
- **text/plain**: Pentru text simplu
- **multipart/form-data**: Pentru upload de fișiere

### Exemple de funcții importante

- `server_constructor`: Inițializează serverul
- `launch`: Pornește serverul și acceptă conexiuni
- `handler`: Procesează cererile HTTP
- `execute_cgi_script`: Execută scripturi CGI
- `execute_php_script`: Procesează scripturi PHP

## Exemple

### Procesarea unei cereri GET

Serverul caută fișierul solicitat în directorul Resources și îl returnează dacă există:

```
GET /index.html HTTP/1.1
Host: localhost:8080
```

### Procesarea unei cereri POST

Serverul poate procesa date trimise prin formulare:

```
POST /post_example HTTP/1.1
Host: localhost:8080
Content-Type: application/x-www-form-urlencoded

name=John&email=john@example.com
```

### Execuția unui script PHP

Serverul poate executa scripturi PHP:

```
GET /test.php?name=John&email=john@example.com HTTP/1.1
Host: localhost:8080
```

## Contribuții

Contribuțiile sunt binevenite! Vă rugăm să urmați acești pași:

1. Fork repository-ul
2. Creați un branch pentru feature-ul dvs. (`git checkout -b feature/amazing-feature`)
3. Commit modificările (`git commit -m 'Adaugă amazing-feature'`)
4. Push la branch (`git push origin feature/amazing-feature`)
5. Deschideți un Pull Request
