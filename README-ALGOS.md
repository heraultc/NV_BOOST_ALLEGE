# NV-BOOST v2 « FAST » — Algorithmes temps-réel pour vision nocturne

## Le principe qui change tout

Dans la v1, les algorithmes lourds travaillaient **en pleine résolution** avec
des **boucles pixel-par-pixel** et des **temporaires flottants** de 11 Mo.
La v2 repose sur une règle unique :

> Tout ce qui est une *statistique* ou une *carte lisse* (histogramme, gamma,
> carte d'illumination, coefficients de guided filter) se calcule en **basse
> résolution** puis s'applique en pleine résolution via **LUT** ou
> **upsampling bilinéaire**. Seules les opérations élément-par-élément touchent
> le plein cadre — et en une seule passe fusionnée.

## Benchmarks mesurés (720p, 1 vCPU Xeon 2.8 GHz, mono-thread, -O2)

Conditions volontairement défavorables : sur un CPU desktop 4-8 cœurs avec le
multithreading OpenCV activé, comptez **2 à 4× plus rapide**.

| Algorithme            | v1 (ms) | v2 (ms) | Gain  | Remarque |
|-----------------------|--------:|--------:|:-----:|----------|
| Histogram Stretch     |    6.8  |    6.1  |  —    | boucles `.at<>` → `calcHist` |
| AGCWD                 |   19.3  |    6.2  |  3×   | médiane via histogramme, YCrCb au lieu de Lab |
| Mertens HDR           |  126.8  |    —    |  —    | **supprimé** → remplacé par Auto-LUT |
| LIME                  |   35.7  |    7.2  |  5×   | Fast-LIME : illumination au 1/4, gain fusionné |
| CLAHE                 |   15.6  |    9.0  | 1.7×  | YCrCb au lieu de Lab |
| Accumulation          |    7.0  |    6.7  |  —    | → **débruitage temporel adaptatif** (sans ghosting) |
| **Nouveau : Auto-LUT**        | — | **3.6**  | | remplace Stretch+AGCWD+Mertens en un seul `cv::LUT` |
| **Pipeline recommandé complet** | ~150+ | **~14** | **10×** | tdenoise + Auto-LUT + CLAHE |

## Les algorithmes qui augmentent VRAIMENT la capacité de nuit

### A. Débruitage temporel adaptatif au mouvement (le plus rentable)
Remplace l'accumulation naïve (qui créait des traînées fantômes). Moyenne
exponentielle par pixel dont le coefficient α dépend du mouvement local
(estimé au 1/2 de résolution) : zones statiques fortement moyennées
(SNR ×3-4), zones mobiles intactes (zéro ghosting). **6.7 ms.**
C'est le prérequis de tout le reste : chaque gain d'exposition amplifie le
bruit, donc on nettoie d'abord.

### B. Auto-LUT adaptative (le remplaçant de Stretch + AGCWD + Mertens)
Une seule courbe tonale de 256 entrées, recalculée chaque frame depuis un
histogramme sous-échantillonné (120 px de large) :
stretch percentile 0.5/99.5 % + gamma adaptatif AGCWD (γ = log 0.45 / log médiane,
ne fait qu'éclaircir) + roll-off des hautes lumières (protège les lampadaires).
La **LUT elle-même est lissée temporellement** (EMA 15 %) → zéro flicker,
zéro pompage d'exposition. Application = 1 appel `cv::LUT`. **3.6 ms.**

### Fast-LIME (option, alternative à l'Auto-LUT)
Carte d'illumination `T = max(B,G,R)` raffinée par **fast guided filter** au
1/4 de résolution, gain `1/T^γ` borné puis ré-agrandi et appliqué en une passe
8 bits. **7.2 ms** contre ~36 ms en v1, halos mieux contenus. Exclusif avec
l'Auto-LUT.

### E-F. CLAHE + netteté
Conservés : CLAHE passe en YCrCb (conversion 2× moins chère que Lab),
l'unsharp passe de σ=3 à σ=1.5 (moins de halos autour des sources lumineuses).

## Pipeline recommandé (ordre optimal)

```
débruitage temporel → Auto-LUT | Fast-LIME → CLAHE → [netteté]
```

Le débruitage vient **avant** le rehaussement (on n'amplifie pas ce qu'on a
nettoyé). Auto-LUT et Fast-LIME sont exclusifs.

## Ce qui a été supprimé et pourquoi

- **Mertens HDR** (127 ms) : pyramides laplaciennes × 3 expositions
  synthétiques — le résultat est reproductible à 95 % par une simple courbe
  tonale bien construite (l'Auto-LUT), pour 35× moins cher. Le toggle reste
  fonctionnel pour comparaison mais est marqué obsolète.
- **Les boucles pixel-par-pixel** (`at<uchar>` dans stretch/AGCWD).
- **Zero-DCE++, SCI, débruitage chroma, defog, deblock et guided denoise** :
  retirés du produit (code, paramètres, IHM, traductions et modèles ONNX).
  La dépendance `opencv_dnn` n'est plus nécessaire.

## Fichiers du pipeline

```
config.h              toggles / paramètres partagés UI ↔ traitement
videoprocessor.h/cpp  pipeline complet (tdenoise, Auto-LUT, Fast-LIME,
                      CLAHE, netteté, héritage stretch/AGCWD/Mertens)
backend.h/cpp         pont QML ↔ VideoProcessor (Q_PROPERTY)
qml/NvPipelinePanel.qml  toggles du pipeline
qml/NvParamsPanel.qml    sliders de paramètres
```

Le **Fast Guided Filter** (`fastGuidedFilter1C`) reste dans
`videoprocessor.cpp` : il est utilisé par Fast-LIME.

## Déploiement

Compilation : `qmake && make`. Les modules `opencv_ximgproc` et `opencv_dnn`
ne sont pas nécessaires.

## Pistes suivantes si tu veux aller encore plus loin

- **GPU** : porter le pipeline en OpenGL/UMat (`cv::UMat` active OpenCL de
  façon transparente).
- **3D-LUT apprises** (Zeng et al. 2020, « Image-Adaptive 3D LUTs ») : qualité
  couleur supérieure, application quasi gratuite par interpolation trilinéaire.
