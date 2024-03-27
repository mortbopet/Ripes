## Implementation of moodle with ripes interaction, configuration of connection to service from moodle side.
Configuring the External Tool it is done through received editing rights then add an external tool for subsequent configuration.
<p align="center">
	<img src="https://github.com/moevm/mse1h2024-ripes/blob/master/resources/images/externaltool.jpg" />
</p>
Add name, URL and select LTI version.
<p align="center">
    <img src="https://github.com/moevm/mse1h2024-ripes/blob/master/resources/images/nameurl.jpg" />
</p>
That’s it.
<p align="center">
    <img src="https://github.com/moevm/mse1h2024-ripes/blob/master/resources/images/reslti.jpg" />
</p>

## Итерация 2 
#### Решенные задачи:
 - Реализован сервер для обработки/отправки запросов на Flask
 - Настроен moodle для подключения ripes 
 - Создана инструкция для настройки подключения moodle к ripes
 - Создан UI для отображения условия заданий в ripes*  
 - Реализован выбор заданий в ripes**
 - Реализованна проверка заданий на синтаксические ошибки
   
#### Задачи в процессе решения:
 - Отправка response в moodle 
 - Обработка request от moodle
 - Первый прототит

*Пример отображения заданий
![example1](https://github.com/moevm/mse1h2024-ripes/blob/master/resources/images/example1.jpeg)

**Пример выбора и валидации задания
![example2](https://github.com/moevm/mse1h2024-ripes/blob/master/resources/images/example2.jpeg)
