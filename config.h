#pragma once

//===========================================================================//
//  CONFIGURATION — partagée entre l'UI et le processeur vidéo
//
//  v2 « FAST » : les algorithmes lourds (Mertens, LIME bilatéral, boucles
//  pixel-par-pixel) sont remplacés par des versions temps-réel :
//    • Auto-LUT adaptative  (remplace HistStretch + AGCWD + Mertens)
//    • Fast-LIME  (carte d'illumination estimée au 1/4, guided filter rapide)
//    • Débruitage temporel adaptatif au mouvement (remplace Accumulation)
//===========================================================================//
struct Config
{
    // ── PIPELINE RECOMMANDÉ (rapide, efficace de nuit) ──────────────────────
    bool use_tdenoise      = true;   // A - Débruitage temporel (motion-adaptive EMA)
    bool use_autolut       = true;   // B - Auto-LUT adaptative (exposition + gamma + stretch)
    bool use_clahe         = true;   // E - CLAHE contraste local (canal Y)
    bool use_sharpen       = false;  // F - Netteté (unsharp léger)

    // ── ÉTAPES OPTIONNELLES ─────────────────────────────────────────────────
    bool use_lime          = false;  // Fast-LIME (alternative à Auto-LUT)

    // ── HÉRITAGE v1 (conservés pour compat backend/QML, réimplémentés) ─────
    bool use_histstretch   = false;  // stretch percentile → désormais via calcHist (rapide)
    bool use_agcwd         = false;  // gamma adaptatif    → désormais via histogramme (rapide)
    bool use_mertens       = false;  // ⚠ obsolète : préférer Auto-LUT

    // ── PARAMÈTRES ──────────────────────────────────────────────────────────
    double autolut_strength   = 0.7;   // Auto-LUT : intensité du boost [0..1]
    double tdenoise_strength  = 0.6;   // Débruitage temporel [0..0.95]
    double lime_gamma         = 0.6;   // Fast-LIME : gamma d'illumination [0.4..0.9]
    double agcwd_alpha        = 0.5;   // AGCWD : pondération [0.1 .. 1.0]
    float  clahe_clip         = 2.5f;  // CLAHE clip limit [1.0 .. 8.0]
    int    accumulation       = 1;     // (héritage) accumulation naïve [1..5]

    // ── AFFICHAGE ───────────────────────────────────────────────────────────
    bool show_overlay      = true;
    bool show_side_by_side = true;
};
