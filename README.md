# neoclock_xp
Experimentos con NeoClock

Se modificó el ejemplo de NeoClock. Se agregó la función ESPTouch
para configurar la red WiFi y una interfaz web para configurar la alarma.  

## Cómo conectar al WiFi
1. Instala la aplicación ESPTouch o una parecida.  
2. Enciende el reloj presionando el botón para parar la alarma.
   Después de un segundo se puede soltar el botón.
   La palabra "UiFi" se debe mantener en la pantalla en color rojo.  
3. Escribir los datos de la red en la aplicación e iniciar el escaneo.  
4. Esperar a que la aplicación envie los datos al ESP32.  
5. La dirección ip del ESP32 aparecerá en la app.  

## Cómo configurar la alarma
1. Abrir un navegador web y escribir la dirección ip que apareció en la aplicación
   o escribe la dirección http://neoclock.local  
2. Se abrirá una página donde se pueden escribir estos datos:  
   - Escribir la hora en formato de 24 horas  
   - Seleccionar los días en los que debe sonar  
   - Indicar si la alarma está activada
   - Guardar la configuración de la alarma

