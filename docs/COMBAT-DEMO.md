# Combat demo

From PowerShell, after building the Windows package:

```powershell
.\win-package\opengoldbox.exe -- --combat-demo
```

The normal language and game-folder setup runs first. The demo then opens the
same campaign combat screen and encounter handoff with six level-one heroes:
Fighter, Paladin, Cleric,
Ranger, Rogue, and Bard. The Fighter is a Goliath. Each hero has a class-trained
weapon and armor equipped. Fourteen Kobolds occupy every square of the outer
ring around the compact party formation. The demo uses the SRD 5.2.1 combat
rules, the portable `combat_zoom` setting, and art decoded at runtime from the
player's installed Pool of Radiance files. No original assets are included in
the repository or package.

Use the existing combat buttons and click highlighted targets. Class-specific
abilities remain limited to the game's currently supported subset; all six
heroes can use basic combat actions.
