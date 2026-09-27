# Tests sans appareil

Le test lie les implémentations réelles `VirtualController`, `ImageRouting`, `LedRouting` et les classes core `RGBController`/`RGBController_Virtual`. `FakeAPI` fournit un registre local et deux contrôleurs synthétiques ; aucun backend USB, BLE, réseau ou capture n’est lié.

```bat
set QT_ROOT=C:\path\to\Qt\6.8.3\msvc2022_64
set OPENRGB_CORE_DIR=C:\path\to\OpenRGB-Room
Build-Tests.cmd
```

Le kit Qt, MSVC et le cœur doivent être compatibles. `VCVARS` peut désigner un autre script d’environnement Visual Studio. La sortie est `.build/routing-tests.exe`. Le test crée une `QApplication` avec la plateforme Qt `offscreen`, aucun navigateur, aucune fenêtre visible ni capture.

Cas couverts :

- Grille 64×64 inchangée ; plan 800×600 réduit à 127×95 pour le SDK LED ; limites 16 bits canoniques.
- Affine d’une matrice entière, rotation enregistrée de 90°, composition avec rotation de 37°/miroir/brightness de la scène.
- Rejet du routage natif pour permutations non affines, masque incomplet et géométrie invalide ; positions physiques conservées.
- Inversion et espacement de LED ; pondération historique inchangée à basse résolution et projection dans une image d’aperçu réduite.
- Image 800×600 réellement spatiale et partagée à l’identique entre producteur/sink ; indépendance du `QImage` source mutable.
- Couleur attendue d’une LED réelle d’après ses coordonnées dans le plan.
- Wrapper core attaché, sortie déclarée, entrée invalide, zone non prise en charge et bail invalide.
- Mailbox latest-only et propriété des frames après retour du producteur.
- Bail actif suspendant les anciennes commandes LED, expiration permettant leur reprise.
- `Busy` et `Invalid` sans repli LED ; `Unsupported` autorisant le repli.
- Vidage du plan avant suppression des membres ; suppression des wrappers et détachement avant destruction du plugin.
- Graphe A→B valide ; A→A et B→A refusés, indépendamment des noms de contrôleurs.
- Hôte sans extension image : grille historique 128×128 conservée et aucune sortie image annoncée.
- Un seul lot de couleurs par contrôleur réunissant deux zones ou six segments, y compris des index recouvrants dont l'ordre est conservé.
- Jeton de topologie capturé au rebind : redimensionnement refusé avec l'ancien jeton, nouvelle route utilisant le jeton actualisé.
- Refus des lots `Busy`, `Invalid` et `Stale` sans appel direct à `SetColor` ; `Unsupported` seul autorisant le repli historique.
- Diagnostic de durée limité à 28 contrôleurs et un rapport par dix secondes, noms sur une ligne et silence pour les routes rapides.

Résultat initial sur Windows x64, MSVC 14.44, Qt 6.8.3 : PASS. Les tests ne valident ni les appareils physiques, ni la boucle d’événements complète de l’application. Le correctif de concurrence du registre des contrôleurs virtuels dans le cœur possède ses propres tests ; ce faux registre est synchrone.
