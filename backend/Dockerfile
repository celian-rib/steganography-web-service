FROM node:17-alpine3.14

RUN apk update
RUN apk add --no-cache build-base gcc musl-dev linux-headers zlib-dev

RUN mkdir -p /usr/src/app

COPY ./ /usr/src/app

WORKDIR /usr/src/app/steganography
RUN make clean
RUN make

WORKDIR /usr/src/app

RUN npm ci
EXPOSE 3000

CMD ["node", "wrapper.js"]
