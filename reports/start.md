# Инструкция по запуску

## Сборка

Для сборки созданы два docker файла:
- [Файл сборки Ripes](../docker/ripes.wasm.dockerfile)
- [Файл сборки сервера](../docker/server.dockerfile)

Сборка проводится из корневой директории

### Сборка Ripes
Сборка Ripes на Windows с помощью wsl не работает.

Для сборки Ripes в консоли введите:

```bash
docker build --rm --tag ripes-wasm:latest -f ./docker/ripes_new.dockerfile .
```

### Сборка сервера
По умолчанию для сборки сервера используется [образ Ripes загруженный на docker.hub](https://hub.docker.com/r/jqnfxa/ripes.wasm/tags)

Если необходимо использовать локально собранный Ripes, то замените в [файле сборки сервера](../docker/server.dockerfile) первую строчку с
```dockerfile
FROM jqnfxa/ripes.wasm:1.0.0 AS wasm
```
на
```dockerfile
FROM ripes-wasm AS wasm
```

Для сборки server в консоли введите:

```bash
docker build -t server -f ./docker/server.dockerfile .
```

# Запуск
Для запуска необходим файл окружения созданный в соответствии с [примером](../moodle/server/.env.example)

Для запуска в консоль введите:
```bash
# Заменить $PATH_TO_ENV_FILE на путь к файлу окружения
docker run -p 5000:5000 --env-file $PATH_TO_ENV_FILE server
```