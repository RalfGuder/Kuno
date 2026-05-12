---
title: Kuno Vault
tags:
  - vault-root
  - kuno
created: 2026-05-10
---

# Kuno Vault

Persönlicher Obsidian-Vault zum Repo `D:\Projekte\Kuno` -- dem Retrofit-Projekt von **"Kuno der Ritter"** (Ralf Guder, 1996).

Hier landen Notizen, Tagesprotokolle, Design-Skizzen und Recherche-Material zu den parallelen Portierungen (DOS-Original, Java/Slick2D, jMonkey, Unity, TRSE/C64). Der Vault liegt im Repo, gehört aber konzeptionell *neben* den Code -- analog zum `iTWO_Vault` im RIB-Projekt.

> [!info] Zweck
> - Tageslogs unter [[Daily Notes]]
> - Designspezifikationen, Plan- und Recherche-Notizen
> - Cross-Referenzen zwischen den Engines (Original-C++ ↔ TRSE-C64 ↔ Unity)

## Struktur

- `Daily Notes/` -- Tagesnotizen `YYYY-MM-DD.md`
- `Engines/` -- pro Stack eine Übersicht (CPP, TRSE, Unity, Java)
- `Specs/` -- Design-/Implementierungs-Spezifikationen (z. B. C64-Sprite-Pipeline)
- `Recherche/` -- Material zu Inspirationen (Manic Miner, KC-85/4-Urversion, ...)

## Konventionen

- Daily Notes: Frontmatter mit `date` und `tags`, Sections **Stand**, **Heute getan**, **Offene Fragen**, **Nächste Schritte**
- Wikilinks für vault-interne Verweise, Standard-Markdown-Links nur für externe URLs
- Keine Code-Duplikation -- bei Code-Bezug stattdessen Pfad+Zeile (`cpp/SPIEL.CPP:42`) verlinken

## Einstiegspunkte

- [[Daily Notes/2026-05-10|Heute]]
