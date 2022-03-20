# syntax=docker/dockerfile:1
FROM python:3.7-alpine

WORKDIR /

RUN apk add --no-cache gcc musl-dev linux-headers
ENV FLASK_APP=wrapper.py
ENV FLASK_RUN_HOST=0.0.0.0

COPY requirements.txt requirements.txt

RUN pip install -r requirements.txt

COPY . .
CMD ["python3", "old.py"]



# FROM python:3.7-alpine

# RUN mkdir -p /usr/src/app
# COPY ./ /usr/src/app

# WORKDIR /usr/src/app


# RUN pip install -r requirements.txt

# CMD ["python3", "old.py"]