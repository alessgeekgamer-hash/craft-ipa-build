# Craft - Puerto iOS (IPA) - uso personal

Build automatico en la nube del puerto de **Craft** (clon open-source de
Minecraft, licencia MIT) para iPhone/iPad.

## Como obtener el IPA
1. Pestaña **Actions** → ultimo build (o botón **Run workflow**)
2. Al terminar (≈6 min): sección **Artifacts** → `Craft-Port-ipa`
   (es un .zip que contiene `Craft-Port.ipa`)
3. Firmarlo con **Sideloadly** (http://sideloadly.io/) usando tu Apple ID
   (certificado gratuito: dura 7 días, se renueva repitiendo el proceso)

## Que contiene el build
- Binario ARM64 (`craft.app/craft`) compilado con Xcode + SDL2 estatico
- Controles tactiles (joystick + camara + botones) identicos al APK
- Texturas y shaders incluidos en el bundle
- Icono de bloque de cesped y Launch Screen

## Notas
- El codigo del puerto esta en `craft/` (mismo codigo que Windows/Android)
- Orientacion: horizontal; funciona en iPhone y iPad
- Compilado en macOS con la toolchain ios-cmake + SDL2 2.30.11 oficial

## Aviso legal
Uso exclusivamente personal: no distribuir el IPA. "Minecraft" es marca
de Microsoft; este proyecto usa Craft (github.com/fogleman/Craft, MIT).
