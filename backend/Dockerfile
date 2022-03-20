FROM node
RUN apk add --no-cache gcc musl-dev linux-headers

RUN mkdir -p /usr/src/app
COPY ./ /usr/src/app

WORKDIR /usr/src/app

RUN npm install

CMD ["node", "index.js"]