FROM node:lts-alpine
WORKDIR /app
COPY . .
EXPOSE 8001
EXPOSE 8002
RUN npm install
RUN apk add --no-cache ffmpeg
CMD ["npm", "run", "start"]