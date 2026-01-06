# Genesis Engine SDK - Guía de Usuario

![Decoración](https://i.pinimg.com/originals/85/a0/74/85a074861f51fc3a560a6077e1162bbb.gif)

Este documento proporciona las instrucciones necesarias para instalar el motor Genesis y la referencia completa de la API de JavaScript para interactuar con el puente nativo (Native Bridge).

## 🚀 Instalación del Sistema

El proceso de instalación automatiza la configuración del entorno de desarrollo y las variables de sistema necesarias para compilar aplicaciones Genesis.

### Requisitos Técnicos
* **Sistema Operativo**: Windows 10 o 11.
* **Compilador**: Microsoft Visual C++ (cl.exe). Se requiere Visual Studio 2022 o Build Tools 2022.

### Proceso de Instalación
1.  **Preparación**: Asegúrate de que las carpetas `core` y `bin` se encuentren en el mismo directorio que el archivo `install.bat`.
2.  **Ejecución**: Ejecuta `install.bat` con privilegios de Administrador.
3.  **Acciones del Instalador**:
    * Crea el directorio raíz en `C:\Genesis`.
    * Copia los archivos del núcleo (`core`), binarios (`bin`) y el script de construcción (`builder.bat`).
    * Registra `C:\Genesis\bin` en la variable de entorno **PATH** del sistema.
4.  **Finalización**: Reinicia la terminal para aplicar los cambios del PATH.

---

## 🛠️ Referencia de la API de JavaScript (`window.genesis`)

La API se inyecta automáticamente antes de que se cargue cualquier página en el WebView2.

### 1. Módulo de Ventana (`genesis.window`)
Controla la interfaz física de la aplicación.

* **`close()`**: Cierra la ventana y termina el proceso.
    * *Uso*: `genesis.window.close();`
* **`minimize()` / `maximize()` / `restore()`**: Cambia el estado de visibilidad de la ventana.
* **`drag()`**: Permite arrastrar la ventana desde un elemento HTML.
    * *Uso*: `<div onmousedown="genesis.window.drag()">Mover</div>`
* **`center()`**: Centra la ventana en la pantalla.
* **`setTitle(texto)`**: Cambia el título de la ventana.
    * *Uso*: `genesis.window.setTitle("Mi App Pro");`
* **`setTransparent(bool)`**: Activa o desactiva la transparencia de la ventana.
* **`setOpacity(0-255)`**: Ajusta la opacidad total de la ventana.

### 2. Módulo de Sistema (`genesis.system`)
* **`isAdmin()`**: Verifica si la aplicación tiene permisos de administrador.
* **`getMemory()`**: Solicita información sobre el uso de memoria.

### 3. Sistema de Archivos (`genesis.fs`)
Gestiona archivos en el directorio AppData de la aplicación.

* **`list(path)`**: Lista archivos y carpetas (las carpetas incluyen `/`).
* **`write(path, data)`**: Escribe datos en un archivo.
* **`read(path)`**: Lee el contenido de un archivo.
* **`mkdir(path)`**: Crea un nuevo directorio.

### 4. Utilidades (`genesis.utils`)
* **`notify(título, mensaje)`**: Muestra un cuadro de mensaje nativo.
    * *Uso*: `genesis.utils.notify("Hola", "Mensaje desde JS");`
* **`openExternal(url)`**: Abre una URL en el navegador predeterminado.
* **`clipboardWrite(texto)`**: Copia texto al portapapeles.

### 5. Integración con Discord (`genesis.discord`)
* **`setActivity(config)`**: Configura el Rich Presence.
    * *Ejemplo*:
        ```javascript
        genesis.discord.setActivity({
            details: "Explorando el motor",
            state: "En el menú principal",
            timer: true
        });
        ```

### 6. Instalador Nativo (`genesis.installer`)
* **`run()`**: Inicia la instalación del SDK en `C:\Genesis` desde el proceso nativo.

![Decoración Final](https://i.pinimg.com/originals/f0/77/c5/f077c538658e2ac93f9a5f885ce1ddbb.gif)