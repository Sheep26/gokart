FROM node:lts-alpine
WORKDIR /app
COPY . .
EXPOSE 8001
EXPOSE 8002
RUN npm install
CMD ["npm", "run", "start"]