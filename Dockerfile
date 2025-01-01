# Use a base image that includes Node.js
FROM node:22-slim

# Set the working directory inside the container
WORKDIR /app

# Copy package.json and package-lock.json (or yarn.lock)
COPY package*.json ./

# Install dependencies
RUN npm ci

# Install Python and set the PYTHON environment variable
RUN apt-get update && apt-get install -y python3 python3-dev
ENV PYTHON=/usr/bin/python3

# Copy the rest of the application code
COPY . .

# Build the native modules
RUN npm run build

# Expose the port your application listens on (if needed)
# EXPOSE 3000

# Start the application
CMD ["npm", "start"]
