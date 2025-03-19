FROM python:3.11-alpine

WORKDIR /app

COPY /moodle/server/requirements.txt requirements.txt
RUN  pip install -r requirements.txt
COPY /moodle/server .

EXPOSE 5000

ENTRYPOINT [ "python3" ]
CMD [ "app.py" ]