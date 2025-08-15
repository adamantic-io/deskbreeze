
#!/bin/bash
APP_NAME="Deskbreeze"
APP_DIR="./build/app/${APP_NAME}.app"

# Deploy Qt frameworks into .app bundle
macdeployqt "$APP_DIR" -qmldir=../qt-app

# Optional: Create a DMG
# create-dmg "$APP_DIR" --overwrite
