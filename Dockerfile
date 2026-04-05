FROM gcc:latest

WORKDIR /app

# We install any necessary dependencies (if any)
RUN apt-get update && apt-get install -y build-essential

# We copied the code folders
COPY src/ ./src/
COPY include/ ./include/

# We compiled the server
RUN gcc src/*.c -I include -o server -lpthread

# We created a folder for the logs
RUN mkdir -p logs

# We are exposing port 8080
EXPOSE 8080

# Command to run the server
CMD ["./server", "8080", "logs/server.log"]