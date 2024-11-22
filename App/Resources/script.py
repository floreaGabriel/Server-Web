import threading
import requests
# import time

# Configurația serverului
SERVER_URL = "http://localhost:8080/"
NUM_CONNECTIONS = 200  # Numărul total de conexiuni
REQUEST_DELAY = 1     # Pauză între fiecare conexiune (în secunde)

# Funcția care simulează un client nou și trimite o cerere GET
def simulate_client(connection_id):
    session = requests.Session()  # Simulează un client nou printr-o sesiune diferită
    try:
        response = session.get(SERVER_URL)
        print(f"[Client {connection_id}] Status: {response.status_code}, Response: {response.text[:100]}")
    except Exception as e:
        print(f"[Client {connection_id}] Error: {e}")
    finally:
        session.close()  # Închide sesiunea pentru a simula client nou

# Funcția principală care gestionează mai mulți clienți
def test_server():
    threads = []
    for i in range(NUM_CONNECTIONS):
        # Creează un nou thread pentru fiecare client
        thread = threading.Thread(target=simulate_client, args=(i,))
        threads.append(thread)
        thread.start()
        # time.sleep(REQUEST_DELAY)  # Pauză între lansarea conexiunilor
    
    # Așteaptă finalizarea tuturor thread-urilor
    for thread in threads:
        thread.join()

if __name__ == "__main__":
    print(f"Testing {NUM_CONNECTIONS} connections to {SERVER_URL} with {REQUEST_DELAY} seconds delay.")
    test_server()
    print("Test finished.")
