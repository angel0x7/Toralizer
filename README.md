#  Toralizer C

**Toralizer** est un outil de "torification" léger développé en C pour Linux/WSL. Il permet de forcer n'importe quelle application réseau (comme `curl`, `wget` ou `nc`) à passer par le réseau **Tor** sans modifier le code source de l'application originale.

---
![Toralizer Explanation process](https://github.com/angel0x7/Toralizer/blob/main/pictures/toralizer_explanation_process.png)
---

##  Fonctionnement Technique

L'outil repose sur la technique du **LD_PRELOAD Hooking**. Il intercepte l'appel système `connect()` de la bibliothèque standard (`libc`) et redirige le flux vers un proxy SOCKS local.



### Points Clés :
* **Interception** : Détournement dynamique des sockets IPv4.
* **Protocole SOCKS4** : Implémentation manuelle du handshake binaire.
* **Synchronisation** : Utilisation de `poll()` pour gérer les latences du réseau Tor sous WSL.
* **Mémoire** : Utilisation de structures "packées" pour garantir l'intégrité des paquets réseau.

---

##  Prérequis

1. **Système** : Linux ou WSL (Windows Subsystem for Linux).
2. **Tor** : Le démon Tor doit être installé et actif sur le port 9050.

```bash
# Installation des dépendances
sudo apt update
sudo apt install build-essential tor curl -y
```

```bash
# Lancement du service Tor
sudo service tor start
# Vérification de l'état d'exécution
sudo service tor status 
```
---
**Exemple de résultat:**

![Status example](https://github.com/angel0x7/Toralizer/blob/main/pictures/tor_Status.png)

---

## Importation du Toralizer 

```bash
git clone https://github.com/angel0x7/Toralizer.git
cd Toralizer
gcc -fPIC -shared -o toralizer.so toralizer.c -ldl
```

##  Compilation

Le code doit être compilé en tant que **bibliothèque partagée** (`.so`) pour être injecté dynamiquement dans d'autres processus lors de leur exécution.

```bash
gcc -fPIC -shared -o toralizer.so toralizer.c -ldl
```
| Option | Description |
| :--- | :--- |
| **`-fPIC`** | *Position Independent Code* : nécessaire pour les bibliothèques partagées. |
| **`-shared`** | Génère un objet partagé utilisable par `LD_PRELOAD`. |
| **`-ldl`** | Lie la bibliothèque `libdl` pour permettre l'usage de `dlsym`. ||

### Test de connectivité (Netcat)

Vérification du tunnel SOCKS vers une IP publique (ex: Cloudflare DNS) :

```bash
LD_PRELOAD=./toralizer.so nc -z -v 1.1.1.1 80
```

---

**Exemple de résultat:**

![Tor exécution example](https://github.com/angel0x7/Toralizer/blob/main/pictures/tor_examples.png)
---

