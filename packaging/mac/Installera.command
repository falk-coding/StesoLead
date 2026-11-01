#!/bin/bash
# Installerar Bakelit (Falk Instruments) – VST3 + AU – för den inloggade användaren och tar bort
# macOS-karantänen så att DAW:en kan ladda pluginet. Ingen admin-behörighet behövs.
set -e
cd "$(dirname "$0")"

VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"
mkdir -p "$VST3_DIR" "$AU_DIR"

echo "Installerar Bakelit …"
# Tidigare namn (StesoLead) har samma plugin-ID – ta bort så att det inte finns två.
rm -rf "$VST3_DIR/StesoLead.vst3" "$AU_DIR/StesoLead.component"
rm -rf "$VST3_DIR/Bakelit.vst3" "$AU_DIR/Bakelit.component"
cp -R Bakelit.vst3 "$VST3_DIR/"
cp -R Bakelit.component "$AU_DIR/"
xattr -dr com.apple.quarantine "$VST3_DIR/Bakelit.vst3" "$AU_DIR/Bakelit.component" 2>/dev/null || true

# Låt macOS läsa in AU-pluginet på nytt.
killall -9 AudioComponentRegistrar 2>/dev/null || true

echo ""
echo "Klart!"
echo "  VST3: $VST3_DIR/Bakelit.vst3"
echo "  AU:   $AU_DIR/Bakelit.component"
echo ""
echo "Starta om din DAW. Bakelit finns under Instruments → Falk Instruments → Bakelit."
echo "Logic/GarageBand använder AU, övriga (Reason, Ableton, Cubase …) VST3."
echo ""
read -n 1 -s -r -p "Tryck på valfri tangent för att stänga."
