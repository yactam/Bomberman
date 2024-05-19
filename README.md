# Bomberman

![Features](https://skillicons.dev/icons?i=c,git,gitlab,github,vim,vscode)

Le but de ce projet est d’implémenter un serveur et un client pour un jeu en réseau de <a src="https://fr.wikipedia.org/wiki/Bomberman">Bomberman</a>.

## Table des Matières

- [Description du jeu](#description-du-jeu)
- [Usage](#usage)
- [Auteurs](#auteurs)

## Description du jeu

Une partie de Bomberman se joue à quatre joueurs sur une grille. La grille comporte des cases
libres sur lesquelles les joueurs peuvent se déplacer et des murs infranchissables. Chaque joueur
peut à tout moment poser une bombe sur une case libre et se déplacer dans une des quatre
directions cardinales.
Il y a deux types de murs, les murs qui peuvent être détruits par une bombe et ceux qui sont
indestructibles. Lorsqu’une bombe est posée, elle explose 3 secondes plus tard en éliminant les
murs destructibles et les joueurs qui se trouvent à proximité. Pour une case C de coordonnées
(i, j), les cases à proximité sont les cases de coordonnées (i, j &plusmn;
 k), (i &plusmn; k, j), (i &plusmn; 1, j &plusmn; 1),0 ⩽ k ⩽ 2, sans mur sur la ligne entre C et la case.

## Usage

**Créer une CA locale et signer la CSR**
(pour une CA privée)

- Créer la clé privée de la CA

```bash
openssl genpkey -algorithm RSA -out ca_private_key.pem -pkeyopt rsa_keygen_bits:2048
```

- Créer un certificat auto-signé pour la CA

```bash
openssl req -x509 -new -nodes -key ca_private_key.pem -sha256 -days 3650 -out ca_certificate.pem
```

- Signer la CSR avec la CA

```bash
openssl x509 -req -in request.csr -CA ca_certificate.pem -CAkey ca_private_key.pem -CAcreateserial -out certificate.pem -days 365 -sha256
```

**Compilation**
Pour compiler le projet, il suffit d'entrer la commande suivante :

```bash
make
```

**Exécution**
Aprés avoir compilé le projet, il suffit de lancer la commande :

Pour lancer le serveur

```bash
./server [-p port_tcp] [-b bomb_timer] [-m multicast_frequency] [-f request_frequency]
```

Pour lancer le client

```bash
./client [-h hostname] [-p tcp_port]
```

## Auteurs

- Voir le fichier: [AUTHORS.md](AUTHORS.md)
