FROM ripes-wasm AS wasm
FROM python:3.11-alpine

WORKDIR /app

COPY /moodle/server/requirements.txt requirements.txt
RUN  pip install -r requirements.txt
COPY /moodle/server .

COPY --from=wasm /opt/Ripes/build/Ripes.js ./static/ripes/Ripes.js
COPY --from=wasm /opt/Ripes/build/Ripes.worker.js ./static/ripes/Ripes.worker.js
COPY --from=wasm /opt/Ripes/build/qtloader.js ./static/ripes/qtloader.js
COPY --from=wasm /opt/Ripes/build/Ripes.wasm ./static/ripes/Ripes.wasm
COPY --from=wasm /opt/Ripes/build/Ripes.html ./static/ripes/Ripes.html

EXPOSE 5000

ENV FLASK_APP=app.py
ENTRYPOINT [ "flask" ]
CMD ["run", "--host=0.0.0.0"]