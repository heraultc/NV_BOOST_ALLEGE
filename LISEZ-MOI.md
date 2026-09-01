# NV-BOOST ALLÉGÉ — prêt pour Windows

**Tout est déjà fait dans ce dossier.** Rien à copier, rien à remplacer,
rien à modifier. Suivez simplement les 4 étapes ci-dessous.

---

## Étape 1 — Créer le dépôt

Allez sur <https://github.com/new>

- **Repository name** : `NV_BOOST_ALLEGE`
- Laissez **Public**
- Ne cochez **rien** d'autre
- Bouton vert **Create repository**

---

## Étape 2 — Envoyer les fichiers

Sur la page qui s'affiche, cliquez sur le lien **uploading an existing file**
(au milieu du texte gris).

Ouvrez ce dossier dans votre explorateur de fichiers, sélectionnez **tout**
(`Ctrl+A`), et faites glisser dans la fenêtre du navigateur.

Attendez que la barre de progression se termine, puis descendez en bas et
cliquez sur le bouton vert **Commit changes**.

---

## Étape 3 — Ajouter le fichier de compilation

Le fichier `build.yml` ne passe pas par le glisser-déposer, parce qu'il est
dans un dossier caché. Il faut le créer à la main. C'est la seule étape un
peu inhabituelle.

1. Sur la page de votre dépôt : bouton **Add file** → **Create new file**
2. Dans le champ du nom, tapez exactement ceci :

   ```
   .github/workflows/build.yml
   ```

   En tapant les barres obliques, GitHub crée les dossiers tout seul.

3. Ouvrez le fichier `.github/workflows/build.yml` de ce dossier avec un
   éditeur de texte, sélectionnez tout, copiez.

   > Ce dossier est caché dans votre explorateur. Appuyez sur **Ctrl+H**
   > pour le faire apparaître.

4. Collez dans la grande zone de texte du navigateur.
5. Bouton vert **Commit changes...** → **Commit changes**

---

## Étape 4 — Récupérer l'exécutable

La compilation démarre toute seule. Allez dans l'onglet **Actions** de votre
dépôt.

Attendez que le rond jaune devienne une coche verte — comptez **10 à 20
minutes** la première fois.

Ensuite :

1. Cliquez sur la ligne du travail terminé
2. Descendez **tout en bas** de la page
3. Section **Artifacts** → cliquez sur **NV-BOOST-ALLEGE-windows-x64**

Un fichier ZIP se télécharge. Décompressez-le **entièrement** avant de lancer
`NightVector.exe` — sinon Windows ne trouvera pas les DLL qui l'accompagnent.

Au premier lancement, Windows affichera un avertissement de sécurité :
**Informations complémentaires** → **Exécuter quand même**. C'est normal,
l'application n'est pas signée.

---

## Ce qui a déjà été corrigé pour vous

Vous n'avez rien à faire, c'est juste pour votre information :

- **Les chemins de fichiers** — sur Windows, choisir une vidéo renvoyait un
  chemin invalide du type `/C:/video.mp4`. Corrigé dans les deux fenêtres de
  sélection.
- **La police d'affichage** — `"Monospace"` n'existe pas sur Windows. Les
  incrustations FPS et REC utilisent maintenant la police système.
- **L'ouverture de la webcam** — 5 à 10 secondes d'attente sur Windows.
  Passage à DirectShow, ouverture immédiate.
- **Le système de compilation** — l'ancien `NightVector.pro` contenait le
  chemin `C:/opencv/build` en dur. Remplacé par `CMakeLists.txt`, qui trouve
  OpenCV tout seul sur les trois systèmes.

Les fichiers `NightVector.pro`, `qml.qrc` et `MainWindow.qml` ont été retirés :
ils n'étaient plus utilisés.

---

## En cas de problème

Dans l'onglet **Actions**, si une ligne devient rouge au lieu de verte :
cliquez dessus, puis sur l'étape marquée d'une croix, et copiez-moi les
dernières lignes affichées.
