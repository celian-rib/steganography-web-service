# syntax=docker/dockerfile:1
FROM python:3.7-alpine
WORKDIR /artifact
ENV FLASK_APP=wrapper/wrapper.py
ENV FLASK_RUN_HOST=0.0.0.0
RUN apk add --no-cache gcc musl-dev linux-headers
COPY requirements.txt requirements.txt
RUN pip install -r requirements.txt
EXPOSE 9090
COPY . .
CMD ["python3", "wrapper/wrapper.py"]
