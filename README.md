# Proyecto_Sistemas_Operativos-Yuyai_Artificial

<h3>Descripción del Proyecto</h3>
<p align="justify">
Yuyay, una compañía de IA ecuatoriana, ha decidido ofrecer un servicio on-line de predicción de precios de vivienda para empresas inmobiliarias. Para realizar esto, Yuyay constantemente consigue datos de precios de casas de otros sitios webs, periódicos, y redes sociales que se compilan en un archivo de texto en formato csv. Un punto de datos en el archivo es el par (precio vivienda, numero de cuartos). El archivo es la entrada de un algoritmo de inteligencia artificial llamado Random Sample Consensus RANSAC, que en base a los mejores puntos de datos genera un modelo de predicción del valor de una casa dado un número de cuartos. 
</p>  
<p align="justify"> 
El algoritmo asume que el modelo de datos es una linea y=mx+b, y usa 2 puntos aleatorios para definir la linea. El modelo, para el m y b estimados, se evalúa midiendo las distancias de todos los puntos de datos a la linea y aquellos a una distancia menor que epsilon se consideran inliers y sino outliers. Un modelo es mejor que otro si su taza= inliers/outliers, es mayor. Este proceso se ejecuta N veces. 
</p>
<p align="justify">
Yuyay tiene una versión secuencial de RANSAC, pero quiere una versión paralela para reducir el tiempo de selección del modelo. Usted debe construir dicho sistema.
Considere que hay varias tareas: Cargar el archivo en memoria, seleccionar una muestra, estimar m y b, ejecutar el modelo, calcular la taza, comparar entre todas las tazas y seleccionar la mayor, y mostrar el avance de la ejecución del sistemas y el resultado final, entre otras. Asi que su diseño y posterior implementación debe tenerlas en cuenta. El objetivo es: para un N dado, reducir el tiempo de respuesta.
</p>

<h3>Compilar:</h3>
<p>gcc -o proyecto proyecto.c -lpthread -lm</p>

<h3>Ejecutar:</h3>
<p>./proyecto</p>
