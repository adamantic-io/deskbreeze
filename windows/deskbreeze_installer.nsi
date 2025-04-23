
!define APPNAME "Deskbreeze"
!define VERSION "1.0"
!define COMPANY "Your Company"
!define DESCRIPTION "Deskbreeze - Fast & Embedded Qt+React App"
!define INSTALLDIR "$PROGRAMFILES\${APPNAME}"

Outfile "DeskbreezeInstaller.exe"
InstallDir "${INSTALLDIR}"

Page directory
Page instfiles

Section ""
  SetOutPath "$INSTDIR"
  File /r "build\qt-app\Deskbreeze.exe"
  CreateShortcut "$DESKTOP\Deskbreeze.lnk" "$INSTDIR\Deskbreeze.exe"
SectionEnd
