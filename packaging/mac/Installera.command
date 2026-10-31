#!/bin/bash
# Installerar StesoLead (VST3 + AU) för den inloggade användaren och tar bort
# macOS-karantänen så att Reason kan ladda pluginet. Ingen admin-behörighet behövs.
set -e
cd "$(dirname "$0")"

VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"
mkdir -p "$VST3_DIR" "$AU_DIR"

echo "Installerar StesoLead …"
rm -rf "$VST3_DIR/StesoLead.vst3" "$AU_DIR/StesoLead.component"
cp -R StesoLead.vst3 "$VST3_DIR/"
cp -R StesoLead.component "$AU_DIR/"
xattr -dr com.apple.quarantine "$VST3_DIR/StesoLead.vst3" "$AU_DIR/StesoLead.component" 2>/dev/null || true
xattr -dr com.apple.quarantine StesoLead.app 2>/dev/null || true

# Låt macOS läsa in AU-pluginet på nytt.
killall -9 AudioComponentRegistrar 2>/dev/null || true

echo ""
echo "Klart!"
echo "  VST3: $VST3_DIR/StesoLead.vst3"
echo "  AU:   $AU_DIR/StesoLead.component"
echo ""
echo "Starta om Reason. StesoLead finns under Instruments → Falk → StesoLead."
echo "Den fristående appen (StesoLead.app) kan startas direkt härifrån."
echo ""
read -n 1 -s -r -p "Tryck på valfri tangent för att stänga."
