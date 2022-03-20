FROM node:17-alpine3.14
RUN apk add --no-cache gcc musl-dev linux-headers

RUN mkdir -p /usr/src/app
COPY ./ /usr/src/app

WORKDIR /usr/src/app

RUN npm ci
EXPOSE 3000

CMD ["node", "wrapper.js"]
